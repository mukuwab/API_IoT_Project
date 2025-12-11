//8.6: Temp_Humidity_Monitor
  //https://docs.sunfounder.com/projects/esp32-starter-kit/en/latest/arduino/iot_projects/ar_iot_adafruitio.html

//Reference for the LCD Display
  //https://docs.sunfounder.com/projects/esp32-starter-kit/en/latest/arduino/basic_projects/ar_lcd.html

//=--LIBRARIES--=//
#include <WiFi.h>//handles wifi connection
#include <HTTPClient.h>//allows sending GET & POSTs
#include <ArduinoJson.h> //JSON helper libaray for parsing JSON
#include "DHT.h" //DHT sensor library, (pin, type), used to obtain sensor values
#include <LiquidCrystal_I2C.h>//control I2C LCD display
#include <Wire.h>//control I2C LCD display

//network credentials
// const char* ssid = "UU-IoT";
// const char* password = "0xDEADBEEF";

const char* ssid = "Monica wifi";
const char* password = "3368844443";

  //API URL
  //String apiUrl = "https://petstoredemo.azure-api.net/pet/1"; //test
String apiUrl = "http://tempandhumidity.azure-api.net";
// String apiUrl = "http://lab1-440.azure-api.net/temp-and-humidity";
//subscription key
//String subscriptionKey = "1b4c7bde7691450ea424c9ed91230c17"; //test
String subscriptionKey  = "edb15d72f99f49479cedd9f83db5fbd0";  //for API class API
  //shared access key server needs for authentication
  //will be included in the header

#define DHTPIN 13 //pin number connected to DHT data line
#define DHTTYPE DHT11 //defines which DHT model

DHT dht(DHTPIN, DHTTYPE); //create DHT object
  //used to call functions such as: dht.readTemperatureF

// set pin numbers
const int ledPin = 15;//number of the led pin

LiquidCrystal_I2C lcd(0x27,16,2);// set the LCD address to 0x27 for a 16 chars and 2 line display

// Forward declarations
  //need to declare functions before setup() so compiler knows they exsist
void makeApiRequest();
void sendSensorData(float temp, float hum);
void getLatestReading();
void checkLEDState(float temperatureF);
String getTimeString();

void setup() {
  
  Serial.begin(115200);
  //start serial monitor at 115200 baud for debugging
  //top right magnifying glass

  Serial.println("Starting setup...");
  //print set-up feedback to serial monitor

  dht.begin();//init DHT sensor lib

  pinMode(ledPin, OUTPUT);//declare LED as output, so modes can be changed (high, low)

  // Initialize LCD
  lcd.init();// initialize the lcd 
  lcd.backlight(); // Turns on the LCD backlight.
  lcd.setCursor(0, 0);//set cursor to column 0, row 0
  lcd.print("Starting...");
    //will be on first row

  WiFi.begin(ssid, password);
  // Connect to WiFi w/ credentials

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  //loop until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected");
    makeApiRequest();
  } 
  
  else {
    Serial.println("\nWiFi failed. Continuing without network...");
  }
}


void loop() {
  //loop doesn't terminate after setup
  //will read sensors and call API


  //read + store sensor data
  float temperatureC = dht.readTemperature();//read temp (c) from DHT sensor
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;  // Fahrenheit
    //returns NaN if errors
  
  float humidity = dht.readHumidity();//read humditity from DHT sensor
    //returns percentage


  //Print sensor values to the serial monitor for testing
  Serial.print("Temperature (F): ");
  Serial.println(temperatureF);
 
  Serial.print("Humidity (%): ");
  Serial.println(humidity);

  //display on LCD
  lcd.setCursor(0, 0);//sets cursor at row 1
  lcd.print("Temp: ");
  lcd.print(temperatureF);
  lcd.print((char)223); // degree symbol
  lcd.print("F  ");

  lcd.setCursor(0, 1);//sets cursor at row 2
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print("%   ");

  if (!isnan(temperatureC) && !isnan(humidity)) {
    sendSensorData(temperatureC, humidity);
    getLatestReading();//Show what server stored
    checkLEDState(temperatureF);//Update LED state from server
  }

  delay(8000);
  // float temperature = dht.readTemperature();
  // float humidity = dht.readHumidity();

}

//Post the temperature and humidity data
void sendSensorData(float temp, float hum) {
  //pass in temp and hum as parameters, done in forward declaration
  
  //create HTTP client
  HTTPClient http;
    //allows for GET and POST requests to API

  //build URL for data endpoint
  String url = apiUrl + "/data";

  //initalize HTTP request
  http.begin(url);
  
  //Header
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

  //JSON body
  // String body = "{\"temperature\": " + String(temp) +
  //               ", \"humidity\": " + String(hum) +
  //               ", \"timestamp\": \"" + getTimeString() + "\"}";

  String body = "{\"temperature\": " + String(temp) +
              ", \"humidity\": " + String(hum) +
              ", \"timestamp\": \"" + getTimeString() + "\"}";

  Serial.println("POST Body:");
  Serial.println(body);

  //Send json body in POST
  int code = http.POST(body);

  //print to serial monitor for testing
  Serial.print("POST /data: ");
  Serial.println(code);

  http.end();//end HTTP client
}

//GET
void makeApiRequest() {
  
  if (WiFi.status() == WL_CONNECTED) {
    
    //Create HTTP client object for GET request
    HTTPClient http;

    //begin request to URL
    http.begin(apiUrl);

    //header
    http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

    //send GET
    int httpResponseCode = http.GET();

    //print response to serial monitor
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

    http.end();//http client
  
  } else {
    Serial.println("WiFi not connected.");
  }
}

//GET
void getLatestReading() {
  
  //Create HTTP client object for GET request
  HTTPClient http;

  http.begin(apiUrl+"/data/latest");//build url
  
  //header, GET only needs key 
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

  //send GET
  int code = http.GET();

  Serial.print("GET /data/latest: ");
  Serial.println(code);

  //if successful print to serial monitor
  if (code == 200) {
    String response = http.getString();
    Serial.println("Latest Data: "+ response);
  }

  http.end();
}

//Get LED state from server
void checkLEDState(float temperatureF) {
  //will POST {"state":"ON"} or {"state":"OFF"}

  HTTPClient http;//new client obj.
 
  http.begin(apiUrl+"/led");//build url
 
  //header
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);

  // Turn LED ON if temp > 80F, OFF otherwise
  String state = (temperatureF > 80) ? "ON" : "OFF";
  int code = http.POST("{\"state\":\"" + state + "\"}");
  digitalWrite(ledPin, state == "ON" ? HIGH : LOW);

  //print response if successful
  // if (code == 200) {
  //   String response = http.getString();
  //   Serial.println("LED Response: " + response);

  //   //check led state: on or off
  //   if (response.indexOf("ON") > -1) {
  //     //method for reading response + checking for ON
     
  //     digitalWrite(ledPin, HIGH);//turn on LED
  //   } 
    
  //   else if (response.indexOf("OFF") > -1) {
  //       //method for reading response + checking for OFF
     
  //     digitalWrite(ledPin, LOW);//turn off LED
  //   }

  Serial.print("POST /led code: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    Serial.println("LED Response: " + response);
  } else {
    Serial.println("LED POST failed");
  }

  digitalWrite(ledPin, state == "ON" ? HIGH : LOW);

  http.end();
  }
  
  // //print values to serial monitor
  // Serial.print("POST /led code: ");
  // Serial.println(code);

  // String payload = http.getString();
  // Serial.println("LED Payload:");
  // Serial.println(payload);


  // http.end();


// String getTimeString() {
//   unsigned long ms = millis() / 1000; // seconds since boot

//   int hours = (ms / 3600) % 24;
//   int minutes = (ms / 60) % 60;
//   int seconds = ms % 60;

//   //ISO-8601 format for API compatibility
//   char buffer[30];
//   sprintf(buffer, "2025-12-02T%02d:%02d:%02dZ", hours, minutes, seconds);

//   return String(buffer);
// }

String getTimeString() {
  unsigned long ms = millis() / 1000; // seconds since boot

  int hours = (ms / 3600) % 24;
  int minutes = (ms / 60) % 60;
  int seconds = ms % 60;

  char buffer[30];
  sprintf(buffer, "2025-12-02T%02d:%02d:%02dZ", hours, minutes, seconds);

  return String(buffer);
}


