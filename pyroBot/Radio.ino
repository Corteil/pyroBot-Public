#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

// Choose one of:
//#define  RADIO_ONLY_GZLL
//#define  RADIO_ONLY_BLE
#define RADIO_TOGGLE
//#define RADIO_NONE


#define GZLL_MAX_MSG_SIZE 32
char  gzllDebugBuf[GZLL_MAX_MSG_SIZE] = {0};
char* gzllDebug=NULL;


#define BLE_UUID                   "7e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_ADVERTISEMENT_DATA_MAX 16
#define BLE_TX_POWER_LEVEL         0

#define BLE_MAX_MSG_SIZE 20

#define TOGGLE_MILLIS                   1500
#define TOGGLE_GZLL_CONNECTION_TIMEOUT   250

void radio_setup() {

#if defined(RADIO_ONLY_GZLL)
  setup_gzll();
#elif defined(RADIO_ONLY_BLE)
  setup_ble();
#elif defined(RADIO_TOGGLE)
  // Setup of BLE & GZLL is handled in radio_loop()
#elif defined(RADIO_NONE)
  // no radio chosen
#else
#error Invalid Radio config, choose: RADIO_ONLY_GZLL, RADIO_ONLY_BLE, RADIO_TOGGLE or RADIO_NONE
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// Radio toggling

#ifdef RADIO_TOGGLE
volatile bool startGZLL = true;
volatile bool bleConnected = false;
volatile unsigned long nextRadioToggleTime = millis();
#endif

volatile bool gzllConnected = false;
volatile unsigned long gzllConnectionTimeout = millis();

volatile unsigned long timeNow = millis();                 // the time at the start of the loop(), use for the 'radio' part

void radio_loop() {
    timeNow = millis(); 
#if defined(RADIO_ONLY_GZLL) || defined(RADIO_TOGGLE)
  loop_gzll();
#endif  

#ifdef RADIO_TOGGLE
  if (!bleConnected && !gzllConnected) {
    if (millis() > nextRadioToggleTime) {
#ifdef DEBUG
      Serial.println("RADIO_TOGGLE");
#endif //DEBUG
      nextRadioToggleTime = millis() + TOGGLE_MILLIS;

      if (startGZLL) {
        while (RFduinoBLE.radioActive);
        RFduinoBLE.end();
        delay(50);
        setup_gzll();
      } else  {
        RFduinoGZLL.end();
        delay(50);
        setup_ble();
      }
      startGZLL   = !startGZLL;
    }
  }
#endif // RADIO_TOGGLE

}



////////////////////////////////////////////////////////////////////////////////////////
// GZLL


void setup_gzll() {
  RFduinoGZLL.hostBaseAddress = GZLL_HOST_ADDRESS;
  RFduinoGZLL.begin(HOST);
}

void loop_gzll() {
  // simulate a GZLL disconnect event
  if ( gzllConnected && (timeNow  > gzllConnectionTimeout)) {
    gzllConnected = false;
    client_disconnected();
  }
}

void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len) {
  gzllConnected = true;
  gzllConnectionTimeout = timeNow + TOGGLE_GZLL_CONNECTION_TIMEOUT;
  process_message(data, len);
  if (gzllDebug) {
    RFduinoGZLL.sendToDevice(device, gzllDebugBuf, strlen(gzllDebugBuf));
    gzllDebug=NULL;   
  } 
}

////////////////////////////////////////////////////////////////////////////////////////
// BLE

char bleName[BLE_ADVERTISEMENT_DATA_MAX] = {0};

void setup_ble() {
  snprintf(bleName, BLE_ADVERTISEMENT_DATA_MAX, "CB_%x%x", getDeviceIdHigh, getDeviceIdLow());
  RFduinoBLE.txPowerLevel      = BLE_TX_POWER_LEVEL;
  RFduinoBLE.customUUID        = BLE_UUID;
  RFduinoBLE.deviceName        = bleName;
  RFduinoBLE.advertisementData = bleName;
  RFduinoBLE.begin();
  //RFduinoBLE_update_conn_interval(20, 20);
}


void RFduinoBLE_onReceive(char *data, int len) {
  process_message(data, len);
}

void RFduinoBLE_onConnect() {
#ifdef RADIO_TOGGLE
  bleConnected = true;
#endif
#ifdef DEBUG
  Serial.println("BLE_ISCON");
#endif
}

void RFduinoBLE_onDisconnect() {
#ifdef RADIO_TOGGLE
  bleConnected = false;
#endif
  client_disconnected();
}



////////////////////////////////////////////////////////////////////////////////////////
// BLE/GZLL shared message processing

// We're expecting messages of 3 bytes in the form:  XYB
// Where;
// X = unsigned byte for xAxis:          0 .. 255 mapped to -254 .. 254
// Y = unsigned byte for yAxis:          0 .. 255 mapped to -254 .. 254
// B = unsigned byte for button pressed: 0 = no, 1 = yes

void process_message(char *data, int len) {
  if (len >= 3) {
    // map x&y values from 0..255 to -255..255
    joypad_update(
      map(data[0], 0, 255, -255, 255),   // x axis
      map(data[1], 0, 255, -255, 255),   // y axis
      data[2]                            // button
    );
  }
}

// tidyup helper for when GZLL connection timesout or BLE client disconnects
void client_disconnected() {
  joypad_update(0, 0, 0);
}


void radio_debug(char* msg) {
    if (!gzllDebug && gzllConnected) {
     snprintf(gzllDebugBuf, GZLL_MAX_MSG_SIZE-1, msg);
     gzllDebug = gzllDebugBuf;
    } 
    
    if (bleConnected) {
      RFduinoBLE.send(msg, min(BLE_MAX_MSG_SIZE, strlen(msg)-1));
    }
}

