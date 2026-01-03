#ifndef ZEPHYR_DRIVERS_VIDEO_ARDUCAM_NORD_H_
#define ZEPHYR_DRIVERS_VIDEO_ARDUCAM_NORD_H_

#include <zephyr/device.h>
#include <drivers/video.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <stdint.h>

/* Camera ID */
enum {
    ARDUCAM_SENSOR_5MP_1 = 0x01,
    ARDUCAM_SENSOR_3MP_1 = 0x02,
    ARDUCAM_SENSOR_5MP_2 = 0x03,
    ARDUCAM_SENSOR_3MP_2 = 0x04,
};

/* Pixel formats */
#define MEGA_PIXELFORMAT_JPG    0x00
#define MEGA_PIXELFORMAT_RGB565 0x01
#define MEGA_PIXELFORMAT_YUV    0x02

/* Video controls */
enum MEGA_BRIGHTNESS_LEVEL { MEGA_BRIGHTNESS_MIN, MEGA_BRIGHTNESS_MED, MEGA_BRIGHTNESS_MAX };
enum MEGA_CONTRAST_LEVEL { MEGA_CONTRAST_MIN, MEGA_CONTRAST_MED, MEGA_CONTRAST_MAX };
enum MEGA_SATURATION_LEVEL { MEGA_SATURATION_MIN, MEGA_SATURATION_MED, MEGA_SATURATION_MAX };
enum MEGA_EV_LEVEL { MEGA_EV_MIN, MEGA_EV_MED, MEGA_EV_MAX };
enum MEGA_SHARPNESS_LEVEL { MEGA_SHARPNESS_MIN, MEGA_SHARPNESS_MED, MEGA_SHARPNESS_MAX };
enum MEGA_COLOR_FX { MEGA_COLOR_FX_NONE, MEGA_COLOR_FX_SEPIA, MEGA_COLOR_FX_NEGATIVE };
enum MEGA_IMAGE_QUALITY { MEGA_IMAGE_QUALITY_LOW, MEGA_IMAGE_QUALITY_MED, MEGA_IMAGE_QUALITY_HIGH };
enum MEGA_WHITE_BALANCE { MEGA_WHITE_BAL_AUTO, MEGA_WHITE_BAL_INCANDESCENT, MEGA_WHITE_BAL_FLUORESCENT };

/* Supported resolutions */
enum MEGA_RESOLUTION {
    MEGA_RESOLUTION_96X96,
    MEGA_RESOLUTION_128X128,
    MEGA_RESOLUTION_QVGA,
    MEGA_RESOLUTION_320X320,
    MEGA_RESOLUTION_VGA,
    MEGA_RESOLUTION_HD,
    MEGA_RESOLUTION_UXGA,
    MEGA_RESOLUTION_FHD,
    MEGA_RESOLUTION_WQXGA2,
    MEGA_RESOLUTION_QXGA,
    MEGA_RESOLUTION_NONE
};

struct mega_sdk_data {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t version;
};

struct arducam_mega_info {
    uint32_t support_resolution;
    uint32_t support_special_effects;
    uint32_t exposure_value_max;
    uint32_t exposure_value_min;
    uint32_t gain_value_max;
    uint32_t gain_value_min;
    uint8_t enable_focus;
    uint8_t enable_sharpness;
    uint8_t device_address;
    uint8_t camera_id;
};

struct arducam_mega_config {
    struct spi_dt_spec bus;
};

struct arducam_mega_data {
    const struct device *dev;
    struct video_format fmt;
    struct k_fifo fifo_in;
    struct k_fifo fifo_out;
    struct k_work buf_work;
    struct k_timer stream_schedule_timer;
    struct k_poll_signal *signal;
    struct arducam_mega_info *info;
    struct mega_sdk_data ver;
    uint8_t fifo_first_read;
    uint32_t fifo_length;
    uint8_t stream_on;
};

/* Public APIs */
int arducam_mega_get_info(const struct device *dev, struct arducam_mega_info *info);

#endif /* ZEPHYR_DRIVERS_VIDEO_ARDUCAM_NORD_H_ */
