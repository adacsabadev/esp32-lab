#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "soc/soc.h"
#include "esp_cpu.h"
#include "esp_rom_sys.h"

/* WS2812 on GPIO48 driven by hand: GPIO matrix + W1TS/W1TC registers + cycle-counter timing. */
#define LED_PIN 48
#define LED_BIT (1u << (LED_PIN - 32)) /* GPIO32..48 live in the *1 registers */
#define CPU_MHZ 240

static portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;

static void led_pin_init(void)
{
    PIN_FUNC_SELECT(IO_MUX_GPIO48_REG, PIN_FUNC_GPIO);          /* IO_MUX: route pad to the GPIO matrix */
    GPIO.func_out_sel_cfg[LED_PIN].func_sel = SIG_GPIO_OUT_IDX; /* output = plain GPIO_OUT register */
    GPIO.func_out_sel_cfg[LED_PIN].oen_sel = 0;                 /* output enable from the enable register */
    GPIO.enable1_w1ts.val = LED_BIT;                            /* enable output driver */
    GPIO.out1_w1tc.val = LED_BIT;                               /* idle low */
}

static inline __attribute__((always_inline)) void wait_until(uint32_t t0, uint32_t cycles)
{
    while ((esp_cpu_get_cycle_count() - t0) < cycles) { }
}

static void IRAM_ATTR led_send(uint8_t r, uint8_t g, uint8_t b)
{
    const uint32_t t0h = CPU_MHZ * 400 / 1000, t1h = CPU_MHZ * 800 / 1000, per = CPU_MHZ * 1250 / 1000;
    uint32_t word = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

    portENTER_CRITICAL(&lock);
    for (int i = 23; i >= 0; i--) {
        uint32_t t = esp_cpu_get_cycle_count();
        GPIO.out1_w1ts.val = LED_BIT;
        wait_until(t, (word >> i) & 1 ? t1h : t0h);
        GPIO.out1_w1tc.val = LED_BIT;
        wait_until(t, per);
    }
    portEXIT_CRITICAL(&lock);
    esp_rom_delay_us(80); /* latch */
}

void app_main(void)
{
    led_pin_init();
    while (1) {
        led_send(0, 2, 0);
        vTaskDelay(pdMS_TO_TICKS(700));
        led_send(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
        led_send(2, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(700));
        led_send(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
