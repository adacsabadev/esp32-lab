#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "esp_log.h"

#define LED_GPIO 48
#define RMT_RES_HZ 10000000 /* 10 MHz -> 0.1 us per tick */

static const char *TAG = "blinky";
static rmt_channel_handle_t chan;
static rmt_encoder_handle_t enc;

/* WS2812 bit timings in 0.1 us ticks: 0 = 0.3 high / 0.9 low, 1 = 0.9 high / 0.3 low */
static void led_init(void)
{
    rmt_tx_channel_config_t ch_cfg = {
        .gpio_num = LED_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RES_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&ch_cfg, &chan));

    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = {.level0 = 1, .duration0 = 3, .level1 = 0, .duration1 = 9},
        .bit1 = {.level0 = 1, .duration0 = 9, .level1 = 0, .duration1 = 3},
        .flags.msb_first = 1,
    };
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&enc_cfg, &enc));
    ESP_ERROR_CHECK(rmt_enable(chan));
}

static void led_set(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t grb[3] = {g, r, b};
    rmt_transmit_config_t tx_cfg = {.loop_count = 0};
    ESP_ERROR_CHECK(rmt_transmit(chan, enc, grb, sizeof(grb), &tx_cfg));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(chan, -1));
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP-IDF RGB blinky starting; LED GPIO=%d", LED_GPIO);
    led_init();

    while (1) {
        ESP_LOGI(TAG, "green");
        led_set(0, 2, 0);
        vTaskDelay(pdMS_TO_TICKS(700));
        led_set(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));

        ESP_LOGI(TAG, "red");
        led_set(2, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(700));
        led_set(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
