/*
 * LiquidMenu library - serial_menu.ino
 *
 * This example uses the serial communication to execute commands.
 *
 * The available commands will be printed on the serial monitor.
 * This example uses attached callback functions which are explained
 * in the functions_menu.ino example.
 *
 * The circuit:
 * https://github.com/VasilKalchev/LiquidMenu/blob/master/examples/B_serial_menu/serial_menu.png
 * - LCD RS pin to Arduino pin 12
 * - LCD E pin to Arduino pin 11
 * - LCD D4 pin to Arduino pin 5
 * - LCD D5 pin to Arduino pin 4
 * - LCD D6 pin to Arduino pin 3
 * - LCD D7 pin to Arduino pin 2
 * - LCD R/W pin to ground
 * - LCD VSS pin to ground
 * - LCD VDD pin to 5V
 * - 10k ohm potentiometer: ends to 5V and ground, wiper to LCD V0
 * - 150 ohm resistor from 5V to LCD Anode
 * - LCD Cathode to ground
 *
 * Created July 24, 2016
 * by Vasil Kalchev
 *
 * https://github.com/VasilKalchev/LiquidMenu
 *
 */

#include <LiquidCrystal.h>
#include <LiquidMenu.h>
#include <Keypad.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#include "cert.h"



const char *ssid = "vince"; // Enter your WiFi name
const char *password = "123456789";  // Enter WiFi password
String client_id = "vending_test";

#define red_led     13
#define yellow_led  2
#define green_led   15
#define relay_pin   12



//firmware update declars
#define LED_BUILTIN   2

String FirmwareVer = {
  "2.1"
};
#define URL_fw_Version "https://songatechnologies.co.ke/version.txt"
#define URL_fw_Bin "https://songatechnologies.co.ke/OTA_firmware_update.bin"

//#define URL_fw_Version "http://cade-make.000webhostapp.com/version.txt"
//#define URL_fw_Bin "http://cade-make.000webhostapp.com/firmware.bin"

void connect_wifi();
void firmwareUpdate();
int FirmwareVersionCheck();

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;



// MQTT Broker
const char *mqtt_broker = "s19165fb.ala.eu-central-1.emqxsl.com";// broker address
const char *subscribe_topic = "adevice_no_1/receive"; // define topic
const char *publish_topic = "device_no_1/send" ;
const char *mqtt_username = "aquawell"; // username for authentication
const char *mqtt_password = "1234";// password for authentication
const int mqtt_port = 8883;// port of MQTT over TLS\SSL


const char* ca_cert= \
"-----BEGIN CERTIFICATE-----\n"
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
"-----END CERTIFICATE-----\n";


byte wifi_connected[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111
};
byte wifi_disconnected[8] = {
  0b10100,
  0b01000,
  0b10100,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111
};
byte valve_closed[8] = {
  0b11100,
  0b11100,
  0b11101,
  0b11111,
  0b10101,
  0b10100,
  0b10100,
  0b10100

};

byte valve_opened[8] = {
  0b11100,
  0b11100,
  0b11101,
  0b11111,
  0b11101,
  0b11100,
  0b11100,
  0b11100

};
byte server_ok[8] = {
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b00100,
  0b11111

};
byte server_error[8] = {

  0b11111,
  0b10101,
  0b10101,
  0b10001,
  0b10101,
  0b11011,
  0b00100,
  0b11111

};

WiFiClientSecure espClient;
PubSubClient client(espClient);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
 String number;

byte rowPins[ROWS] = {23, 26, 3, 21}; 
byte colPins[COLS] = {19, 18, 5, 17};

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 



// Pin mapping for the display
const byte LCD_RS = 33;
const byte LCD_E = 22;
const byte LCD_D4 = 27;
const byte LCD_D5 = 14;
const byte LCD_D6 = 16;
const byte LCD_D7 = 4;
//LCD R/W pin to ground
//10K potentiometer wiper to VO
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);


LiquidLine welcome_line1(1, 1, "");
LiquidLine welcome_line2(1, 2, "Starting");

LiquidScreen welcome_screen(welcome_line1, welcome_line2);
LiquidMenu start_menu(lcd, welcome_screen);


LiquidLine line21(4, 1, "CASH");
LiquidLine type_line(4, 2, "MPESA");
LiquidScreen pay(line21, type_line);
LiquidMenu pay_menu(lcd, pay);


;

LiquidSystem menu_system(start_menu, pay_menu);



void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version:");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
       Serial.println("wifi connected");
   }
   else
   {
    connect_wifi();
   }
  }
}


/*
 * This function will be attached to one or more LiquidLine
 * objects and will be called when the line is focused and
 * bool LiquidMenu::call_function(byte number) is called with
 * number - the number specified when attaching the function.
 * More on this in the function_menu.ino example.
*/
void reconnect(){
  if(client.connect(client_id.c_str(), mqtt_username, mqtt_password)){
    digitalWrite(yellow_led, HIGH);
  }
}

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}


void callback(char *topic, byte *payload, unsigned int length) {
  
  
}

void callback_function() {
  Serial.println(F("You called the callback function."));
}


void clr(int x){

    lcd.setCursor(0, x);
    lcd.print("                    ");
    vTaskDelay(20/portTICK_PERIOD_MS);
}


void goToPayMenu() {
  Serial.println("pay menu was called");
  menu_system.change_menu(pay_menu);
}

void go_to_cash_amount() {
  Serial.println("cash amount called");
  lcd.setCursor(0, 1);
  lcd.print("Amount: (KShs)");
  lcd.setCursor(0, 2);
  lcd.print("                  ");
  lcd.setCursor(4, 2);
  int cursor_pos;
  int numberInput= 0;
  lcd.blink();

  
  while(1){
  char key = customKeypad.getKey();

  if(key){
    Serial.println(key);
    if (key >= '0' && key <= '9') {
      // Convert key to integer and add to current input
      int digit = String(key).toInt();
      numberInput = numberInput * 10 + digit;
      lcd.setCursor(4, 2);
       lcd.blink();
      lcd.print(numberInput);
      cursor_pos++;
        
      vTaskDelay(100/portTICK_PERIOD_MS);
      
    }
    else if(key == '*'){
      numberInput = 0;
      lcd.setCursor(4, 2);
      clr(2);
      lcd.setCursor(4, 2);
       
    }
    else if(key == '#'){
      numberInput = 0;
      menu_system.change_menu(pay_menu);
      lcd.noBlink();
      break;
      
      
    }
    else if (key== 'D'){
      lcd.noBlink();
      go_to_vending(numberInput);
      //menu_system.change_menu(pay_menu);
     //go restart this menu
     go_to_cash_amount();
    }

  }
  vTaskDelay(100/portTICK_PERIOD_MS);
}
}

void go_to_vending(int amount){
    clr(1);
    clr(2);
    lcd.setCursor(0, 1);
    lcd.print("dispensing...");
    lcd.setCursor(0, 2);
    lcd.print(amount);
    lcd.setCursor(15, 2);
    int ml = amount*16.66;
    int on_time = (ml*30);
    int then = millis();
    digitalWrite(relay_pin, HIGH);
    digitalWrite(green_led, HIGH);
    lcd.print(ml);
    
  while(1){
    Serial.println(millis());
    int now = millis()-then;
    char key = customKeypad.getKey();
    int now_ml = ((now/1000)*33.33);
    lcd.setCursor(5,3);
    lcd.print(now_ml);
    if (now >= on_time){
      digitalWrite(relay_pin, LOW);
      digitalWrite(green_led, LOW);
      clr(3);
      break;
    }
    if (key == '#'){
      break;
    }
    
    vTaskDelay(2/portTICK_PERIOD_MS);
  }
}


void setup() {

  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  lcd.begin(20, 4);
  
  lcd.createChar(1, wifi_connected);
  lcd.createChar(2, wifi_disconnected);
  lcd.createChar(3, valve_closed);
  lcd.createChar(4, valve_opened);
  lcd.createChar(8, server_ok);
  lcd.createChar(9, server_error);

  lcd.setCursor(15, 0);
  lcd.write(3);
  
  Serial.begin(115200);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
  pinMode(LED_BUILTIN, OUTPUT);
  connect_wifi();

  // Here we attach the function defined earlier to four LiquidLine objects.
  line21.attach_function(1, go_to_cash_amount);

  menu_system.set_focusPosition(Position::LEFT);
  menu_system.update();

  for(int x = 0; x<3; x++){
    int y =9+x;
    lcd.setCursor(y, 2);
    lcd.print(".");
    delay(100);
  
  }
digitalWrite(red_led, HIGH);

menu_system.change_menu(pay_menu);

  WiFi.begin(ssid, password);
//connecting to a mqtt broker
 espClient.setCACert(ca_cert);
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);

    xTaskCreate(vending_task, // Task function
              "mqtt client loop", // Task name
              4096, // Stack size 
              NULL, 
              2, // Priority
              NULL); // Task handler
    xTaskCreate(connection_task, // Task function
              "mqtt client loop", // Task name
              4096, // Stack size 
              NULL, 
              1, // Priority
              NULL); // Task handler
}

void vending_task(void *pvParameters) {  
  (void)pvParameters;
  while(1){
     char Key = customKeypad.getKey();
  // This handles the serial input.
  if (Key) {

    if (Key == 'A') {
      Serial.println(Key);
      menu_system.next_screen();
    }
    else if (Key == 'B') {
      Serial.println(Key);
      menu_system.previous_screen();
    }
    else if (Key == 'C') {
      Serial.println(Key);
      menu_system.switch_focus(); // switches focus to the next line
      // focus is used with functions (see 'functions_menu' example)
    }
    else if (Key == '1') {
      Serial.println(Key);
      // The attached function is called.
      menu_system.call_function(1);
    }

    else if (Key == 'D') {
      Serial.println(Key);
      menu_system.call_function(1);
 
    }
    else {
      Serial.print(Key); Serial.println(F(" - unknown Key"));
    };
    menu_system.update();
  }
  if(!client.connected()){
      lcd.setCursor(16,0);
      lcd.write(9);
      if(WiFi.status() != WL_CONNECTED){
       lcd.setCursor(18,0);
         lcd.write(2);
      }
      else{
        lcd.setCursor(18,0);
         lcd.write(1);
      }
    }
    else{
      lcd.setCursor(16,0);
      lcd.write(8);
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
  vTaskDelay(50/portTICK_PERIOD_MS);
}
///
  
void connection_task(void *pvParameters){
  (void)pvParameters;
  while(1){
    if(!client.connected()){
        digitalWrite(yellow_led, LOW);
        

      if(WiFi.status() != WL_CONNECTED){
        digitalWrite(yellow_led, LOW);
        
      }
      else{
        void reconnect();
      }
    }
    else if(client.connected()){
      digitalWrite(yellow_led, HIGH);
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void loop() {
  
  
}
