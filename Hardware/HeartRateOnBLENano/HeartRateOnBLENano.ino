#include <BLE_API.h>
#include "simple_uart.h"

#define HRM_TIME APP_TIMER_TICKS(1000, 0)
#define BPM_TIME APP_TIMER_TICKS(10, 0)
#define ARRAY_SIZE 8
#define DEAD_MAX_HR 200
#define DEAD_MIN_HR 10
//#define FINGER_CUTOFF 560
//#define WRIST_CUTOFF 540

boolean abc = true;
boolean sensorOn = false;

//Define BLE
BLEDevice  ble;
const static char  DEVICE_NAME[] = "SurWirer";

//HRV Calculation
const int analogInPin = A5;
const int sensorPowerPin = 7;
int sensorValueNew = 0;        // value read from the pot
int sensorValueOld = 0;
unsigned long pulseNew = 0;
unsigned long pulseOld = 0;
unsigned long duration = 0;
int heartRate = 0;
int hrCount = 0;
int hrArray[ARRAY_SIZE];
int currentHR = 0;
int CUTOFF = 600;
uint8_t FINGER_CUTOFF = 56;
uint8_t WRIST_CUTOFF = 54;


//for HRM Service use
static uint8_t hrmCounter = 0;
static uint8_t bpm[2] = {0x00, hrmCounter};
static uint8_t location = 0x02; //0x02/0x03

static app_timer_id_t m_hrs_timer_id;
static app_timer_id_t m_bpm_timer_id;

static const uint8_t hrmFingerCutoff_uuid[] = {0x71, 0x3D, 0, 3, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t hrmWristCutoff_uuid[] = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};

//GattService:HeartRateMeasurement
GattCharacteristic hrmRate(GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR, bpm, sizeof(bpm), sizeof(bpm), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic hrmLocation(GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR,&location, sizeof(location), sizeof(location), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
GattCharacteristic hrmFingerCutoff(hrmFingerCutoff_uuid,&FINGER_CUTOFF, sizeof(FINGER_CUTOFF), sizeof(FINGER_CUTOFF), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
GattCharacteristic hrmWristCutoff(hrmWristCutoff_uuid,&WRIST_CUTOFF, sizeof(WRIST_CUTOFF), sizeof(WRIST_CUTOFF), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

GattCharacteristic *hrmChars[] = {&hrmRate, &hrmLocation, &hrmFingerCutoff, &hrmWristCutoff};
GattService        hrmService(GattService::UUID_HEART_RATE_SERVICE, hrmChars, sizeof(hrmChars) / sizeof(GattCharacteristic *));

//All GattService
static const uint16_t uuid16_list[] = {GattService::UUID_HEART_RATE_SERVICE};


void disconnectionCallback(void) {
    if(abc==1) Serial.println("Disconnected!");
    if(abc==1) Serial.println("Restarting the advertising process");
    sensorOn = false;
    digitalWrite(sensorPowerPin, LOW);
    hrmCounter = 0;
    bpm[1] = hrmCounter;
    ble.startAdvertising();
}

void periodicCallback( void * p_context ) {
    if (ble.getGapState().connected) {
        /* Update the HRM measurement */
        /* First byte = 8-bit values, no extra info, Second byte = uint8_t HRM value */
        /* See --> https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml */
        //hrmCounter++;
        uint16_t bytesRead0;
        uint8_t buf0[1];
        ble.readCharacteristicValue(hrmLocation.getHandle(), buf0, &bytesRead0);
        location = buf0[0];

        uint16_t bytesRead1;
        uint8_t buf1[1];
        ble.readCharacteristicValue(hrmFingerCutoff.getHandle(), buf1, &bytesRead1);
        FINGER_CUTOFF = (int)buf1[0];

        uint16_t bytesRead2;
        uint8_t buf2[1];
        ble.readCharacteristicValue(hrmWristCutoff.getHandle(), buf2, &bytesRead2);
        WRIST_CUTOFF = (int)buf2[0];

        if (location == 0x02)
            CUTOFF = FINGER_CUTOFF * 10; 
        else if (location == 0x03)
            CUTOFF = WRIST_CUTOFF * 10;
        bpm[1] = abs(hrmCounter);
        ble.updateCharacteristicValue(hrmRate.getHandle(), bpm, sizeof(bpm));
    }
}

void periodicBPMCallback( void * p_context ) {
    if (ble.getGapState().connected) {
        if (!sensorOn) {
            digitalWrite(sensorPowerPin, HIGH);
            sensorOn = true;
        }
        sensorValueOld = sensorValueNew;
        sensorValueNew = analogRead(analogInPin);
        if ((sensorValueNew >= CUTOFF)&&(sensorValueOld < CUTOFF)) {
            pulseOld = pulseNew;
            pulseNew = millis();
            duration = pulseNew - pulseOld;
            heartRate = 60000/duration;
            if ((heartRate <= DEAD_MAX_HR)&&(heartRate > DEAD_MIN_HR)) {
                for (int i=0; i < ARRAY_SIZE - 1; i++)
                    hrArray[i] = hrArray[i+1];
                hrArray[ARRAY_SIZE-1] = abs(heartRate);
                if(hrCount<ARRAY_SIZE) {
                    if(abc==1) Serial.println("Waiting for a more accurate result!");
                    //hrmCounter = -1;
                    hrCount++;
                } else {
                    //Serial.print("D");
                    //Serial.println((float)duration/1000,4);
                    if(abc==1) Serial.print("HR");
                    currentHR = trimMean(hrArray,5);
                    if(abc==1) Serial.println(currentHR);
                    hrmCounter = currentHR;
                    if(abc==1) Serial.println("----------");
                }
            } else {
                if(abc==1) Serial.println("Waiting for a more accurate result!");
                //hrmCounter = -1;
                if(hrCount>0) hrCount--;
            }
        }
        if(abc==1) Serial.print("V");
        if(abc==1) Serial.print(sensorValueNew);
        if(abc==1) Serial.print("-");
    }
}

int * combSort(int ar[ARRAY_SIZE]) {
  int i, j;
  int gap;
  int swapped = 1;
  int temp;

  gap = ARRAY_SIZE;
  while (gap > 1 || swapped == 1) {
    if (gap > 1) {
      gap = gap * 10/13;
      if (gap == 9 || gap == 10) gap = 11;
    }
    swapped = 0;
    for (i = 0, j = gap; j < ARRAY_SIZE; i++, j++) {
      if (ar[i] > ar[j]) {
        temp = ar[i];
        ar[i] = ar[j];
        ar[j] = temp;
        swapped = 1;
      }
    }
  }
  return ar;
}

float trimMean(int ar[ARRAY_SIZE], int p) {
    int k = 0;//ARRAY_SIZE*p/100;
    ar = combSort(ar);
    int trimTotal = 0;
    for (int i = k;i < ARRAY_SIZE - k;i++)
        trimTotal += ar[i];
    return (float)trimTotal/(ARRAY_SIZE-2*k);
}

void setup(void) {    
    uint32_t err_code;  
    pinMode(sensorPowerPin, OUTPUT);
  
    Serial.begin(115200);
    if(abc==1) Serial.println("Initialising the nRF51822");
    ble.init();
    
    err_code = app_timer_create(&m_hrs_timer_id, APP_TIMER_MODE_REPEATED, periodicCallback );
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_bpm_timer_id, APP_TIMER_MODE_REPEATED, periodicBPMCallback );
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_hrs_timer_id, HRM_TIME, NULL);
    APP_ERROR_CHECK(err_code);	

    err_code = app_timer_start(m_bpm_timer_id, BPM_TIME, NULL);
    APP_ERROR_CHECK(err_code);  
    
    ble.onDisconnection(disconnectionCallback);
    
    if(abc==1) Serial.println("setup advertising");
    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t*)uuid16_list, sizeof(uuid16_list));
    ble.accumulateAdvertisingPayload(GapAdvertisingData::HEART_RATE_SENSOR_HEART_RATE_BELT);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    
    ble.addService(hrmService);

    err_code = RBL_SetDevName(DEVICE_NAME);
    APP_ERROR_CHECK(err_code);
    
    ble.startAdvertising();
    
    if(abc==1) Serial.println("start advertising");
}

void loop(void) {
    ble.waitForEvent();
}








