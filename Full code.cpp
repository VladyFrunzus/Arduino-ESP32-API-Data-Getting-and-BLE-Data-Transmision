#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fccc5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-
b7f5-ea07361b26a8"
const char* ssid = "placeholder_ssid";
const char* password = "placeholder_password";
bool deviceConnected = false;
BLECharacteristic indexCharacteristic(
 "bc4196e1-05fd-4bcc-bfa7-7e2e00d03bba",
 BLECharacteristic::PROPERTY_READ |
BLECharacteristic::PROPERTY_WRITE |
BLECharacteristic::PROPERTY_NOTIFY
);
BLECharacteristic detailsCharacteristic(
 "385e7304-06a1-4235-ba49-d084db02fca9",
 BLECharacteristic::PROPERTY_READ |
BLECharacteristic::PROPERTY_WRITE |
BLECharacteristic::PROPERTY_NOTIFY
);
BLEDescriptor *indexDescriptor = new
BLEDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor *detailsDescriptor = new
BLEDescriptor(BLEUUID((uint16_t)0x2902));
class MyServerCallbacks: public BLEServerCallbacks {
 void onConnect(BLEServer* pServer) {
 deviceConnected = true;
 Serial.println("Device connected");
 };
 void onDisconnect(BLEServer* pServer) {
 deviceConnected = false;
 Serial.println("Device disconnected");
 }
};
class Constant{
public:
String bleServerName = "Marvel Movies";
String apiList = "http://proiectia.bogdanflorea.ro/api/marvelcinematic-universe-movies/movies";
String apiFetch =
"http://proiectia.bogdanflorea.ro/api/marvel-cinematicuniverse-movies/movie/";
String idProperty= "id";
String titleProperty = "title";
String coverProperty = "cover_url";
String durationProperty= "duration";
String releaseProperty = "release_date";
String directorProperty = "directed_by";
String sagaProperty = "saga";
String chronologyProperty = "chronology";
};
Constant *constant = new Constant();
class Utils{
public:
static DynamicJsonDocument fetchJson(String url){
DynamicJsonDocument doc(8096);
HTTPClient http;
String response;
http.setTimeout(15000);
http.begin(url);
http.GET();
response = http.getString();
deserializeJson(doc, response);
return doc;
}
static DynamicJsonDocument
createListDocumentJson(JsonObject object){
DynamicJsonDocument listDocumentJson(8096);
listDocumentJson["id"] = object[constant->idProperty];
listDocumentJson["name"] = object[constant-
>titleProperty];
listDocumentJson["image"] = object[constant-
>coverProperty];
return listDocumentJson;
}
static DynamicJsonDocument
createDetailsDocumentJson(DynamicJsonDocument doc){
DynamicJsonDocument detailsDocumentJson(8096);
detailsDocumentJson["id"] = doc[constant->idProperty];
detailsDocumentJson["name"] = doc[constant-
>titleProperty];
detailsDocumentJson["image"] = doc[constant-
>coverProperty];
detailsDocumentJson["description"] = String("Release date:
") + doc[constant->releaseProperty].as <String>() +
String("\nDuration: ") + doc[constant->durationProperty].as
<String>() + String("\nDirected by: ") +
doc[constant->directorProperty].as<String>() +
String("\nSaga: ") + doc[constant->sagaProperty].as
<String>() +
String("\nChronology: ") + doc[constant-
>chronologyProperty].as <String>();
return detailsDocumentJson;
}
static void sendJson(DynamicJsonDocument doc,
BLECharacteristic *characteristic){
String returned;
serializeJson(doc, returned);
characteristic->setValue(returned.c_str());
characteristic->notify();
} };
class CharacteristicsCallbacks: public
BLECharacteristicCallbacks
{
void onWrite(BLECharacteristic *characteristic)
{
DynamicJsonDocument appRequest(8096);
deserializeJson(appRequest, characteristic-
>getValue().c_str());
if(appRequest["action"] == "fetchData")
 {DynamicJsonDocument webJSON =
Utils::fetchJson(constant->apiList);

for(JsonObject object : webJSON.as<JsonArray>())
 {DynamicJsonDocument returnJSON =
Utils::createListDocumentJson(object);
 Utils::sendJson(returnJSON, characteristic);
 }
 }else if(appRequest["action"] == "fetchDetails")
 {DynamicJsonDocument webJSON =
Utils::fetchJson(constant->apiFetch +
appRequest["id"].as<String>());
 DynamicJsonDocument returnJSON =
Utils::createDetailsDocumentJson(webJSON);
 Utils::sendJson(returnJSON, characteristic);
 }
}
};
void setup() {
 Serial.begin(115200);
 delay(5000);
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);
 Serial.print("Connecting to WiFi.");
 while (WiFi.status() != WL_CONNECTED)
 {
 delay(1500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.print("WiFi connected with IP: ");
 Serial.println(WiFi.localIP());
 BLEDevice::init("Marvel Movies");
 BLEServer *pServer = BLEDevice::createServer();
 pServer->setCallbacks(new MyServerCallbacks());
 BLEService *bmeService = pServer-
>createService(SERVICE_UUID);
 bmeService->addCharacteristic(&indexCharacteristic);
 indexDescriptor->setValue("Get data list");
 indexCharacteristic.addDescriptor(indexDescriptor);
 indexCharacteristic.setValue("Get data List");
 indexCharacteristic.setCallbacks(new
CharacteristicsCallbacks());

bmeService->addCharacteristic(&detailsCharacteristic);
 detailsDescriptor->setValue("Get data details");
 detailsCharacteristic.addDescriptor(detailsDescriptor);
 detailsCharacteristic.setValue("Get data details");
 detailsCharacteristic.setCallbacks(new
CharacteristicsCallbacks());
 bmeService->start();
 BLEAdvertising *pAdvertising =
BLEDevice::getAdvertising();
 pAdvertising->addServiceUUID(SERVICE_UUID);
 pAdvertising->setScanResponse(true);
 pAdvertising->setMinPreferred(0x06);
 pAdvertising->setMinPreferred(0x12);
 pServer->getAdvertising()->start();
 Serial.println("Bluetooth Low Energy is ready to use!");
}
void loop() {}
