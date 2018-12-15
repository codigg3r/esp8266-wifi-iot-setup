#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

IPAddress local_IP(10,10,10,2);
IPAddress gateway(10,10,10,5);
IPAddress subnet(255,255,255,0);
WiFiServer server(80);
String ssid = "";
String pass = "";
String uid  = "";

//====================================
void setup() {
  Serial.begin(115200);
}
//====================================
bool configRes = false;
bool hardR = true;


//====================================
//======= LOOP =======================
//====================================
void loop() {
  if(hardR){
  hardReset();
  hardR = false;
  }

  bool res = veri(true,uid,"tekrar","paz","onTime","11:30");
  if(res){
    delay(1000);  
  }


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
    Serial.println("Konfigürasyon verisi alýnamdý, tekrar deneniyor!") ;
    configRes = configAl();
    tOut --;
   }}
   WiFi.mode(WIFI_STA);
   Serial.println("hata burada");
   WiFi.begin( ssid.c_str(),pass.c_str());
   Serial.println("---Baglanti moduna geçildi---");
   while ((WiFi.status() != WL_CONNECTED)){
    delay(100);
    Serial.print(".");
   }
    Serial.println("Wifi Baaðlandý") ;
    Serial.println("---Hard Reset Sona Erdi.---");
  
}

//====================================
//======== VERÝ ======================
//====================================

  // type = true(veri gönder), false(veri al)
  // uid = F48cHLaoG1dSVUzrsls0onx2ItT2
  // tablo = vana , tekrar
  // sutun = gün adý(pzt , sal ,car, per, cum, cmt, paz)
  // key = onTime, offTime , vanaDurum
  // data = {onTime,offTime}(10:30) , vanaDurum(on off)
bool veri(bool type , String uid, String tablo, String sutun, String key,String data ){
   int timeOut = 3;
   int httpCode = 0;
   String path = "https://vana-cc295.firebaseio.com/users/"+uid+"/"+tablo+"";
   if(timeOut<1){
      return false;
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
          Serial.println(payload);
          return true;
        }
      } else {
        Serial.printf("Data alýnýrken veya yazýlýrken hata oldu, hata kodu: %s\n", http.errorToString(httpCode).c_str());
        return false;
      }
      http.end();
  
  }else{
    timeOut --;
    Serial.println("Wifi Baglantý hatasý, Yeniden Baglanýlýyor .. ");
    delay(1000);
    }
  }

  
//====================================
//======== WÝFÝ YAYIN ================
//====================================
  
  // wifi yayýný baþlatýr 
  
  void wifiYay(){
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "---Config Hazýr---" : "Config Hata!");
  Serial.println(WiFi.softAP("vana","vanakey00") ? "---Wifi Yayýnýn Baþladý---" : "Wifi Yayýný Baþlayamadý!");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  server.begin();
  }

//====================================
//========= CONFÝG ===================
//====================================
 
  // ssid, pass, ve uid alma
  bool configAl(){
  WiFiClient client = server.available();
  while (!client) {
    delay(100);
    
    client = server.available();
  }
  Serial.println("istemci geldi.");
  // Ýstemci veri gönderenne kadar bekle
  while(!client.available()){
    delay(1);
  }
  Serial.println("istemci veri gönderdi.");
  
  String req = client.readStringUntil('\r');
  client.flush();

  Serial.println(req);
  // Match the request
  ssid = req.substring(req.indexOf("/")+1,req.indexOf(":"));
  Serial.println(ssid);
  String passBuf = req.substring(req.indexOf(":")+1,req.indexOf("\r"));
  pass = passBuf.substring(0,req.indexOf(":"));
  Serial.println(pass);
  uid = passBuf.substring(passBuf.indexOf(":")+1,passBuf.indexOf("\r"));
  uid = uid.substring(0,uid.indexOf(":"));
  Serial.println(uid);
  
   client.flush();
   String HTML = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
   HTML += ssid + pass +uid + "</html>";
    
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
//====================================
//====================================