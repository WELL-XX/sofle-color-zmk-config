/* SPDX-License-Identifier: MIT
 * G2 compile/initialization probe. This is not a DH06 or RGB display driver.
 */
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>

#define DOMAINS DT_NODELABEL(sc_domains)
static const struct gpio_dt_spec parent = GPIO_DT_SPEC_GET(DOMAINS, parent_gpios);
static const struct gpio_dt_spec lcd_gate = GPIO_DT_SPEC_GET(DOMAINS, lcd_gate_gpios);
static const struct gpio_dt_spec rgb_gate = GPIO_DT_SPEC_GET(DOMAINS, rgb_gate_gpios);
static const struct gpio_dt_spec rgb_data = GPIO_DT_SPEC_GET(DOMAINS, rgb_data_gpios);
static const struct pwm_dt_spec rgb_pwm = PWM_DT_SPEC_GET(DOMAINS);
static const struct gpio_dt_spec lcd_signals[] = {
    GPIO_DT_SPEC_GET(DOMAINS, lcd_sck_gpios),
    GPIO_DT_SPEC_GET(DOMAINS, lcd_mosi_gpios),
    GPIO_DT_SPEC_GET(DOMAINS, lcd_cs_gpios),
};
static const struct spi_dt_spec panel =
    SPI_DT_SPEC_GET(DT_NODELABEL(dh06_probe),
                    SPI_WORD_SET(8) | SPI_TRANSFER_LSB | SPI_CS_ACTIVE_HIGH, 0);

BUILD_ASSERT(DT_PROP(DT_NODELABEL(dh06_probe), spi_max_frequency) == 500000);
BUILD_ASSERT(DT_PWMS_PERIOD(DOMAINS) == 1250);
BUILD_ASSERT(DT_SAME_NODE(DT_PWMS_CTLR(DOMAINS), DT_NODELABEL(pwm0)));
BUILD_ASSERT(DT_GPIO_PIN(DOMAINS, parent_gpios) == 13);
BUILD_ASSERT(DT_GPIO_PIN(DOMAINS, rgb_data_gpios) == 20);
BUILD_ASSERT(DT_GPIO_PIN(DOMAINS, lcd_gate_gpios) == 22);
BUILD_ASSERT(DT_GPIO_FLAGS(DOMAINS, lcd_gate_gpios) & GPIO_ACTIVE_LOW);
BUILD_ASSERT(DT_GPIO_FLAGS(DOMAINS, lcd_gate_gpios) & GPIO_OPEN_DRAIN);
BUILD_ASSERT(DT_GPIO_FLAGS(DOMAINS, rgb_gate_gpios) & GPIO_ACTIVE_LOW);
BUILD_ASSERT(DT_GPIO_FLAGS(DOMAINS, rgb_gate_gpios) & GPIO_OPEN_DRAIN);

static int g2_interfaces_init(void) {
    if (!gpio_is_ready_dt(&parent) || !gpio_is_ready_dt(&lcd_gate) ||
        !gpio_is_ready_dt(&rgb_gate) || !gpio_is_ready_dt(&rgb_data) ||
        !spi_is_ready_dt(&panel) || !pwm_is_ready_dt(&rgb_pwm)) {
        return -ENODEV;
    }
    int rc = gpio_pin_configure_dt(&rgb_data, GPIO_OUTPUT_INACTIVE);
    if (rc) { return rc; }
    /* This probe starts no PWM sequences and sends no SPI transactions:
     * explicitly quiesce all LCD pins and the shared PWM/data output.
     * The production driver must reapply SPI pinctrl on wake and verify the
     * entire reset/startup waveform on hardware, including before this init.
     */
    for (size_t i = 0; i < ARRAY_SIZE(lcd_signals); i++) {
        if (!gpio_is_ready_dt(&lcd_signals[i])) { return -ENODEV; }
        rc = gpio_pin_configure_dt(&lcd_signals[i], GPIO_OUTPUT_INACTIVE);
        if (rc) { return rc; }
    }
    rc = gpio_pin_configure_dt(&lcd_gate, GPIO_OUTPUT_INACTIVE | GPIO_OPEN_DRAIN);
    if (rc) { return rc; }
    rc = gpio_pin_configure_dt(&rgb_gate, GPIO_OUTPUT_INACTIVE | GPIO_OPEN_DRAIN);
    if (rc) { return rc; }
    rc = gpio_pin_configure_dt(&parent, GPIO_OUTPUT_INACTIVE);
    if (rc) { return rc; }
    /* Active-low gates are released (Hi-Z); parent rail is low/off.
     * No SPI transfers are issued. The production drivers must coordinate
     * quiesce/LOW data, 500ms VCOM, DMA completion and independent rail state.
     */
    return 0;
}

SYS_INIT(g2_interfaces_init, APPLICATION, 90);
