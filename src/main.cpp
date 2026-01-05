#include "WiFiS3.h"
#include "wifi.h"
#include "WiFiUdp.h"

#include "RTC.h"

#include <NTPClient.h>
#include "DHT.h"

// wifi shit
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

//time shit
WiFiUDP Udp;
NTPClient timeClient(Udp);




#define DHTPIN 2

#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

RTCTime latestSample;
unsigned long latestSampleMillis = 0;


uint8_t quarterHourSamples[192][2];
uint8_t hourSamples[72][2];

void setup()
{
  Serial.begin(9600);

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  server.begin();
  printWifiStatus();

  RTC.begin();
  timeClient.begin();
  timeClient.update();

  RTCTime timeToSet = RTCTime(timeClient.getEpochTime());
  RTC.setTime(timeToSet);

  dht.begin();

  float measurements[2] = {dht.readHumidity(), dht.readTemperature()};

  RTC.getTime(latestSample);
  latestSampleMillis = millis();

  for (int i = 191; i >= 0; i--)
  {
    quarterHourSamples[i][0] = (uint8_t)measurements[0];
    quarterHourSamples[i][1] = (uint8_t)measurements[1];

  }
    for (int i = 71; i >= 0; i--)
  {
    hourSamples[i][0] = (uint8_t)measurements[0];
    hourSamples[i][1] = (uint8_t)measurements[1];

  }
  

}






void loop()
{
  WiFiClient client = server.available();

  if((15*60)*1000 <= millis() - latestSampleMillis){
    RTC.getTime(latestSample);
    latestSampleMillis = millis();

    float measurements[2] = {dht.readHumidity(), dht.readTemperature()};

    

  }

  if (client)
  {
    Serial.println("new client");
    String currentLine = "";
    bool isDataReq = false;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);

        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            if (isDataReq)
            {
              client.println("Content-type:application/json");
              client.println();
              client.print("{'gayAssNigga' : true}");
            }
            else
            {

              client.println("Content-type:text/html");
              client.println();
              client.print("gayer ass nigga balls");
            }

            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /data.json"))
        {
          isDataReq = true;
        }
      }
    }

    client.stop();
    Serial.println("client disconnected");
  }
}

