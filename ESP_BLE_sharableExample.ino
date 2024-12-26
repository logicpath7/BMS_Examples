// Bellingham Makerspace example by Jim
//
// ESP_BLE_sharableExample.ino
// This is a bare bones example Arduino Sketch.
// It connects an ESP32 to an external device via Bluetooth Low Energy (BLE).
// For a tutorial on BLE, go to https://randomnerdtutorials.com/esp32-ble-server-client/
//
// This example has been tested using a computer with a Chrome browser as an external device.
// It should work with other device apps as well.
// Note: No guarantees that this is the easiest, best, or the correct way to do this!
// See companion HTML file for external device connection: ESP_BLE_sharableExample.html


// Bluetooth functionality libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// You should get your own numbers to avoid device conflicts
// Needs to match connected device UUIDs!
#define SERVICE_UUID               "19b10000-e8f2-537e-4f6c-d104768a1214"
#define TEXT_CHARACTERISTIC_UUID   "19b10003-e8f2-537e-4f6c-d104768a1214"

// Global variables:
// flags
bool deviceConnected = false;
bool oldDeviceConnected = false;

BLEServer* pServer = NULL;
BLECharacteristic* pTextCharacteristic = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");  // not required
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");  // not required
  }
};

class textCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pTextCharacteristic) {
    String txt = pTextCharacteristic->getValue();
    if (txt.length() > 0) {
      Serial.print("Got from BLE: "); Serial.println(txt);     // not required

      // Put your event function calls here  
            
    }
  }
};

void initializeBLE(){
    // Create the BLE Device
  BLEDevice::init("ESP32 device");  // Rename devise as you wish 

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTextCharacteristic = pService->createCharacteristic(
                      TEXT_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Register the callback for the TEXT characteristic
  pTextCharacteristic->setCallbacks(new textCharacteristicCallbacks());  

  // Create a BLE Descriptor, the CCCD
  pTextCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  Serial.println("Waiting for a client connection to notify...");
  delay(2000); //??

  
} 


void SendToBrowser(String dataout){
  // Send string to external Bluetooth device
  if (deviceConnected) {
    if (dataout != ""){     
      Serial.println("BLE SendToBrowser: " + dataout);
      pTextCharacteristic->setValue(dataout); 
      pTextCharacteristic->notify();      
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");    // not required
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Advertising...");    // not required
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");    // not required
  }
}

void setup() {

  Serial.begin(115200);    // not required if no USB interface needed
  initializeBLE();
  

}
 
void loop() {


}
