
/*
 * High Tech Hive Bee Temperature Monitor and Logger
 */

// Temperature Sensor Libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// SD Card Libraries
#include <SPI.h>
#include <SD.h>

#define ONE_WIRE_BUS 5 // Data wire is plugged into digital pin 5 on the Feather
#define chipSelect 10   // Data Pin for SD Card

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Create File for Data on SD Card
File tempData;

// Create SensorMap
// uint8_t sensorMap[] = {28975B0E000000DA,
//                   28BC2B10000000D4,
//                   288B8F0F00000032,
//                   28E9E80E00000050,
//                   282B180E00000007,
//                   282E1A1000000042,
//                   285FCC0F0000008D,
//                   28CD5A0F00000079,
//                   28044D0F0000005D};
const uint8_t sensorMap[] = {2924906598688096474,
                      2935268404858192084,
                      2921586077867180082};

void setup(void){
  sensors.begin();  // Start up the library
  Serial.begin(9600);
  Serial.print(sensors.getDeviceCount());
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("initializiong SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);  
  }

  Serial.println("initialization done.");

  
}

// Main Loop
void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();
  Serial.println(sensors.getDeviceCount()); // Print the number of sensors to the Serial Terminal
  
  for(int i=0;i<sensors.getDeviceCount();i++){
      print_sensor(i); // Print Data to Serial
      SDWrite(i); // Write Data to SD Card

  }

  
  delay(1000); // Change Delay to 10 seconds
}

double Farenhight(double temp){
  return ((temp* 9.0) / 5.0 + 32.0);
}

void print_sensor(uint8_t index){
  double temp = sensors.getTempCByIndex(index);
  //Serial.print("Temperature:"+String(index)+ "Sensor:");
  DeviceAddress address;
  if (sensors.getAddress(address,index)){
    if(temp>30.0){
          printAddress(address);
          Serial.println();
          Serial.println(temp);

    }
//    Serial.print(String(index)+":   ");
//    Serial.print(temp);
//    Serial.print((char)176);//shows degrees character
//    Serial.print("C  |  ");
//    
//    printAddress(address);
//    Serial.println();
  }
  
  //print the temperature in Fahrenheit
  Serial.print(String(index)+":    ");
  Serial.print(Farenhight(temp));
  Serial.print(char(176));//shows degrees character
  Serial.print("F  |  ");

  printAddress(address);
  Serial.println();
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

//Write to SD Function
void SDWrite(uint8_t index){
  tempData = SD.open("test.txt", FILE_WRITE);
   double temp = sensors.getTempCByIndex(index);  
  // if the file opened okay, write to it:
  if (tempData) {
    // //Serial.print("Writing to test.txt...");
    // //tempData.println(String(temp));
    // tempData.print(String(index)+":    ");
    // tempData.print(Farenhight(temp));
    // tempData.print(char(176));//shows degrees character
    // tempData.print("F  |  ");
    // tempData.println();

    tempData.print(String(Farenhight(temp))+",");
    tempData.println();

    // close the file:
    tempData.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

}
