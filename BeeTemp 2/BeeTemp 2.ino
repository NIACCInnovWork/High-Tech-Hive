
/*
 * High Tech Hive Bee Temperature Monitor and Logger
 */

// Temperature Sensor Libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// SD Card Libraries
#include <SPI.h>
#include <SD.h>

// RTC LIbrary
#include <RTClib.h>

#define ONE_WIRE_BUS 5 // Data wire is plugged into digital pin 5 on the Feather
#define chipSelect 10   // Data Pin for SD Card

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Create File for Data on SD Card
File tempData;
//File dataFile;

// Create Device Count Variable
int device_count;

// Create SensorMap
uint8_t sensor0_id[8] = { 0x28, 0x97, 0x5B, 0x0E, 0x00, 0x00, 0x00, 0xDA}; // Sensor Located on Board 1, Sensor 0
uint8_t sensor1_id[8] = { 0x28, 0xBC, 0x2B, 0x10, 0x00, 0x00, 0x00, 0xD4}; // Sensor Located on Board 1, Sensor 1
uint8_t sensor2_id[8] = { 0x28, 0x8B, 0x8F, 0x0F, 0x00, 0x00, 0x00, 0x32}; // Sensor Located on Board 1, Sensor 2

uint8_t sensor3_id[8] = { 0x28, 0xE9, 0xE8, 0x0E, 0x00, 0x00, 0x00, 0x50}; // Sensor Located on Board 2, Sensor 0
uint8_t sensor4_id[8] = { 0x28, 0x2B, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x07}; // Sensor Located on Board 2, Sensor 1
uint8_t sensor5_id[8] = { 0x28, 0x2E, 0x1A, 0x10, 0x00, 0x00, 0x00, 0x42}; // Sensor Located on Board 2, Sensor 2

uint8_t sensor6_id[8] = { 0x28, 0x5F, 0xCC, 0x0F, 0x00, 0x00, 0x00, 0x8D}; // Sensor Located on Board 4, Sensor 0
uint8_t sensor7_id[8] = { 0x28, 0xCD, 0x5A, 0x0F, 0x00, 0x00, 0x00, 0x79}; // Sensor Located on Board 4, Sensor 1
uint8_t sensor8_id[8] = { 0x28, 0x04, 0x4D, 0x0F, 0x00, 0x00, 0x00, 0x5D}; // Sensor Located on Board 4, Sensor 2

uint8_t *sensor_ids[] = { sensor0_id, sensor1_id, sensor2_id, 
                          sensor3_id, sensor4_id, sensor5_id, 
                          sensor6_id, sensor7_id, sensor8_id, 
                          };

void setup(void){
  sensors.begin();  // Start up the library
  Serial.begin(9600);

  device_count = sensors.getDeviceCount();

  Serial.println("initializiong SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);  
  }
  Serial.println("initialization done."); 

  write_to_file(compose_headers(device_count));
}

void loop(){
  logHiveData();
  delay(15000);
}

// LogHiveData Function
void logHiveData(){
  digitalWrite(8, HIGH);
  String timeStamp = String(millis());
  String data = read_data();
  write_to_file(timeStamp + ", " + data);
  digitalWrite(8, LOW);
}

// ReadData Function
String read_data(){
  sensors.requestTemperatures();
  String data = "";
  for (int i = 0; i < device_count; i++){
    double sensor_reading = Farenhight(sensors.getTempC(sensor_ids[i]));
    data += String(sensor_reading) + ", ";
  }
  return data;
}

// WriteToFile Function
void write_to_file(String data) {
  tempData = SD.open("test.txt", FILE_WRITE);
  Serial.println("Opening File: " + String(tempData));
  // dataFile returns null=false if unable to open
  if (tempData) {
    Serial.println("File Opened, Writing Data");
    tempData.println(data);
    tempData.close();
  }
  else{
    Serial.println("SD Card Error");
  }
}

String compose_headers(int count){
  String text = "Timestamp, ";
  for (int i = 0; i < count; i++){
    text += ("Sensor " + String(i) + ", ");
  }
  return text;
}

double Farenhight(double temp){
  return ((temp* 9.0) / 5.0 + 32.0);
}
