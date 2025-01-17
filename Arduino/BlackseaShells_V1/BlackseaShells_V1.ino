/*
Blacksea Shells
 created 07 April 2020
 by Ts. Georgieva
 */
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

#define DHT11TYPE DHT11
#define DHT21TYPE DHT21
#define DHT11PIN 2
#define DHT21PIN1 5
#define DHT21PIN2 6
DHT dht11(DHT11PIN, DHT11TYPE);
DHT dht21_1(DHT21PIN1, DHT21TYPE);
DHT dht21_2(DHT21PIN2, DHT21TYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte hubIP[]={78,108,247,249};
//byte monitoringAppIP[]={192,168,1,251};
byte monitoringAppIP[]={78,108,247,249};
int hubPort = 8000;

EthernetClient client;

DHT dhts[]={dht11, dht21_1, dht21_2};

bool connectedToAnyServer=false;
bool hubConnection=true;
struct sensorsData
{
   float temperature=0;
   float humidity=0;
   time_t timeStamp= now();
};

float humidity11;
float temperature11;
float humidity21_1;
float temperature21_1;
float humidity21_2;
float temperature21_2;
typedef struct sensorsData SensorsData;
SensorsData roomData[3]={};

bool printWebData = false;
unsigned long byteCount = 0;

void ConnectToHub(){
  Serial.println("connecting...");
  /*if (client.connect(hubIP, 25000)) 
  {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    PostData();
  }
  else */if (client.connect(monitoringAppIP, 8000))
  {
    connectedToAnyServer=true;
    hubConnection=false;
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } 
  else 
  {
    Serial.println("connection failed");
    connectedToAnyServer= false;
  }
}

void PostData(bool connectedToHub=true)
{
  if(!connectedToHub)
  {
    for(int i=0; i<3 ; i++)
    {
      client.println("POST /blackseashells/" + (String)(roomData[i].humidity) + "/" + (String)(roomData[i].temperature) + " HTTP/1.1");
      client.println();
      Serial.println("POST /blackseashells/" + String(roomData[i].humidity) + "/" + String(roomData[i].temperature) + " HTTP/1.1");
      delay(500);
    }
  }
  else
  {
    for(int i=0; i< 3; i++)
    {
      Serial.println("{\"Room\":\"Room " + (String)(i+1) + "\",\"T\":" + roomData[i].temperature + ",\"H\":" + roomData[i].humidity + ",\"Timestamp\":" + (String)roomData[i].timeStamp + ",\"ID\":23,\"ClientID\":\"e4d21c11-88c1-4ee1-aae0-a81849b8f8e7\",\"ClientName\":\"Black sea shell\"}");
      client.println("{\"Room\":\"Room " + (String)(i+1) + "\",\"T\":" + roomData[i].temperature + ",\"H\":" + roomData[i].humidity + ",\"Timestamp\":" + (String)roomData[i].timeStamp + ",\"ID\":23,\"ClientID\":\"e4d21c11-88c1-4ee1-aae0-a81849b8f8e7\",\"ClientName\":\"Black sea shell\"}");
    }
  }
  Serial.println();
} 

bool startEthernetClient()
{
  if (Ethernet.begin(mac)== 0)
   {
      Serial.println("Failed to configure Ethernet using DHCP");
      return false;
   }
   else
   {
     Serial.print("Arduino is on : ");
     Serial.println(Ethernet.localIP());
   }
   return true;
}

void setup()
{
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  dht11.begin();
  dht21_1.begin();
  dht21_2.begin();
  client.setConnectionTimeout(3000);

  Serial.begin(9600);
  while (!Serial)
  {
        // wait for serial port to connect. Needed for native USB port only
  }
  delay(3000);

  while(!startEthernetClient())
  {
    delay(10000);
  }
   
   ConnectToHub();
}
void loop()
{
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    //Serial.println();
    //Serial.println("disconnecting.");
    //client.stop();
    ConnectToHub();
  }
    readDHT();
  delay(10000);
}

void readDHT()
{ 
    roomData[0].humidity=dht11.readHumidity();
    roomData[0].temperature = dht11.readTemperature();
    roomData[0].timeStamp = now();
    /*Serial.println("11:    " + (String)roomData[0].humidity);
    Serial.println("11:    " + (String)roomData[0].temperature);
    Serial.println("11:    " + (String)roomData[0].timeStamp);
    Serial.println();*/

    roomData[1].humidity=dht21_1.readHumidity();
    roomData[1].temperature = dht21_1.readTemperature();
    roomData[1].timeStamp = now();
    /*Serial.println("21_1:    " + (String)roomData[1].humidity);
    Serial.println("21_1:    " + (String)roomData[1].temperature);
    Serial.println("21_1:    " + (String)roomData[1].timeStamp);
    Serial.println();*/

    roomData[2].humidity=dht21_2.readHumidity();
    roomData[2].temperature = dht21_2.readTemperature();
    roomData[2].timeStamp = now();
    /*Serial.println("21_2:    " + (String)roomData[2].humidity);
    Serial.println("21_2:    " + (String)roomData[2].temperature);
    Serial.println("21_2:    " + (String)roomData[2].timeStamp);
    Serial.println();*/

  PostData(hubConnection);
}
