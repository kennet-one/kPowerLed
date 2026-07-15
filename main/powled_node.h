#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"

esp_err_t powled_node_init(void);
bool powled_node_handle_command(const char *text, char *reply, size_t reply_size);
bool powled_node_state(void);
