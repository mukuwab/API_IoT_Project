//Aligns w/ spec--> uses HTTP REST, no longer MQTT

//Azure overview >>> Gateway URL
//https://lab1-440.azure-api.net/

//Lib curl
//aRest
//MQTT --> data transport option

//HTTP client library

//try GETs and POSTs using the petstore API
  //will change the server and header when you get new API

//shared access key

//Dr.Glas' sample API key
  //API key: 1b4c7bde7691450ea424c9ed91230c17

//API Example
//GET https://petstoredemo.azure-api.net/pet/1 HTTP/1.1
//Ocp-Apim-Subscription-Key: 1b4c7bde7691450ea424c9ed91230c17
 
//IMPORTS//
#include <WiFi.h> //wifi library, enables wifi conenction
#include <HTTPClient.h> //provides methods for HTTP request
#include <ArduinoJson.h> //JSON helper libaray for parsing JSON
#include "DHT.h" //DHT sensor library, (pin, type), used to obtain sensor values

/************************* WiFi Access Point *********************************/

#define WLAN_SSID "UU-IoT"
#define WLAN_PASS "0xDEADBEEF"

/*****************************************************************************/

//#define AIO_SERVER "https://petstoredemo.azure-api.net/pet/1"
//this will be changed to the azure link

String apiBase = "http://tempandhumidity.azure-api.net"; //actual API
String apiBase = "https://petstoredemo.azure-api.net/pet/1";

#define API_KEY  "1b4c7bde7691450ea424c9ed91230c17"
//sever in OPEN API spec, baseURL

String apiKey  = "1b4c7bde7691450ea424c9ed91230c17";  //for API class API
//String apiKey  = "1b4c7bde7691450ea424c9ed91230c17"; //for API example
//shared access key server needs for authentication
//will be included in the header

#define DHTPIN 13 //pin number connected to DHT data line
#define DHTTYPE DHT11 //defines which DHT model

DHT dht(DHTPIN, DHTTYPE); //create DHT object

// set pin numbers
const int ledPin = 15;//number of the led pin

/************************* Set up *********************************/

void setup() {
  Serial.begin(115200); //start serial communication
  //allows printing debug messages to Serial Monitor
  
  dht.begin();//init DHT sensor lib

  pinMode(ledPin, OUTPUT);//declare LED as output, so modes can be changed (high, low)

  //Connect to Wi-fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
 
  //Loop until Wifi connects
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
}//end: setup

/************************* main loop *********************************/

void loop() {
  //loop doesn't terminate after setup
  //will read sensors and call API
  
  if (WiFi.status() != WL_CONNECTED) return;
  //if wifi drops, do nothing until connected again

  float temperature = dht.readTemperature();//read temp (c) from DHT sensor
  //returns NaN if errors
  float humidity = dht.readHumidity();//read humditity from DHT sensor
  //returns percentage

  
  if (!isnan(temperature) && !isnan(humidity)) {
    sendSensorData(temperature, humidity);
  }

  getLatestReading();       // Show what server stored
  checkLEDState();          // Update LED state from server

  delay(8000);              // Adjust as needed
}

//Post the temperature and humidity data
void sendSensorData(float temp, float hum) {
  HTTPClient http;

  String url = apiBase + "/data";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", apiKey);

  String body = "{ \"temperature\": " + String(temp) +
                ", \"humidity\": " + String(hum) +
                ", \"timestamp\": \"" + getTimeString() + "\" }";

  int code = http.POST(body);

  Serial.print("POST /data → ");
  Serial.println(code);

  http.end();
}


//get data and latest
void getLatestReading() {
  HTTPClient http;

  http.begin(apiBase + "/data/latest");
  http.addHeader("Ocp-Apim-Subscription-Key", apiKey);

  int code = http.GET();
  Serial.print("GET /data/latest → ");
  Serial.println(code);

  if (code == 200) {
    String response = http.getString();
    Serial.println("Latest Data: " + response);
  }

  http.end();
}

/***********************************************************
 *               POST /led   → Get LED State From Server
 ***********************************************************/
void checkLEDState() {
  // For class demo we assume teacher will POST {"state":"ON"} or {"state":"OFF"}

  HTTPClient http;
  http.begin(apiBase + "/led");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", apiKey);

  // Dummy request so server can respond
  int code = http.POST("{\"state\":\"CHECK\"}");

  if (code == 200) {
    String response = http.getString();
    Serial.println("LED Response: " + response);

    if (response.indexOf("ON") > -1) {
      digitalWrite(ledPin, HIGH);
    } else if (response.indexOf("OFF") > -1) {
      digitalWrite(ledPin, LOW);
    }
  }

  http.end();
}


//Basic time loop
String getTimeString() {
  // Real projects use RTC or NTP; this is OK for class assignment
  unsigned long ms = millis();
  return "2025-12-02T" + String((ms/1000)%60) + ":00Z";
}