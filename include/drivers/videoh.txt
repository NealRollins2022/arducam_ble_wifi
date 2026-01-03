/**
 * @file
 *
 * @brief Public APIs for Video.
 */

/*
 * Copyright (c) 2019 Linaro Limited.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_VIDEO_H_
#define ZEPHYR_INCLUDE_VIDEO_H_

/**
 * @brief Video Interface
 * @defgroup video_interface Video Interface
 * @since 2.1
 * @version 1.0.0
 * @ingroup io_interfaces
 * @{
 */

#include <zephyr/device.h>
#include <stddef.h>
#include <zephyr/kernel.h>

#include <zephyr/types.h>

#include <drivers/video-controls.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief video_frame_fragmented_status enum
 *
 * Indicates the receiving status of fragmented frames.
 */
enum video_frame_fragmented_status {
    VIDEO_BUF_FRAG,
    VIDEO_BUF_EOF,
};

/**
 * @struct video_format
 * @brief Video format structure
 *
 * Used to configure frame format.
 */
struct video_format {
    /** FourCC pixel format value (\ref video_pixel_formats) */
    uint32_t pixelformat;
    /** frame width in pixels. */
    uint32_t width;
    /** frame height in pixels. */
    uint32_t height;
    /**
     * @brief line stride.
     *
     * This is the number of bytes that needs to be added to the address in the
     * first pixel of a row in order to go to the address of the first pixel of
     * the next row (>=width).
     */
    uint32_t pitch;
};

/**
 * @struct video_format_cap
 * @brief Video format capability
 *
 * Used to describe a video endpoint format capability.
 */
struct video_format_cap {
    /** FourCC pixel format value (\ref video_pixel_formats). */
    uint32_t pixelformat;
    /** minimum supported frame width in pixels. */
    uint32_t width_min;
    /** maximum supported frame width in pixels. */
    uint32_t width_max;
    /** minimum supported frame height in pixels. */
    uint32_t height_min;
    /** maximum supported frame height in pixels. */
    uint32_t height_max;
    /** width step size in pixels. */
    uint16_t width_step;
    /** height step size in pixels. */
    uint16_t height_step;
};

/**
 * @struct video_caps
 * @brief Video format capabilities
 *
 * Used to describe video endpoint capabilities.
 */
struct video_caps {
    /** list of video format capabilities (zero terminated). */
    const struct video_format_cap *format_caps;
    /** minimal count of video buffers to enqueue before being able to start
     * the stream.
     */
    uint8_t min_vbuf_count;
};

/**
 * @struct video_buffer
 * @brief Video buffer structure
 *
 * Represent a video frame.
 */
struct video_buffer {
    /** pointer to driver specific data. */
    void *driver_data;
    /** pointer to the start of the buffer. */
    uint8_t *buffer;
    /** size of the buffer in bytes. */
    uint32_t size;
    /** number of bytes occupied by the valid data in the buffer. */
    uint32_t bytesused;
    /** time reference in milliseconds at which the last data byte was
     * actually received for input endpoints or to be consumed for output
     * endpoints.
     */
    uint32_t timestamp;
    /** frame length for fragmented frames. */
    uint32_t bytesframe;
    /** receiving status for fragmented frames. */
    uint32_t flags;
};

/**
 * @brief video_endpoint_id enum
 *
 * Identify the video device endpoint.
 */
enum video_endpoint_id {
    VIDEO_EP_NONE,
    VIDEO_EP_ANY,
    VIDEO_EP_IN,
    VIDEO_EP_OUT,
};

/**
 * @brief video_event enum
 *
 * Identify video event.
 */
enum video_signal_result {
    VIDEO_BUF_DONE,
    VIDEO_BUF_ABORTED,
    VIDEO_BUF_ERROR,
};

/**
 * @typedef video_api_set_format_t
 * @brief Set video format
 *
 * See video_set_format() for argument descriptions.
 */
typedef int (*video_api_set_format_t)(const struct device *dev,
                      enum video_endpoint_id ep,
                      struct video_format *fmt);

/**
 * @typedef video_api_get_format_t
 * @brief Get current video format
 *
 * See video_get_format() for argument descriptions.
 */
typedef int (*video_api_get_format_t)(const struct device *dev,
                      enum video_endpoint_id ep,
                      struct video_format *fmt);

/**
 * @typedef video_api_enqueue_t
 * @brief Enqueue a buffer in the driver’s incoming queue.
 *
 * See video_enqueue() for argument descriptions.
 */
typedef int (*video_api_enqueue_t)(const struct device *dev,
                   enum video_endpoint_id ep,
                   struct video_buffer *buf);

/**
 * @typedef video_api_dequeue_t
 * @brief Dequeue a buffer from the driver’s outgoing queue.
 *
 * See video_dequeue() for argument descriptions.
 */
typedef int (*video_api_dequeue_t)(const struct device *dev,
                   enum video_endpoint_id ep,
                   struct video_buffer **buf,
                   k_timeout_t timeout);

/**
 * @typedef video_api_flush_t
 * @brief Flush endpoint buffers, buffer are moved from incoming queue to
 *        outgoing queue.
 *
 * See video_flush() for argument descriptions.
 */
typedef int (*video_api_flush_t)(const struct device *dev,
                 enum video_endpoint_id ep,
                 bool cancel);

/**
 * @typedef video_api_stream_start_t
 * @brief Start the capture or output process.
 *
 * See video_stream_start() for argument descriptions.
 */
typedef int (*video_api_stream_start_t)(const struct device *dev);

/**
 * @typedef video_api_stream_stop_t
 * @brief Stop the capture or output process.
 *
 * See video_stream_stop() for argument descriptions.
 */
typedef int (*video_api_stream_stop_t)(const struct device *dev);

/**
 * @typedef video_api_set_ctrl_t
 * @brief Set a video control value.
 *
 * See video_set_ctrl() for argument descriptions.
 */
typedef int (*video_api_set_ctrl_t)(const struct device *dev,
                    unsigned int cid,
                    void *value);

/**
 * @typedef video_api_get_ctrl_t
 * @brief Get a video control value.
 *
 * See video_get_ctrl() for argument descriptions.
 */
typedef int (*video_api_get_ctrl_t)(const struct device *dev,
                    unsigned int cid,
                    void *value);

/**
 * @typedef video_api_get_caps_t
 * @brief Get capabilities of a video endpoint.
 *
 * See video_get_caps() for argument descriptions.
 */
typedef int (*video_api_get_caps_t)(const struct device *dev,
                    enum video_endpoint_id ep,
                    struct video_caps *caps);

/**
 * @typedef video_api_set_signal_t
 * @brief Register/Unregister poll signal for buffer events.
 *
 * See video_set_signal() for argument descriptions.
 */
typedef int (*video_api_set_signal_t)(const struct device *dev,
                      enum video_endpoint_id ep,
                      struct k_poll_signal *signal);

__subsystem struct video_driver_api {
    /* mandatory callbacks */
    video_api_set_format_t set_format;
    video_api_get_format_t get_format;
    video_api_stream_start_t stream_start;
    video_api_stream_stop_t stream_stop;
    video_api_get_caps_t get_caps;
    /* optional callbacks */
    video_api_enqueue_t enqueue;
    video_api_dequeue_t dequeue;
    video_api_flush_t flush;
    video_api_set_ctrl_t set_ctrl;
    video_api_set_ctrl_t get_ctrl;
    video_api_set_signal_t set_signal;
};

/* ... the rest of the file (functions like video_set_format, etc.) remains the same as upstream ... */

#ifdef __cplusplus
}
#endif

/**
 * @}
 */


#endif /* ZEPHYR_INCLUDE_VIDEO_H_ */
