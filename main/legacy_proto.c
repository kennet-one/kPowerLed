#include "legacy_proto.h"

#include "esp_log.h"
#include "legacy_root_sender.h"
#include "powled_node.h"

static const char *TAG = "legacy";

bool legacy_handle_command(const char *text)
{
	char reply[LEGACY_ROOT_MSG_MAX_LEN] = {0};
	if (!powled_node_handle_command(text, reply, sizeof(reply))) return false;
	return legacy_send_to_root(reply);
}

void legacy_handle_text(const char *text)
{
	if (!text || !text[0]) return;
	ESP_LOGI(TAG, "RX: \"%s\"", text);
	if (!legacy_handle_command(text)) {
		ESP_LOGD(TAG, "unsupported command: \"%s\"", text);
	}
}
