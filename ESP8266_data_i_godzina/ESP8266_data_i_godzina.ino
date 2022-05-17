#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// for no-ip
#include <EasyDDNS.h>
//#include <ESP8266HTTPClient.h>

//edit
//for ota
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//for d2d
#include <Math.h>
#include <Dusk2Dawn.h>

Dusk2Dawn Gliwice(50.2833, 18.6667, +2);

float PROGRAM_VERSION = 8.1;

// Replace with your network credentials
const char *ssid     = "pozdrawiam";
const char *ssid1     = "pozdrawiam2";
const char *ssid2     = "pozdrawiam_plus";
const char *ssid3     = "osiek";

const char *password = "osiekrulz";

const int ms = 10;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//WifiServer

int port=300;
WiFiServer server(port);

//Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String request;

int aktywacja = 999999;
int warAktywacji = 5 * 60 /*sekund*/ / (ms / 1000.0) ;
unsigned int x_times_up = 0, x_times_down = 0;
bool debug = 0;

/******************************************SETUP*************************************/
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //PINY DO UP DOWN
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);//OTA

  /* jak fryzjer to tutaj zmienić na stałe ip */
  /* PAMIĘTAĆ */
  Serial.println(WiFi.localIP());
   
  WiFi.begin(ssid, password);
  //
  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }*/
    /*************************************wifi connection **************/
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    //ESP.restart();
  }
  //OTA

  // No authentication by default
  ArduinoOTA.setPassword("home");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //END OF OTA

  //Wifi Server Startup
  if(WiFi.localIP()[3] == 36){
    ArduinoOTA.setHostname("esp8266_mateusz");
    port=302;
  }else if(WiFi.localIP()[3]==49){
    ArduinoOTA.setHostname("esp8266_lauba");
    port=303;
  }else if(WiFi.localIP()[3]==50){
    ArduinoOTA.setHostname("esp8266_50");
    port=304;
  }else if(WiFi.localIP()[3]==64){
    ArduinoOTA.setHostname("esp8266_kuchnia");
    port=305;
  }else if(WiFi.localIP()[3]==67){
    ArduinoOTA.setHostname("esp8266_fryzjer");
    port=300;
  }else if(WiFi.localIP()[3]==70){
    ArduinoOTA.setHostname("esp8266_salon");
    port=306;
  }else if(WiFi.localIP()[3]==29){
    ArduinoOTA.setHostname("esp8266_jadalnia");
    port=307;
  }
  
  server.begin(port);
  
  Serial.println("Server started");
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  //no-ip ddns
  EasyDDNS.service("noip");  
  EasyDDNS.client("osiek.zapto.org","osiekowski","osiekowski123.NOIP");
  
  EasyDDNS.onUpdate([&](const char* oldIP, const char* newIP){
    Serial.print("EasyDDNS - IP Change Detected: ");
    Serial.println(newIP);
  });
  
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  
  timeClient.setTimeOffset(7200);
      
  /*
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;

  if((currentMonth>=4 && currentMonth<=9) 
      || (currentMonth==3 && monthDay>=27)
      || (currentMonth==10 && monthDay<30)){
      timeClient.setTimeOffset(7200);
  }else{
      timeClient.setTimeOffset(3600);
  }
  */
  
  //pinMode(2,OUTPUT);
}


int SunriseHourOffset = 0;
int SunriseMinuteOffset = 0;
int SunsetHourOffset = 0;
int SunsetMinuteOffset = 0;

float szacowany_stopien_otwarcia = 0.0;
float szacowany_stopien_otwarcia2 = 0.0;
unsigned long szacowaneOtwieranieMillis = millis();
unsigned long szacowaneOtwieranie2Millis = millis();
bool going_up=LOW;
bool going_down=LOW;
bool going_up2=LOW;
bool going_down2=LOW;

int openning_level = 1;
bool na_raz=HIGH;
int tryb=0;
int wedlog_godzina_otwarcia=7;
int wedlog_minuta_otwarcia=00;
int wedlog_godzina_zamkniecia=19;
int wedlog_minuta_zamkniecia=00;

/**********************************LOOP***************************************/
bool automatyczny = HIGH;
bool wedlog_godziny = LOW;
unsigned long previousMillis = millis();
//unsigned long previous20sec = millis();
unsigned long currentMillis = millis();

/*definicje var z kodu które są do obsługi*/
int currentHour;
int currentMinute;
int currentSecond;
//Get a time structure
struct tm *ptm;
int monthDay;
int currentMonth;
int currentYear;

String currentDate;

int Sunrise;
int Sunset;
    

int SunriseHour;
int SunriseMinute;
int SunsetHour;
int SunsetMinute;

float jakaCzescDnia;
int ileCzasuSlonca;

int request_fulfilled=0;
      

void loop() {
  ArduinoOTA.handle();
  MDNS.update();
  EasyDDNS.update(10000);

 
  if (WiFi.status() != WL_CONNECTED) {
    //ESP.reset();
    Serial.println("Connection Failed!");
    WiFi.begin(ssid, password);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connection Failed! 0");
      WiFi.begin(ssid1, password);
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connection Failed! 1");
        WiFi.begin(ssid2, password);
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Connection Failed! 2");
          WiFi.begin(ssid3, password);
          if (WiFi.status() != WL_CONNECTED) {
           Serial.println("Connection Failed! 3");
          }
        }
      }
    }
  }

  currentMillis = millis();
  /****************************reset_request*******************************/
  /*
  if(currentMillis - previous20sec >= 20000){
    previous20sec = currentMillis;
    request = "";
  }
  */
  
  ObslugaKlienta();
    
  /******************************ms****************************************/
  if (currentMillis - previousMillis >= ms) {
    previousMillis = currentMillis;

    //szacowanie
    if(going_up==HIGH){
      szacowany_stopien_otwarcia += 0.001*(currentMillis-szacowaneOtwieranieMillis)*3.5;
      szacowaneOtwieranieMillis=currentMillis;
    }else if(going_down==HIGH){
      szacowany_stopien_otwarcia -= 0.001*(currentMillis-szacowaneOtwieranieMillis)*3.5;
      szacowaneOtwieranieMillis=currentMillis;
    }  else{
      szacowaneOtwieranieMillis=currentMillis;
    }
    if(going_up2==HIGH){
      szacowany_stopien_otwarcia2 += 0.001*(currentMillis-szacowaneOtwieranie2Millis)*7;
      szacowaneOtwieranie2Millis=currentMillis;
    }else if(going_down2==HIGH){
      szacowany_stopien_otwarcia2 -= 0.001*(currentMillis-szacowaneOtwieranie2Millis)*7;
      szacowaneOtwieranie2Millis=currentMillis;
    }else{
      szacowaneOtwieranie2Millis=currentMillis;    
    }
    
    if(szacowany_stopien_otwarcia>100)szacowany_stopien_otwarcia=100;
    if(szacowany_stopien_otwarcia2>100)szacowany_stopien_otwarcia2=100;
    if(szacowany_stopien_otwarcia<0)szacowany_stopien_otwarcia=0;
    if(szacowany_stopien_otwarcia2<0)szacowany_stopien_otwarcia2=0;
    

    timeClient.update();

    time_t epochTime = timeClient.getEpochTime();
    currentHour = timeClient.getHours();
    currentMinute = timeClient.getMinutes();
    currentSecond = timeClient.getSeconds();
    //Get a time structure
    tm *ptm = gmtime ((time_t *)&epochTime);
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon + 1;
    currentYear = ptm->tm_year + 1900;

    if (debug == 1) {
      Serial.print("Epoch Time: ");
      Serial.println(epochTime);

      String formattedTime = timeClient.getFormattedTime();
      Serial.print("Formatted Time: ");
      Serial.println(formattedTime);

      Serial.print("Hour: ");
      Serial.println(currentHour);

      Serial.print("Minutes: ");
      Serial.println(currentMinute);

      Serial.print("Seconds: ");
      Serial.println(currentSecond);

      String weekDay = weekDays[timeClient.getDay()];
      Serial.print("Week Day: ");
      Serial.println(weekDay);

      Serial.print("Month day: ");
      Serial.println(monthDay);

      Serial.print("Month: ");
      Serial.println(currentMonth);

      String currentMonthName = months[currentMonth - 1];
      Serial.print("Month name: ");
      Serial.println(currentMonthName);

      Serial.print("Year: ");
      Serial.println(currentYear);

    }

    //Print complete date:
    currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) + " " + String(currentHour) + ":" + String(currentMinute) + ":" + String(currentSecond);
    Serial.print("Current date: ");
    Serial.println(currentDate);

    //Serial.println("");

    //delay(2000);
    Sunrise  = Gliwice.sunrise(currentYear, currentMonth, monthDay, false);
    Sunset   = Gliwice.sunset(currentYear, currentMonth, monthDay, false);
    if(tryb==1){
      Sunrise=wedlog_godzina_otwarcia*60 + wedlog_minuta_otwarcia;
      Sunset=wedlog_godzina_zamkniecia*60 + wedlog_minuta_zamkniecia;
    }
    if((currentMonth>=4 && currentMonth<=9) 
        || (currentMonth==3 && monthDay>=27)
        || (currentMonth==10 && monthDay<30)){

    }else{
      Sunrise=Sunrise-60;
      Sunset=Sunset-60;
    }
  
    SunriseHour = Sunrise / 60;
    SunriseMinute = Sunrise % 60;
    SunsetHour = Sunset / 60;
    SunsetMinute = Sunset % 60;

    jakaCzescDnia = abs(Sunrise - Sunset) / (24.0 * 60.0);
    ileCzasuSlonca = abs(Sunrise - Sunset);

    if (debug == 1) {
      Serial.println("Czesc dnia");
      Serial.print(jakaCzescDnia * 100.0);
      Serial.print("%");
      Serial.println();
      Serial.println("Czasu slonca");
      Serial.print(ileCzasuSlonca / 60);
      Serial.print("h ");
      Serial.print(ileCzasuSlonca % 60);
      Serial.print("m ");
      Serial.println();
      Serial.println("Dusk2Dawn");
      Serial.print(Sunrise);  // 418
      Serial.print(" z tego godzina = ");
      Serial.print(SunriseHour);
      Serial.print(":");
      Serial.print(SunriseMinute);
      Serial.println();
      Serial.print(Sunset);
      Serial.print(" z tego godzina = ");
      Serial.print(SunsetHour);
      Serial.print(":");
      Serial.print(SunsetMinute);
      Serial.println();
    }
    //delay(2000);
    
    // AKTYWAXJA GDY AUTO
    if (aktywacja >= warAktywacji && automatyczny ) {
      aktywacja = 0;
      request = "";
      digitalWrite(D0, HIGH);
      digitalWrite(D1, HIGH);
      if(WiFi.localIP()[3]==64){
        digitalWrite(D2, HIGH);
        digitalWrite(D3, HIGH);
      }
        
      //otwarcie gdy jasno
      if ((currentHour * 60 + currentMinute) >= (Sunrise + SunriseHourOffset * 60 + SunriseMinuteOffset) && (currentHour * 60 + currentMinute) < (Sunset + SunsetHourOffset * 60 + SunsetMinuteOffset) && x_times_up < 10) {
        Serial.print("UP \n");
        delay(2000);
        digitalWrite(D0, LOW);
        if(na_raz==HIGH){
          delay((10*2000)/openning_level);
          x_times_up=10;
          szacowany_stopien_otwarcia=100.0/openning_level;
        }else{
          delay(2000/openning_level);
          szacowany_stopien_otwarcia+=20/openning_level;
        }
        digitalWrite(D0, HIGH);
        delay(2000);
        if(WiFi.localIP()[3]==64){
          digitalWrite(D2, LOW);
          if(na_raz==HIGH){
            delay((10*2000*2)/openning_level);
            x_times_up=10;
            szacowany_stopien_otwarcia2=100.0/openning_level;
          }else{
            delay((2000*2)/openning_level);
            szacowany_stopien_otwarcia2+=20/openning_level;
          }
          digitalWrite(D2, HIGH);
          delay(2000);
        } 
        x_times_up++;
        x_times_down = 0;

        // zamknięcie gdy ciemno
      } else if ((currentHour * 60 + currentMinute) >= (Sunset + SunsetHourOffset * 60 + SunsetMinuteOffset) && x_times_down < 5) {
        Serial.print("DOWN \n");
        delay(2000);
        digitalWrite(D1, LOW);
        if(na_raz==HIGH){
          delay((5*4000)/openning_level/openning_level);
          x_times_down=5;
          szacowany_stopien_otwarcia=0.0;
        }else{
          delay(4000/openning_level);
          szacowany_stopien_otwarcia-=20/openning_level;
        }
        digitalWrite(D1, HIGH);
        delay(2000);
        if(WiFi.localIP()[3]==64){
          digitalWrite(D3, LOW);
          if(na_raz==HIGH){
            delay((5*4000*2)/openning_level);
            x_times_down=5;
            szacowany_stopien_otwarcia2=0.0;
          }else{
            delay((4000*2)/openning_level);
            szacowany_stopien_otwarcia2-=20/openning_level;
          }
          digitalWrite(D3, HIGH);
          delay(2000);
        } 
        x_times_down++;
        x_times_up = 0;
      }
    } else {
      aktywacja++;
      if (aktywacja > 999999)aktywacja = 999999;
    }
  }
}

void ObslugaKlienta(){
  
    // SERVER WIFI
    WiFiClient client = server.available();
    if (client) {
      //while(!client.available()){
      // Check if a client has connected
      //   delay(1);
      //  }
      request = client.readStringUntil('\r');
      if(debug==1){
        Serial.println("request");
        Serial.println(request);
      }
      client.flush();

      if (request.indexOf("/LED=ON") != -1) {
        Serial.println("ON");
        automatyczny = HIGH;
        request_fulfilled=1;
      }else if (request.indexOf("/LED=OFF") != -1) {
        Serial.println("OFF");
        automatyczny = LOW;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL=UP") != -1) {
        Serial.print("UP \n");
        digitalWrite(D0, LOW);
        digitalWrite(D1, HIGH);
        going_up=HIGH;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL=DOWN") != -1) {
        Serial.print("DOWN \n");
        digitalWrite(D1, LOW);
        digitalWrite(D0, HIGH);
        going_down=HIGH;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL=STOP") != -1) {
        Serial.print("STOP \n");
        digitalWrite(D0, HIGH);
        digitalWrite(D1, HIGH);
        going_up=LOW;
        going_down=LOW;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL2=UP") != -1) {
        Serial.print("UP2 \n");
        digitalWrite(D2, LOW);
        digitalWrite(D3, HIGH);
        going_up2=HIGH;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL2=DOWN") != -1) {
        Serial.print("DOWN2 \n");
        digitalWrite(D3, LOW);
        digitalWrite(D2, HIGH);
        going_down2=HIGH;
        request_fulfilled=1;
      }else if (request.indexOf("/WIFI_CONTROL2=STOP") != -1) {
        Serial.print("STOP2 \n");
        digitalWrite(D2, HIGH);
        digitalWrite(D3, HIGH);
        going_up2=LOW;
        going_down2=LOW;
        request_fulfilled=1;
      }

      else if (request.indexOf("/OFFSET_SUNRISE_H=-1") != -1){
        SunriseHourOffset = -1;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_H=0") != -1){
        SunriseHourOffset = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_H=1") != -1){
        SunriseHourOffset = 1;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_H=2") != -1){
        SunriseHourOffset = 2;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_H=3") != -1){
        SunriseHourOffset = 3;
        request_fulfilled=1;
      }

      else if (request.indexOf("/OFFSET_SUNRISE_M=0") != -1){
        SunriseMinuteOffset = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=15") != -1){
        SunriseMinuteOffset = 15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=30") != -1){
        SunriseMinuteOffset = 30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=45") != -1){
        SunriseMinuteOffset = 45;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=-15") != -1){
        SunriseMinuteOffset = -15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=-30") != -1){
        SunriseMinuteOffset = -30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNRISE_M=-45") != -1){
        SunriseMinuteOffset = -45;
        request_fulfilled=1;
      }

      else if (request.indexOf("/OFFSET_SUNSET_H=-1") != -1){
        SunsetHourOffset = -1;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_H=0") != -1){
        SunsetHourOffset = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_H=1") != -1){
        SunsetHourOffset = 1;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_H=2") != -1){
        SunsetHourOffset = 2;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_H=3") != -1){
        SunsetHourOffset = 3;
        request_fulfilled=1;
      }

      else if (request.indexOf("/OFFSET_SUNSET_M=0") != -1){
        SunsetMinuteOffset = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=15") != -1){
        SunsetMinuteOffset = 15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=30") != -1){
        SunsetMinuteOffset = 30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=45") != -1){
        SunsetMinuteOffset = 45;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=-15") != -1){
        SunsetMinuteOffset = -15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=-30") != -1){
        SunsetMinuteOffset = -30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/OFFSET_SUNSET_M=-45") != -1){
        SunsetMinuteOffset = -45;
        request_fulfilled=1;
      }

      else if (request.indexOf("TRYB=wschodzachod") != -1){
        tryb = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("TRYB=godziny") != -1){
        tryb = 1;
        request_fulfilled=1;
      }
      
      else if (request.indexOf("openning_LEVEL=1") != -1){
        openning_level = 1;
        request_fulfilled=1;
      }
      else if (request.indexOf("openning_LEVEL=2") != -1){
        openning_level = 2;
        request_fulfilled=1;
      }
      else if (request.indexOf("openning_LEVEL=3") != -1){
        openning_level = 3;
        request_fulfilled=1;
      }
      else if (request.indexOf("openning_LEVEL=4") != -1){
        openning_level = 4;
        request_fulfilled=1;
      }
      
      else if (request.indexOf("NARAZ=ON") != -1){
        na_raz = HIGH;
        request_fulfilled=1;
      }
      else if (request.indexOf("NARAZ=OFF") != -1){
        na_raz = LOW;
        request_fulfilled=1;
      }
      
      else if (request.indexOf("/SET_TIME_OPEN=4") != -1){
        wedlog_godzina_otwarcia = 4;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=5") != -1){
        wedlog_godzina_otwarcia = 5;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=6") != -1){
        wedlog_godzina_otwarcia = 6;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=7") != -1){
        wedlog_godzina_otwarcia = 7;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=8") != -1){
        wedlog_godzina_otwarcia = 8;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=9") != -1){
        wedlog_godzina_otwarcia = 9;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=10") != -1){
        wedlog_godzina_otwarcia = 10;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=11") != -1){
        wedlog_godzina_otwarcia = 11;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN=12") != -1){
        wedlog_godzina_otwarcia = 12;
        request_fulfilled=1;
      }
      
      else if (request.indexOf("/SET_TIME_OPEN_MIN=0") != -1){
        wedlog_minuta_otwarcia = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN_MIN=15") != -1){
        wedlog_minuta_otwarcia = 15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN_MIN=30") != -1){
        wedlog_minuta_otwarcia = 30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_OPEN_MIN=45") != -1){
        wedlog_minuta_otwarcia = 45;  
        request_fulfilled=1;
      }
          
      else if (request.indexOf("/SET_TIME_CLOSE=14") != -1){
        wedlog_godzina_zamkniecia = 14;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=15") != -1){
        wedlog_godzina_zamkniecia = 15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=16") != -1){
        wedlog_godzina_zamkniecia = 16;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=17") != -1){
        wedlog_godzina_zamkniecia = 17;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=18") != -1){
        wedlog_godzina_zamkniecia = 18;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=19") != -1){
        wedlog_godzina_zamkniecia = 19;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=20") != -1){
        wedlog_godzina_zamkniecia = 20;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=21") != -1){
        wedlog_godzina_zamkniecia = 21;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=22") != -1){
        wedlog_godzina_zamkniecia = 22;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE=23") != -1){
        wedlog_godzina_zamkniecia = 23;
        request_fulfilled=1;
      }
      
      else if (request.indexOf("/SET_TIME_CLOSE_MIN=0") != -1){
        wedlog_minuta_zamkniecia = 0;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE_MIN=15") != -1){
        wedlog_minuta_zamkniecia = 15;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE_MIN=30") != -1){
        wedlog_minuta_zamkniecia = 30;
        request_fulfilled=1;
      }
      else if (request.indexOf("/SET_TIME_CLOSE_MIN=45") != -1){
        wedlog_minuta_zamkniecia = 45;
        request_fulfilled=1;
      }

      // Return the response
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println(""); //  do not forget this one
      client.println("<!DOCTYPE HTML>");
      client.println("<html><head><title>ESP8266_n1</title><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>");
      client.println("<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>");
      client.println("<link rel='stylesheet' href='https://fonts.googleapis.com/css?family=Roboto'>");
      client.println("<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>");
      client.println("<style>html,body,h1,h2,h3,h4,h5,h6 {font-family: 'Roboto', sans-serif}*{text-decoration:none;} .w3-teal{padding:10px;}");
      client.println(".topnav {overflow: hidden;background-color: #333;}.topnav a {float: left;color: #f2f2f2;text-align: center;padding: 14px 16px;text-decoration: none;font-size: 17px;}.topnav a:hover {background-color: #ddd;color: black;}.topnav a.active {background-color: #009688;color: white;}");
      client.println(".dropdown {position: relative;display: inline-block;}");
      client.println(".dropdown-content {display: none;position: absolute;background-color: #f9f9f9;min-width: 160px;box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);padding: 12px 16px;z-index: 1;}.dropdown:hover .dropdown-content {display: block;}");
      client.println(".dropdown>* {margin: 0}");
      client.println(".dropdown>p {padding:10px; border:2px #009688 solid;border-radius:4px;}");
      client.println(".dropdown>ul {padding:10px; border:2px #009688 solid;border-radius:4px;}");
      client.println("hr {margin: 10px 0}");
      client.println("</style>");
      if (request_fulfilled==1){
        request="";
        client.println("<meta http-equiv='refresh' content='0;URL=/'osiek.zapto.org:");
        client.print(port);
        client.print("/'' /> ");
        client.println("</head>");
        request_fulfilled=0;
      }else{
      client.println("</head>");
      client.println("<body class='w3-light-grey'>");
      client.println("<!-- Page Container -->");
      client.println("<div class='w3-content w3-margin-top' style='max-width:1400px;'>");
      client.println("  <!-- The Grid -->");
      client.println("  <div class='w3-row-padding'>");
      client.println("    <!-- Right Column -->");
      client.println("    <div class='w3-twothird'>");
            client.println("<div class='topnav'>");
      if(WiFi.localIP()[3] == 36){
          client.println("<a class='active' href=\"http://osiek.zapto.org:302/\">Mateusz</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:302/\">Mateusz</a>");
        }
      if(WiFi.localIP()[3] == 49){
          client.println("<a class='active' href=\"http://osiek.zapto.org:303/\">Lauba</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:303/\">Lauba</a>");
        }
      if(WiFi.localIP()[3] == 67){
          client.println("<a class='active' href=\"http://osiek.zapto.org:304/\">Fryzjer</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:304/\">Fryzjer</a>");
        }
      if(WiFi.localIP()[3] == 64){
          client.println("<a class='active' href=\"http://osiek.zapto.org:305/\">Kuchnia</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:305/\">Kuchnia</a>");
        }      
      if(WiFi.localIP()[3] == 70){
          client.println("<a class='active' href=\"http://osiek.zapto.org:306/\">Salon</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:306/\">Salon</a>");
        }
        if(WiFi.localIP()[3] == 29){
          client.println("<a class='active' href=\"http://osiek.zapto.org:307/\">Jadalnia</a>");
        }else{
          client.println("<a href=\"http://osiek.zapto.org:307/\">Jadalnia</a>");
        }
      client.println("</div>");
      client.println("      <div class='w3-container w3-card w3-white'>");
      client.println("        <h2 class='w3-text-grey w3-padding-16'><i class='fa fa-certificate fa-fw w3-margin-right w3-xxlarge w3-text-teal'></i>");
      if(WiFi.localIP()[3] == 36){
          client.println("Mateusz");
      }
      if(WiFi.localIP()[3] == 49){
          client.println("Lauba");
      }
      if(WiFi.localIP()[3] == 67){
          client.println("Fryzjer");
      }
      if(WiFi.localIP()[3] == 64){
          client.println("Kuchnia");
      }      
      if(WiFi.localIP()[3] == 29){
          client.println("Jadalnia");
      }else{
          client.println(WiFi.localIP());
      }
      client.println("</h2>");
      client.println("<hr>");
      client.println("        <div class='w3-container'>");

      client.println("          <h6 class='w3-text-teal'><i class='fa fa-calendar fa-fw w3-margin-right'></i>");
      client.print(currentHour); client.print(":");
      if (currentMinute < 10)client.print("0");
      client.print(currentMinute); client.print(":");
      if (currentSecond < 10)client.print("0");
      client.print(currentSecond);
      client.print(" "); client.print(monthDay); client.print(".");
      if (currentMonth < 10) {
        client.print("0");
        client.print(currentMonth);
      } else {
        client.print(currentMonth);
      }
      client.print("."); client.print(currentYear);
      client.println("</h6>");
      client.println("<hr>");

      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=UP\">&uarr;</a></span>");
      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=STOP\">□</a></span>");
      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=DOWN\">&darr;</a></span>");
      if(WiFi.localIP()[3]==64){
        client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL2=UP\">&uarr;</a></span>");
        client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL2=STOP\">□</a></span>");
        client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL2=DOWN\">&darr;</a></span>");
      }
     
      client.println("<hr>");
      client.println("<p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Szacowany stopień otwarcia</b></p>");
      client.println("<div class='w3-light-grey w3-round-xlarge w3-small'>");
      client.println("<div class='w3-container w3-center w3-round-xlarge w3-teal' style='width:");
      client.print(szacowany_stopien_otwarcia);
      client.print("%'>");
      client.println(szacowany_stopien_otwarcia);
      client.println("%</div>");
      client.println("</div>");
      
      if(WiFi.localIP()[3] == 64){
        client.println("<div class='w3-light-grey w3-round-xlarge w3-small'>");
        client.println("<div class='w3-container w3-center w3-round-xlarge w3-teal' style='width:");
        client.print(szacowany_stopien_otwarcia2);
        client.print("%'>");
        client.println(szacowany_stopien_otwarcia2);
        client.println("%</div>");
        client.println("</div>");
      }
      client.println("<hr>"); 

      client.println("<p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Ustawienia</b></p>");
      client.println("<div class='dropdown'>");
      client.println("<p class='w3-large'>Wybór trybu</p>");
      client.println("<ul class='dropdown-content'>");
      client.println("<li><a href='/TRYB=wschodzachod'>wschód i zachód</a></li>");
      client.println("<li><a href='/TRYB=godziny'>według godziny</a></li>");
      client.println("</ul></div>");
      client.println("<div class='dropdown'>");
      client.println("<p class='w3-large'>Ograniczenie otwarcia</p>");
      client.println("<ul class='dropdown-content'>");
      client.println("<li><a href='/openning_LEVEL=1'> całe</a></li>");
      client.println("<li><a href='/openning_LEVEL=2'> połowa</a></li>");
      client.println("<li><a href='/openning_LEVEL=3'> 1/3</a></li>");
      client.println("<li><a href='/openning_LEVEL=4'> 1/4</a></li>");
      client.println("</ul></div>");
      client.println("<hr>");

      if(tryb==0){      
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Wschód");
      }else{
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Godzina otwarcia");  
      }
      client.print(SunriseHour); client.print(":");
      if (SunriseMinute < 10)client.print("0");
      client.print(SunriseMinute);
      if (SunriseHourOffset != 0) {
        if (SunriseHourOffset > 0)client.print(" +");
        if (SunriseHourOffset < 0)client.print(" -");
        client.print(SunriseHourOffset); client.print("h ");
      }
      if (SunriseMinuteOffset != 0) {
        if (SunriseMinuteOffset > 0)client.print(" +");
        if (SunriseMinuteOffset < 0)client.print(" -");
        client.print(SunriseMinuteOffset); client.print("m");
      }
      client.println("</b></p>");
      if(tryb==0){      
        client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Zachód");
      }else{
        client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Godzina zamknięcia");
      }
      client.print(SunsetHour); client.print(":");
      if (SunsetMinute < 10)client.print("0");
      client.print(SunsetMinute);
      if (SunsetHourOffset != 0) {
        if (SunsetHourOffset > 0)client.print(" +");
        if (SunsetHourOffset < 0)client.print(" -");
        client.print(SunsetHourOffset); client.print("h ");
      }
      if (SunsetMinuteOffset != 0) {
        if (SunsetMinuteOffset > 0)client.print(" +");
        if (SunsetMinuteOffset < 0)client.print(" -");
        client.print(SunsetMinuteOffset); client.print("m");
      }
      client.println("</b></p>");
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Stopień otwarcia: ");
      if(openning_level==1){
         client.println("całe");
      }if(openning_level==2){
         client.println("połowa");
      }if(openning_level==3){
         client.println("1/3");
      }if(openning_level==4){
         client.println("1/4");
      }
      client.println("</b></p>");

      if(tryb==1){
        client.println("<div class='dropdown'>");
        client.println("<p class='w3-large'>Otwarcie</p>");
        client.println("<ul class='dropdown-content'>");
        client.println("<li><a href='/SET_TIME_OPEN=4'>4:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=5'>5:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=6'>6:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=7'>7:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=8'>8:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=9'>9:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=10'>10:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=11'>11:XX</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN=12'>12:XX</a></li>");
        client.println("<li></li>");
        client.println("<li><a href='/SET_TIME_OPEN_MIN=0'>XX:00</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN_MIN=15'>XX:15</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN_MIN=30'>XX:30</a></li>");
        client.println("<li><a href='/SET_TIME_OPEN_MIN=45'>XX:45</a></li>");
        client.println("</ul></div>");
        client.println("<div class='dropdown'>");
        client.println("<p class='w3-large'>Zamknięcie</p>");
        client.println("<ul class='dropdown-content'>");
        client.println("<li><a href='/SET_TIME_CLOSE=14'>14:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=15'>15:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=16'>16:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=17'>17:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=18'>18:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=19'>19:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=20'>20:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=21'>21:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=22'>22:XX</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE=23'>23:XX</a></li>");
        client.println("<li></li>");
        client.println("<li><a href='/SET_TIME_CLOSE_MIN=0'>XX:00</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE_MIN=15'>XX:15</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE_MIN=30'>XX:30</a></li>");
        client.println("<li><a href='/SET_TIME_CLOSE_MIN=45'>XX:45</a></li>");
        client.println("</ul></div>");
      }
      client.println("<hr>");
      client.println("          <h5 class='w3-opacity'>");
      if (automatyczny == LOW) {
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/LED=ON\">WŁĄCZ</a></span>");
      } else {
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/LED=OFF\">WYŁĄCZ</a></span>");
      }
      client.println("<b>Automatyczny tryb: ");
      if (automatyczny == HIGH) {
        client.print("<span style='color:green;'>Włączony</span>");
      } else {
        client.print("<span style='color:red;'>Wylączony</span>");
      }
      client.print("</b></h5>");
      client.println("          <h5 class='w3-opacity'>");
      if (na_raz == LOW) {
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/NARAZ=ON\">WŁĄCZ</a></span>");
      } else {
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/NARAZ=OFF\">WYŁĄCZ</a></span>");
      }
      client.println("<b>Zamykanie i otwieranie na raz: ");
      if (na_raz == HIGH) {
        client.print("<span style='color:green;'>Włączone</span>");
      } else {
        client.print("<span style='color:red;'>Wylączone</span>");
      }
      client.print("</b></h5>");
      
      client.println("</div>");
      client.println("    <!-- End Right Column -->");
      client.println("    </div>");
      client.println("    </div>");

      /**********************************************************************************/
      client.println("    <!-- Left Column -->");
      client.println("    <div class='w3-third'>");
      client.println("      <div class='w3-white w3-text-grey w3-card-4'>");
      client.println("        <div class='w3-container'>");
      client.println("          <p class='w3-large'><i class='fa fa-home fa-fw w3-margin-right w3-large w3-text-teal'></i>");
      client.println(WiFi.localIP());
      client.println("</p>");
      client.println("          <hr>");
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Dane</b></p>");
      client.println("          <p>Słońce jest przez");
      client.print(ileCzasuSlonca / 60); client.print("h "); client.print(ileCzasuSlonca % 60); client.print("m ");

      client.print("</p>");
      client.println("          <div class='w3-light-grey w3-round-xlarge w3-small'>");
      client.println("            <div class='w3-container w3-center w3-round-xlarge w3-teal' style='width:");
      client.print(jakaCzescDnia * 100.0);
      client.print("%'>");
      client.print(jakaCzescDnia * 100.0);
      client.println("%</div>");
      client.println("          </div>");
      client.println("<hr>");
      client.println("<p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Offset</b></p>");
      
      client.println("<div class='dropdown'>");
      client.println("<p class='w3-large'>Wschód</p>");
      client.println("<div class='dropdown-content'>");
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=-1'>-1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=0'>0 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=1'>+1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=2'>+2 h</a></li>");
      client.println("</ul><ul>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-45'>-45 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-30'>-30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-15'>-15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=0'>0 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=15'>+15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=30'>+30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=45'>+45 m</a></li>");
      client.println("</ul></div></div>");
      
      client.println("<div class='dropdown'>");
      client.println("<p class='w3-large'>Zachód</p>");
      client.println("<div class='dropdown-content'>");
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNSET_H=-1'>-1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=0'>0 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=1'>+1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=2'>+2 h</a></li>");
      client.println("</ul> <ul>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-45'>-45 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-30'>-30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-15'>-15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=0'>0 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=15'>+15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=30'>+30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=45'>+45 m</a></li>");
      client.println("</ul></div></div>");
      
      client.println(" <br>");
      client.println("<p>Rolety Osiek wersja ");
      client.println(PROGRAM_VERSION);
      client.println("<p>");
      client.println("        </div>");
      client.println("      </div><br>");
      client.println("    <!-- End Left Column -->");
      client.println("    </div>");
      client.println("  <!-- End Grid -->");
      client.println("  </div>");
      client.println("  <!-- End Page Container -->");
      client.println("</div>");
      client.println("</body></html>");
      }
      Serial.println("Client disconnected");
      Serial.println("");
    }
    
}
