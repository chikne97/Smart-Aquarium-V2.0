/*
 * Initial Beta Version
 * V1.0 OTA update, oled connection, wifi icons, connection/disconnecyion and WiFi signal strength display 15/11/21
 * v1.2 Added DS3231 for back time keeping, added a way to power up when there is no wifi signal 16/11/21
 * v1.3 Added relays to the circuit (4 channel active-low) 17/11/21
 * v1.4 
 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <DS3231.h>
DS3231 Clock;
bool century = false;
bool h12Flag = false;
bool pmFlag;

#ifndef STASSID
#define STASSID "SonyBraviaX400"
#define STAPSK "79756622761"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const long utcOffsetInSeconds = 19800;
char week[7][20] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C    //0x3c for 0x78
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM wifiFull[] = {
    0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x3F, 0xFC, 0x70, 0x0E, 0xE0, 0x07, 0x47, 0xE2, 0x1E, 0x78,
    0x18, 0x18, 0x03, 0xC0, 0x07, 0xE0, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM wifiMed[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x1E, 0x78,
    0x18, 0x18, 0x03, 0xC0, 0x07, 0xE0, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM wifiHalf[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xC0, 0x07, 0xE0, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM wifiLow[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM wifiNo[] = {
    0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x7F, 0xFC, 0x78, 0x0E, 0xFC, 0x07, 0x4F, 0xE2, 0x1F, 0xB8,
    0x19, 0xD8, 0x03, 0xE0, 0x07, 0xF8, 0x00, 0x08, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};

const unsigned char picture[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const int relayPin1 = 0;
const int relayPin2 = 14;
const int relayPin3 = 12;
const int relayPin4 = 13;

#define RELAY1 true //mark true to activate, and false to deactivate
#define RELAY2 false
#define RELAY3 false
#define RELAY4 false

#define RELAY1OnOff (12, 00, 00, 19, 00, 00, 1) //first three arguments are RELAY ON Timings as h, m, s, and next three are off timings, and the 7th or the last argument is relay number
#define RELAY1ON digitalWrite(relayPin1, LOW)   //for example, to turn on Relay 1 at 12 PM or 12 Hours and turn it off at 7 PM or 19 Hours use (12, 00, 00, 19, 00, 00, 1)
#define RELAY1OFF digitalWrite(relayPin1, HIGH)

#define RELAY2OnOff (12, 00, 00, 19, 00, 00, 2)
#define RELAY2ON digitalWrite(relayPin2, LOW)
#define RELAY2OFF digitalWrite(relayPin2, HIGH)

#define RELAY3OnOff (12, 00, 00, 19, 00, 00, 3)
#define RELAY3ON digitalWrite(relayPin3, LOW)
#define RELAY3OFF digitalWrite(relayPin3, HIGH)

#define RELAY4OnOff (12, 00, 00, 19, 00, 00, 4)
#define RELAY4ON digitalWrite(relayPin4, LOW)
#define RELAY4OFF digitalWrite(relayPin4, HIGH)

void setup()
{
  Wire.begin();
  Serial.begin(57600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.drawBitmap(0, 0, picture, 128, 64, WHITE);
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setCursor(30, 25);   // Start at top-left corner
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text

  display.println(F("Connecting..."));
  display.display();

  //Wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0); // Start at top-left corner
    display.println(F("Connection\n\nFailed!"));
    display.display();
    delay(5000);
    //ESP.restart();
    count = 1;
    break;
  }

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       display.clearDisplay();
                       display.setTextSize(2);
                       display.setCursor(0, 0); // Start at top-left corner
                       display.println(F("OTA Update\n"));
                       display.setTextSize(1);
                       display.print("Type: ");
                       display.print(type);
                       display.display();
                       delay(2000);
                     });
  ArduinoOTA.onEnd([]()
                   {
                     display.clearDisplay();
                     display.setTextSize(2);
                     display.setCursor(15, 25); // Start at top-left corner
                     display.println(F("Rebooting"));
                     display.display();
                     //delay(2000);
                   });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          display.clearDisplay();
                          display.setTextSize(2);
                          display.setCursor(0, 10); // Start at top-left corner
                          display.print("Progress: ");
                          display.setCursor(50, 35);
                          display.print((progress / (total / 100)));
                          display.setCursor(88, 35);
                          display.print("%");
                          display.display();
                        });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       display.clearDisplay();
                       display.setTextSize(2);
                       display.setCursor(0, 10); // Start at top-left corner
                       display.print("Error!");
                       display.display();
                       //Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         display.clearDisplay();
                         display.setTextSize(2);
                         display.setCursor(0, 10); // Start at top-left corner
                         display.print("Auth Failed");
                         display.display();
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         display.clearDisplay();
                         display.setTextSize(2);
                         display.setCursor(0, 10); // Start at top-left corner
                         display.print("Begin Failed");
                         display.display();
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         display.clearDisplay();
                         display.setTextSize(2);
                         display.setCursor(0, 10); // Start at top-left corner
                         display.print("Connect Failed");
                         display.display();
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         display.clearDisplay();
                         display.setTextSize(2);
                         display.setCursor(0, 10); // Start at top-left corner
                         display.print("Receive Failed");
                         display.display();
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         display.clearDisplay();
                         display.setTextSize(2);
                         display.setCursor(0, 10); // Start at top-left corner
                         display.print("End Failed");
                         display.display();
                       }
                     });
  ArduinoOTA.begin();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); // Start at top-left corner
  display.println(F("Connected!"));
  display.setTextSize(1);
  display.println("");
  display.println(WiFi.SSID());
  display.print("\nIP address:\n");
  display.println(WiFi.localIP());
  display.display();
  if (count == 0)
  {
    timeClient.begin(); //NTP time
    timeClient.update();

    if ((Clock.getHour(h12Flag, pmFlag) != timeClient.getHours()) || (Clock.getMinute() != timeClient.getMinutes()))
      getTimeStampString();
  }
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  RELAY1OFF;
  RELAY2OFF;
  RELAY3OFF;
  RELAY4OFF;
  delay(2000);
}

void loop()
{
  ArduinoOTA.handle();
  display.clearDisplay();

  showWifiSignal(); //at 112,0
  showTime();
  if (RELAY1)
  {
    checkTimeFor RELAY1OnOff;
  }
  if (RELAY2)
  {
    checkTimeFor RELAY2OnOff;
  }
  if (RELAY3)
  {
    checkTimeFor RELAY3OnOff;
  }
  if (RELAY4)
  {
    checkTimeFor RELAY4OnOff;
  }
  display.display();
}

void showWifiSignal()
{
  int x = WiFi.RSSI();
  if (WiFi.status() != WL_CONNECTED)
  {
    display.drawBitmap(112, 0, wifiNo, 16, 16, WHITE);
  }
  else
  {
    if (x <= (-80))
    {
      display.drawBitmap(112, 0, wifiLow, 16, 16, WHITE); //worst signal
    }
    else if ((x <= (-70)) && (x > (-80)))
    {
      display.drawBitmap(112, 0, wifiHalf, 16, 16, WHITE); //poor signal
    }
    else if ((x <= (-60)) && (x > (-70))) //good signal
    {
      display.drawBitmap(112, 0, wifiMed, 16, 16, WHITE); //best signal
    }
    else if (x > (-60))
    {
      display.drawBitmap(112, 0, wifiFull, 16, 16, WHITE); //excellent signal
    }
  }
}

void showTime()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (Clock.getHour(h12Flag, pmFlag) < 10)
    display.print(0, DEC);

  display.print(Clock.getHour(h12Flag, pmFlag)); //Time
  display.print(":");
  if (Clock.getMinute() < 10)
    display.print(0, DEC);
  display.print(Clock.getMinute());
  display.print(":");
  if (Clock.getSecond() < 10)
    display.print(0, DEC);
  display.println(Clock.getSecond());

  display.setTextSize(1);
  display.print(week[Clock.getDoW()]);
  display.print(", ");
  display.print(Clock.getDate());
  display.print("-");
  display.print(Clock.getMonth(century));
  display.print("-20");
  display.println(Clock.getYear());

  display.setCursor(90, 25);
  display.print(Clock.getTemperature(), 1);
  display.println(" C");
}

void getTimeStampString()
{
  timeClient.update();
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);

  uint16_t year = ti->tm_year + 1900;
  uint8_t x = year % 10;
  year = year / 10;
  uint8_t y = year % 10;
  year = y * 10 + x;

  uint8_t month = ti->tm_mon + 1;

  uint8_t day = ti->tm_mday;

  uint8_t hours = ti->tm_hour;

  uint8_t minutes = ti->tm_min;

  uint8_t seconds = ti->tm_sec;

  uint8_t dow = ti->tm_wday;

  Clock.setClockMode(false); // set to 24h
  //setClockMode(true); // set to 12h

  Clock.setYear(year);
  Clock.setMonth(month);
  Clock.setDate(day);
  Clock.setDoW(dow);
  Clock.setHour(hours);
  Clock.setMinute(minutes);
  Clock.setSecond(seconds);
}

void checkTimeFor(byte hOn, byte mOn, byte sOn, byte hOff, byte mOff, byte sOff, byte number)
{
  byte h = Clock.getHour(h12Flag, pmFlag);
  byte m = Clock.getMinute();
  byte s = Clock.getSecond();
  display.setTextSize(2);

  if (h == hOn && m == mOn && s == sOn)
  {
    if (number == 1)
    {
      display.setCursor(0, 35);
      display.print("R1: ON");
      RELAY1ON;
    }
    if (number == 2)
    {
      display.setCursor(15, 35);
      display.print("R2: ON");
      RELAY2ON;
    }
    if (number == 3)
    {
      display.setCursor(30, 35);
      display.print("R3: ON");
      RELAY3ON;
    }
    if (number == 4)
    {
      display.setCursor(45, 35);
      display.print("R4: ON");
      RELAY4ON;
    }
  }

  if (h == hOff && m == mOff && s == sOff)
  {
    if (number == 1)
    {
      display.setCursor(0, 35);
      display.print("R1: OFF");
      RELAY1OFF;
    }
    if (number == 2)
    {
      display.setCursor(15, 35);
      display.print("R2: OFF");
      RELAY2OFF;
    }
    if (number == 3)
    {
      display.setCursor(30, 35);
      display.print("R3: OFF");
      RELAY3OFF;
    }
    if (number == 4)
    {
      display.setCursor(45, 35);
      display.print("R4: OFF");
      RELAY4OFF;
    }
  }
}
