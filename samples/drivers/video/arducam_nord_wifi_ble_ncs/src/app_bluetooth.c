#include "app_bluetooth.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>
#include <bluetooth/services/lbs.h>
#include "its.h"

LOG_MODULE_REGISTER(app_bluetooth, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/* Connection parameters */
static const struct bt_le_conn_param *conn_param = BT_LE_CONN_PARAM(9, 9, 0, 400);
static struct bt_conn *current_conn;

/* Application callbacks */
static app_bt_connected_cb app_callback_connected;
static app_bt_ready_cb app_callback_ready;
static app_bt_disconnected_cb app_callback_disconnected;
static app_bt_take_picture_cb app_callback_take_picture;
static app_bt_enable_stream_cb app_callback_enable_stream;
static app_bt_change_resolution_cb app_callback_change_resolution;

/* ITS BLE params */
static struct its_ble_params_info_t ble_params_info = {
    .con_interval = 0, .mtu = 23, .tx_phy = 1, .rx_phy = 1
};
static int le_tx_data_length = 20;

/* Semaphore to wait for bt_enable to complete over IPC */
static K_SEM_DEFINE(bt_init_ok, 0, 1);

/* Atomic flags for camera and BT readiness - prevents race where BT advertising
   triggers work items that touch uninitialized camera subsystem */
static atomic_t bt_ready_flag = 0;
static atomic_t cam_ready_flag = 0;
static atomic_t adv_started_flag = 0;

/* Internal commands for BLE/ITS handling */
enum app_bt_internal_commands {
    APP_BT_INT_ITS_RX_EVT,
    APP_BT_INT_SCHEDULE_CONNECTED_CB,
    APP_BT_INT_SCHEDULE_READY_CB,
    APP_BT_INT_SCHEDULE_DISCONNECTED_CB,
    APP_BT_INT_SCHEDULE_BLE_PARAMS_INFO_UPDATE
};

static struct its_rx_cb_evt_t internal_command_evt;
struct app_bt_command {
    enum app_bt_internal_commands command;
    struct its_rx_cb_evt_t its_rx_event;
};
K_MSGQ_DEFINE(msgq_its_rx_commands, sizeof(struct app_bt_command), 8, 4);

/* Advertising data */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_ITS_VAL),
};

/* Forward declarations */
static void advertising_start(void);
static struct k_work adv_work;

/* Internal command scheduling */
static int schedule_internal_command(enum app_bt_internal_commands command)
{
    internal_command_evt.command = command;
    if (k_msgq_put(&msgq_its_rx_commands, &internal_command_evt, K_NO_WAIT) != 0) {
        LOG_ERR("RX cmd message queue full!");
        return -ENOMEM;
    }
    return 0;
}

void schedule_ble_params_info_update(void)
{
    schedule_internal_command(APP_BT_INT_SCHEDULE_BLE_PARAMS_INFO_UPDATE);
}

/* Forward declaration of gate function */
static void try_start_adv(void);

/* bt_enable async callback - signals semaphore when net core IPC is ready */
static void bt_ready(int err)
{
    if (err) {
        LOG_WRN("bt_ready err %d - ignoring 0x0c33 known nCS3 issue", err);
    }
    
    atomic_set(&bt_ready_flag, 1);
    LOG_INF("BT stack ready flag set");
    
    k_sem_give(&bt_init_ok);
    
    /* Check if camera is already ready so we can start advertising */
    try_start_adv();
}

/* GATT callbacks */
void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
    LOG_INF("MTU Updated: tx %d, rx %d", tx, rx);
}

static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err,
                             struct bt_gatt_exchange_params *params)
{
    if (err) {
        LOG_WRN("MTU exchange failed (err %d)", err);
    } else {
        LOG_INF("MTU exchange done, MTU: %d", bt_gatt_get_mtu(conn));
    }
}

static struct bt_gatt_cb gatt_cb = {
    .att_mtu_updated = att_mtu_updated,
};

/* Connection callbacks */
static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Connected %s", addr);

    current_conn = bt_conn_ref(conn);

    static struct bt_gatt_exchange_params exchange_params;
    exchange_params.func = mtu_exchange_cb;

    err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (err) {
        LOG_WRN("MTU exchange request failed (err %d)", err);
    }

    err = bt_conn_le_param_update(conn, conn_param);
    if (err) {
        LOG_ERR("Cannot update connection parameter (err: %d)", err);
    }

    schedule_internal_command(APP_BT_INT_SCHEDULE_CONNECTED_CB);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Disconnected: %s (reason %u)", addr, reason);

    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }

    schedule_internal_command(APP_BT_INT_SCHEDULE_DISCONNECTED_CB);
}

static void connection_param_update(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    LOG_INF("Con params updated. Interval %d ms, latency %i, timeout %i ms",
            (int)((float)interval * 1.25f), latency, timeout * 10);
    ble_params_info.con_interval = interval;
    schedule_ble_params_info_update();
}

static void le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    LOG_INF("LE PHY updated: TX PHY %i, RX PHY %i", param->tx_phy, param->rx_phy);
    ble_params_info.tx_phy = param->tx_phy;
    ble_params_info.rx_phy = param->rx_phy;
    schedule_ble_params_info_update();
}

/* Advertising logic */
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    (BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY),
    BT_GAP_ADV_FAST_INT_MIN_1,
    BT_GAP_ADV_FAST_INT_MAX_1,
    NULL);

/* Gate function: only start advertising when BOTH BT and camera are ready */
static void try_start_adv(void)
{
    if (!atomic_get(&bt_ready_flag)) {
        LOG_DBG("try_start_adv: BT not ready yet (CAM=%ld)", atomic_get(&cam_ready_flag));
        return;
    }

    if (!atomic_get(&cam_ready_flag)) {
        LOG_DBG("try_start_adv: Camera not ready yet (BT=%ld)", atomic_get(&bt_ready_flag));
        return;
    }

    /* One-shot guard: only call bt_le_adv_start once */
    if (!atomic_cas(&adv_started_flag, 0, 1)) {
        LOG_DBG("Advertising already started");
        return;
    }

    printk("try_start_adv: Both ready! BT=%ld CAM=%ld - Starting advertising\n",
           atomic_get(&bt_ready_flag), atomic_get(&cam_ready_flag));

    int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        /* Reset flag so we can retry */
        atomic_set(&adv_started_flag, 0);
        return;
    }
    LOG_INF("Advertising successfully started");
}

static void adv_work_handler(struct k_work *work)
{
    LOG_DBG("adv_work_handler: Checking if both subsystems ready");
    try_start_adv();
}

static void advertising_start(void)
{
    k_work_submit(&adv_work);
}

static void recycled_cb(void)
{
    LOG_INF("Previous connection object recycled, restarting advertising");
    if (current_conn == NULL) {
        advertising_start();
    }
}

/* Single combined connection callbacks - two BT_CONN_CB_DEFINE blocks cause null ptr crash in nCS 3.x */
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .le_param_updated = connection_param_update,
    .le_phy_updated = le_phy_updated,
    .recycled = recycled_cb,
};

/* ITS callbacks */
void its_ready_callback(void)
{
    struct app_bt_command bt_cmd = {.command = APP_BT_INT_SCHEDULE_READY_CB};
    if (k_msgq_put(&msgq_its_rx_commands, &bt_cmd, K_NO_WAIT) != 0) {
        LOG_ERR("APP BT RX CMD queue full");
    }
}

void its_rx_callback(struct its_rx_cb_evt_t *evt)
{
    struct app_bt_command bt_cmd = {.command = APP_BT_INT_ITS_RX_EVT};
    bt_cmd.its_rx_event = *evt;
    if (k_msgq_put(&msgq_its_rx_commands, &bt_cmd, K_NO_WAIT) != 0) {
        LOG_ERR("APP BT RX CMD queue full");
    }
}

static struct bt_its_cb its_cb = {
    .ready_cb = its_ready_callback,
    .rx_cb = its_rx_callback,
};

/* BT message handling thread */
static void app_bt_thread_func(void)
{
    int err;
    struct app_bt_command app_cmd;

    while (1) {
        k_msgq_get(&msgq_its_rx_commands, &app_cmd, K_FOREVER);

        if (app_cmd.command == APP_BT_INT_ITS_RX_EVT) {
            switch (app_cmd.its_rx_event.command) {
                case ITS_RX_CMD_SINGLE_CAPTURE:
                    if (app_callback_take_picture) app_callback_take_picture();
                    break;
                case ITS_RX_CMD_START_STREAM:
                    if (app_callback_enable_stream) app_callback_enable_stream(true);
                    break;
                case ITS_RX_CMD_STOP_STREAM:
                    if (app_callback_enable_stream) app_callback_enable_stream(false);
                    break;
                case ITS_RX_CMD_CHANGE_RESOLUTION:
                    if (app_callback_change_resolution)
                        app_callback_change_resolution(app_cmd.its_rx_event.data[0]);
                    break;
                case ITS_RX_CMD_CHANGE_PHY:
                    if (current_conn) {
                        err = bt_conn_le_phy_update(current_conn,
                            (app_cmd.its_rx_event.data[0] == 1) ? BT_CONN_LE_PHY_PARAM_2M : BT_CONN_LE_PHY_PARAM_1M);
                        if (err) LOG_ERR("PHY update failed: %d", err);
                    }
                    break;
                default:
                    LOG_ERR("Invalid ITS RX command");
                    break;
            }
        } else {
            switch (app_cmd.command) {
                case APP_BT_INT_SCHEDULE_CONNECTED_CB:
                    if (app_callback_connected) app_callback_connected();
                    break;
                case APP_BT_INT_SCHEDULE_READY_CB:
                    if (app_callback_ready) app_callback_ready();
                    break;
                case APP_BT_INT_SCHEDULE_DISCONNECTED_CB:
                    if (app_callback_disconnected) app_callback_disconnected();
                    break;
                case APP_BT_INT_SCHEDULE_BLE_PARAMS_INFO_UPDATE:
                    err = bt_its_send_ble_params_info(&ble_params_info);
                    if (err) LOG_ERR("Error sending BLE params");
                    break;
                default:
                    break;
            }
        }
    }
}

/* Public API: Call this from main/camera init when camera is fully ready */
void app_bt_mark_camera_ready(void)
{
    atomic_set(&cam_ready_flag, 1);
    LOG_INF("Camera ready flag set");
    printk("app_bt_mark_camera_ready: CAM=1, BT=%ld\n", atomic_get(&bt_ready_flag));
    
    /* Check if BT is already ready so we can start advertising */
    try_start_adv();
}

int app_bt_init(const struct app_bt_cb *callbacks)
{
    if (callbacks) {
        app_callback_connected = callbacks->connected;
        app_callback_ready = callbacks->ready;
        app_callback_disconnected = callbacks->disconnected;
        app_callback_take_picture = callbacks->take_picture;
        app_callback_enable_stream = callbacks->enable_stream;
        app_callback_change_resolution = callbacks->change_resolution;
    }

    /* On nRF5340, bt_enable is async over IPC - must use callback and wait */
    int err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("bt_enable failed (err %d)", err);
        return err;
    }

    /* Wait for net core controller to be ready via IPC */
    k_sem_take(&bt_init_ok, K_FOREVER);


LOG_INF("Bluetooth initialized (continuing past known 0x0c33 warning)");

    bt_gatt_cb_register(&gatt_cb);

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    err = bt_its_init(&its_cb);
    if (err) {
        LOG_ERR("Failed to initialize Image Transfer Service (err: %d)", err);
        return err;
    }

    k_work_init(&adv_work, adv_work_handler);
    advertising_start();

    return 0;
}

int app_bt_send_picture_header(uint32_t pic_size)
{
    struct its_img_info_t img_info = {.file_size_bytes = pic_size};
    return bt_its_send_img_info(&img_info);
}

int app_bt_send_picture_data(uint8_t *buf, uint16_t len)
{
    return bt_its_send_img_data(current_conn, buf, len, le_tx_data_length);
}

int app_bt_send_client_status(uint8_t cam_model, uint8_t resolution)
{
    struct its_client_status_t client_status = {
        .camera_type = cam_model,
        .selected_resolution_index = resolution
    };
    return bt_its_send_client_status(&client_status);
}

K_THREAD_DEFINE(app_bt_thread, 2048, app_bt_thread_func, NULL, NULL, NULL,
                K_PRIO_PREEMPT(K_LOWEST_APPLICATION_THREAD_PRIO), 0, 0);
