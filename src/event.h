
#define SWIPE_THRESHOLD 30 // 最小滑动距离阈值

extern int mp28167_flag;
extern int pwm_flag;
extern int i2cscan_flag;
extern int uart_helper_flag;
extern int BluetoothSerial_flag;
extern int FREcount_flag;

extern lv_obj_t *btn1;
extern lv_obj_t *btn2;
extern lv_obj_t *btn3;
extern lv_obj_t *btn4;
extern lv_obj_t *btn5;
extern lv_obj_t *btn6;
extern lv_obj_t *btn7;
extern lv_obj_t *btn8;
extern lv_obj_t *btn9;
extern lv_obj_t *btn10;

void ui_event_Screen1(lv_event_t *e);
extern lv_obj_t *ui_Screen1;
extern lv_obj_t *ui_Label1;
void ui_event_Image1(lv_event_t *e);
extern lv_obj_t *ui_Image1;
void ui_event_Screen2(lv_event_t *e);
extern lv_obj_t *ui_Screen2;
void ui_event_Image2(lv_event_t *e);
extern lv_obj_t *ui_Image2;
void anim_cb1(void *obj, int32_t v);
void anim_cb2(void *obj, int32_t v);
void anim_end_callback();

void btn1_event_cb(lv_event_t *e);
void btn2_event_cb(lv_event_t *e);
void btn3_event_cb(lv_event_t *e);
void btn4_event_cb(lv_event_t *e);
void btn5_event_cb(lv_event_t *e);
void btn6_event_cb(lv_event_t *e);
void btn7_event_cb(lv_event_t *e);
void btn8_event_cb(lv_event_t *e);
void btn9_event_cb(lv_event_t *e);
void btn10_event_cb(lv_event_t *e);

void i2conbtn_event_cb(lv_event_t *e);

void poweronbtn_event_cb(lv_event_t *e);
void pwm_btn_event_cb(lv_event_t *e);
void wireless_uart_btn_event_cb(lv_event_t *e);

extern lv_obj_t *mp28167_volt_keyboard;
extern int mp28167_flag;
extern void mp28167_set_voltage(float target_volts);

void mp28167_volt_keyboard_event_handler(lv_event_t *e);
void clear_mp28167_keyboard_event_handler(lv_event_t *e);
void VUPbtn_event_cb(lv_event_t *e);
void VDOWNbtn_event_cb(lv_event_t *e);
void V11btn_event_cb(lv_event_t *e);
void V5btn_event_cb(lv_event_t *e);
void V3btn_event_cb(lv_event_t *e);
void uart_btn_event_cb(lv_event_t *e);
void event_handler_back(lv_event_t *e);
void longpress_event_handler_back(lv_event_t *e);

void keyboard_event_handler(lv_event_t *e);
void fretext_event_handler(lv_event_t *e);
void dutytext_event_handler(lv_event_t *e);
void clear_keyboard_event_handler(lv_event_t *e);

void slider_event_cb(lv_event_t *e);

extern lv_timer_t *updatelabel_timer1;
extern lv_timer_t *updatelabel_timer2;
extern lv_timer_t *adddata_timer;
extern lv_timer_t *slider_update_timer;

extern lv_obj_t *panel;
