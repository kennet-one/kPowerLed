idf_component_register(SRCS "powled_node.c" "legacy_root_sender.c" "mesh_main.c"
                        "legacy_proto.c"
                        "stack_monitor.c"
                        "legacy_root_sender.c"
                        "powled_node.c"
                    PRIV_REQUIRES esp_wifi esp_driver_gpio nvs_flash esp_adc driver 
                    INCLUDE_DIRS "." "include")
