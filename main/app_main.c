#include "app_main.h"

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "screen_driver.h"
static const char *TAG = "esp-apriltag";

scr_driver_t driver;

static camera_config_t camera_config = {
    .pin_pwdn = CAMERA_PIN_PWDN,
    .pin_reset = CAMERA_PIN_RESET,
    .pin_xclk = CAMERA_PIN_XCLK,
    .pin_sscb_sda = CAMERA_PIN_SIOD,
    .pin_sscb_scl = CAMERA_PIN_SIOC,

    .pin_d7 = CAMERA_PIN_D7,
    .pin_d6 = CAMERA_PIN_D6,
    .pin_d5 = CAMERA_PIN_D5,
    .pin_d4 = CAMERA_PIN_D4,
    .pin_d3 = CAMERA_PIN_D3,
    .pin_d2 = CAMERA_PIN_D2,
    .pin_d1 = CAMERA_PIN_D1,
    .pin_d0 = CAMERA_PIN_D0,
    .pin_vsync = CAMERA_PIN_VSYNC,
    .pin_href = CAMERA_PIN_HREF,
    .pin_pclk = CAMERA_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = XCLK_FREQ_HZ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_240X240,     // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, // 0-63 lower number means higher quality
    .fb_count = 1,      // if more than one, i2s runs in continuous mode. Use only with JPEG
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera()
{
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

static esp_err_t init_lcd()
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
        .clk_freq = 40 * 1000000,
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
        .rotate = SCR_DIR_LRBT,//SCR_DIR_RLBT,//SCR_DIR_RLTB,//SCR_DIR_TBRL,
    };

    if (ESP_OK != driver.init(&lcd_cfg))
    {
        ESP_LOGE(TAG, "screen initialize failed");
        return ESP_FAIL;
    }

    scr_info_t lcd_info;
    driver.get_info(&lcd_info);
    ESP_LOGI(TAG, "Screen name:%s | width:%d | height:%d", lcd_info.name, lcd_info.width, lcd_info.height);
    return ESP_OK;
}

void app_main(void)
{
    if (ESP_OK != init_camera() && ESP_OK != init_lcd())
    {
        return;
    }

    if ( ESP_OK != init_lcd())
    {
        return;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // use pic->buf to access the image
        ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        driver.draw_bitmap(0, 0, pic->width, pic->height, (uint16_t *)pic->buf);
        esp_camera_fb_return(pic);
    }
}
