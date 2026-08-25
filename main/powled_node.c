#include "powled_node.h"

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define POWLED_GPIO GPIO_NUM_33

static const char *TAG = "powled";
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static bool s_initialized;
static bool s_state;

esp_err_t powled_node_set_state(bool enabled)
{
	if (!s_initialized) return ESP_ERR_INVALID_STATE;
	portENTER_CRITICAL(&s_lock);
	bool previous = s_state;
	s_state = enabled;
	esp_err_t err = gpio_set_level(POWLED_GPIO, enabled ? 1 : 0);
	if (err != ESP_OK) s_state = previous;
	portEXIT_CRITICAL(&s_lock);
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "GPIO%d=%u", (int)POWLED_GPIO, enabled ? 1U : 0U);
	}
	return err;
}

static void toggle_state(void)
{
	portENTER_CRITICAL(&s_lock);
	s_state = !s_state;
	bool enabled = s_state;
	gpio_set_level(POWLED_GPIO, enabled ? 1 : 0);
	portEXIT_CRITICAL(&s_lock);
	ESP_LOGI(TAG, "GPIO%d=%u", (int)POWLED_GPIO, enabled ? 1U : 0U);
}

esp_err_t powled_node_init(void)
{
	esp_err_t err = gpio_reset_pin(POWLED_GPIO);
	if (err != ESP_OK) return err;
	err = gpio_set_direction(POWLED_GPIO, GPIO_MODE_OUTPUT);
	if (err != ESP_OK) return err;
	err = gpio_set_level(POWLED_GPIO, 0);
	if (err != ESP_OK) return err;

	portENTER_CRITICAL(&s_lock);
	s_state = false;
	s_initialized = true;
	portEXIT_CRITICAL(&s_lock);
	ESP_LOGI(TAG, "GPIO%d initialized OFF (active high)", (int)POWLED_GPIO);
	return ESP_OK;
}

bool powled_node_state(void)
{
	portENTER_CRITICAL(&s_lock);
	bool state = s_state;
	portEXIT_CRITICAL(&s_lock);
	return state;
}

bool powled_node_handle_command(const char *text, char *reply, size_t reply_size)
{
	if (!text || !reply || reply_size == 0 || !s_initialized) return false;

	if (strcmp(text, "powled0") == 0) {
		if (powled_node_set_state(false) != ESP_OK) return false;
	} else if (strcmp(text, "powled1") == 0) {
		if (powled_node_set_state(true) != ESP_OK) return false;
	} else if (strcmp(text, "powled") == 0) {
		toggle_state();
	} else if (strcmp(text, "pwech") != 0) {
		return false;
	}

	snprintf(reply, reply_size, "feedpowled%u", powled_node_state() ? 1U : 0U);
	reply[reply_size - 1] = '\0';
	return true;
}
