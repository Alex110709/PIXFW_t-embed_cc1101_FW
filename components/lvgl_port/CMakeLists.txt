idf_component_register(SRCS "lvgl_port.c"
                       "display_driver.c"
                       "input_driver.c"
                       "lvgl_helpers.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver spi_flash lvgl)