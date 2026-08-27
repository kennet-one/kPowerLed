#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"
#include "keemash_weekly_schedule.h"

esp_err_t powled_schedule_start(void);
esp_err_t powled_schedule_publisher_start(void);
bool powled_schedule_execute_command(const char *text, esp_err_t *error,
				     char *reply, size_t reply_size);
void powled_schedule_get_status(keemash_weekly_schedule_status_t *status);
