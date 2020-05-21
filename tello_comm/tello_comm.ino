#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

const char* WIFI_ID = "TELLO-WP";
const char* WIFI_PASS = NULL;

const char* TELLO_IP = "192.168.10.1";
const int TELLO_PORT = 8889;
const int TELLO_STATE_PORT = 8890;
  
WiFiUDP udpCommand;
WiFiUDP udpStatus;


class TelloStatus{
  public: 
    byte battery = 0;
    int temperature = 0;
    int velX = 0;
    int velY = 0;
    int velZ = 0;
    int height = 0;
};

void setup() {
  Serial.begin(74880);

  WiFi.hostname("WeMos");
  WiFi.begin();

  // Wait until we're connected.
  Serial.print("Connecting to WiFi: ");
  Serial.print(WIFI_ID);
  WiFi.begin(WIFI_ID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" connected");
  Serial.println(WiFi.localIP());

  Serial.print("Listening status on port ");
  Serial.println(TELLO_STATE_PORT);
  udpStatus.begin(TELLO_STATE_PORT);

  sendCommand("command");
  delay(1000);
}

void sendNonVerboseCommand(char *cmd){
  udpCommand.beginPacket(TELLO_IP, TELLO_PORT);
  udpCommand.write(cmd);
  udpCommand.endPacket();
}

void sendCommand(char *cmd){
  Serial.print("Sending '");
  Serial.print(cmd);
  Serial.println("'");
  sendNonVerboseCommand(cmd);
}

// pitch:1;roll:1;yaw:20;vgx:0;vgy:0;vgz:0;templ:74;temph:77;tof:10;h:0;bat:61;baro:38.82;time:43;agx:10.00;agy:-31.00;agz:-1001.00;
TelloStatus *parseStatus(char *statusData){
  int i = 0;
  int lastSemicolonPos = -1;
  int colonPos = 0;
  TelloStatus *telloStatus = new TelloStatus;
  char * currentPart;
  while(statusData[i] != '\0'){
    if (statusData[i] == ':'){
      colonPos = i;
    } else if (statusData[i] == ';'){
      int keySize = colonPos - lastSemicolonPos;
      char key[keySize + 1];
      for (int j=0; j<keySize; j++){
        key[j] = statusData[lastSemicolonPos + j + 1];
      }
      key[keySize - 1] = '\0';

      int valueSize = i - colonPos;
      char value[valueSize + 1];
      for (int j=0; j<valueSize; j++){
        value[j] = statusData[colonPos + j + 1];
      }
      value[valueSize - 1] = '\0';

      if (strcmp(key, "bat") == 0){
        telloStatus->battery = atoi(value);
      } else if (strcmp(key, "temph") == 0){
        telloStatus->temperature = atoi(value);
      } else if (strcmp(key, "vgx") == 0){
        telloStatus->velX = atoi(value);
      } else if (strcmp(key, "vgy") == 0){
        telloStatus->velY = atoi(value);
      } else if (strcmp(key, "vgz") == 0){
        telloStatus->velZ = atoi(value);
      } else if (strcmp(key, "h") == 0){
        telloStatus->height = atoi(value);
      }
      lastSemicolonPos = i;
    }
    i++;
  }
  return telloStatus;
}

TelloStatus * readStatus(){
  int statusPacketSize = udpStatus.parsePacket();
  if (statusPacketSize > 0){
      char reply[statusPacketSize + 1];
      udpStatus.read(reply, statusPacketSize);
      reply[statusPacketSize] = '\0';
//      Serial.println(reply);
      return parseStatus(reply);
  }
  return NULL;
}

int channelValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = channelValues[channelInput - 1];
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

bool redSwitch(byte channelInput, int defaultValue){
  int ch = readChannel(channelInput, 0, 100, defaultValue);
  return (ch < 50);
}

int red3StepsSwitch(byte channelInput, int defaultValue){
  int ch = readChannel(channelInput, 0, 100, defaultValue);
  if (ch < 33) return 0;
  if (ch < 66) return 1;
  return 2;
}

// a = left/right (-100, 100)
// b = forward / backward (-100, 100)
// c = up / down (-100, 100)
// d = yaw (-100, 100)
void sendRcCommandGimbal(){
  char cmd[100];
  int a = readChannel(1, -100, 100, 0);
  int b = readChannel(2, -100, 100, 0);
  int c = readChannel(3, -100, 100, 0);
  int d = readChannel(4, -100, 100, 0);
  sprintf(cmd, "rc %d %d %d %d", a, b, c, d);
  sendNonVerboseCommand(cmd);
}

TelloStatus* currentStatus = new TelloStatus();
bool isFlying = false;
unsigned long takeOffTime = 0;
int loopCounter = 0;
String serialComBuffer;
void loop() {
  while (Serial.available()) {
    serialComBuffer = Serial.readStringUntil('\n');
    if (serialComBuffer.startsWith(">")){
      int splitPos = serialComBuffer.indexOf(":");
      int channelIndex = serialComBuffer.substring(1, splitPos).toInt();
      int channelValue = serialComBuffer.substring(splitPos + 1).toInt();
      channelValues[channelIndex] = channelValue;
    }
//    Serial.print("Ch: ");
//    for (int i=0; i<10; i++){
//      Serial.print(i);
//      Serial.print(": ");
//      Serial.print(channelValues[i]);
//      Serial.print(", ");
//    }
//    Serial.println("");
  }
  
  if (loopCounter == 0){
    TelloStatus *newTelloStatus = readStatus();
    if (newTelloStatus != NULL){
      if (newTelloStatus->battery != currentStatus->battery){
        Serial.print(">bat:");
        Serial.println(currentStatus->battery);
      }
//      if (newTelloStatus->temperature != currentStatus->temperature){
//        Serial.print(">temp:");
//        Serial.println(currentStatus->temperature);
//      }
      delete(currentStatus);
      currentStatus = newTelloStatus;
    }

//    long dBm = WiFi.RSSI();
//    int quality = int(100.0 * (1.0 - (-20.0 - dBm) / (-20.0 - -100.0)));
//    Serial.print(">signal:");
//    Serial.println(quality);
  }
  loopCounter = (loopCounter + 1) % 1000;
  
  bool takeoff = !redSwitch(5, 0);
  if (!isFlying && takeoff){
    sendCommand("takeoff");
    takeOffTime = millis();
    isFlying = true;
  }
  if (isFlying && !takeoff){
    sendCommand("land");
    sendNonVerboseCommand("rc 0 0 0 0");
    isFlying = false;
  }
  if (currentStatus->height > 0){
    sendRcCommandGimbal();
  }
}
