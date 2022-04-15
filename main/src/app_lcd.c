#include "app_lcd.h"

// #define GRAYTORGB16(t) ((t >> 3)|((t & ~3) << 3)|((t & ~7) << 8))

static const char *TAG = "app_lcd";

static scr_driver_t driver;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;

static bool gReturnFB = true;

// +-------+--------------------+----------+
// |       |       RGB565       |  RGB888  |
// +=======+====================+==========+
// |  Red  | 0b0000000011111000 | 0x0000FF |
// +-------+--------------------+----------+
// | Green | 0b1110000000000111 | 0x00FF00 |
// +-------+--------------------+----------+
// |  Blue | 0b0001111100000000 | 0xFF0000 |
// +-------+--------------------+----------+

static uint16_t gray_to_565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint16_t)((r & 0xF8) | ((b & 0xF8) << 5) | ((g & 0x1c) << 11) | ((g & 0xE0) >> 5)));
}

static void show_gray(uint8_t *greybuffer, uint16_t *rgbbuffer, size_t length)
{
    uint8_t r, g, b;
    uint16_t w = 240;
    uint16_t h = 240;
    if (length < w)
    {
        return;
    }
    r = 0x00;
    for (size_t y = 0; y < h; y++)
    {
        uint16_t color = 0;
        for (size_t x = 0; x < w; x++)
        {
            color = gray_to_565(*(greybuffer), *(greybuffer), *(greybuffer));
            greybuffer++;
            rgbbuffer[x] = color;
        }
        driver.draw_bitmap(0, y, w, 1, rgbbuffer);
    }
}

static void task_process_handler(void *arg)
{
    uint16_t *rgbframe = (uint16_t *)heap_caps_calloc(1, 2 * 240, MALLOC_CAP_DEFAULT);
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
        {
            show_gray(frame->buf, rgbframe, 1 * 240 * 240);
            // driver.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)rgbframe);

            if (xQueueFrameO)
            {
                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
            }
            else
            {
                free(frame);
                free(rgbframe);
            }
        }
    }
}

esp_err_t register_lcd(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb)
{
    spi_config_t bus_conf = {
        .miso_io_num = (gpio_num_t)BOARD_LCD_MISO,
        .mosi_io_num = (gpio_num_t)BOARD_LCD_MOSI,
        .sclk_io_num = (gpio_num_t)BOARD_LCD_SCK,
        .max_transfer_sz = 2 * 240 * 240 + 10,
    };
    spi_bus_handle_t spi_bus = spi_bus_create(SPI2_HOST, &bus_conf);

    scr_interface_spi_config_t spi_lcd_cfg = {
        .spi_bus = spi_bus,
        .pin_num_cs = BOARD_LCD_CS,
        .pin_num_dc = BOARD_LCD_DC,
        .clk_freq = 80 * 1000000,
        .swap_data = 0,
    };

    scr_interface_driver_t *iface_drv;
    scr_interface_create(SCREEN_IFACE_SPI, &spi_lcd_cfg, &iface_drv);
    if (ESP_OK != scr_find_driver(SCREEN_CONTROLLER_ST7789, &driver))
    {
        ESP_LOGE(TAG, "screen find failed");
        return ESP_FAIL;
    }

    scr_controller_config_t lcd_cfg = {
        .interface_drv = iface_drv,
        .pin_num_rst = BOARD_LCD_RST,
        .pin_num_bckl = BOARD_LCD_BL,
        .rst_active_level = 0,
        .bckl_active_level = 0,
        .width = 240,
        .height = 240,
        .offset_hor = 0,
        .offset_ver = 0,
        .rotate = 0, // SCR_DIR_LRBT,//SCR_DIR_RLBT,//SCR_DIR_RLTB,//SCR_DIR_TBRL,
    };

    if (ESP_OK != driver.init(&lcd_cfg))
    {
        ESP_LOGE(TAG, "screen initialize failed");
        return ESP_FAIL;
    }

    scr_info_t lcd_info;
    driver.get_info(&lcd_info);
    ESP_LOGI(TAG, "Screen name:%s | width:%d | height:%d", lcd_info.name, lcd_info.width, lcd_info.height);

    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    gReturnFB = return_fb;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 4, NULL, 0);

    return ESP_OK;
}