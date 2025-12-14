#include <string.h>
#include "powled_node.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "powled";

#define POWLED_GPIO	GPIO_NUM_33

// 0 -> LOW, 1 -> HIGH (як у твоєму Arduino-коді)
static uint8_t s_state = 0;

static void apply_state(void)
{
	gpio_set_level(POWLED_GPIO, s_state ? 1 : 0);
	ESP_LOGI(TAG, "GPIO%d=%d (state=%u)", (int)POWLED_GPIO, s_state ? 1 : 0, (unsigned)s_state);
}

void powled_node_init(void)
{
	gpio_config_t io = {
		.pin_bit_mask = 1ULL << POWLED_GPIO,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};
	gpio_config(&io);

	s_state = 0;	// дефолт як у тебе: powled0
	apply_state();
}

void powled_node_legacy_cmd(const char *txt)
{
	if (!txt) return;

	if (strcmp(txt, "powled0") == 0) {
		s_state = 0;
		apply_state();
		return;
	}
	if (strcmp(txt, "powled1") == 0) {
		s_state = 1;
		apply_state();
		return;
	}

	// інші команди ігноруємо (або логай, якщо хочеш)
}
