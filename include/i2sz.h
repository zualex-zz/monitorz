#ifndef I2SZ_H
#define I2SZ_H

#include <Arduino.h>
#include "driver/i2s.h"

#define I2S_WS 33
#define I2S_SD 32
#define I2S_SCK 25

#define I2S_PORT I2S_NUM_1

#define ESP_NOW_MAX_DATA_LEN 250       /*!< Maximum length of ESPNOW data which is sent very time */

class I2sz {
    static const i2s_pin_config_t in_pin_config;
    static const i2s_pin_config_t out_pin_config;
public:
    // I2S.
    static void startMic() {
        const i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = 11025, // or 44100 if you like
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 is 24 bits, but it doesn't work if we set 24 bit here.
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Ground the L/R pin on the INMP441.
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            // .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .intr_alloc_flags = 0, // default interrupt priority
            .dma_buf_count = 4,
            .dma_buf_len = ESP_NOW_MAX_DATA_LEN * 4, // * 4 for 32 bit.
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0,
        };
        if (ESP_OK != i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL)) {
            SERIALZ.println("i2s_driver_install: error");
        }

        if (ESP_OK != i2s_set_pin(I2S_PORT, &in_pin_config)) {
            SERIALZ.println("i2s_set_pin: error");
        }

        i2s_zero_dma_buffer(I2S_PORT);
        i2s_start(I2S_PORT);
    }

    static void startDacBuiltIn() {
        // Setup I2S first, because the ESP-Now listener uses it.
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
            .sample_rate = 11025,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .intr_alloc_flags = 0,
            .dma_buf_count = 4,
            .dma_buf_len = ESP_NOW_MAX_DATA_LEN * 2,
            .use_apll = false
        };
        i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        i2s_zero_dma_buffer(I2S_NUM_0);
        i2s_set_pin(I2S_NUM_0, NULL);
        i2s_start(I2S_NUM_0);
    }

    static void startDacExternal() {
        // Setup I2S first, because the ESP-Now listener uses it.
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
            .sample_rate = 11025,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .intr_alloc_flags = 0,
            .dma_buf_count = 4,
            .dma_buf_len = ESP_NOW_MAX_DATA_LEN * 2,
            .use_apll = false
        };
        i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        i2s_zero_dma_buffer(I2S_NUM_0);
        i2s_set_pin(I2S_NUM_0, &out_pin_config);
        i2s_start(I2S_NUM_0);
    }

};

const i2s_pin_config_t I2sz::in_pin_config = {
    .bck_io_num = 25,
    .ws_io_num = 33,
    .data_out_num = -1,
    .data_in_num = 32
};

const i2s_pin_config_t I2sz::out_pin_config = {
    .bck_io_num = 27,
    .ws_io_num = 26,
    .data_out_num = 25
};

#endif