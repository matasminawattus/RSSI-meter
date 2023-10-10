//this is currently used fw

#include "WiFi.h"

String ssidString = "";

WiFiClient deeper;
IPAddress deeperIP(192,168,10,1);

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  scanNetwork();
}

static int deeperProtocolbyteNr = 0;
static int deeperProtocolCommand = 0;
static int deeperProtocolPayloadLength = 0;

char sbuf1[]={0x16, 0x04, 0x00, 0xC4, 0x92};
char sbuf2[]={0x16, 0x03, 0x00, 0xF4, 0x90};
char sbuf[]={0x16, 0x02, 0x00, 0x64, 0x91};
static unsigned long timeWas = 0;

void sendToDeeper(boolean readWait)
{  
  if (!deeper.connected())
    return;
    
  if (deeper.availableForWrite() >= 5)
  {
    deeper.write(sbuf2, 5);
    
    if(readWait)
    {
      unsigned long millisStart = millis();
      while (!deeper.available())
      {
        if ((millis() - millisStart) > 1000) break; //timeout 1s
      }
    }
  }
}
unsigned long totalCnt;

void scanNetwork() 
{
  deeper.stop();
  delay(500);
  WiFi.disconnect();
  delay(500);
  
  while(WiFi.status() == WL_CONNECTED);
  
  totalCnt = 0;
  while(WiFi.status() != WL_CONNECTED)
  {
    WiFi.scanDelete();
    WiFi.scanNetworks(0,0,0); //WiFi.scanNetworks(0,0,1);
    ssidString = "";
    int Nr = -1;

    while(ssidString == "")
    {
      Serial.print("\r\nScanning...");
      int n = 0;
      while (n == 0) {
        n = WiFi.scanNetworks(0,0,0); //WiFi.scanNetworks(0,0,1);
        Serial.print(".");
        delay(100);
      }
      Serial.println();
      
      for (int i = 0; i < n; ++i)
      {
        //Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(WiFi.SSID(i) + " " + WiFi.RSSI(i) + " dBm");
      }
      while (Serial.available() > 0) Serial.read();
      Serial.println("\r\nSelect SSID Nr");
  
      Serial.setTimeout(60000);
      String terminalText = Serial.readStringUntil('\r');
      Serial.println(terminalText);
      Serial.println();
      terminalText.trim();
      if (terminalText == "1") Nr = 0;
      else if (terminalText == "2") Nr = 1;
      else if (terminalText == "3") Nr = 2;
      else if (terminalText == "4") Nr = 3;
      else if (terminalText == "5") Nr = 4;
      else if (terminalText == "6") Nr = 5;
      else if (terminalText == "7") Nr = 6;
      else if (terminalText == "8") Nr = 7;
      else if (terminalText == "9") Nr = 8;
      else if (terminalText == "10") Nr = 9;

      ssidString = WiFi.SSID(Nr);
    }
    Serial.print("Selected ");
    Serial.println(ssidString);
    if (WiFi.encryptionType(Nr) == WIFI_AUTH_OPEN) WiFi.begin((const char*)ssidString.c_str());
    else WiFi.begin((const char*)ssidString.c_str(), "12345678");
    
    Serial.print("Connecting");
    while (Serial.available() > 0) Serial.read();
    while (WiFi.status() != WL_CONNECTED)
    {
      if (Serial.read() == '\r') break;
      delay(500);
      Serial.print(WiFi.status());
      Serial.print('-');
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Couldn't get a wifi connection");
    }
    else
    {
      deeper.connect(deeperIP, 50007);
      while (Serial.available() > 0) Serial.read();
      while(!deeper.connected())
      {
        if (Serial.read() == '\r') break;
        delay(500);
        Serial.print("."); 
      }
      if (deeper.connected()) {
        deeper.setNoDelay(true);
        Serial.print("\r\nTCP ");
      }
      else Serial.print("\r\nWiFi ");
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
    }
  }
  while (Serial.available() > 0) Serial.read();
}
int counter = 0;
long rssi = 0;

void loop() {
  counter = 0;
  rssi = 0;
  int RssiMin = 1000;
  int RssiMax = -1000;
  if (totalCnt == 0) Serial.println("Nr; TCP; SSID;            AVG dBm; MAX dBm; MIN dBm");
  
  for (int i = 0; i < 1000; i++)
  {
    delay(10);
    long rs = WiFi.RSSI();
    //Serial.print("*");

    if (rs < 0) 
    {
      rssi += rs;
      counter++;
      if (rs > RssiMax) RssiMax = rs;
      else if (rs < RssiMin) RssiMin = rs;
    }  
  }
  if (counter > 0)
  {
    rssi /= counter;
    Serial.println();
    Serial.print(totalCnt++);
    Serial.print("; ");
//    if (deeper.connected()) Serial.print("TCP; ");
//    else Serial.print("---; ");
    Serial.print(WiFi.SSID());
    Serial.print("; ");
    Serial.print(rssi);
    Serial.print("; ");
    Serial.print(RssiMax);
    Serial.print("; ");
    Serial.print(RssiMin);
    Serial.print("; ");
  }
  
  sendToDeeper(true);

  char check = Serial.read();
  if ((check == '\r')||(check == 'n')||(check == 27))
  {
    Serial.println();
    Serial.println(check, DEC);
    scanNetwork();
  }
}
