#ifndef CAMZ_H
#define CAMZ_H

#include <Arduino.h>
#include <esp_camera.h>
#include <esp_http_server.h>

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define FLASH_PIN 4

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

class Camz
{

    boolean flashEnabled = false;
    camera_fb_t *fb;

public:
    static esp_err_t capture_handler(httpd_req_t *req);
    static esp_err_t stream_handler(httpd_req_t *req);

    Camz()
    {

        pinMode(FLASH_PIN, OUTPUT);

        // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0);//disable brownout detector

        camera_config_t configg;
        configg.ledc_channel = LEDC_CHANNEL_0;
        configg.ledc_timer = LEDC_TIMER_0;
        configg.pin_d0 = Y2_GPIO_NUM;
        configg.pin_d1 = Y3_GPIO_NUM;
        configg.pin_d2 = Y4_GPIO_NUM;
        configg.pin_d3 = Y5_GPIO_NUM;
        configg.pin_d4 = Y6_GPIO_NUM;
        configg.pin_d5 = Y7_GPIO_NUM;
        configg.pin_d6 = Y8_GPIO_NUM;
        configg.pin_d7 = Y9_GPIO_NUM;
        configg.pin_xclk = XCLK_GPIO_NUM;
        configg.pin_pclk = PCLK_GPIO_NUM;
        configg.pin_vsync = VSYNC_GPIO_NUM;
        configg.pin_href = HREF_GPIO_NUM;
        configg.pin_sccb_sda = SIOD_GPIO_NUM;
        configg.pin_sccb_scl = SIOC_GPIO_NUM;
        configg.pin_pwdn = PWDN_GPIO_NUM;
        configg.pin_reset = RESET_GPIO_NUM;
        configg.xclk_freq_hz = 20000000;
        configg.pixel_format = PIXFORMAT_JPEG;

        // rtc_gpio_hold_dis(GPIO_NUM_4);

        if (psramFound())
        {
            Serial.printf("Ps ram found!");
            // configg.frame_size = FRAMESIZE_QXGA;
            configg.frame_size = FRAMESIZE_UXGA;
            configg.jpeg_quality = 10;
            configg.fb_count = 2;
            configg.grab_mode = CAMERA_GRAB_LATEST;
        }
        else
        {
            configg.frame_size = FRAMESIZE_SVGA;
            configg.jpeg_quality = 12;
            configg.fb_count = 1;
        }

        // Init Camera
        esp_err_t err = esp_camera_init(&configg);
        if (err != ESP_OK)
        {
            Serial.printf("Camera init failed with error");
        }
    }

    camera_fb_t *takePhoto()
    {
        return esp_camera_fb_get();
    }

    void reuseBuffer(camera_fb_t *fb)
    {
        esp_camera_fb_return(fb);
    }

    bool toggleFlash()
    {
        flashEnabled = !flashEnabled;
        digitalWrite(FLASH_PIN, flashEnabled ? HIGH : LOW);
        return flashEnabled;
    }

    static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
    {
        jpg_chunking_t *j = (jpg_chunking_t *)arg;
        if (!index)
        {
            j->len = 0;
        }
        if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
        {
            return 0;
        }
        j->len += len;
        return len;
    }
};

esp_err_t Camz::capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t out_len, out_width, out_height;
    uint8_t *out_buf;
    bool s;
    bool detected = false;
    int face_id = 0;
    if (true)
    {
        size_t fb_len = 0;
        if (fb->format == PIXFORMAT_JPEG)
        {
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        }
        else
        {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
        esp_camera_fb_return(fb);
        int64_t fr_end = esp_timer_get_time();
        Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));
        return res;
    }
}

esp_err_t Camz::stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        }
        else
        {
            if (fb->width > 400)
            {
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }
        // Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
    }
    return res;
}

#endif