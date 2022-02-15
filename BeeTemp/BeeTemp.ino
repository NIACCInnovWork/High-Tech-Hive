
/*
 * High Tech Hive Bee Temperature Monitor and Logger
 */

// Temperature Sensor Libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// SD Card Libraries
#include <SPI.h>
#include <SD.h>

#define ONE_WIRE_BUS A1 // Data wire is plugged into digital pin 2 on the Arduino
#define chipSelect 10   // Data Pin for SD Card

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

void setup(void)
{
  sensors.begin();  // Start up the library
  Serial.begin(9600);
  Serial.print(sensors.getDeviceCount());
}

void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();
  Serial.println(sensors.getDeviceCount());
  
  for(int i=0;i<sensors.getDeviceCount();i++){
      print_sensor(i); 
  }
  
  
  delay(500);
}

double Farenhight(double temp){
  return ((temp* 9.0) / 5.0 + 32.0);
}

void print_sensor(uint8_t index){
  double temp = sensors.getTempCByIndex(index);
  //Serial.print("Temperature:"+String(index)+ "Sensor:");
  DeviceAddress address;
  if (sensors.getAddress(address,index)){
//    if(temp>30.0){
//          printAddress(address);
//          Serial.println();
//          Serial.println(temp);

//    }
    //Serial.print(String(index)+":");
    //Serial.print(temp);
    //Serial.print((char)176);//shows degrees character
    //Serial.print("C  |  ");
    
    //printAddress(address);
    //Serial.println();
  }
  
  //print the temperature in Fahrenheit
  Serial.print(Farenhight(temp));
  Serial.print((char)176);//shows degrees character
  Serial.println("F");
}

//Prints in Hex
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}