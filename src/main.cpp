#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const long utcOffsetInSeconds = 3 * 60 * 60;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", utcOffsetInSeconds);

// This defines the LCD wiring to the DIGITALpins
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "FlyButter";
const char *password = "t0rT01se";

// initial Time display is 12:59:45 PM
int h;
int m;
int s;

// Backlight Time Out
const int backlightTimeout = 30 * 5;
int backlightTimer = backlightTimeout; // Backlight Time-Out

// Backlight button
const int buttonPin = D5;

// Network request timeout
const int net_timeout = 60 * 60 * 24; // seconds
int net_timer = net_timeout;

// For accurate Time reading, use Arduino Real Time Clock and not just delay()
static uint32_t last_time, now = 0; // RTC

void updateTime();
void printTime();
void incrementTime();
void checkBacklight();

void setup()
{
  lcd.init();
  lcd.backlight();
  WiFi.begin(ssid, password);
  lcd.print("Connecting.");
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    dots++;
    for (int i = 0; i < dots; ++i)
      lcd.print(".");
    if (dots == 3)
    {
      lcd.clear();
      lcd.print("Connecting.");
      dots = 0;
    }
  }
  lcd.clear();
  lcd.print("Connected!");
  timeClient.begin();
  pinMode(buttonPin, INPUT);
  now = millis(); // read RTC initial value
}

void loop()
{
  if (net_timer == net_timeout)
  {
    updateTime();
    net_timer = 0;
  }

  printTime();

  for (int i = 0; i < 5; i++) // make 5 time 200ms loop, for faster Button response
  {

    do
    {
      now = millis();
    } while ((now - last_time) < 200); //delay200ms

    // inner 200ms loop
    last_time = now; // prepare for next loop

    checkBacklight();
  }
  // outer 1000ms loop

  incrementTime();

  // Loop end
}

void checkBacklight()
{
  //Backlight time out
  backlightTimer -= backlightTimer < 0 ? 0 : 1;
  if (!backlightTimer)
  {
    lcd.noBacklight(); // Backlight OFF
  }

  // Hit button to activate Backlight
  if (backlightTimer <= 0 && digitalRead(buttonPin) == HIGH)
  {
    backlightTimer = backlightTimeout;
    lcd.backlight();
  }
}

void incrementTime()
{

  s++; //increment sec. counting

  // ---- manage seconds, minutes, hours am/pm overflow ----
  if (s == 60)
  {
    s = 0;
    m++;
  }
  if (m == 60)
  {
    m = 0;
    h++;
  }
  if (h == 24)
  {
    h = 0;
  }
  net_timer++;
}

void printTime()
{
  lcd.setCursor(0, 0);
  lcd.print("Time ");
  if (h < 10)
    lcd.print("0"); // always 2 digits
  lcd.print(h);
  lcd.print(":");
  if (m < 10)
    lcd.print("0");
  lcd.print(m);
  lcd.print(":");
  if (s < 10)
    lcd.print("0");
  lcd.print(s);
  lcd.setCursor(0, 1);
  lcd.print("NTP clock");
}

void updateTime()
{
  timeClient.update();
  h = timeClient.getHours();
  m = timeClient.getMinutes();
  s = timeClient.getSeconds();
}