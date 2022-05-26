#include "app_main.h"

#include "app_camera.h"
#include "app_lcd.h"
#include "app_apriltag.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "taskmonitor.h"

static const char *TAG = "esp-apriltag";

QueueHandle_t xQueueAIFrame = NULL;
QueueHandle_t xQueueLCDFrame = NULL;

void app_main(void)
{
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    register_camera(PIXFORMAT_GRAYSCALE, FRAMESIZE_240X240, 2, xQueueAIFrame);
    register_apriltag_detection(xQueueAIFrame,xQueueLCDFrame, tag25h9);
    register_lcd(xQueueLCDFrame, NULL, true);

    // taskMonitorInit();
    // taskMonitorStart();

}
