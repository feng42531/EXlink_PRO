#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

#include <lvgl.h>
// #include <demos/lv_demos.h>

#include <Wire.h>
#include "CST816T.h"

#include <WiFi.h>

#include "ui.h"

// #define I2C_SDA SDA
// #define I2C_SCL SCL
#define I2C_SDA 16
#define I2C_SCL 15
#define RST_N_PIN 17
#define INT_N_PIN -1

CST816T cst816t(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); 

hw_timer_t *tim1 = NULL;

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

/*Change to your screen resolution*/
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 280;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 2];
// static lv_color_t buf[screenWidth * 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    TouchInfos tp;
    tp = cst816t.GetTouchInfo();
    // bool touched = (tp.touching == 1&&tp.isValid == 1);
    bool touched = tp.touching;

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = tp.x;
        data->point.y = tp.y;

        Serial.printf("touchX = %d, touchY = %d\n", tp.x, tp.y);
    }
}

void tim1Interrupt()
{
    lv_tick_inc(1);
}

void lvgl_handler(void *pvParameters)
{
	while(1)
	{
        lv_timer_handler(); /* let the GUI do its work */
        delay(5);
	}
}

void setup()
{
    Serial.begin(115200); /* prepare for possible serial debug */
    Serial.println("Hello Arduino! (V8.0.X)");

    pinMode(10, OUTPUT);
    digitalWrite(10, LOW);

    tft.init();

    //显示屏背光
    ledcSetup(1, 5000/*freq*/, 10 /*resolution*/);
    ledcAttachPin(14, 1);
    analogReadResolution(10);
    ledcWrite(1,200);

    cst816t.begin();  

    lv_init();

    // WiFi.setTxPower(WIFI_POWER_2dBm);
    // Serial.println(WiFi.localIP().toString().c_str());

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 2);
    // lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * 100);
 
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // source tick for lvgl
    tim1 = timerBegin(0, 80, true);
    timerAttachInterrupt(tim1, tim1Interrupt, true);
    timerAlarmWrite(tim1, 1000, true);
    timerAlarmEnable(tim1);

#if 1
    Serial.println("ui_init start");
    ui_init();
    Serial.println("ui_init done");
#else
    /* Try an example from the lv_examples Arduino library
       make sure to include it as written above.
    lv_example_btn_1();
    */

    // uncomment one of these demos
    // lv_demo_widgets(); // OK
    lv_demo_benchmark();          // OK
    // lv_demo_keypad_encoder();     // works, but I haven't an encoder
    // lv_demo_music();              // NOK 
    // lv_demo_printer();
    // lv_demo_stress();             // seems to be OK
#endif
    Serial.println("Setup done");

    xTaskCreate(lvgl_handler, "lvgl_handler", 4096, NULL, 2, NULL);
}

void loop()
{
    delay(1);
}