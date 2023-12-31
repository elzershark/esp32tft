// elzershark_TFT Display ESP32-2432S028R
// https://www.elzershark.com/forum/index.php?thread/201 <-- Anleitung im Forum
//
// https://www.barth-dev.de/online/rgb565-color-picker/ gelb = 255,255,0 = 0xFFE0; 
// https://calculator.name/baseconvert/hexadecimal/decimal/0xFFE0 = 65504;
// TFT_YELLOW = 65504 
//测试通过
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include <vector>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include "ESP32_SPI_9341.h"
#include "EspMQTTClient.h"

// DEBUG ein/ausschalten
//#define DEBUG

#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5

#define LIGHT_ADC 34

int led_pin[3] = {17, 4, 16};

int pos[2] = {0, 0};

String pox;
String poy;
String oldfs = "0";
String oldtext = "0;0;0;0;0; ";
String oldtext1 = "0;0;0;0;0; ";
String oldtext2 = "0;0;0;0;0; ";
// MQTT parameters
char smqttServer[20];
char smqttUser[20];
char smqttPwd[20];
char smqttPort[10];
char smqttName[20] = "esp32tft";
// Set web server port number to 80
WiFiServer server(80);
//flag for saving data
bool shouldSaveConfig = false;

EspMQTTClient client;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

SPIClass SD_SPI;

LGFX lcd;

void setup(void)
{
    Serial.begin(115200);
    lcd.init();
      uint16_t calData[] = {3749, 3619, 3737, 207, 361, 3595, 267, 221};
    lcd.setTouchCalibrate(calData);
      Serial.begin(115200);

  //read configuration from FS json
  #ifdef DEBUG 
    Serial.println("mounting FS...");
  #endif
  
  if (SPIFFS.begin()) {
    #ifdef DEBUG
      Serial.println("mounted file system");
    #endif
    
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      #ifdef DEBUG 
        Serial.println("reading config file");
      #endif
      
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        #ifdef DEBUG 
          Serial.println("opened config file");
        #endif
        
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        #ifdef DEBUG 
          
        #endif

        if ( ! deserializeError ) {
          #ifdef DEBUG 
            Serial.println("\nparsed json");
          #endif
          strcpy(smqttServer, json["mqttServer"]);
          strcpy(smqttUser, json["mqttUser"]);
          strcpy(smqttPwd, json["mqttPwd"]);
          strcpy(smqttPort, json["mqttPort"]);
          strcpy(smqttName, json["mqttName"]);
        } else {
          #ifdef DEBUG 
            Serial.println("failed to load json config");
          #endif
        }
      }
    }	
  } else {
    Serial.println("failed to mount FS. Please wait...");
    SPIFFS.format();
    Serial.println("Restarting in 5 seconds");	
    delay(5000);
    ESP.restart();
    #ifdef DEBUG
      Serial.println("failed to mount FS");
    #endif
  }
  //end read
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFi.hostname(smqttName);
      #ifdef DEBUG 
  Serial.println(smqttName);
    #endif
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  WiFiManagerParameter custom_mqttServer("server", "MQTT Server", smqttServer, 20);
  WiFiManagerParameter custom_mqttUser("user", "MQTT Username", smqttUser, 20);
  WiFiManagerParameter custom_mqttPwd("password", "MQTT Password", smqttPwd, 20);
  WiFiManagerParameter custom_mqttPort("port", "MQTT Port", smqttPort, 10);
  WiFiManagerParameter custom_mqttName("name", "MQTT and Host Name", smqttName, 20);
  wifiManager.addParameter(&custom_mqttServer);
  wifiManager.addParameter(&custom_mqttUser);
  wifiManager.addParameter(&custom_mqttPwd);
  wifiManager.addParameter(&custom_mqttPort);
  wifiManager.addParameter(&custom_mqttName);  
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  
  // if you get here you have connected to the WiFi
  #ifdef DEBUG
    Serial.println("Connected.");
  #endif
    // Copy parameters into variables
  strcpy(smqttServer, custom_mqttServer.getValue());
  strcpy(smqttUser, custom_mqttUser.getValue());
  strcpy(smqttPwd, custom_mqttPwd.getValue());
  strcpy(smqttPort, custom_mqttPort.getValue());
  strcpy(smqttName, custom_mqttName.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    #ifdef DEBUG 
      Serial.println("The values in the file are: ");
      Serial.println("\smqttServer : " + String(smqttServer));
      Serial.println("\smqttUser : " + String(smqttUser));
      Serial.println("\smqttPwd : " + String(smqttPwd));
      Serial.println("\smqttPort : " + String(smqttPort));
      Serial.println("\smqttName : " + String(smqttName));      
      Serial.println("saving config");
    #endif

    DynamicJsonDocument json(1024);
    json["mqttServer"] = smqttServer;
    json["mqttUser"] = smqttUser;
    json["mqttPwd"] = smqttPwd;
    json["mqttPort"] = smqttPort;
    json["mqttName"] = smqttName;    
    File configFile = SPIFFS.open("/config.json", "w");
    #ifdef DEBUG
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
    #endif
    
    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    //end save
  }
  

	// MQTT
  #ifdef DEBUG
    Serial.println("Starting MQTT config!");
  #endif
  
  client.setMqttClientName(smqttName);

  if (strlen(smqttUser) > 0) {
    #ifdef DEBUG
      Serial.println("\smqttServer : " + String(smqttServer));
      Serial.println("\smqttUser : " + String(smqttUser));
      Serial.println("\smqttPwd : " + String(smqttPwd));
      Serial.println("\smqttPort : " + String(smqttPort));
      Serial.println("\smqttName : " + String(smqttName));     
    #endif 
    client.setMqttServer(smqttServer, smqttUser, smqttPwd, atoi(smqttPort));
  } else {
    #ifdef DEBUG
      Serial.println("MQTT config without username.");
    #endif 
    client.setMqttServer(smqttServer);
  }
	client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  #ifdef DEBUG
    client.enableDebuggingMessages();
  #endif

  #ifdef DEBUG
    Serial.println("MQTT client initialized!");
    Serial.println(client.getMqttClientName());
    Serial.println(client.getMqttServerIp());
  #endif

}

static int colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW};

int i = 0;
long runtime_0 = 0;
long runtime_1 = 0;



void onConnectionEstablished()
{

// Ausrichtung Display
//WiFi.macAddress()
  client.subscribe(String(smqttName) + "/rotation", [](const String & payload) {
    if (payload.length() > 0) {
    lcd.setRotation(atoi(payload.c_str()));
    }});
    
// Hintergrundfarbe
    client.subscribe(String(smqttName) + "/fillScreen", [](const String & payload) {
    if (payload.length() > 0) {
    lcd.fillScreen(atoi(payload.c_str()));
    oldfs = payload;
     }});

// Helligkeit
    client.subscribe(String(smqttName) + "/Brightness", [](const String & payload) {
    if (payload.length() > 0) {
    lcd.setBrightness(atoi(payload.c_str()));
    
    }});

// Textfarbe
    client.subscribe(String(smqttName) + "/Textfarbe", [](const String & payload) {
    if (payload.length() > 0) {
    lcd.setTextColor(atoi(payload.c_str()));
        
    }});

// Flächenfüller
    client.subscribe(String(smqttName) + "/fillRect", [](const String & payload) {
    if (payload.length() > 0) {

    char *filler[5];
    filler[0]=strtok((char*)payload.c_str(),",");
 
    for (int iiii=1;iiii<5;iiii++) filler[iiii]=strtok(NULL,",");
    lcd.fillRect(atoi(filler[0]),atoi(filler[1]),atoi(filler[2]),atoi(filler[3]),atoi(filler[4])); 
    }});

// Button Rund
    client.subscribe(String(smqttName) + "/fillRoundRect", [](const String & payload) {
    if (payload.length() > 0) {

    char *fillRoundRect[6];
    fillRoundRect[0]=strtok((char*)payload.c_str(),",");
 
    for (int iiiii=1;iiiii<6;iiiii++) fillRoundRect[iiiii]=strtok(NULL,",");
    lcd.fillRoundRect(atoi(fillRoundRect[0]),atoi(fillRoundRect[1]),atoi(fillRoundRect[2]),atoi(fillRoundRect[3]),atoi(fillRoundRect[4]),atoi(fillRoundRect[5])); 
    }});

// Button Rund Rand
    client.subscribe(String(smqttName) + "/drawRoundRect", [](const String & payload) {
    if (payload.length() > 0) {

    char *drawRoundRect[6];
    drawRoundRect[0]=strtok((char*)payload.c_str(),",");
 
    for (int iiiiii=1;iiiiii<6;iiiiii++) drawRoundRect[iiiiii]=strtok(NULL,",");
    lcd.drawRoundRect(atoi(drawRoundRect[0]),atoi(drawRoundRect[1]),atoi(drawRoundRect[2]),atoi(drawRoundRect[3]),atoi(drawRoundRect[4]),atoi(drawRoundRect[5])); 
    }});

// Der Text
  client.subscribe(String(smqttName) + "/text", [](const String & payload) {
    
    if (payload.length() > 0) {

  char *teiler[6];
  teiler[0]=strtok((char*)oldtext.c_str(),";");
 
  for (int i=1;i<6;i++) teiler[i]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler[0]));
      lcd.setCursor(atoi(teiler[2]),atoi(teiler[3]));
      lcd.setTextSize(atoi(teiler[4]));
      lcd.println((const char*)teiler[5]);

      oldtext = payload;

  teiler[0]=strtok((char*)payload.c_str(),";");

  for (int i=1;i<6;i++) teiler[i]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler[1]));
      lcd.setCursor(atoi(teiler[2]),atoi(teiler[3]));
      lcd.setTextSize(atoi(teiler[4]));
      lcd.println((const char*)teiler[5]);
   }});

   
// Der Text1
  client.subscribe(String(smqttName) + "/text1", [](const String & payload) {
    
    if (payload.length() > 0) {
  
  char *teiler1[6];
  teiler1[0]=strtok((char*)oldtext1.c_str(),";");
 
  for (int ii=1;ii<6;ii++) teiler1[ii]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler1[0]));
      lcd.setCursor(atoi(teiler1[2]),atoi(teiler1[3]));
      lcd.setTextSize(atoi(teiler1[4]));
      lcd.println((const char*)teiler1[5]);

      oldtext1 = payload;

  teiler1[0]=strtok((char*)payload.c_str(),";");

  for (int ii=1;ii<6;ii++) teiler1[ii]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler1[1]));
      lcd.setCursor(atoi(teiler1[2]),atoi(teiler1[3]));
      lcd.setTextSize(atoi(teiler1[4]));
      lcd.println((const char*)teiler1[5]);
   }});

   
// Der Text2
  client.subscribe(String(smqttName) + "/text2", [](const String & payload) {
    
    if (payload.length() > 0) {
  
  char *teiler2[6];
  teiler2[0]=strtok((char*)oldtext2.c_str(),";");
 
  for (int iii=1;iii<6;iii++) teiler2[iii]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler2[0]));
      lcd.setCursor(atoi(teiler2[2]),atoi(teiler2[3]));
      lcd.setTextSize(atoi(teiler2[4]));
      lcd.println((const char*)teiler2[5]);

      oldtext2 = payload;

  teiler2[0]=strtok((char*)payload.c_str(),";");

  for (int iii=1;iii<6;iii++) teiler2[iii]=strtok(NULL,";");

      lcd.setTextColor(atoi(teiler2[1]));
      lcd.setCursor(atoi(teiler2[2]),atoi(teiler2[3]));
      lcd.setTextSize(atoi(teiler2[4]));
      lcd.println((const char*)teiler2[5]);
   }});

   //lcd.fillRect(60, 100, 120, 120, TFT_BLACK);





  // Execute delayed instructions
//  client.executeDelayed(5 * 1000, []() {
  //  client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
 // });
}


void loop(void)
{
      if ((millis() - runtime_1) > 5000)
    {
        int adc_value = analogRead(LIGHT_ADC);
    client.publish(String(smqttName) + "/ADC", (const String)adc_value);
  #ifdef DEBUG
        Serial.printf("ADC:%d\n", adc_value);
  #endif
        runtime_1 = millis();
    }
    delay(10);

     if (lcd.getTouch(&pos[0], &pos[1])) {  
         delay(100);
  #ifdef DEBUG
         Serial.println("po0");
         Serial.println(pos[0]);
         Serial.println("po1");
         Serial.println(pos[1]);
  #endif
        pox = pos[0];
        poy = pos[1];
    client.publish(String(smqttName) + "/PosX", pox);
    client.publish(String(smqttName) + "/PosY", poy);
       }
    
  client.loop();
}

void sd_init()
{
    SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
    if (!SD.begin(SD_CS, SD_SPI, 40000000))
    {
        Serial.println("Card Mount Failed");
        lcd.setCursor(10, 10);
        lcd.println("SD Card Failed");
        while (1)
            delay(1000);
    }
    else
    {
        Serial.println("Card Mount Successed");
    }

    Serial.println("SD init over.");
}

void sd_test()
{
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    listDir(SD, "/", 0);
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

// Display image from file
int print_img(fs::FS &fs, String filename, int x, int y)
{

    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = x;
    int Y = y;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);

        lcd.pushImage(0, row, X, 1, (lgfx::rgb888_t *)RGB);
    }

    f.close();
    return 0;
}

void touch_calibration()
{
    lcd.fillScreen(TFT_YELLOW);

    lcd.setTextColor(TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(70, 110);
    lcd.println("SCREEN");
    lcd.setCursor(70, 150);
    lcd.println("CALIBRATION");

    // タッチを使用する場合、キャリブレーションを行います。画面の四隅に表示される矢印の先端を順にタッチしてください。
    std::uint16_t fg = TFT_WHITE;
    std::uint16_t bg = TFT_BLACK;
    if (lcd.isEPD())
        std::swap(fg, bg);
        
    lcd.calibrateTouch(nullptr, fg, bg, std::max(lcd.width(), lcd.height()) >> 3);
    Serial.println(fg);
    Serial.println(bg);
    Serial.println(std::max(lcd.width(), lcd.height()));
}

void touch_continue()
{
 //   lcd.fillScreen(TFT_YELLOW);
 //   lcd.setTextColor(TFT_BLUE);
 //   lcd.setTextSize(2);
 //   lcd.fillRect(60, 100, 120, 120, TFT_BLACK);
 //   lcd.setCursor(70, 110);
 //   lcd.println(" TOUCH");
 //   lcd.setCursor(70, 130);
 //   lcd.println("  TO");
 //   lcd.setCursor(70, 150);
 //   lcd.println("CONTINUE");

    int pos[2] = {0, 0};

    while (1)
    {

        if (lcd.getTouch(&pos[0], &pos[1]))
        {
//            if (pos[0] > 1054 && pos[0] < 173 && pos[1] > 17 && pos[1] < 65)
//                break;
            delay(100);
            Serial.println("po0");
            Serial.println(pos[0]);
            Serial.println("po1");
            Serial.println(pos[1]);
        }
    }
}

void led_set(int i)
{
    if (i == 0)
    {
        digitalWrite(led_pin[0], LOW);
        digitalWrite(led_pin[1], HIGH);
        digitalWrite(led_pin[2], HIGH);
    }
    if (i == 1)
    {
        digitalWrite(led_pin[0], HIGH);
        digitalWrite(led_pin[1], LOW);
        digitalWrite(led_pin[2], HIGH);
    }
    if (i == 2)
    {
        digitalWrite(led_pin[0], HIGH);
        digitalWrite(led_pin[1], HIGH);
        digitalWrite(led_pin[2], LOW);
    }

    if (i == 3)
    {
        digitalWrite(led_pin[0], LOW);
        digitalWrite(led_pin[1], LOW);
        digitalWrite(led_pin[2], LOW);
    }
}