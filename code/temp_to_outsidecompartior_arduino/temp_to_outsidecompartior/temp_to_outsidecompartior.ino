

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <BME280I2C.h>
#include <analogWrite.h>
#define SERIAL_BAUD 115200
BME280I2C bme;
#include "config.h"

#define redPin 25   // GIOP23
#define greenPin 33 // GIOP22
#define bluePin 32  // GIOP21
int outSideTemp = 22;
int insideRoomMargin = 2;
float insideTemp = 20;

unsigned long delayTime;


// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
// unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    } // Wait
    Wire.begin();
    while (!bme.begin())
    {
        Serial.println("Could not find BME280 sensor!");
        delay(1000);
    }

    switch (bme.chipModel())
    {
    case BME280::ChipModel_BME280:
        Serial.println("Found BME280 sensor! Success.");
        break;
    case BME280::ChipModel_BMP280:
        Serial.println("Found BMP280 sensor! No Humidity available.");
        break;
    default:
        Serial.println("Found UNKNOWN sensor! Error!");
    }

    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}

void loop()
{
    // Send an HTTP GET request
    if ((millis() - lastTime) > timerDelay)
    {
        // Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED)
        {
            String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

            jsonBuffer = httpGETRequest(serverPath.c_str());
            Serial.println(jsonBuffer);
            JSONVar myObject = JSON.parse(jsonBuffer);

            // JSON.typeof(jsonVar) can be used to get the type of the var
            if (JSON.typeof(myObject) == "undefined")
            {
                Serial.println("Parsing input failed!");
                return;
            }

            
           // Serial.println(myObject);
            Serial.print("Outside emperature: ");
            outSideTemp = int(myObject["main"]["temp"]);
            myObject = 0;
            outSideTemp = outSideTemp - 272;
            Serial.println(outSideTemp);
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
        lastTime = millis();

        getInsideTemp();

        if (outSideTemp > insideTemp)
        {
            redOn();
            Serial.println("OutsideTemp>Insidetemp");
        }
        if (outSideTemp < insideTemp)
        {
            blueOn();
            Serial.println("OutsideTemp<insidetemp");
        }
    }
}

void redOn()
{
    Serial.println("Red light on!");
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
}
void greenOn()
{
    Serial.println("Green light on!");
    analogWrite(redPin, 0);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);
}
void blueOn()
{

    Serial.println("Blue light on!");
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 255);
}

String httpGETRequest(const char *serverName)
{
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0)
    {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}
void getInsideTemp()
{

    printBME280Data(&Serial);
}
void printBME280Data(
    // get bme data also outputs to console in json format. 
    Stream *client)
{
    float temp(NAN), hum(NAN), pres(NAN);

    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);

    bme.read(pres, temp, hum, tempUnit, presUnit);

    client->print("InsideTemp: ");
    insideTemp = temp;
    client->print(temp);
    client->print("°" + String(tempUnit == BME280::TempUnit_Celsius ? 'C' : 'F'));
    
    // json return
    Serial.println();
    Serial.print("{");
    Serial.print("\"temperature\":");
    Serial.print(temp);
    Serial.print(",");
    Serial.print("\"humidity\":");
    Serial.print(hum);
    Serial.print("}");    
    Serial.println();
}