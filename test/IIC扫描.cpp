// Wire Master Writer
// by devyte
// based on the example of the same name by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// This example code is in the public domain.


#include <Wire.h>
#include <WiFi.h> 
#include <Arduino.h>

const int16_t I2C_MASTER = 0x42;
uint8_t I2C_SLAVE = 0x08;

void setup() {
  Serial.begin(115200);           // start serial for output
//   Wire.begin();
  Serial.println("\nI2C Scanner");
  Wire.begin(16, 15); 
}

void loop(){
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++ ){
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }else if (error == 4){
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  delay(1000); // wait 5 seconds for next scan
}