#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> //JSON helper libaray for parsing JSON
#include "DHT.h" //DHT sensor library, (pin, type), used to obtain sensor values

//network credentials
const char* ssid = "UU-IoT";
const char* password = "0xDEADBEEF";

//API URL
// String apiUrl = "https://petstoredemo.azure-api.net/pet/1"; //test
String apiUrl = "http://tempandhumidity.azure-api.net"; //actual API

//subscription key
// String subscriptionKey = "1b4c7bde7691450ea424c9ed91230c17"; //test
String subscriptionKey  = "1b4c7bde7691450ea424c9ed91230c17";  //for API class API
//String apiKey  = "1b4c7bde7691450ea424c9ed91230c17"; //for API example
//shared access key server needs for authentication
//will be included in the header

#define DHTPIN 13 //pin number connected to DHT data line
#define DHTTYPE DHT11 //defines which DHT model

DHT dht(DHTPIN, DHTTYPE); //create DHT object

// set pin numbers
const int ledPin = 15;//number of the led pin

void setup() {
  Serial.begin(115200);
  //start serial baud at 115200
    //top right magnifying glass



  dht.begin();//init DHT sensor lib

  pinMode(ledPin, OUTPUT);//declare LED as output, so modes can be changed (high, low)



  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  //Make the API call
  makeApiRequest();
}

void makeApiRequest() {
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    //Begin request to URL
    http.begin(apiUrl);

    //Add header
    http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

    // Send GET request
    int httpResponseCode = http.GET();

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    //If success, print body
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response:");
      Serial.println(payload);
    } else {
      Serial.println("Request failed!");
    }

    http.end();
  
  } else {
    Serial.println("WiFi not connected.");
  }
}

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

  getLatestReading();//Show what server stored
  checkLEDState();//Update LED state from server

  delay(8000);//can adjust
}

//Post the temperature and humidity data
void sendSensorData(float temp, float hum) {
  HTTPClient http;

  String url = apiUrl + "/data";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

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

  http.begin(apiUrl + "/data/latest");
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

  int code = http.GET();
  Serial.print("GET /data/latest → ");
  Serial.println(code);

  if (code == 200) {
    String response = http.getString();
    Serial.println("Latest Data: " + response);
  }

  http.end();
}

//POST /led   → Get LED State From Server
void checkLEDState() {
  // will POST {"state":"ON"} or {"state":"OFF"}

  HTTPClient http;
  http.begin(apiUrl + "/led");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

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
  unsigned long ms = millis();
  return "2025-12-02T" + String((ms/1000)%60) + ":00Z";
}


/*.
Connected!
HTTP Response code: 200
Response:
{"id":1,"name":"Pet1","photoUrls":["test1","test2"],"tags":[],"status":"available"}
*/
