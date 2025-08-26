idf_component_register(SRCS "cc1101.c"
                       "cc1101_spi.c"
                       "cc1101_config.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver spi_flash)