#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <TimeLib.h> 

IPAddress local_IP(10,10,10,2);
IPAddress gateway(10,10,10,5);
IPAddress subnet(255,255,255,0);
IPAddress timeServer(192, 241, 211, 46);
WiFiServer server(80);
WiFiUDP Udp;
time_t prevDisplay = 0;

String ssid = "";
String pass = "";
String uid  = "";

String ssid1 = "";
String pass1 = "";
String uid1  = "";
int Sensor_on = D6;
int Sensor_off = D7;
int ROLE   = D8 ;   
int KEY_OUT = D2 ;
int RED_LED = D3;
int YEL_LED = D5;
const int timeZone = 3; 
unsigned int localPort = 8888;  

//====================================
void setup() {
  Serial.begin(115200);
  pinMode(Sensor_on, INPUT) ;
  pinMode(Sensor_off, INPUT) ;
  pinMode(ROLE,OUTPUT);
  pinMode(KEY_OUT,INPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(YEL_LED,OUTPUT);
  EEPROM.begin(512);

}
//====================================
bool configRes = false;
bool hardR = true;
String resBuf = "";
String  res = "";
int zaman = 0;
//====================================
//======= LOOP =======================
//====================================
void loop() {
  if(hardR){
  hardReset();
  hardR = false;
  }
 
  digitalWrite(ROLE,LOW);
  //connectWiFi();
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  while(1){
    if(now() != prevDisplay){
      zaman = weekday()*1000000 + hour()*600 * minute()*10;
      Serial.println(zaman);
       Serial.print(hour());
        Serial.print(" ");
        Serial.print(weekday());
         Serial.print(" ");
        Serial.print(minute());
      String data = veri(false,"vana","tekrar","node","");
      vanaDurum();
    }
  }
}
//====================================
//============ WiFi ==================
//====================================
void connectWiFi(){
  //EEPROM.begin(512);
  EEPROM.get(0, ssid1);
  EEPROM.get(0+sizeof(ssid1), pass1);
  EEPROM.get(0+sizeof(ssid1)+sizeof(pass1), uid1);
  EEPROM.end();
  Serial.println("Recovered credentials:");
  Serial.println(ssid1);
  Serial.println(pass1);
  Serial.println(uid1);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid1.c_str(),pass1.c_str());
  Serial.println("---Baglanti moduna geçildi---");
  while ((WiFi.status() != WL_CONNECTED)){
    delay(100);
    Serial.print(".");
   }
  Serial.println("Wifi Bağlandı") ;

  
  }
//====================================
//=========  VANA DURUM  =============
//====================================
void vanaDurum(){
    res = veri(false,"vana","","vanaDurum","");
    Serial.println(res);
    if(((res.equals("\"on\"")) || (digitalRead(KEY_OUT) == 1))  && digitalRead(Sensor_off) == LOW){
      digitalWrite(ROLE,HIGH);
      Serial.println("vana kapanıyor!");
      delay(2500);
      while(1){
        Serial.print("Kapnama döngüsü key: " );
        Serial.println(digitalRead(KEY_OUT));
        delay(50);
        if(digitalRead(Sensor_on) == LOW){
          delay(100);
          digitalWrite(ROLE,LOW);
          Serial.println("Kapnama döngüsü break");
          break;
        }
        }
        Serial.println("Kapanma döngüsü bitti");
      }
    
   
    if(((res.equals("\"off\"")) || (digitalRead(KEY_OUT) == 1)) && digitalRead(Sensor_on) == LOW){
      digitalWrite(ROLE,HIGH);
      Serial.print("Vana açılıyor key: " );
      Serial.println(digitalRead(KEY_OUT));
      delay(2500);
      while(1){
        Serial.println("Açılma döngüsü");
        delay(50);
        if(digitalRead(Sensor_off) == LOW){
          delay(100);
          digitalWrite(ROLE,LOW);
          Serial.println("Açılma döngüsü break");
          break;
        }
        }
        Serial.println("Açılma döngüsü bitti");
      }
        resBuf = res;
    }
//====================================
//====== HARD RESET ==================
//====================================
void hardReset(){
 
 configRes = false;
 int tOut = 3;
 wifiYay();
 configRes = configAl();
  
   if(tOut>0){
  
   if(configRes){
    WiFi.softAPdisconnect(true);
   }else{
    Serial.println("Konfigürasyon verisi alınamdı, tekrar deneniyor!") ;
    configRes = configAl();
    tOut --;
   }}
   WiFi.mode(WIFI_STA);
   WiFi.begin( ssid.c_str(),pass.c_str());
   Serial.println("---Baglanti moduna geçildi---");
   while ((WiFi.status() != WL_CONNECTED)){
    delay(100);
    Serial.print(".");
   }
    Serial.println("Wifi Baağlandı") ;
    // Write data to EEPROM
    EEPROM.put(0, ssid);
    EEPROM.put(0+sizeof(ssid), pass);
    EEPROM.put(0+sizeof(ssid)+sizeof(pass), uid);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("---Veri Yazıldı ,  Hard Reset Sona Erdi.---");
    
  
}

//====================================
//======== VERİ ======================
//====================================

  // type = true(veri gönder), false(veri al)
  // uid = F48cHLaoG1dSVUzrsls0onx2ItT2
  // tablo = vana , tekrar
  // sutun = gün adı(pzt , sal ,car, per, cum, cmt, paz)
  // key = onTime, offTime , vanaDurum
  // data = {onTime,offTime}(10:30) , vanaDurum(on off)
String veri(bool type ,String tablo, String sutun, String key,String data ){
   int timeOut = 3;
   int httpCode = 0;
   String path = "https://vana-cc295.firebaseio.com/users/"+uid+"/"+tablo+"";
   if(timeOut<1){
      return "error";
    }
   if ((WiFi.status() == WL_CONNECTED)) {
    
    HTTPClient http;  
    
      if(sutun.length()<1){
          if(type){    
             http.begin(path+".json?auth=vG6jMuRx2Ic1hPwXfrBUu83mdfRbOnUXFMyytZQL", "6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26");
          }else{
             http.begin(path+"/"+key+".json?auth=vG6jMuRx2Ic1hPwXfrBUu83mdfRbOnUXFMyytZQL", "6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26");  
          }
      } else{
          if(type){
             http.begin(path+"/"+sutun+".json?auth=vG6jMuRx2Ic1hPwXfrBUu83mdfRbOnUXFMyytZQL", "6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26");
          } else{
             http.begin(path+"/"+sutun+"/"+key+".json?auth=vG6jMuRx2Ic1hPwXfrBUu83mdfRbOnUXFMyytZQL", "6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26");

          }
      }

      
      if(type){
        httpCode = http.PUT("{\""+key+"\":\""+data+"\"}");
      } else{
        httpCode = http.GET();
      }

     
      if (httpCode > 0) {
        Serial.printf("HTTP code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          return payload;
        }
      } else {
        Serial.printf("Data alınırken veya yazılırken hata oldu, hata kodu: %s\n", http.errorToString(httpCode).c_str());
        return "error";
      }
      http.end();
  
  }else{
    timeOut --;
    Serial.println("Wifi Baglantı hatası, Yeniden Baglanılıyor .. ");
    delay(1000);
    }
  }

//====================================
//=========  ZAMANLAYICI =============
//====================================
void zamanlayici(){
    
  
  }
//====================================
//======== WİFİ YAYIN ================
//====================================
  
  // wifi yayını başlatır 
  
  void wifiYay(){
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "---Config Hazır---" : "Config Hata!");
  Serial.println(WiFi.softAP("vana","vanakey00") ? "---Wifi Yayının Başladı---" : "Wifi Yayını Başlayamadı!");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  server.begin();
  }

//====================================
//========= CONFİG ===================
//====================================
 
  // ssid, pass, ve uid alma
  bool configAl(){
  WiFiClient client = server.available();
  while (!client) {
    delay(100);
    
    client = server.available();
  }
  Serial.println("istemci geldi.");
  // İstemci veri gönderenne kadar bekle
  while(!client.available()){
    delay(1);
  }
  Serial.println("istemci veri gönderdi.");
  
  String req = client.readStringUntil('\n');
  client.flush();

  Serial.println(req);
  // Match the request
  ssid = req.substring(req.indexOf("/")+1,req.indexOf(":"));
  String passBuf = req.substring(req.indexOf(":")+1,req.length());
  pass = passBuf.substring(0,passBuf.indexOf(":"));
  uid = passBuf.substring(passBuf.indexOf(":")+1,passBuf.length());
  uid = uid.substring(0,uid.indexOf(":"));
 
  int spaceIndex = ssid.indexOf("%");
    while(spaceIndex != -1){
        ssid = ssid.substring(0,spaceIndex) +" "+ ssid.substring(spaceIndex+3,ssid.length());
        spaceIndex = ssid.indexOf("%");
      }
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(uid);
    
  
   client.flush();
   String HTML = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html></html>";
  
    
   client.print(HTML);
   delay(10);

   // veri kontrol 
   // sorun yoksa true, varsa false
   if (uid.length()> 10){
      return true;
   }else {
      return false;
   }
  
  }
//====================================
//==== NTP CONNECTION ================
//====================================
/*-------- NTP code ----------*/
/*--- Thanks to Michael Margolis <3 -------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
int tries = 5;
time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  tries--;
  if(tries>=0){
    getNtpTime();
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}  
//====================================
//====================================
//====================================
