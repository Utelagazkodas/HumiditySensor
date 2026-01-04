// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#include "Arduino_LED_Matrix.h"

#define DHTPIN 2 // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

byte numberPatterns[10][7][4] = {
    // 0
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 1
    {
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 1}},

    // 2
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 1, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 3
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 4
    {
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {1, 0, 1, 0},
        {1, 1, 1, 1},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0}},

    // 5
    {
        {0, 1, 1, 1},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 6
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 7
    {
        {1, 1, 1, 1},
        {0, 0, 0, 1},
        {0, 0, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}},

    // 8
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0}},

    // 9
    {
        {0, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 1},
        {1, 0, 0, 1},
        {0, 1, 1, 1}}};

byte cornerPatterns[2][5][3] = {
    {{1, 0, 1},
     {0, 0, 1},
     {0, 1, 0}, // %
     {1, 0, 0},
     {1, 0, 1}},

    {{0, 1, 1}, 
     {1, 0, 0},
     {1, 0, 0}, // C
     {1, 0, 0},
     {0, 1, 1}}};

ArduinoLEDMatrix matrix;

void setup()
{
  // Serial.begin(9600);
  // Serial.println(F("DHTxx test!"));
  matrix.begin();
  dht.begin();
}

int repeat = 0;

void loop()
{
  // Wait a few seconds between measurements.
  delay(3000);

  float measurements[2] = {dht.readHumidity(), dht.readTemperature()};

  byte frame[8][12] = {0};

  int value = int(measurements[repeat % 2]) % 100;
  //int value = repeat % 100;


  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 12; x++)
    {

      if (x < 4 && y > 0)
      {
        //tens place
        frame[y][x] = numberPatterns[value/10][y-1][x];
      }

      else if (x > 4 && y > 0 && x < 9)
      {
        //ones place
        frame[y][x] = numberPatterns[value%10][y-1][x-5];
      }

      else if(x>8 && y < 5){
        frame[y][x] = cornerPatterns[repeat%2][y][x-9];
      }
    }
  }

  matrix.renderBitmap(frame, 8, 12);

  repeat += 1;
}
