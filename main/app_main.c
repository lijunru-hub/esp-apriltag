#include "app_main.h"

#include "app_camera.h"
#include "app_lcd.h"
#include "app_apriltag.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

static const char *TAG = "esp-apriltag";

scr_driver_t driver;

QueueHandle_t xQueueAIFrame = NULL;
QueueHandle_t xQueueLCDFrame = NULL;

// void vTaskGetRunTimeStats( char *pcWriteBuffer )
//  {
//     TaskStatus_t *pxTaskStatusArray;
//     volatile UBaseType_t uxArraySize, x;
//     uint32_t ulTotalRunTime, ulStatsAsPercentage;

//      // Make sure the write buffer does not contain a string.
//     *pcWriteBuffer = 0x00;

//      // Take a snapshot of the number of tasks in case it changes while this
//      // function is executing.
//      uxArraySize = uxTaskGetNumberOfTasks();

//      // Allocate a TaskStatus_t structure for each task.  An array could be
//      // allocated statically at compile time.
//      pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

//      if( pxTaskStatusArray != NULL )
//      {
//          // Generate raw status information about each task.
//          uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

//          // For percentage calculations.
//          ulTotalRunTime /= 100UL;

//          // Avoid divide by zero errors.
//          if( ulTotalRunTime > 0 )
//          {
//              // For each populated position in the pxTaskStatusArray array,
//              // format the raw data as human readable ASCII data
//              for( x = 0; x < uxArraySize; x++ )
//              {
//                  // What percentage of the total run time has the task used?
//                  // This will always be rounded down to the nearest integer.
//                  // ulTotalRunTimeDiv100 has already been divided by 100.
//                  ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime;

//                  if( ulStatsAsPercentage > 0UL )
//                  {
//                      sprintf( pcWriteBuffer, "%s\t\t%d\t\t%d%%\r\n", pxTaskStatusArray[ x ].pcTaskName, pxTaskStatusArray[ x ].ulRunTimeCounter, ulStatsAsPercentage );
//                  }
//                  else
//                  {
//                      // If the percentage is zero here then the task has
//                      // consumed less than 1% of the total run time.
//                      sprintf( pcWriteBuffer, "%s\t\t%d\t\t<1%%\r\n", pxTaskStatusArray[ x ].pcTaskName, pxTaskStatusArray[ x ].ulRunTimeCounter );
//                  }

//                  pcWriteBuffer += strlen( ( char * ) pcWriteBuffer );
//              }
//          }

//          // The array is no longer needed, free the memory it consumes.
//          vPortFree( pxTaskStatusArray );
//      }
//  }

// char a[100]={0};

void app_main(void)
{
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    
    register_camera(PIXFORMAT_GRAYSCALE, FRAMESIZE_240X240, 2, xQueueAIFrame);
    register_apriltag_detection(xQueueAIFrame,xQueueLCDFrame, tag16h5);
    register_lcd(xQueueLCDFrame, NULL, true);

    // vTaskDelay(200 / portTICK_PERIOD_MS);

    // vTaskGetRunTimeStats(a);
    // ESP_LOGI(TAG,"%s",a);
}
