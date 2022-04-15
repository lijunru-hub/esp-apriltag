#include "app_apriltag.h"


static const char *TAG = "app_apriltag";
static tagformat_t tag_format;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;

obj_info_t tag_info={.obj_number=0};

apriltag_detector_t *td=NULL;
apriltag_family_t *tf=NULL;

void Find_AprilTag_old(uint8_t *addr)
{
    list_t out;
    imlib_find_apriltags(&out, addr,tag_format);

    tag_info.obj_number = 0;
    for (uint8_t i = 0; i < out.no&& i < 10; i++)
    {
        tag_info.obj[i].x1 = out.obj[i].x1;
        tag_info.obj[i].y1 = out.obj[i].y1;
        tag_info.obj[i].x2 = out.obj[i].x2;
        tag_info.obj[i].y2 = out.obj[i].y2;
        tag_info.obj[i].class_id = out.obj[i].payload_len;
        tag_info.obj[i].rotation = (out.obj[i].rotation * 180) / M_PI;
        tag_info.obj_number++;
    }
    ESP_LOGI(TAG,"apriltag:%d",tag_info.obj_number);
}

static void ApriTag_init(tagformat_t families) {
    if (families==tag16h5) {
        tf = tag16h5_create();
    }
    else if (families==tag25h9) {
        tf = tag25h9_create();
    }
    else {
        tf = tag36h11_create();
    }
    td = apriltag_detector_create();
    apriltag_detector_add_family(td, tf);
    td->quad_sigma = 0.0;
    td->quad_decimate = 1.0;
    td->refine_edges = 0;
    td->decode_sharpening = 0;
    td->nthreads = 1;
    td->debug = 0;
}

static void ApriTag_deinit(uint8_t *addr,tagformat_t families) {
    apriltag_detector_destroy(td);
    tag25h9_destroy(tf);
}

void Find_AprilTag(camera_fb_t *fb)
{
    image_u8_t im = {
        .width = fb->width,
        .height = fb->height,
        .stride = fb->width,
        .buf = fb->buf
    };
        
    zarray_t *detections = apriltag_detector_detect(td, &im);

    for (int i = 0; i < zarray_size(detections); i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);
        printf("%d, ",det->id);
    }
    printf("\n");
    apriltag_detections_destroy(detections);
    // double t =  timeprofile_total_utime(td->tp) / 1.0E3;
    // printf("%12.3f \n", t);
}

void task_process_handler(void *arg)
{
    camera_fb_t *frame = NULL;
    char display_string[30]="apriltag mode:yes para:yes";
    ApriTag_init(tag_format);
    while (1)
    {
        if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {
            //driver.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)frame->buf);
            Find_AprilTag(frame);
            //apriltag_show((uint32_t *)display_image.addr);
            if (xQueueFrameO) {
                //esp_camera_fb_return(frame);
                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            }
            else {
                free(frame);
            }
        }
    }
}

void register_apriltag_detection(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const tagformat_t tag)
{
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    tag_format = tag;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 4, NULL, 0);
    //uxTaskGetStackHighWaterMark(TAG);
}