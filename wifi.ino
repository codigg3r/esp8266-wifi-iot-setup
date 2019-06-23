#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h> 
#include <DS1302.h>


IPAddress local_IP(10,10,10,2);
IPAddress gateway(10,10,10,5);
IPAddress subnet(255,255,255,0);
WiFiServer server(80);
DS1302 rtc(D3, D4, D5);


String ssid 	= "";
String pass 	= "";
String uid  	= "";
String offPass 	= "";

String r_ssid       = "";
String r_pass       = "";
String r_uid        = "";
int    Sensor 	    = D7;
int    ROLE         = D2;   
int    RED_LED      = D6;
int    HRD_RST      = D1;



//====================================
void setup() {
  Serial.begin(115200);
  pinMode(Sensor, INPUT) ;
  pinMode(ROLE,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(HRD_RST,INPUT);
  rtc.halt(false);
  rtc.writeProtect(false);

//  attachInterrupt(digitalPinToInterrupt(HRD_RST), hardReset, FALLING); 

  EEPROM.begin(512);
  // first time set-up settings
  if (!(read_string(310,300).equals("1") || read_string(310,300).equals("0"))) {
  	write_EEPROM("1;",300);
  	EEPROM.commit();
  }
  //hard reset if require
    if(read_string(310,300).equals("1")){
  	hardReset();
  	write_EEPROM("0;",300);
  	EEPROM.commit();
  }else{
  	if (read_string(320,310).equals("1")) {
  		connectWiFi();
  	}
  	else{
  		//TODO 
  	}
  }
  

}
//====================================
bool    configRes  = false;
bool    hardR      = true;
String  resBuf     = "" ;
String  res        = "" ;
int  	zaman 	   = 0  ;
String  buffer     = "" ;
int 	tZaman     = 0  ;
//====================================
//======= LOOP =======================
//====================================
void loop() {

  while(1){
	  zaman = getTime();
	  // Timer...
	  if (zaman != tZaman) {
	  	String timer = veri(false,"vana","tekrar","node","");
	  	if (timer.length()>4) {
			timer = timer.substring(1,timer.length()-1) + ",";
			buffer  =	timer.substring(0,timer.indexOf(","));      
			}
	   	while(buffer.length()>0){     
	  	Serial.println(buffer);
	  	timer  = timer.substring(timer.indexOf(",")+1);
	  	buffer = timer.substring(0,timer.indexOf(","));
	  	if (String(zaman).equals(buffer)) {
	  		//TODO vana kapanma
	  		vanaDurum(false);
	  		for (int i = 0; i< 1 ;i++ ) {
	  			digitalWrite(RED_LED,HIGH);
	  			delay(500);
	  			digitalWrite(RED_LED,LOW);
	  			delay(500);
	  			}
	  		}
	  	if (String(zaman + 1).equals(buffer)) {	
	  		//TODO vana açılma
	  		vanaDurum(true);
				for (int i = 0; i< 10 ;i++ ) {
	  			digitalWrite(RED_LED,HIGH);
	  			delay(100);
	  			digitalWrite(RED_LED,LOW);
	  			delay(100);
	  			}
			}
			}
	  	}
      
     
    	res = veri(false,"vana","","vanaDurum","");
	  	vanaDurum(res.equals("\"on\""));
     
     	tZaman = zaman;
      	// HARD RESET
  		if (digitalRead(HRD_RST) == HIGH ){
      		hardReset();
      		write_EEPROM("0;",300);
  			EEPROM.commit();
  		}
	}
}
/*
*It will get current Time
*
*/
int getTime(){
	String t 	= rtc.getTimeStr(FORMAT_SHORT);
	String h 	= t.substring(0,t.indexOf(":"));
	String m  	= t.substring(t.indexOf(":")+1);
	String day 	= rtc.getDOWStr(FORMAT_SHORT);
	String dow[7] ={ "Monday", "Tuesday", "Wednesday","Thursday", "Friday", "Saturday", "Sunday"};
	int res = h.toInt()*600 + m.toInt()*10;
	for (int i = 0; i < 7 ; i++ ) {
		if (day.equals(dow[i])) {
			res = res + (i+1)*100000;
		}
	}
	return res ; 
}
//====================================
//============ WiFi ==================
//====================================
void connectWiFi(){
  EEPROM.begin(512);
  r_ssid = read_string(100,0);
  r_pass = read_string(200,100);
  r_uid  = read_string(300,200);
  uid = r_uid;
  Serial.println("Recovered credentials:");
  Serial.println(r_ssid);
  Serial.println(r_pass);
  Serial.println(r_uid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(r_ssid.c_str(),r_pass.c_str());
  Serial.println("---Baglanti moduna geçildi");
  int a  = 0;
  while ((WiFi.status() != WL_CONNECTED)){
    delay(100);
    Serial.print(".");
    a = a + 1;
    if(a == 50){
    	a = 0;
    	hardReset();
    	write_EEPROM("0;",300);
  		EEPROM.commit();
    }
   }
  Serial.println("Wifi Bağlandı") ;
  }
//====================================
//=========  VANA DURUM  =============
//====================================
void vanaDurum(boolean state){
	if (state) {
		Serial.println("State : TRUE");
    	if (digitalRead(Sensor) == HIGH) {
    		Serial.println("Sensor : HIGH");
    		digitalWrite(ROLE,HIGH);
    		delay(5060);
    		digitalWrite(ROLE,LOW);
    	}
    }else{
    	Serial.println("State : FALSE");
    	if (digitalRead(Sensor) == LOW) {
    		Serial.println("Sensor : LOW");
    		digitalWrite(ROLE,HIGH);
    		delay(100);
    		while(1){
    			Serial.println("__WHILE LOOP");
    			delay(50);
    			if (digitalRead(Sensor) == HIGH) {
    				delay(1200);
    				Serial.println("Sensor : HIGH");
    				digitalWrite(ROLE,LOW);
    				break;
    			}
    		}
    	}
    }    

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
   String path = "https://vana-6640a.firebaseio.com/users/"+uid+"/"+tablo+"";
   String fingerprint = "B6:F5:80:C8:B1:DA:61:C1:07:9D:80:42:D8:A9:1F:AF:9F:C8:96:7D";


    if(timeOut<1){
      	return "error";
 	}
	   	
   	if ((WiFi.status() == WL_CONNECTED)) {
    
		HTTPClient http;  

		if(sutun.length()<1){
		  if(type){    
		     http.begin(path+".json", fingerprint);
		  }else{
		     http.begin(path+"/"+key+".json", fingerprint);  
		  }
		}else{
		  if(type){
		     http.begin(path+"/"+sutun+".json", fingerprint);
		  } else{
		     http.begin(path+"/"+sutun+"/"+key+".json", fingerprint);

		  }
		}


		if(type){
		httpCode = http.PUT("{\""+key+"\":\""+data+"\"}");
		} else{
		httpCode = http.GET();
		}

		if (httpCode == HTTP_CODE_OK) {
		  String payload = http.getString();
		  return payload;
		}
		else {
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
//========= HARD RESET ===============
//====================================
// ssid, pass, ve uid alma
void hardReset(){
	clear_eeprom();
	digitalWrite(RED_LED,HIGH);
	wifiYay();
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
  
  	String mode = req.substring(req.indexOf("/")+1,req.indexOf("//"));
	EEPROM.begin(512);

  	if (mode.equals("offline")) {
	  	offline_mode(req);
	  	write_EEPROM("0;",310);
	  	EEPROM.commit();
	}else{
	  	online_mode(req);
	  	write_EEPROM("1;",310);
	  	EEPROM.commit();
	}
	  
	client.flush();
   	String HTML = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html></html>";    
   	client.print(HTML);
   	delay(10);
  	digitalWrite(RED_LED,LOW);


}

  /*
  *Online Mode
  *
  */
void online_mode(String req){
	ssid = req.substring(req.indexOf("//")+2,req.indexOf(":"));
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


    WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_STA);
   	WiFi.begin( ssid.c_str(),pass.c_str());
   	Serial.println("---Baglanti moduna geçildi---");
   	
   	while ((WiFi.status() != WL_CONNECTED)){
    	delay(100);
    	Serial.print("*");
    	}
    Serial.println("Wifi Bağlandı") ;
    // Write data to EEPROM
 	write_to_Memory(ssid,pass,uid);
    Serial.println("---Veri Yazıldı ,  Hard Reset Sona Erdi.---");
 
   } 

/*
 *Offline Mode
 *
 */
void offline_mode(String req){
	offPass = req.substring(req.indexOf("//")+2);
	EEPROM.begin(512);
	write_EEPROM(offPass + ";", 320);
	EEPROM.commit();
}

/*
 *EEPROM read and write methods
 *
 */

  String read_string(int l, int p){
    String temp;
    for (int n = p; n < l+p; ++n)
      {
       if(char(EEPROM.read(n))!=';'){
          if(isWhitespace(char(EEPROM.read(n)))){
            //do nothing
          }else temp += String(char(EEPROM.read(n)));
        
       }else n=l+p;
       
      }
    return temp;
  }

  void write_to_Memory(String s, String p, String u) {
  s += ";";
  write_EEPROM(s, 0);
  p += ";";
  write_EEPROM(p, 100);
  u += ";";
  write_EEPROM(u, 200);
  EEPROM.commit();
}

void write_EEPROM(String x, int pos) {
  for (int n = pos; n < x.length() + pos; n++) {
    EEPROM.write(n, x[n - pos]);
  }
}
/*
 *Clear EEPROM
 *
 */
void clear_eeprom(){
	EEPROM.begin(512);
    // write a 0 to all 512 bytes of the EEPROM
  	for (int i = 0; i < 512; i++) {
    	EEPROM.write(i, 0);
  	}
	EEPROM.end();
}
//====================================
//====================================
//====================================
