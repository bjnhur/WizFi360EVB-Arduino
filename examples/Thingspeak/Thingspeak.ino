/*
 WiFiEsp example: Thingspeak

 This sketch Request to thingspeak using an ESP8266 module to

*/

#include "WiFiEsp.h"
#include <DHT.h>
#include <SoftwareSerial.h>
#include <stdlib.h>
#define DHTPIN SDA
#define CDSPIN A0
#define DHTTYPE DHT11

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "wizms1";               // your network SSID (name)
char pass[] = "maker0701";            // your network password
int status = WL_IDLE_STATUS;          // the Wifi radio's status
char server[] = "api.thingspeak.com"; //server address
String apiKey ="";                    //apki key

//senser buffer
char temp_buf[5];
char humi_buf[5];
char cds_buf[5];

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 30000; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;
// Initialize the DHT object
DHT dht(DHTPIN, DHTTYPE); 

void setup() {              
  //initialize sensor
  pinMode(CDSPIN, INPUT);
  dht.begin();
  
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  
  printWifiStatus();

  //First transmitting
  sensorRead();
  thingspeakTrans();
}


void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only   
  while (client.available()) {
    char c = client.read();
    Serial.print("recv data: ");
    Serial.write(c);
    Serial.println();
  }
  
  // if 30 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    sensorRead();
    thingspeakTrans();
  }
  
  
}

// Read sendsor value
void sensorRead(){
  float cdsValue= analogRead(CDSPIN);
  float tempValue=dht.readTemperature();
  float humiValue=dht.readHumidity();
  String strTemp = dtostrf(tempValue, 4, 1, temp_buf);
  String strHumi = dtostrf(humiValue, 4, 1, humi_buf);
  String strCds = dtostrf(cdsValue, 4, 1, cds_buf);
  Serial.print("Temperature: ");
  Serial.println(strTemp);
  Serial.print("Humidity: ");
  Serial.println(strHumi);
  Serial.print("Cds: ");
  Serial.println(strCds);
}

//Transmitting sensor value to thingspeak
void thingspeakTrans()
{
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the Get request
    client.print(F("GET /update?api_key="));
    client.print(apiKey);
    client.print(F("&field1="));
    client.print(temp_buf);
    client.print(F("&field2="));
    client.print(humi_buf);
    client.print(F("&field3="));
    client.print(cds_buf);
    client.println();
    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
