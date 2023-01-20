#define BLYNK_TEMPLATE_ID "your id"
#define BLYNK_DEVICE_NAME "your name"
#define BLYNK_AUTH_TOKEN "your token"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
//リレーテスト
//GPSテスト
#include <TinyGPS++.h> 
TinyGPSPlus gps;
HardwareSerial GPSRaw(2);  
//IFTTTテスト
String makerEvent ="your Event";
String makerKey ="your key";
const char* server = "maker.ifttt.com"; 
WiFiClient client;

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.

char ssid[] = "your ssid";
char pass[] = "your pass";
//char ssid[]="dsfree-g";
//char pass[] = "";
BlynkTimer timer;

bool checkWifiConnected() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  return true;
}
void send(String value1, String value2, String value3) {
  while (!checkWifiConnected()) {
    Serial.print("Attempting to connect to WiFi");
    WiFi.begin(ssid, pass);
  }
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 80)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
    String url = "/trigger/" + makerEvent + "/with/key/" + makerKey;
    url += "?value1=" + value1 + "&value2=" + value2 + "&value3=" + value3;
    client.println("GET " + url + " HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    Serial.print("Waiting for response "); 

    int count = 0;
    while (!client.available()) {
      delay(50); //
      Serial.print(".");
    }
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
  }
}
// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0){
  int val = param[0].asInt();
  Blynk.virtualWrite(V1, val);
  if(val == 1){
    digitalWrite(21, HIGH);
    delay(1000);
    Blynk.virtualWrite(V1, val);
  }else{
    digitalWrite(21, LOW);
  }
}
BLYNK_WRITE(V3)
{
  int val = param[0].asInt();  
  Blynk.virtualWrite(V4, val);
  Blynk.virtualWrite(V5,String("no signal"));
  Blynk.virtualWrite(V6,String("no signal"));
  Blynk.virtualWrite(V7,String("no signal"));
  if (val==1){
    //leray
    digitalWrite(22,HIGH);
    //GPS
    while(GPSRaw.available()) {
      gps.encode(GPSRaw.read());
      if (gps.location.isUpdated()) {
      Blynk.virtualWrite(V5,String(gps.location.lat(),6));
      Blynk.virtualWrite(V6,String(gps.location.lng(),6));
      Blynk.virtualWrite(V7,String(gps.altitude.meters()));
      } 
    }
  }else{
    digitalWrite(22,LOW);
  }
}

BLYNK_CONNECTED()
{
  Serial.println("Connected!");
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void setup()
{
  M5.begin();
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
  //リレーテスト
    pinMode(21, OUTPUT);  
    pinMode(22, OUTPUT);  

    digitalWrite(21, LOW);
    digitalWrite(22, LOW);
  
  //GPSテスト
  GPSRaw.begin(9600); 
  while (!GPSRaw) {
  ;   
  }    
  //PIRテスト
  pinMode(36,INPUT);
  pinMode(26,INPUT);
  //IFTTTテスト
     WiFi.begin(ssid, pass);
  while (!checkWifiConnected()) {
   WiFi.begin(ssid, pass);
  }  
}
void myTimerEvent()
{
  Blynk.virtualWrite(V2, millis() / 1000);
}
  void loop()
{
  Blynk.run();
  timer.run();
  M5.update();
  if(digitalRead(36)==1){
    digitalWrite(22,HIGH);
    send("detected","by","pir");
    delay(60000);
  }else{
    digitalWrite(22,LOW);
  }
    while(GPSRaw.available()) {
    gps.encode(GPSRaw.read());
     if (gps.location.isUpdated()) {
      Blynk.virtualWrite(V5,String(gps.location.lat(),6));
      Blynk.virtualWrite(V6,String(gps.location.lng(),6));
      Blynk.virtualWrite(V7,String(gps.altitude.meters()));
      if (M5.BtnA.wasReleased()) {
        send(String(gps.location.lat(),6),String(gps.location.lng(),6),String(gps.altitude.meters()));//任意の文字列3つ
      }
    }  
  }
}

