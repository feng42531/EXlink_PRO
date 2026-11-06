#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

    extern int ina266_flag;
    char batStr[20];
    lv_obj_t *btn1;
    lv_obj_t *btn2;
    lv_obj_t *btn3;
    lv_obj_t *btn4;
    lv_obj_t *btn5;
    lv_obj_t *btn6;
    lv_obj_t *btn7;
    lv_obj_t *btn8;
    lv_obj_t *btn9;
    lv_obj_t *btn10;
    lv_obj_t *bat_label;

    lv_obj_t *ui_Screen1;
    lv_obj_t *ui_Label1;
    lv_obj_t *ui_Image1;
    lv_obj_t *ui_Screen2;
    lv_obj_t *ui_Image2;
    lv_obj_t *label1;
    lv_obj_t *label2;
    lv_obj_t *label3;
    lv_obj_t *label4;
    lv_obj_t *chart5;
    lv_obj_t *panel;
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_obj_t *voltmeter_chart5;
    lv_obj_t *volt_chart;
    lv_obj_t *cur_chart;
    lv_obj_t *mp28167_volt;
    lv_obj_t *mp28167_volt_keyboard;
    lv_obj_t *poweron_label;
    lv_obj_t *fre;
    lv_obj_t *duty;
    lv_obj_t *fre_keyboard;
    lv_obj_t *duty_keyboard;
    extern lv_indev_t *indev_keypad;
    lv_group_t *group;
    lv_obj_t *i2c_extarea;
    lv_obj_t *i2con;
    lv_obj_t *uart_extarea;
    lv_obj_t *wireless_uart_extarea;
    lv_obj_t *pwm_btn;
    lv_obj_t *uart_btn;
    lv_obj_t *uart_list;
    lv_obj_t *wireless_uart_list;
    lv_obj_t *wireless_uart_btn;

    lv_timer_t *updatelabel_timer1;
    lv_timer_t *updatelabel_timer2;
    lv_timer_t *updatelabel_timer3;

    lv_timer_t *DSO_update_timer1;
    lv_timer_t *DSO_update_timer2;
    lv_timer_t *DSO_update_timer3;

    lv_timer_t *slider_update_timer;

    lv_timer_t *FRE_label_update_timer;

    extern char voltageStr[20], currentStr[20], powerStr[20], mAHStr[20];
    extern char maxValueStr[20], minValueStr[20], peakToPeakValueStr[20];
    extern char freqencyStr[20];
    lv_timer_t *adddata_timer;
    lv_timer_t *adddata_timer2;

#define SAMPLE_COUNT 256

    extern int DSO_flag;
    extern int adcValues[SAMPLE_COUNT]; // Store ADC values

    lv_timer_t *DSO_adddata_timer;
    lv_obj_t *DSO_chart;
    lv_chart_series_t *DSO_ser;
    lv_obj_t *FRE_label;

    lv_obj_t *slider;

    LV_IMG_DECLARE(ui_img_game3_png);
    LV_IMG_DECLARE(Exlink_png);
    LV_IMG_DECLARE(pinmap_png);
    LV_IMG_DECLARE(power_png);
    LV_IMG_DECLARE(pwm_png);
    LV_IMG_DECLARE(usarthelper_png);
    LV_IMG_DECLARE(i2c_png);
    LV_IMG_DECLARE(voltmeter_png);
    LV_IMG_DECLARE(DSO_png);
    LV_IMG_DECLARE(wireless_png);
    LV_IMG_DECLARE(readme_png);
    LV_IMG_DECLARE(pwmint_png);
    LV_IMG_DECLARE(FREcounter_png);
    LV_IMG_DECLARE(kobe_png);

    void create_boot_animation(void);
    void my_gui(void);
    void ui_Screen1_screen_init(void);
    void ui_init(void);
    void pinmap_init(void);
    void power_init(void);
    void pwm_init(void);
    void uarthelper_init(void);
    void i2c_init(void);
    void voltmeter_init(void);
    void DSO_init(void);
    void wirelessuart_init(void);
    void FREcount_init(void);
    void readme_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
