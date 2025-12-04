#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "UU-IoT";
const char* password = "0xDEADBEEF";

//API URL
String apiUrl = "https://petstoredemo.azure-api.net/pet/1";

//subscription key
String subscriptionKey = "1b4c7bde7691450ea424c9ed91230c17";

void setup() {
  Serial.begin(115200);
  //start serial baud at 115200
    //top right magnifying glass

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

}


/*.
Connected!
HTTP Response code: 200
Response:
{"id":1,"name":"Pet1","photoUrls":["test1","test2"],"tags":[],"status":"available"}
 */