#include "WiFiS3.h"
#include "wifi.h"
#include "WiFiUdp.h"

#include "RTC.h"

#include <NTPClient.h>
#include "DHT.h"

#include <math.h>

// wifi shit
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

// time shit
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

unsigned long lastInbetweenSample = latestSampleMillis;

const size_t samplesInBetween = 5;

float avgHumidities[samplesInBetween] = {};
float avgTemperatures[samplesInBetween] = {};

int samplesTakenInbeetween = 0;

uint8_t quarterHourSamples[192][2];
uint8_t hourSamples[72][2];

int lastHourMeasurementPush = 0;
void addMeasurement(uint8_t measurement[2])
{

  uint8_t shiftTemp[2] = {measurement[0], measurement[1]};
  for (int i = 191; i >= 0; i--)
  {
    quarterHourSamples[i][0] = shiftTemp[0];
    quarterHourSamples[i][1] = shiftTemp[1];

    if (i == 0)
    {
      lastHourMeasurementPush += 1;

      if (lastHourMeasurementPush % 4 == 0)
      {

        for (int i = 71; i >= 0; i--)
        {
          hourSamples[i][0] = (uint8_t)shiftTemp[0];
          hourSamples[i][1] = (uint8_t)shiftTemp[1];

          if (i > 0)
          {
            shiftTemp[0] = hourSamples[i - 1][0];
            shiftTemp[1] = hourSamples[i - 1][1];
          }
        }

        lastHourMeasurementPush = 0;
        RTCTime timeToSet = RTCTime(timeClient.getEpochTime());
        RTC.setTime(timeToSet);
      }
    }
    else
    {
      shiftTemp[0] = quarterHourSamples[i - 1][0];
      shiftTemp[1] = quarterHourSamples[i - 1][1];
    }
  }
}

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

  uint8_t measurements[2] = {(uint8_t)(dht.readHumidity() * 2), (uint8_t)(dht.readTemperature() * 4)};

  RTC.getTime(latestSample);
  latestSampleMillis = millis();

  lastInbetweenSample = latestSampleMillis;

  for (size_t i = 0; i < samplesInBetween; i++)
  {
    avgHumidities[i] = NAN;
    avgTemperatures[i] = NAN;
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t))
  {
    avgHumidities[0] = h;
    avgTemperatures[0] = t;
    samplesTakenInbeetween = 1;
  }

  addMeasurement(measurements);

  /*for (int i = 191; i >= 0; i--)
  {
    quarterHourSamples[i][0] = measurements[0];
    quarterHourSamples[i][1] = measurements[1];
  }
  for (int i = 71; i >= 0; i--)
  {
    hourSamples[i][0] = measurements[0];
    hourSamples[i][1] = measurements[1];
  }*/
}

// this was written by chatgtp
String generateJSON(RTCTime latestSample, uint8_t hourSamples[72][2], uint8_t quarterHourSamples[192][2])
{
  String json = "{";

  // Latest sample as epoch
  json += "  \"latestSample\": ";
  json += String((long)(latestSample.getUnixTime())); // or latestSample.timestamp depending on your RTC lib
  json += ",";

  // Hour samples
  json += "  \"hourSamples\": [";
  for (int i = 0; i < 72; i++)
  {
    json += "    [";
    json += String(hourSamples[i][0]);
    json += ",";
    json += String(hourSamples[i][1]);
    json += "]";
    if (i < 71)
      json += ",";
    json += "";
  }
  json += "  ],";

  // Quarter-hour samples
  json += "  \"quarterHourSamples\": [";
  for (int i = 0; i < 192; i++)
  {
    json += "    [";
    json += String(quarterHourSamples[i][0]);
    json += ",";
    json += String(quarterHourSamples[i][1]);
    json += "]";
    if (i < 191)
      json += ",";
    json += "";
  }
  json += "  ]";

  json += "}";

  return json;
}

void loop()
{
  WiFiClient client = server.available();

  if ((15 * 60) * 1000 <= millis() - latestSampleMillis)
  {

    RTC.getTime(latestSample);
    latestSampleMillis = millis();

    float avgTemperature = 0;
    float avgHumidity = 0;
    int amountOfSamples = 0;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      avgTemperature += t;
      avgHumidity += h;
      amountOfSamples++;
    }

    for (size_t i = 0; i < samplesInBetween; i++)
    {
      if (!isnan(avgHumidities[i]) && !isnan(avgTemperatures[i]))

      {
        avgHumidity += avgHumidities[i];
        avgTemperature += avgTemperatures[i];
        amountOfSamples += 1;
        avgHumidities[i] = NAN;
        avgTemperatures[i] = NAN;
      }
    }

    if (amountOfSamples == 0)
    {
      samplesTakenInbeetween = 0;
      return;
    }

    uint8_t measurements[2] = {(uint8_t)(avgHumidity / amountOfSamples * 2), (uint8_t)(avgTemperature / amountOfSamples * 4)};

    addMeasurement(measurements);
  }

  if (millis() - lastInbetweenSample >= (15 * 60) * 1000 / samplesInBetween)
  {
    if (samplesInBetween > samplesTakenInbeetween)
    {
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      if (!isnan(h) && !isnan(t))
      {
        avgHumidities[samplesTakenInbeetween] = h;
        avgTemperatures[samplesTakenInbeetween] = t;
        samplesTakenInbeetween++;
      }

      lastInbetweenSample = millis();
    }
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
              String json = generateJSON(latestSample, hourSamples, quarterHourSamples);
              client.print(json); 
            }
            else
            {

              client.println("Content-type:text/html");
              client.println();
              client.print("<!doctype html><html lang='en' style='background-color:#655967'><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Document</title><div id='online'><canvas id='myChart' width='80vw' height='25vh'></canvas></div><div id='offline'><div>Current Relative Humiditiy: <span id='curHum'>85</span></div><div>Current Temperature: <span id='curTemp'>24</span></div></div><script src='https://cdn.jsdelivr.net/npm/chart.js'></script><script src='https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns/dist/chartjs-adapter-date-fns.bundle.min.js'></script><script type='module'>var o=document.getElementById('myChart'),m=!1,t={labels:[],tempData:[],humidityData:[]},u=await fetch('/data.json'),a=await u.json(),r=a.latestSample*1e3,l=[],i=[],s=[];for(;a.quarterHourSamples.length!=0;){let e=a.quarterHourSamples.pop();if(!e)break;l.push(r),r-=900*1e3,i.push(e[0]),s.push(e[1])}for(;a.hourSamples.length!=0;){let e=a.hourSamples.pop();if(!e)break;l.push(r),r-=3600*1e3,i.push(e[0]),s.push(e[1])}t.labels=l.reverse();t.humidityData=i.reverse();t.tempData=s.reverse();try{p=new Chart(o,{type:'line',data:{labels:t.labels,datasets:[{label:'Relative Humidity (%)',data:t.humidityData,borderWidth:2,yAxisID:'H',borderColor:'#7891c5ff'},{label:'Temperature (C)',data:t.tempData,borderWidth:2,yAxisID:'T',borderColor:'#ce7b67ff'}]},options:{scales:{H:{position:'left',min:50,max:100,grid:{color:'#5e76a8ff'},ticks:{color:'#adbbdaff'},title:{display:!0,text:'Humidity (%)',color:'#adbbdaff'}},T:{position:'right',min:15,max:30,grid:{color:'#975845ff'},ticks:{color:'#e98043ff'},title:{display:!0,text:'Humidity (%)',color:'#e98043ff'}},x:{type:'time',axis:'x',time:{minUnit:'minute'},grid:{display:!1},ticks:{color:'#cccccc'}}}}}),m=!0}catch{m=!1,document.getElementById('online')?.remove()}var p;</script>");   
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
