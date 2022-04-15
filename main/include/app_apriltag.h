#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "apriltag.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>

#include "tag36h11.h"
#include "tag16h5.h"
#include "tag25h9.h"

void register_apriltag_detection(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const tagformat_t tag);

#ifdef __cplusplus
}
#endif
