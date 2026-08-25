#include "powled_schedule.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "powled_node.h"

#define POWLED_SCHEDULE_BLOB_MAGIC 0x3153504bUL
#define POWLED_SCHEDULE_ACTION_OFF 0U
#define POWLED_SCHEDULE_ACTION_ON 1U

static const char *TAG = "powled_sched";
static keemash_weekly_schedule_t *s_schedule;

static bool parse_hex_field(const char *text, size_t offset, size_t digits,
			    uint32_t *value)
{
	if (!text || !value || digits == 0 || digits > 8) return false;
	uint32_t parsed = 0;
	for (size_t i = 0; i < digits; i++) {
		char c = text[offset + i];
		uint8_t nibble;
		if (c >= '0' && c <= '9') nibble = (uint8_t)(c - '0');
		else if (c >= 'a' && c <= 'f') nibble = (uint8_t)(c - 'a' + 10);
		else if (c >= 'A' && c <= 'F') nibble = (uint8_t)(c - 'A' + 10);
		else return false;
		parsed = (parsed << 4) | nibble;
	}
	*value = parsed;
	return true;
}

static esp_err_t apply_point(void *user,
			     const keemash_weekly_schedule_point_t *point,
			     uint8_t index, bool catch_up)
{
	(void)user;
	if (!point || point->action > POWLED_SCHEDULE_ACTION_ON) {
		return ESP_ERR_INVALID_ARG;
	}
	esp_err_t err = powled_node_set_state(point->action == POWLED_SCHEDULE_ACTION_ON);
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "%s point %u applied state=%s",
			 catch_up ? "catch-up" : "scheduled", (unsigned)index,
			 point->action == POWLED_SCHEDULE_ACTION_ON ? "ON" : "OFF");
	}
	return err;
}

static void format_meta(char *out, size_t out_size,
			const keemash_weekly_schedule_status_t *status)
{
	uint8_t active = status->active_index == KEEMASH_WEEKLY_SCHEDULE_NO_INDEX
		? 0x0fU : status->active_index;
	uint8_t next = status->next_index == KEEMASH_WEEKLY_SCHEDULE_NO_INDEX
		? 0x0fU : status->next_index;
	snprintf(out, out_size, "PSM%08" PRIX32 "%X%u%u%u%X%X%u",
		 status->config.generation, (unsigned)status->config.count,
		 status->config.enabled ? 1U : 0U,
		 status->config.persistence_enabled ? 1U : 0U,
		 status->clock_valid ? 1U : 0U, (unsigned)active, (unsigned)next,
		 powled_node_state() ? 1U : 0U);
}

static void format_point(char *out, size_t out_size, uint32_t generation,
			 uint8_t index,
			 const keemash_weekly_schedule_point_t *point)
{
	snprintf(out, out_size, "PSP%08" PRIX32 "%X%u%03X%u%02X",
		 generation, (unsigned)index, point->enabled ? 1U : 0U,
		 (unsigned)point->minute_of_day, (unsigned)point->action,
		 (unsigned)point->days_mask);
}

esp_err_t powled_schedule_start(void)
{
	if (s_schedule) return ESP_OK;
	keemash_weekly_schedule_options_t options = {
		.nvs_namespace = "powled",
		.nvs_key = "schedule_v1",
		.blob_magic = POWLED_SCHEDULE_BLOB_MAGIC,
		.max_action = POWLED_SCHEDULE_ACTION_ON,
		.catch_up_on_clock_ready = true,
		.stage_timeout_ms = 30000,
		.task_period_ms = 1000,
		.task_stack_words = 3584,
		.task_priority = 7,
		.task_name = "powled_sched",
		.apply = apply_point,
	};
	return keemash_weekly_schedule_start(&s_schedule, &options);
}

void powled_schedule_get_status(keemash_weekly_schedule_status_t *status)
{
	keemash_weekly_schedule_get_status(s_schedule, status);
}

bool powled_schedule_execute_command(const char *text, esp_err_t *error,
				     char *reply, size_t reply_size)
{
	if (!text || !error || !reply || reply_size == 0 || !s_schedule) return false;
	*error = ESP_OK;
	reply[0] = '\0';
	size_t length = strlen(text);
	uint32_t generation = 0;
	if (length == 14 && strncmp(text, "PSB", 3) == 0) {
		uint32_t count = 0, enabled = 0, persist = 0;
		if (!parse_hex_field(text, 3, 8, &generation) || generation == 0 ||
		    !parse_hex_field(text, 11, 1, &count) ||
		    !parse_hex_field(text, 12, 1, &enabled) ||
		    !parse_hex_field(text, 13, 1, &persist) ||
		    count > KEEMASH_WEEKLY_SCHEDULE_MAX_POINTS ||
		    enabled > 1 || persist > 1) {
			*error = ESP_ERR_INVALID_ARG;
		} else {
			*error = keemash_weekly_schedule_stage_begin(
				s_schedule, generation, (uint8_t)count,
				enabled != 0, persist != 0);
		}
	} else if (length == 19 && strncmp(text, "PSP", 3) == 0) {
		uint32_t index = 0, enabled = 0, minute = 0, action = 0, days = 0;
		if (!parse_hex_field(text, 3, 8, &generation) || generation == 0 ||
		    !parse_hex_field(text, 11, 1, &index) ||
		    !parse_hex_field(text, 12, 1, &enabled) ||
		    !parse_hex_field(text, 13, 3, &minute) ||
		    !parse_hex_field(text, 16, 1, &action) ||
		    !parse_hex_field(text, 17, 2, &days) || enabled > 1 ||
		    index >= KEEMASH_WEEKLY_SCHEDULE_MAX_POINTS ||
		    minute >= 24U * 60U || action > POWLED_SCHEDULE_ACTION_ON ||
		    days == 0 || days > KEEMASH_WEEKLY_SCHEDULE_ALL_DAYS) {
			*error = ESP_ERR_INVALID_ARG;
		} else {
			keemash_weekly_schedule_point_t point = {
				.enabled = enabled != 0,
				.minute_of_day = (uint16_t)minute,
				.action = (uint8_t)action,
				.days_mask = (uint8_t)days,
			};
			*error = keemash_weekly_schedule_stage_point(
				s_schedule, generation, (uint8_t)index, &point);
		}
	} else if (length == 11 && strncmp(text, "PSC", 3) == 0) {
		if (!parse_hex_field(text, 3, 8, &generation) || generation == 0) {
			*error = ESP_ERR_INVALID_ARG;
		} else {
			*error = keemash_weekly_schedule_stage_commit(s_schedule, generation);
			if (*error == ESP_OK) {
				keemash_weekly_schedule_status_t status;
				powled_schedule_get_status(&status);
				format_meta(reply, reply_size, &status);
			}
		}
	} else if (length == 3 && strcmp(text, "PSQ") == 0) {
		keemash_weekly_schedule_status_t status;
		powled_schedule_get_status(&status);
		format_meta(reply, reply_size, &status);
	} else if (length == 4 && strncmp(text, "PSQ", 3) == 0) {
		uint32_t index = 0;
		keemash_weekly_schedule_status_t status;
		powled_schedule_get_status(&status);
		if (!parse_hex_field(text, 3, 1, &index) || index >= status.config.count) {
			*error = ESP_ERR_NOT_FOUND;
		} else {
			format_point(reply, reply_size, status.config.generation,
				     (uint8_t)index, &status.config.points[index]);
		}
	} else {
		return false;
	}

	if (*error != ESP_OK && reply[0] == '\0') {
		snprintf(reply, reply_size, "ERR:PS:%d", (int)*error);
	}
	reply[reply_size - 1] = '\0';
	return true;
}
