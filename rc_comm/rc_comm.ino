#include <IBusBM.h>

IBusBM ibusRc;
IBusBM ibusFeedback;

HardwareSerial& debugSerial = Serial;
HardwareSerial& ibusRcSerial = Serial1;
HardwareSerial& ibusFeedbackSerial = Serial2;
HardwareSerial& wifiComSerial = Serial3;

const byte NUM_CHANNELS = 10;

void setup() {
  debugSerial.begin(74880);
  wifiComSerial.begin(74880);

  ibusRc.begin(ibusRcSerial);

  ibusFeedback.begin(ibusFeedbackSerial);
  ibusFeedback.addSensor(IBUSS_EXTV);
  ibusFeedback.addSensor(IBUSS_TEMP);
  
  ibusFeedback.setSensorMeasurement(1, 0);
  ibusFeedback.setSensorMeasurement(2, 600);

  debugSerial.println("Starting Tello RC");
}

void sendChannelToWifi(uint8_t channel, uint16_t value){
  wifiComSerial.print(">");
  wifiComSerial.print(channel);
  wifiComSerial.print(":");
  wifiComSerial.println(value);
}

void loop() {
//  debugSerial.print("Arduino Ch: ");
  for (uint8_t i = 0; i<NUM_CHANNELS; i++){
    uint16_t value = ibusRc.readChannel(i);
    if (value > 0) sendChannelToWifi(i, value);
//    debugSerial.print(i);
//    debugSerial.print(": ");
//    debugSerial.print(value);
//    debugSerial.print(", ");
  }
//  debugSerial.println("");
  delay(5);
} 

void serialEvent3() {
  while (Serial3.available()) {
    String wifiComBuffer = Serial3.readStringUntil('\n');
    if (wifiComBuffer.startsWith(">bat:")){
      uint16_t bat = wifiComBuffer.substring(5).toInt() * 100;
      ibusFeedback.setSensorMeasurement(1, bat);
      Serial.print("Battery: ");
      Serial.println(bat);
    } else if (wifiComBuffer.startsWith(">temp:")){
      uint16_t temperature = wifiComBuffer.substring(6).toInt();
      int relativeTemp = (temperature * 10) + 400;
      ibusFeedback.setSensorMeasurement(2, relativeTemp);
      Serial.print("Temperature: ");
      Serial.println(temperature);
    } else {
      debugSerial.println(wifiComBuffer);
    }
  }
}
