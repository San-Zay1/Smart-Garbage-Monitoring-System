#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "24987_ClassicTech";
const char *password = "499_ClassicT";
WiFiUDP ntpUDP; //for time
NTPClient timeClient(ntpUDP);
int pm;

// rfid
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
const int buzzer = D8;            // the number of the buzzer pin
String uidString, payload;                 //tag uid to be sent to server

//ifttt
String key = "iD7YT_7KiAZde55T5p-7IPdod5z7swB96A_D8QFA23d"; //your webhooks key
String event_name = "studentsecuritysystem";

WiFiServer server(80);
void readCard(); //function declaration
void verifyStudent();
void invalidCard();
void validCard();
void sendData();
void sms();

void setup()
{
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  lcd.backlight();
  lcd.init();
  lcd.home();
  lcd.print("SMART ATTENDANCE");
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println(WiFi.localIP()); // Print the IP address
  timeClient.begin();
  timeClient.setTimeOffset(20700);
}

void loop()
{
  timeClient.update();                           //updating time
  Serial.println(timeClient.getFormattedTime()); //printing time
  Serial.print("\t");
  Serial.print("Swipe card   ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(timeClient.getFormattedTime());
  lcd.setCursor(0, 1);
  lcd.print("Swipe your card");
  delay(500);
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  // Dump debug info about the card. PICC_HaltA() is automatically called.
  // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  readCard();
}

void readCard()
{
  mfrc522.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(mfrc522.uid.uidByte[0]) + " " + String(mfrc522.uid.uidByte[1]) + " " + String(mfrc522.uid.uidByte[2]) + " " + String(mfrc522.uid.uidByte[3]);
  Serial.println(uidString);
  sendData();
  delay(500);
}

void invalidCard()
{
  //lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(" INVALID CARD  ");

  digitalWrite(buzzer, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                 // wait for a second
  digitalWrite(buzzer, LOW);  // turn the LED off by making the voltage LOW
  delay(100);                 // wait for a second
  digitalWrite(buzzer, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                 // wait for a second
  digitalWrite(buzzer, LOW);  // turn the LED off by making the voltage LOW
  delay(100);                 // wait for a second
  digitalWrite(buzzer, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                 // wait for a second
  digitalWrite(buzzer, LOW);  // turn the LED off by making the voltage LOW
  delay(100);                 // wait for a second
  digitalWrite(buzzer, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                 // wait for a second
  digitalWrite(buzzer, LOW);  // turn the LED off by making the voltage LOW
  delay(100);                 // wait for a second
}

void validCard()
{
  //lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("WELCOME  IICian");
  digitalWrite(buzzer, 1);
  delay(150);
  digitalWrite(buzzer, 0);
  delay(50);
  digitalWrite(buzzer, 1);
  delay(150);
  digitalWrite(buzzer, 0);
  delay(50);
}

void verifyStudent()
{
}

void sendData()
{
  HTTPClient http; //Declare object of class HTTPClient
  //strings to send== status, lat_str, lng_str, quality;

  String postData;

  //Post Data
  postData = "sensor1=" + uidString;
  Serial.print(F("Performing HTTP post request... "));

  http.begin("http://192.168.254.3/iottest/attendance.php");           //change the ip to your computer ip address
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData);                                  //Send the request
  payload = http.getString();                                   //Get the response payload

  Serial.println(httpCode); //Print HTTP return code
  Serial.println(payload);
  if (payload == "Invalid Card")
    invalidCard();
  else{
        validCard();
        sms();
  }
  delay(2000);
  http.end(); //Close connection
}

void sms()
{
  HTTPClient http;
  http.begin("http://maker.ifttt.com/trigger/" + event_name + "/with/key/" + key+"?value1="+ payload);
  http.GET();
  http.end();
  Serial.print("done");
