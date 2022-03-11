#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//for ota
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//for d2d
#include <Math.h>
#include <Dusk2Dawn.h>

Dusk2Dawn Gliwice(50.2833, 18.6667, +2);

// Replace with your network credentials
const char *ssid     = "pozdrawiam";
const char *password = "osiekrulz";

const int ms = 20;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//WifiServer
WiFiServer server(302);

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

int aktywacja=999999;
int warAktywacji= 60 /*sekund*/ / (ms/1000.0) ;
unsigned int x_times_up=0,x_times_down=0;
bool debug=0;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //PINY DO UP DOWN
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  digitalWrite(D0,HIGH);
  digitalWrite(D1,HIGH);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);//OTA
  WiFi.begin(ssid, password);
  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    //ESP.restart();
  }
  //OTA
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266_nr1");

  // No authentication by default
  ArduinoOTA.setPassword("1");

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
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);

  //pinMode(2,OUTPUT);
}


int SunriseHourOffset = 0;
int SunriseMinuteOffset = 0;
int SunsetHourOffset = 0;
int SunsetMinuteOffset = 0;

int automatyczny = HIGH;
unsigned long previousMillis = millis();
unsigned long currentMillis = millis();
void loop() {
  ArduinoOTA.handle();

  if(WiFi.status() !=WL_CONNECTED){
    ESP.reset();
  }
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= ms) {
    previousMillis = currentMillis;
    
    timeClient.update();
  
    time_t epochTime = timeClient.getEpochTime();
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentSecond = timeClient.getSeconds();
    //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime); 
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon+1;
    int currentYear = ptm->tm_year+1900;
  
    if(debug==1){
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
    
      String currentMonthName = months[currentMonth-1];
      Serial.print("Month name: ");
      Serial.println(currentMonthName);
    
      Serial.print("Year: ");
      Serial.println(currentYear);
      
    }
      
      //Print complete date:
      String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay)+" "+String(currentHour)+":"+String(currentMinute)+":"+String(currentSecond);
      Serial.print("Current date: ");
      Serial.println(currentDate);
      
     //Serial.println("");

    //delay(2000);
     int Sunrise  = Gliwice.sunrise(currentYear, currentMonth, monthDay, false);
    int Sunset   = Gliwice.sunset(currentYear, currentMonth, monthDay, false);
    
    int SunriseHour = Sunrise/60;
    int SunriseMinute = Sunrise%60;
    int SunsetHour = Sunset/60;
    int SunsetMinute = Sunset%60;
  
    float jakaCzescDnia = abs(Sunrise-Sunset)/(24.0*60.0);
    int ileCzasuSlonca = abs(Sunrise-Sunset);
    
    if(debug==1){
      Serial.println("Czesc dnia");
      Serial.print(jakaCzescDnia*100.0);
      Serial.print("%");
      Serial.println();
      Serial.println("Czasu slonca");
      Serial.print(ileCzasuSlonca/60);
      Serial.print("h ");
      Serial.print(ileCzasuSlonca%60);
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
    if(aktywacja>=warAktywacji && automatyczny ){
      aktywacja=0;
      // powinno otworzyć do prawie połowy
      //}else 
      if((currentHour*60 + currentMinute)>=(Sunrise+SunriseHourOffset*60+SunriseMinuteOffset) && (currentHour*60 + currentMinute)<(Sunset+SunsetHourOffset*60+SunsetMinuteOffset) && x_times_up<10){
        Serial.print("UP \n");
        delay(2000);
        digitalWrite(D0,LOW);
        delay(2000);
        digitalWrite(D0,HIGH);
        delay(2000);
        x_times_up++;
        x_times_down=0;
        
        // zamknięcie gdy ciemno
        }else if((currentHour*60 + currentMinute)>=(Sunset+SunsetHourOffset*60+SunsetMinuteOffset) && x_times_down<5){
          Serial.print("DOWN \n");
          delay(2000);
          digitalWrite(D1,LOW);
          delay(4000);
          digitalWrite(D1,HIGH);
          delay(2000);
          x_times_down++;
          x_times_up=0;
        }
      }else{
        aktywacja++;
        if(aktywacja>999999)aktywacja=999999;
    }
    
  
  // SERVER WIFI
    WiFiClient client = server.available();
    if (client){
      //while(!client.available()){
      // Check if a client has connected
      //   delay(1);
      //  }
      String request = client.readStringUntil('\r');
      Serial.println(request);
      client.flush();
    
      if (request.indexOf("/LED=ON") != -1) {
      Serial.println("ON");
      automatyczny=HIGH;
      } 
      if (request.indexOf("/LED=OFF") != -1){
      Serial.println("OFF");
      automatyczny=LOW;
      }
      if (request.indexOf("/WIFI_CONTROL=UP") != -1) {
        Serial.print("UP \n");
        digitalWrite(D0,LOW);
        digitalWrite(D1,HIGH);
      } 
      if (request.indexOf("/WIFI_CONTROL=DOWN") != -1) {
        Serial.print("DOWN \n");
        digitalWrite(D1,LOW);
        digitalWrite(D0,HIGH);
      } 
      if (request.indexOf("/WIFI_CONTROL=STOP") != -1) {
        Serial.print("STOP \n");
        digitalWrite(D0,HIGH);
        digitalWrite(D1,HIGH);
      }
      
      if (request.indexOf("/OFFSET_SUNRISE_H=-1") != -1)SunriseHourOffset=-1;
      if (request.indexOf("/OFFSET_SUNRISE_H=0") != -1)SunriseHourOffset=0;
      if (request.indexOf("/OFFSET_SUNRISE_H=1") != -1)SunriseHourOffset=1;
      if (request.indexOf("/OFFSET_SUNRISE_H=2") != -1)SunriseHourOffset=2;
      if (request.indexOf("/OFFSET_SUNRISE_H=3") != -1)SunriseHourOffset=3;
      
      if (request.indexOf("/OFFSET_SUNRISE_M=0") != -1)SunriseMinuteOffset=0;
      if (request.indexOf("/OFFSET_SUNRISE_M=15") != -1)SunriseMinuteOffset=15;
      if (request.indexOf("/OFFSET_SUNRISE_M=30") != -1)SunriseMinuteOffset=30;
      if (request.indexOf("/OFFSET_SUNRISE_M=45") != -1)SunriseMinuteOffset=45;
      if (request.indexOf("/OFFSET_SUNRISE_M=-15") != -1)SunriseMinuteOffset=-15;
      if (request.indexOf("/OFFSET_SUNRISE_M=-30") != -1)SunriseMinuteOffset=-30;
      if (request.indexOf("/OFFSET_SUNRISE_M=-45") != -1)SunriseMinuteOffset=-45;
          
      if (request.indexOf("/OFFSET_SUNSET_H=-1") != -1)SunsetHourOffset=-1;
      if (request.indexOf("/OFFSET_SUNSET_H=0") != -1)SunsetHourOffset=0;
      if (request.indexOf("/OFFSET_SUNSET_H=1") != -1)SunsetHourOffset=1;
      if (request.indexOf("/OFFSET_SUNSET_H=2") != -1)SunsetHourOffset=2;
      if (request.indexOf("/OFFSET_SUNSET_H=3") != -1)SunsetHourOffset=3;
      
      if (request.indexOf("/OFFSET_SUNSET_M=0") != -1)SunsetMinuteOffset=0;
      if (request.indexOf("/OFFSET_SUNSET_M=15") != -1)SunsetMinuteOffset=15;
      if (request.indexOf("/OFFSET_SUNSET_M=30") != -1)SunsetMinuteOffset=30;
      if (request.indexOf("/OFFSET_SUNSET_M=45") != -1)SunsetMinuteOffset=45;
      if (request.indexOf("/OFFSET_SUNSET_M=-15") != -1)SunsetMinuteOffset=-15;
      if (request.indexOf("/OFFSET_SUNSET_M=-30") != -1)SunsetMinuteOffset=-30;
      if (request.indexOf("/OFFSET_SUNSET_M=-45") != -1)SunsetMinuteOffset=-45;
    
      // Return the response
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println(""); //  do not forget this one
      client.println("<!DOCTYPE HTML>");
      client.println("<html><head><title>ESP8266_n1</title><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>");
      client.println("<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>");
      client.println("<link rel='stylesheet' href='https://fonts.googleapis.com/css?family=Roboto'>");
      client.println("<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>");
      client.println("<style>html,body,h1,h2,h3,h4,h5,h6 {font-family: 'Roboto', sans-serif}*{text-decoration:none;}</style>");
      client.println("</head>");
      client.println("<body class='w3-light-grey'>");
      client.println("<!-- Page Container -->");
      client.println("<div class='w3-content w3-margin-top' style='max-width:1400px;'>");
      client.println("  <!-- The Grid -->");
      client.println("  <div class='w3-row-padding'>");
      client.println("    <!-- Right Column -->");
      client.println("    <div class='w3-twothird'>");
      client.println("      <div class='w3-container w3-card w3-white'>");
      client.println("        <h2 class='w3-text-grey w3-padding-16'><i class='fa fa-certificate fa-fw w3-margin-right w3-xxlarge w3-text-teal'></i>Kontroler nr 1</h2>");
      client.println("        <div class='w3-container'>");
      client.println("          <h6 class='w3-text-teal'><i class='fa fa-calendar fa-fw w3-margin-right'></i>");
      client.print(currentHour);client.print(":");
      if(currentMinute<10)client.print("0");
      client.print(currentMinute);client.print(":");
      if(currentSecond<10)client.print("0");
      client.print(currentSecond);
      client.print(" ");client.print(monthDay);client.print(".");
      if(currentMonth<10){
        client.print("0");
        client.print(currentMonth);
      }else{
        client.print(currentMonth);
      }
      client.print(".");client.print(currentYear);
      client.println("</h6>");
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Wschód");
      client.print(SunriseHour);client.print(":");
      if(SunriseMinute<10)client.print("0");
      client.print(SunriseMinute);
      if(SunriseHourOffset!=0){
        if(SunriseHourOffset>0)client.print(" +");
        if(SunriseHourOffset<0)client.print(" -");
        client.print(SunriseHourOffset);client.print("h ");
      }
      if(SunriseMinuteOffset!=0){
        if(SunriseMinuteOffset>0)client.print(" +");
        if(SunriseMinuteOffset<0)client.print(" -");
        client.print(SunriseMinuteOffset);client.print("m");
      }
      client.println("</b></p>");
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Zachód");
      client.print(SunsetHour);client.print(":");
      if(SunsetMinute<10)client.print("0");
      client.print(SunsetMinute);
      if(SunsetHourOffset!=0){
        if(SunsetHourOffset>0)client.print(" +");
        if(SunsetHourOffset<0)client.print(" -");
        client.print(SunsetHourOffset);client.print("h ");
      }
      if(SunsetMinuteOffset!=0){
        if(SunsetMinuteOffset>0)client.print(" +");
        if(SunsetMinuteOffset<0)client.print(" -");
        client.print(SunsetMinuteOffset);client.print("m");
      }
      client.println("</b></p>");
      client.println("          <h5 class='w3-opacity'><b>Automatyczny tryb: ");
      if(automatyczny == HIGH) {
        client.print("<span style='color:green;'>Włączony</span>");  
      } else {
        client.print("<span style='color:red;'>Wylączony</span>");
      }
      client.print("</b></h5>");
      client.println("          <p>");
      if(automatyczny == LOW) {
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/LED=ON\">WŁĄCZ</a></span>");
        }else{
        client.println("<span class='w3-tag w3-teal w3-round'><a href=\"/LED=OFF\">WYŁĄCZ</a></span>");
        }
      client.println("</p>");
      
      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=UP\">&uarr;</a></span>");
      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=STOP\">&#9209;</a></span>");
      client.println("<span style='font-size:100px;'><a href=\"/WIFI_CONTROL=DOWN\">&darr;</a></span>");
      client.println("      </div>");
      client.println("    <!-- End Right Column -->");
      client.println("    </div>");
      client.println("    </div>");
      
      /**********************************************************************************/
      client.println("    <!-- Left Column -->");
      client.println("    <div class='w3-third'>");
      client.println("      <div class='w3-white w3-text-grey w3-card-4'>");
      client.println("        <div class='w3-container'>");
      client.println("          <p class='w3-large'><i class='fa fa-home fa-fw w3-margin-right w3-large w3-text-teal'></i>Salon</p>");
      client.println("          <hr>");
      client.println("          <p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Dane</b></p>");
      client.println("          <p>Słońce jest przez"); 
      client.print(jakaCzescDnia*100.0);
      client.print("%</p>");
      client.println("          <div class='w3-light-grey w3-round-xlarge w3-small'>");
      client.println("            <div class='w3-container w3-center w3-round-xlarge w3-teal' style='width:");
      client.print(jakaCzescDnia*100.0);
      client.print("%'>");
      client.print(ileCzasuSlonca/60);
      client.print("h ");
      client.print(ileCzasuSlonca%60);
      client.print("m ");
      client.println("</div>");
      client.println("          </div>");
      client.println("<p class='w3-large'><b><i class='fa fa-asterisk fa-fw w3-margin-right w3-text-teal'></i>Offset</b></p>");
     client.println("<p class='w3-large'>Wschód</b></p>");
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=-1\'>-1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=0\'>0 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=1\'>1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_H=2\'>2 h</a></li>");
      client.println("</ul>");
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-45\'>-45 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-30\'>-30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=-15\'>-15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=0\'>0 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=15\'>15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=30\'>30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNRISE_M=45\'>45 m</a></li>");
      client.println("</ul>");
      client.println("<p class='w3-large'>Wschód</b></p>");
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNSET_H=-1\'>-1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=0\'>0 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=1\'>1 h</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_H=2\'>2 h</a></li>");
      client.println("</ul>");      
      client.println("<ul>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-45\'>-45 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-30\'>-30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=-15\'>-15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=0\'>0 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=15\'>15 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=30\'>30 m</a></li>");
      client.println("<li><a href='/OFFSET_SUNSET_M=45\'>45 m</a></li>");
      client.println("</ul>");
      client.println("          <br>");
      client.println("        </div>");
      client.println("      </div><br>");
      client.println("    <!-- End Left Column -->");
      client.println("    </div>");
      client.println("  <!-- End Grid -->");
      client.println("  </div>");
      client.println("  <!-- End Page Container -->");
      client.println("</div>");
      client.println("</body></html>");
      
      delay(1);
      Serial.println("Client disconnected");
      Serial.println("");
    }
  }
}
