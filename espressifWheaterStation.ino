#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MPL115A2.h>
#include <SPIFFS.h>
#include <Adafruit_CCS811.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);//set the LCD address to 0x27 for a 16 chars and 2 line display


const char *SSID = "**";
const char *PASSWORD = "**";

AsyncWebServer server(80);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_MPL115A2 mpl115a2;
Adafruit_CCS811 ccs;

float humidity = 0;
float temperature = 0;
float pressure = 5;
float eCO2 = 0;
float tvoc = 0;

void setup()
{
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("Dateisystem konnte nicht initialisiert werden.");
        return;
    }

    htu.begin();
    mpl115a2.begin();
    ccs.begin();
    while (!ccs.available())
        ;

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    if (MDNS.begin("esp32"))
    {
        Serial.println("MDNS-Responder gestartet.");
    }

    server.on("/gauge.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/gauge.min.js");
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false, replaceVariable);
    });

    server.begin();

    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Hello, world!");
    lcd.setCursor(0,1);
    lcd.print("by EasyIoT");

}

void loop()
{
    humidity = htu.readHumidity();
    temperature = htu.readTemperature();
    pressure = mpl115a2.getPressure();
    ccs.readData();
    eCO2 = ccs.geteCO2();
    tvoc = ccs.getTVOC();

    lcd.setCursor(0,0);
    lcd.print(String("Humidity    ") + String(humidity, 1)    + String(" C"));
    lcd.setCursor(0,1);
    lcd.print(String("Temperature ") + String(temperature, 1) + String(" %"));
    lcd.setCursor(0,2);
    lcd.print(String("Pressure    ") + String(pressure, 1)    + String(" kPa"));
    lcd.setCursor(0,3);
    lcd.print(String("eCO2       ") + String(eCO2, 1)         + String(" ppm"));
 
    delay(5000);
}

String replaceVariable(const String &var)
{
    if (var == "HUMIDITY")
        return String(humidity, 1);
    if (var == "TEMPERATURE")
        return String(temperature, 2);
    if (var == "PRESSURE")
        return String(pressure, 2);
    if (var == "CO2")
        return String(eCO2, 1);
    if (var == "TVOC")
        return String(tvoc, 1);
    return String();
}
