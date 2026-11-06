#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Wire.h>
//#include "CST816T.h"   // ==========【删掉CST816T头文件】==========
#include <WiFi.h>
#include "ui.h"
#include "INA.h"
#include "stdio.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "soc/pcnt_struct.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_task_wdt.h>
#include "event.h"
#include <Adafruit_TSC2007.h>
#include <Adafruit_BusIO_Register.h> // TSC2007 库依赖

#define I2C_SDA 18 // 触摸SDA引脚
#define I2C_SCL 21 // 触摸SCL引脚
#define RST_N_PIN 16
#define INT_N_PIN 17
#define RIGHT_PIN 39
#define LEFT_PIN 38
#define PUSH_PIN 40
#define PCNT_UNIT PCNT_UNIT_0
#define LONG_PRESS_THRESHOLD 300
#define SHORT_PRESS_THRESHOLD 10
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


int adcValues[SAMPLE_COUNT];
char maxValueStr[20], minValueStr[20], peakToPeakValueStr[20];

volatile int pulseCount = 0;
float frequency = 0.0;
char freqencyStr[20];
int16_t count = 0;

unsigned long previousMillis = 0;

const long interval = 125;

INA_Class INA;
int8_t request_index = 0;
char voltageStr[20], currentStr[20], powerStr[20], mAHStr[20];
uint64_t vol, cur, wat;
double v, a, w;
volatile uint8_t deviceNumber = UINT8_MAX;
volatile uint64_t sumBusMillVolts = 0;
volatile int64_t sumBusMicroAmps = 0;
volatile uint8_t readings = 0;
volatile int64_t mAH = 0;

int ina266_flag = 0;
int mp28167_flag = 0;
int pwm_flag = 0;
int i2cscan_flag = 0;
int uart_helper_flag = 0;
int DSO_flag = 0;
int BluetoothSerial_flag = 0;
int FREcount_flag = 0;

int pwm_Freq = 0;
int pwm_Duty = 0;

int new_pwm_Freq = 0;
int new_pwm_Duty = 0;

int mcp4107_value;
long currentBaudRate = 115200;
lv_indev_t *indev_keypad;
byte error, address;
int nDevices;

static const uint32_t screenWidth = 320;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 2];

// ==========【删除CST816T对象】==========
// CST816T cst816t(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN);
Adafruit_TSC2007 tsc;
hw_timer_t *tim1 = NULL;
TFT_eSPI tft = TFT_eSPI();

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.println(rxValue[i]);
      }
    }
  }
};

#define MP28167_ADDR 0x60
#define REG_VREF_L 0x00
#define REG_VREF_H 0x01
#define REG_VREF_GO 0x02
const float MAX_OUTPUT_V = 20;

float volt_output;

void mp28167_i2c_write(uint8_t dev, uint8_t reg, uint8_t data)
{
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t mp28167_i2c_read(uint8_t dev, uint8_t reg)
{
  uint8_t data = 0;
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(dev, 1);
  if (Wire.available())
    data = Wire.read();
  return data;
}

void mp28167_set_voltage(float target_volts)
{
  if (target_volts < 0)
    target_volts = 0;
  if (target_volts > MAX_OUTPUT_V)
  {
    target_volts = MAX_OUTPUT_V;
  }

  uint32_t target_mv = target_volts * 1000;
  uint32_t reg_val = (target_mv * 100) / 1119;//设定分压系数

  uint8_t v_h = (reg_val >> 3) & 0xFF;
  uint8_t v_l = reg_val & 0x07;

  mp28167_i2c_write(MP28167_ADDR, REG_VREF_L, v_l);
  mp28167_i2c_write(MP28167_ADDR, REG_VREF_H, v_h);

  uint8_t go_reg = mp28167_i2c_read(MP28167_ADDR, REG_VREF_GO);
  mp28167_i2c_write(MP28167_ADDR, REG_VREF_GO, go_reg | 0x01);
}

void mp28167_init()
{
  mp28167_set_voltage(0);
}

void mp28167_task()
{
  if (mp28167_flag == 1)
  {
    const char *volt_str = lv_textarea_get_text(mp28167_volt);
    if (strlen(volt_str) == 0)
    {
      mp28167_flag = 0;
      return;
    }
    volt_output = atof(volt_str);
    mp28167_set_voltage(volt_output);
    mp28167_flag = 0;
  }

  if (mp28167_flag == 2)
  {
    volt_output += 0.1;
    mp28167_set_voltage(volt_output);
    mp28167_flag = 0;
  }

  if (mp28167_flag == 3)
  {
    volt_output -= 0.1;
    mp28167_set_voltage(volt_output);
    mp28167_flag = 0;
  }

  if (mp28167_flag == 4)
  {
    mp28167_set_voltage(0);
    mp28167_flag = 0;
  }
}

void buzzer_task()
{
  ledcWrite(1, 100);
  delay(100);
  ledcWrite(1, 0);
}

void pwm_task()
{

  if (pwm_flag == 1)
  {

    const char *freq_str = lv_textarea_get_text(fre);
    const char *duty_str = lv_textarea_get_text(duty);
    pwm_Freq = atoi(freq_str);
    pwm_Duty = constrain(atoi(duty_str), 0, 100);
    pwm_Duty = map(pwm_Duty, 0, 100, 0, 255);

    if (pwm_Freq != new_pwm_Freq | pwm_Duty != new_pwm_Duty)
    {
      ledcAttachPin(5, 2);
      ledcSetup(2, pwm_Freq, 8);
      ledcWrite(2, pwm_Duty);
      new_pwm_Freq = pwm_Freq;
      new_pwm_Duty = pwm_Duty;
    }
  }

  if (pwm_flag == 0)
  {
    new_pwm_Freq = 0;
    new_pwm_Duty = 0;
    ledcWrite(2, 0);
  }
}

void i2cscan_task()
{
  if (i2cscan_flag == 1)
  {

    Wire.begin(6, 7);
    lv_textarea_add_text(i2c_extarea, "Scanning...\n");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
      Wire1.begin(6, 7);
      Wire1.beginTransmission(address);
      error = Wire1.endTransmission();
      if (error == 0)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "I2C device address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);

        nDevices++;
      }
      else if (error == 4)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "Unknown error at address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);
      }
    }
    if (nDevices == 0)
      lv_textarea_add_text(i2c_extarea, "No I2C devices found");
    else
      lv_textarea_add_text(i2c_extarea, "done\n");

    i2cscan_flag = 0;
  }
}

void uart_helper_task()
{

  if (uart_helper_flag == 1)
  {

    char buf[10];
    lv_dropdown_get_selected_str(uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();
      Serial.begin(newBaudRate);
      currentBaudRate = newBaudRate;
    }
    if (Serial.available() > 0)
    {
      String input = Serial.readStringUntil('\n');
      lv_textarea_add_text(uart_extarea, input.c_str());
    }
  }
}

void DSO_task()
{

  if (DSO_flag == 1)
  {
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      adcValues[i] = analogRead(4);
      lv_chart_set_next_value(DSO_chart, DSO_ser, adcValues[i]);
    }

    float maxValue;
    float minValue = 4095;
    float peakToPeakValue;

    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      if (adcValues[i] > maxValue)
      {
        maxValue = adcValues[i];
      }
      if (adcValues[i] < minValue)
      {
        minValue = adcValues[i];
      }
    }
    maxValue = maxValue / 4095 * 3.3;
    minValue = minValue / 4095 * 3.3;

    peakToPeakValue = maxValue - minValue;

    sprintf(maxValueStr, "%0.2f", maxValue);
    sprintf(minValueStr, "%0.2f", minValue);
    sprintf(peakToPeakValueStr, "%0.2f", peakToPeakValue);
  }
}

void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
  static uint32_t last_key = 0;
  static uint32_t push_start_time = 0;
  static bool push_pressed = false;
  uint8_t key_state = 0;
  uint32_t act_key;

  if (digitalRead(RIGHT_PIN) == LOW)
  {
    key_state = 1;
  }
  else if (digitalRead(LEFT_PIN) == LOW)
  {
    key_state = 2;
  }
  else if (digitalRead(PUSH_PIN) == LOW)
  {
    if (!push_pressed)
    {
      push_pressed = true;
      push_start_time = millis();
    }
    key_state = 3;
  }
  else
  {
    key_state = 0;
    push_pressed = false;
  }

  if (key_state != 0)
  {
    data->state = LV_INDEV_STATE_PR;

    switch (key_state)
    {
    case 1:
      act_key = LV_KEY_NEXT;
      last_key = act_key;
      break;
    case 2:
      act_key = LV_KEY_PREV;
      last_key = act_key;
      break;
    case 3:
      if (push_pressed)
      {
        if (millis() - push_start_time >= LONG_PRESS_THRESHOLD)
        {
          lv_event_send(lv_scr_act(), LV_EVENT_LONG_PRESSED, NULL);
          last_key = act_key;
        }
        if (push_pressed && (millis() - push_start_time < LONG_PRESS_THRESHOLD))
        {
          act_key = LV_KEY_ENTER;
          last_key = act_key;
        }
      }
      break;
    default:
      break;
    }
    buzzer_task();
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }

  data->key = last_key;
}

void ina266_task()
{

  if (ina266_flag == 1)
  {
    vol = INA.getBusMilliVolts(deviceNumber);
    cur = INA.getBusMicroAmps(deviceNumber) / 4936;
    wat = INA.getBusMicroWatts(deviceNumber);

    if (cur > 20 * 1000)
    {
      cur = 0;
    }

    v = vol / 1000.0;
    a = cur / 100.0;
    w = v * a;

    if (v > 10)
    {
      sprintf(voltageStr, "%0.2f", v);
    }
    else
    {
      sprintf(voltageStr, "%0.3f", v);
    }

    if (a > 10)
    {
      sprintf(currentStr, "%0.2f", a);
    }
    else
    {
      sprintf(currentStr, "%0.3f", a);
    }

    if (w > 10)
    {
      sprintf(powerStr, "%0.2f", w);
    }
    else
    {
      sprintf(powerStr, "%0.3f", w);
    }

    mAH += cur;
    sprintf(mAHStr, "%0.3f", mAH / (60 * 60) / 1000.0);
  }
}

void BluetoothSerial_task()
{
  if (BluetoothSerial_flag == 1)
  {
    BLEDevice::init("Exlink");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    pServer->getAdvertising()->start();
    BluetoothSerial_flag = 2;
  }
  if (BluetoothSerial_flag == 2)
  {
    char buf[10];
    lv_dropdown_get_selected_str(wireless_uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();
      Serial.begin(newBaudRate);
      currentBaudRate = newBaudRate;
    }

    if (deviceConnected)
    {
      if (Serial.available() > 0)
      {
        String strValue = Serial.readStringUntil('\n');
        pTxCharacteristic->setValue((uint8_t *)strValue.c_str(), strValue.length());
        pTxCharacteristic->notify();
        lv_textarea_add_text(wireless_uart_extarea, strValue.c_str());
        delay(100);
      }
    }

    if (!deviceConnected && oldDeviceConnected)
    {
      delay(500);
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected)
    {
      oldDeviceConnected = deviceConnected;
    }
  }
}

void FREcount_task()
{
  if (FREcount_flag == 1)
  {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = 5,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DIS,
        .unit = PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config);
    pcnt_counter_clear(PCNT_UNIT);
    FREcount_flag = 2;
  }
  if (FREcount_flag == 2)
  {
    unsigned long currentMillis = millis();
    pcnt_get_counter_value(PCNT_UNIT, &count);
    unsigned long currentinterval = currentMillis - previousMillis;
    if (currentinterval >= interval)
    {

      sprintf(freqencyStr, "%d", count * 8);
      pcnt_counter_clear(PCNT_UNIT);
      previousMillis = currentMillis;
    }
  }
}

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

// ==========【重写 my_touchpad_read，TSC2007版本】==========
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t x, y, z1, z2;

  // read_touch() 返回 true 表示有触摸
  if (tsc.read_touch(&x, &y, &z1, &z2)) {
    data->state = LV_INDEV_STATE_PR;

    // ===== 坐标映射（根据你的屏幕旋转和接线调整）=====
    // 原 CST816T 的映射是：x = 320 - tp.y;  y = tp.x;
    // TSC2007 输出的是电阻屏原始 ADC 值，方向取决于触摸屏排线。
    // 下面给出一个初始映射（假设与 CST816T 方向类似，但需要实测校准）：

    // 先按屏幕分辨率做线性映射（TSC2007 典型 12 位 ADC，0~4095）
    // 如果你实测发现方向反了，就交换 x/y，或使用 320-x / 240-y。
    int16_t mapped_x = map(x, 0, 4095, 0, screenWidth);   // 或 320 - map(...)
    int16_t mapped_y =240- map(y, 0, 4095, 0, screenHeight);  // 或 240 - map(...)

    // 如果方向不对，可以启用下面的交换（类似原 CST816T 的旋转处理）：
    // data->point.x = screenWidth  - mapped_y;
    // data->point.y = mapped_x;

    data->point.x = mapped_x;
    data->point.y = mapped_y;

  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}





void tim1Interrupt()
{
  lv_tick_inc(1);
}

void lvgl_handler(void *pvParameters)
{
  while (1)
  {
    lv_timer_handler();
    delay(5);
  }
}

void setup()
{
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  Wire.begin(18, 21);
  mp28167_init();

  Serial.begin(115200);

  pinMode(3, OUTPUT);
  ledcAttachPin(3, 1);
  ledcSetup(1, 3000, 8);
  ledcWrite(1, 128);

  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(PUSH_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(2, ADC_11db);
  analogSetPinAttenuation(4, ADC_11db);
//delay(5000);
  tft.init();
  tft.setRotation(5);

  ledcWrite(1, 0);

  //cst816t.begin();   // 注释掉CST816T初始化
if (!tsc.begin()) {
  Serial.println("TSC2007 not found!");
  // 可根据需要 while(1); 或继续运行
} else {
  Serial.println("TSC2007 initialized");
}

// 放在 tsc.begin() 之前或之后，临时调试用
Serial.println("Scanning Wire (18/21)...");
for (uint8_t addr = 1; addr < 127; addr++) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    Serial.printf("Found: 0x%02X\n", addr);
  }
}
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 2);
  static lv_disp_drv_t disp_drv;

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 0;//关闭软件旋转
  disp_drv.rotated = LV_DISP_ROT_180;//LVGL `LV_DISP_ROT_0 / 90 / 180 / 270`
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv1;

  lv_indev_drv_init(&indev_drv1);
  indev_drv1.type = LV_INDEV_TYPE_POINTER;
  indev_drv1.read_cb = my_touchpad_read;   // ==========【取消注释，启用TSC2007触摸回调】==========
  lv_indev_drv_register(&indev_drv1);

  static lv_indev_drv_t indev_drv2;
  lv_indev_drv_init(&indev_drv2);
  indev_drv2.read_cb = keypad_read;
  indev_drv2.type = LV_INDEV_TYPE_KEYPAD;
  indev_keypad = lv_indev_drv_register(&indev_drv2);

  tim1 = timerBegin(0, 80, true);
  timerAttachInterrupt(tim1, tim1Interrupt, true);
  timerAlarmWrite(tim1, 1000, true);
  timerAlarmEnable(tim1);

  uint8_t devicesFound = 0;
  while (deviceNumber == UINT8_MAX)
  {
    devicesFound = INA.begin(10, 10000);
    Serial.println(INA.getDeviceName(devicesFound - 1));
    for (uint8_t i = 0; i < devicesFound; i++)
    {

      if (strcmp(INA.getDeviceName(i), "INA226") == 0)
      {
        deviceNumber = i;
        INA.reset(deviceNumber);

        break;
      }
    }
    if (deviceNumber == UINT8_MAX)
    {
      Serial.print(F("No INA found. Waiting 5s and retrying...\n"));
      delay(5000);
    }
  }
  Serial.print(F("Found INA at device number "));
  Serial.println(deviceNumber);
  Serial.println();
  INA.setAveraging(4, deviceNumber);
  INA.setBusConversion(8244, deviceNumber);
  INA.setShuntConversion(8244, deviceNumber);
  INA.setMode(INA_MODE_CONTINUOUS_BOTH, deviceNumber);

  Serial.println("ui_init start");

  pinMode(15, OUTPUT);
  ledcAttachPin(15, 7);
  ledcSetup(7, 5000, 8);
  ledcWrite(7, 155);

  ui_init();
}

void loop()
{
  lv_task_handler();
  ina266_task();
  mp28167_task();
  pwm_task();
  i2cscan_task();
  uart_helper_task();
  DSO_task();
  BluetoothSerial_task();
  FREcount_task();
  //Serial.println("loop。。。。。。。。。。。");
}
