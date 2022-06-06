#include <RTClib.h>
#include <avr/sleep.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SD.h>

const int CLOCK_INTERRUPT_PIN = 0;  // the pin that is connected to SQW
const int BUTTON_INTERRUPT_PIN = 1;
const int ONE_WIRE_BUS = 19;
const int EXPECTED_DEVICE_COUNT = 10;
const int ONE_WIRE_PIN = 2;
const int GREEN_LED = 8;
const int RED_LED = LED_BUILTIN;
const int CHIP_SELECT = 4;
const int READ_INTERVAL_IN_MINUTES = 15;  
const int STARTUP_LED_DURATION_MILLIS = 5000;
const int VBATPIN = A9;
const int VOLTAGE_DIVIDER_RATIO = 2;
const int VOLTAGE_RESOLUTION = 1024;
const float REFERENCE_VOLTAGE = 3.3;
const float LOW_BATTERY_CUTOFF = 3.6;


enum Error {
  NO_SD_CARD,
  WRONG_DEVICE_COUNT,
  SENSOR_NOT_READY,
  LOW_BATTERY
};

uint8_t sensor0_id[8] = { 0x28, 0x71, 0xB6, 0x06, 0x00, 0x00, 0x00, 0x6B };
uint8_t sensor1_id[8] = { 0x28, 0x14, 0xEA, 0x06, 0x00, 0x00, 0x00, 0xF7 };
uint8_t sensor2_id[8] = { 0x28, 0xD6, 0x86, 0x07, 0x00, 0x00, 0x00, 0xB9 };
uint8_t sensor3_id[8] = { 0x28, 0xC1, 0x33, 0x07, 0x00, 0x00, 0x00, 0xFA };
uint8_t sensor4_id[8] = { 0x28, 0xE0, 0x83, 0x07, 0x00, 0x00, 0x00, 0x34 };
uint8_t sensor5_id[8] = { 0x28, 0x8D, 0x1C, 0x07, 0x00, 0x00, 0x00, 0x65 };
uint8_t sensor6_id[8] = { 0x28, 0xB1, 0x09, 0x07, 0x00, 0x00, 0x00, 0x5B };
uint8_t sensor7_id[8] = { 0x28, 0x5D, 0xE0, 0x06, 0x00, 0x00, 0x00, 0xA9 };
uint8_t sensor8_id[8] = { 0x28, 0x22, 0x98, 0x06, 0x00, 0x00, 0x00, 0x46 };
uint8_t sensor9_id[8] = { 0x28, 0xA2, 0x83, 0x07, 0x00, 0x00, 0x00, 0x2F };

uint8_t *sensor_ids[] = { sensor0_id, sensor1_id, sensor2_id, sensor3_id, sensor4_id,
                          sensor5_id, sensor6_id, sensor7_id, sensor8_id, sensor9_id
                        };

RTC_DS3231 rtc;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
File logfile;
volatile bool exportingData = false;
int device_count = 0;
void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_INTERRUPT_PIN, INPUT_PULLUP);
  setUpRealTimeClock();
  sensors.begin();

  device_count = sensors.getDeviceCount();
  while (device_count != EXPECTED_DEVICE_COUNT) {
    flash_error(WRONG_DEVICE_COUNT);
    device_count = sensors.getDeviceCount();
  }

  while (!SD.begin(CHIP_SELECT)) {
    flash_error(NO_SD_CARD);
  }

  while (read_battery_voltage() < LOW_BATTERY_CUTOFF) {
    flash_error(LOW_BATTERY);
  }

  write_to_file(compose_headers(device_count));
  delay(STARTUP_LED_DURATION_MILLIS);
}

void loop() {
  if (exportingData) {
    exportData();
    exportingData = false;
  } else {
    logHiveData();
    setAlarm();
  }
  goToSleep();
}

void logHiveData() {
  String timeStamp = String(get_time());
  String data = read_data();
  String voltage = String(read_battery_voltage(), 2);
  write_to_file(timeStamp + ", " + data + " " + voltage);
  digitalWrite(GREEN_LED, LOW);
}

void exportData() {
  digitalWrite(RED_LED, HIGH);
  Serial.begin(150200);
  while (!Serial) {
  }
  Serial.print("Connection re-established.");

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(1000);
  digitalWrite(RED_LED, LOW);
}

void setAlarm() {
  rtc.clearAlarm(1);
  rtc.setAlarm1(rtc.now() + TimeSpan(0, 0, READ_INTERVAL_IN_MINUTES, 0), DS3231_A1_Date);
    // last argument is alarm mode: means seconds, minutes, hour, and date all need to match.
}

void flash_status_ok(boolean ok) {
  if (ok) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

float read_battery_voltage() {
  return  analogRead(VBATPIN)
          * VOLTAGE_DIVIDER_RATIO
          * REFERENCE_VOLTAGE
          / VOLTAGE_RESOLUTION;
}

String compose_headers(int count) {
  String text = "Timestamp, ";
  for (int i = 0; i < count; i++) {
    text += ("Sensor " + String(i) + ", ");
  }
  text += "Battery Voltage";
  return text;
}

void write_to_file(String data) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // dataFile returns null=false if unable to open
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
  }
}

void flash_setup_complete() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(STARTUP_LED_DURATION_MILLIS);
}

uint32_t get_time() {
  DateTime now = rtc.now();
  return now.unixtime();
}

void flash_error(Error error) {
  switch (error) {
    case NO_SD_CARD:
      blink(2); break;
    case WRONG_DEVICE_COUNT:
      blink(3); break;
    case LOW_BATTERY:
      blink(4); break;
    case SENSOR_NOT_READY:
      blink(5); break;
    default:
      blink(6); break;
  }
}

void setUpRealTimeClock() {
  // initializing the rtc
  if (!rtc.begin()) {
    //Serial.println("Couldn't find RTC!");
    //Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    // this will adjust to the date and time at compilation
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // schedule an alarm 10 seconds in the future

  rtc.setAlarm1(rtc.now() + TimeSpan(10), DS3231_A1_Second);
  //we don't need the 32K Pin, so disable it
  rtc.disable32K();

  // Making it so, that the alarm will trigger an interrupt
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);

  // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // stop oscillating signals at SQW Pin
  // otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);
}

void goToSleep() {
  Serial.end();
  exportingData = false;
  digitalWrite(RED_LED, exportingData);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), clockWakeUp, LOW);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), buttonWakeUp, LOW);
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  delay(1000);
  sleep_cpu();
}

void clockWakeUp() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN));
  detachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN));
}

void buttonWakeUp() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN));
  detachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN));
  exportingData = true;
}

void logTemperatures() {
  Serial.begin(115200);
  while (!Serial) {
  }
  sensors.requestTemperatures();
  int deviceCount = sensors.getDeviceCount();
}

String read_data() {
  sensors.requestTemperatures();
  String data = "";
  for (int i = 0; i < device_count; i++) {
    double sensor_reading = sensors.getTempC(sensor_ids[i]);
    data += String(sensor_reading) + ", ";
  }
  return data;
}

void blink(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  delay(1000);
}