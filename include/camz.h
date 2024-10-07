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

// Image Settings
#define FRAME_SIZE_MOTION FRAMESIZE_QVGA // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA - Do not use sizes above QVGA when not JPEG
#define FRAME_SIZE_PHOTO FRAMESIZE_XGA   // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
#define BLOCK_SIZE_X 20                  // size of image blocks used for motion sensing (20)
#define BLOCK_SIZE_Y 20

#define FWIDTH 320 // motion sensing frame size
#define FHEIGHT 240
#define FW (FWIDTH / BLOCK_SIZE_X) // number of blocks in image
#define FH (FHEIGHT / BLOCK_SIZE_Y)

typedef void (*CamCallBack)();

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
    // boolean detectionEnabled = true;
    uint16_t Block_threshold = 10;          // average pixel variation in block required to count as changed - range 0 to 255
    uint16_t Image_thresholdL = 15;         // min changed blocks in image required to count as motion detected in percent
    uint16_t Image_thresholdH = 100;        // max changed blocks in image required to count as motion detected in percent
    uint16_t tCounter = 0;                  // count number of consecutive triggers (i.e. how many times in a row movement has been detected)
    uint16_t tCounterTrigger = 1;           // only trigger if movement detected in more than one consequitive frames
    uint32_t TRIGGERtimer = 0;              // used for limiting camera motion trigger rate
    uint16_t TriggerLimitTime = 2;          // min time between motion detection trigger events (seconds)
    uint16_t AveragePix = 0;                // average pixel reading from captured image (used for nighttime compensation) - bright day = around 120
    float thresholdGainCompensation = 0.65; // motion detection level compensation for increased noise in image when gain increased (i.e. in darker conditions)
    float cameraImageGain = 0;              // Image gain (loaded from spiffs)
    float cameraImageExposure = 0;          // Camera exposure (loaded from spiffs)
    bool cameraImageInvert = 0;             // Invert image  (i.e. upside down)
    int8_t cameraImageBrightness = 0;       // image brighness (-2 to 2) - Note: has no effect?
    int8_t cameraImageContrast = 0;         // image contrast (-2 to 2) - Note: has no effect?
    bool cameraImageFlip = 0;               // flip image  (i.e. left to right)
    // frame stores (blocks)
    uint16_t prev_frame[FH][FW] = {0};    // previously captured frame
    uint16_t current_frame[FH][FW] = {0}; // current frame
    // bool block_active(uint16_t x, uint16_t y);
    // uint16_t latestChanges = 0; // store most current motion detection reading for display on main page
    CamCallBack camMotionCallback;

    boolean flashEnabled = false;
    camera_fb_t *fb;

public:
    static esp_err_t capture_handler(httpd_req_t *req);
    static esp_err_t stream_handler(httpd_req_t *req);

    Camz(CamCallBack camMotion = []() {})
    {
        camMotionCallback = camMotion;

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
        configg.xclk_freq_hz = 20000000;            // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        configg.pixel_format = PIXFORMAT_GRAYSCALE; // PIXFORMAT_ + YUV422, GRAYSCALE, RGB565, JPEG, RGB888?
        // configg.pixel_format = PIXFORMAT_JPEG;

        // rtc_gpio_hold_dis(GPIO_NUM_4);

        if (psramFound())
        {
            Serial.printf("Ps ram found!");
            configg.frame_size = FRAME_SIZE_MOTION; // FRAMESIZE_ + QVGA, CIF, VGA, SVGA, XGA, SXGA, UXGA
            configg.jpeg_quality = 10;              // 0-63 lower number means higher quality (can cause failed image capture if set too low at higher resolutions)
            configg.fb_count = 1;                   // if more than one, i2s runs in continuous mode. Use only with JPEG
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
        // cameraImageSettings(FRAME_SIZE_MOTION);
        TRIGGERtimer = millis(); // reset the retrigger timer to stop instant triggering of motion detection
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

    // https://github.com/alanesq/CameraWifiMotion/blob/master/CameraWifiMotion/motion.h
    // ---------------------------------------------------------------
    //             -apply camera sensor/image settings
    // ---------------------------------------------------------------
    // see: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/

    bool cameraImageSettings(framesize_t fsize)
    {
        Serial.printf("cameraImageSettings");
        sensor_t *s = esp_camera_sensor_get();
        Serial.printf("esp_camera_sensor_get");
        if (s == NULL)
        {
            // if (serialDebug)
            Serial.println("Error: problem getting camera sensor settings");
            return 0;
        }

        // #if IMAGE_SETTINGS // Implement adjustment of image settings

        // Image resolution / type  (may not be required?)
        Serial.printf("set_framesize");
        // Serial.printf(fsize);
        s->set_framesize(s, fsize); // FRAME_SIZE_PHOTO , FRAME_SIZE_MOTION
        if (fsize == FRAME_SIZE_MOTION)
            Serial.printf("set_fset_pixformatramesize");
        // Serial.printf(PIXFORMAT_GRAYSCALE);
        s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
        if (fsize == FRAME_SIZE_PHOTO)
            s->set_pixformat(s, PIXFORMAT_JPEG);

        Serial.printf("set_gain_ctrl");
        // If you enable gain_ctrl or exposure_ctrl it will prevent a lot of the other settings having any effect
        // more info on settings here: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
        s->set_gain_ctrl(s, 0);                      // auto gain off (1 or 0)
        s->set_exposure_ctrl(s, 0);                  // auto exposure off (1 or 0)
        s->set_agc_gain(s, cameraImageGain);         // set gain manually (0 - 31)
        s->set_aec_value(s, cameraImageExposure);    // set exposure manually  (0-1200)
        s->set_vflip(s, cameraImageInvert);          // Invert image (0 or 1)
        s->set_quality(s, 10);                       // (0 - 63)
        s->set_gainceiling(s, GAINCEILING_32X);      // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
        s->set_brightness(s, cameraImageBrightness); // (-2 to 2) - set brightness
        s->set_lenc(s, 1);                           // lens correction? (1 or 0)
        s->set_saturation(s, 0);                     // (-2 to 2)
        s->set_contrast(s, cameraImageContrast);     // (-2 to 2)
        s->set_sharpness(s, 0);                      // (-2 to 2)
        s->set_hmirror(s, cameraImageFlip);          // (0 or 1) flip horizontally
        s->set_colorbar(s, 0);                       // (0 or 1) - show a testcard
        s->set_special_effect(s, 0);                 // (0 to 6?) apply special effect
                                                     //       s->set_whitebal(s, 0);                        // white balance enable (0 or 1)
                                                     //       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
                                                     //       s->set_wb_mode(s, 0);                         // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
                                                     //       s->set_dcw(s, 0);                             // downsize enable? (1 or 0)?
                                                     //       s->set_raw_gma(s, 1);                         // (1 or 0)
                                                     //       s->set_aec2(s, 0);                            // automatic exposure sensor?  (0 or 1)
                                                     //       s->set_ae_level(s, 0);                        // auto exposure levels (-2 to 2)
        s->set_bpc(s, 0);                            // black pixel correction
        s->set_wpc(s, 0);                            // white pixel correction
                                                     // #endif

        // capture a frame to ensure settings apply (not sure if this is really needed)
        // camera_fb_t *frame_buffer = esp_camera_fb_get(); // capture frame from camera
        // esp_camera_fb_return(frame_buffer);              // return frame so memory can be released

        return 1;
    }
    // ---------------------------------------------------------------
    //                          -capture image
    // ---------------------------------------------------------------
    // Capture image and down-sample in to blocks
    // this sets all blocks to value zero then goes through each pixel in the greyscale image and adds its value to
    // the relevant blocks total.  After this each blocks value is divided by the number of pixels in it
    // resulting in each blocks value being the average value of all the pixels within it.

    bool capture_still()
    {
        // Serial.println("capture_still");
        // checkCameraIsFree(); // try to avoid using camera if already in use
        // if (!detectionEnabled)
        //     return 0; // if detection is paused another process may be using camera

        Serial.flush(); // wait for serial data to be sent first as I suspect this can cause problems capturing an image
                        //      although I have read that this command has changed and no longer performs this function?

        uint32_t temp_frame[FH][FW] = {0};

        // capture image from camera
        // cameraImageSettings(FRAME_SIZE_MOTION);          // apply camera sensor settings
        camera_fb_t *frame_buffer = esp_camera_fb_get(); // capture frame from camera
        if (!frame_buffer)
        { // if there was a problem grabbing a frame try again
            frame_buffer = esp_camera_fb_get();
            if (!frame_buffer)
                return false; // failed to capture image
        }

        // down-sample image in to blocks
        for (uint32_t i = 0; i < (FWIDTH * FHEIGHT); i++)
        {                                  // step through all pixels in image
            const uint16_t x = i % FWIDTH; // calculate x and y location of this pixel in the image
            const uint16_t y = floor(i / FWIDTH);
            const uint8_t block_x = floor(x / BLOCK_SIZE_X); // calculate which block this pixel is in
            const uint8_t block_y = floor(y / BLOCK_SIZE_Y);
            const uint8_t pixel = frame_buffer->buf[i]; // get the pixels brightness (0 to 255)
            temp_frame[block_y][block_x] += pixel;      // add this pixel to the blocks running total
        }

        esp_camera_fb_return(frame_buffer); // return frame so memory can be released

        // average the values for all pixels in each block
        bool frameChanged = 0;       // flag if any change at all since last frame (used to detect problem)
        uint16_t TempAveragePix = 0; // average pixel reading (used for calculating image brightness)
        for (int y = 0; y < FH; y++)
        {
            for (int x = 0; x < FW; x++)
            {
                uint16_t currentBlock = temp_frame[y][x] / (BLOCK_SIZE_X * BLOCK_SIZE_Y); // average pixel brightness in the block
                if (current_frame[y][x] != currentBlock)
                    frameChanged = 1;
                current_frame[y][x] = currentBlock;
                TempAveragePix += currentBlock; // used to calculate average brightness of whole image
            }
        }
        if (!frameChanged)
            Serial.println("Suspect camera problem as no change at all since previous image was captured");
        // AveragePix = TempAveragePix / (FH * FW); // calculate the average pixel brightness in whole image
        // if (serialDebug && showFrames)
        //     print_frame(current_frame); // show captured frame on serial port for debugging
        // Serial.print("AveragePix ");
        // Serial.println(AveragePix);
        return true;
    }

    // ---------------------------------------------------------------
    //     -Compute the number of different blocks in the frames
    // ---------------------------------------------------------------
    // If there are enough, then motion has happened returns the number of changed active blocks

    float motion_detect()
    {
        // Serial.println("motion_detect");
        uint16_t changes = 0;
        // const uint16_t blocks = (FWIDTH * FHEIGHT) / (BLOCK_SIZE_X * BLOCK_SIZE_Y);     // total number of blocks in image

        // adjust block_threshold for gain setting (to compensate for noise introduced with gain)
        uint16_t tThreshold = Block_threshold; // + (float)(cameraImageGain * thresholdGainCompensation);

        // go through all blocks in current frame and check for changes since previous frame
        for (int y = 0; y < FH; y++)
        {
            for (int x = 0; x < FW; x++)
            {
                uint16_t current = current_frame[y][x];
                uint16_t prev = prev_frame[y][x];
                uint16_t pChange = abs(current - prev); // modified code Feb20 - gives blocks average pixels variation in range 0 to 255
                // float pChange = abs(current - prev) / prev;   // original code
                if (pChange >= tThreshold)
                {                 // if change in block is enough to qualify as changed
                                  // if (block_active(x, y))
                    changes += 1; // if detection mask is enabled for this block increment changed block count
                                  // if (serialDebug)
                                  // {
                    // Serial.print("diff\t");
                    // Serial.print(y);
                    // Serial.print('\t');
                    // Serial.print(x);
                    // Serial.print('\t');
                    // Serial.print(current);
                    // Serial.print('\t');
                    // Serial.print(prev);
                    // Serial.print('\t');
                    // Serial.println(pChange);
                    // }
                }
            }
        }

        // if (changes > latestChanges)
        //     latestChanges = changes; // store highest reading for display on main page (it is zeroed when displayed)

        // Consecutive triggers counter (i.e. how many times in a row movement has been detected)
        if (changes >= Image_thresholdL && changes <= Image_thresholdH)
        {
            tCounter++;
            // Serial.print("C ");
            // Serial.print(changes);
            // Serial.println(" ");
        }
        else
            tCounter = 0;

        // if (serialDebug)
        // {
        // Serial.println();
        // Serial.print("Changed ");
        // Serial.print("C ");
        // Serial.println(changes);
        // Serial.println(" ");
        // Serial.print(" out of ");
        // Serial.println(mask_active * blocksPerMaskUnit);
        // }

        return changes; // return number of changed blocks
    }

    // ---------------------------------------------------------------
    //              -Copy current frame to previous
    // ---------------------------------------------------------------

    void update_frame()
    {
        memcpy(prev_frame, current_frame, sizeof(prev_frame));
    }

    void loop()
    {
        capture_still();
        uint16_t changes = motion_detect();
        update_frame();
        if ((changes >= Image_thresholdL) && (changes <= Image_thresholdH))
        { // if enough change to count as motion detected
            if (tCounter >= tCounterTrigger)
            { // only trigger if movement detected in more than one consequitive frames
                Serial.print(tCounter);
                // Serial.println(" MotionDetected !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                tCounter = 0;
                if ((unsigned long)(millis() - TRIGGERtimer) >= (TriggerLimitTime * 1000))
                {                            // limit time between triggers
                    TRIGGERtimer = millis(); // update last trigger time
                                             // run motion detected procedure (blocked if io high is required)
                                             // if (ioRequiredHighToTrigger == 0 || SensorStatus == 1)
                                             // {
                    // MotionDetected(changes);
                    camMotionCallback();
                    // Serial.print(tCounter);
                    Serial.println(" MotionDetected !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                }
                // else
                // {
                //     Serial.println("Too soon to re-trigger");
                // }
            }
            else
            {
                // if (serialDebug)
                // {
                Serial.println("enough change to count as motion detectedToo soon to re-trigger");
                // }
            }
        }
        // else
        // {
        //     // if (serialDebug)
        //     // {
        //     Serial.println("Changes under treshold ");
        //     Serial.println(changes);
        //     // }
        // }
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
    Serial.println("stream_handler");
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
    Serial.println("stream_handler END");
    return res;
}

#endif