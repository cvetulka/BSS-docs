/*
Blacksea Shells Arduino_1

 created 11 May 2021
 by Ts. Georgieva

 */
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

#define DHT11TYPE DHT11
#define DHT21TYPE DHT21
#define DHT11PIN 2
#define DHT21PIN1 6

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const IPAddress arduinoDNS(8, 8, 8, 8);
const IPAddress arduinoGateway(192, 168, 101, 1);
const IPAddress arduinoSubnet(255, 255, 255, 0);
const byte arduinoIP[] = {192,168,101,101};

const byte hubIP[]={78,108,247,249};
const int hubPort=8000;
EthernetClient client;
DHT dht11(DHT11PIN, DHT11TYPE);
DHT dht21(DHT21PIN1, DHT21TYPE);
int alarm = 8;
int door = 9;

bool printIncomingData = false;

void connectHubClient(){
    if (client.connect(hubIP, hubPort))
    {
        Serial.print("Ethernet client connected to ");
        Serial.print(client.remoteIP());
        Serial.print(" : ");
        Serial.println(client.remotePort());
    }
    else Serial.println("Client connection failed");
}

void disconnectHubClient()
{
  //if (client.connected())
  //{
    client.stop();
    Serial.println("Client disconnected");
    Serial.println();
  //}
}

bool startEthernetClient()
{
  Serial.println("Starting Ethernet shield...");
  Serial.println();
  Ethernet.begin(mac, arduinoIP, arduinoDNS, arduinoGateway, arduinoSubnet);
  delay(5000);
  Serial.println("Ethernet is running on " + Ethernet.localIP());

  //DHCP
    /*if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        return false;
    }
    else
    {
        Serial.print("Arduino IP : ");
        Serial.println(Ethernet.localIP());
    }
    return true;*/
}

void printDataFromHub()
{
  int len = client.available();
  if (len > 0)
  {
    byte buffer[80];
    if (len > 80)
    len = 80;
    client.read(buffer, len);
    if (printIncomingData)
    {
      Serial.write(buffer, len); // prints buffer
      String message = "";
      switch ((int)buffer){
        case 0: message="Responce from Hub - OK"; break;
        case 1: message = "Error from Hub's responce"; break;
        default: Serial.write(buffer, len); break;
      }
      Serial.println(message); // show in the serial monitor (slows some boards)
    }
  }
}

void sendData()
{
  if( Ethernet.localIP() == arduinoIP)
  {
    connectHubClient();
    delay(500);
    
    String temp11 = isnan(dht11.readTemperature()) ? "NAN" : (String)dht11.readTemperature();
    Serial.println((String)dht11.readTemperature());
    client.println("POST /bss/1/1/" + temp11);
    client.println(); //!!!!!!!important!!!!!!
    delay(500);
  
    String temp21 = isnan(dht21.readTemperature()) ? "NAN" : (String)dht21.readTemperature();
    Serial.println((String)dht21.readTemperature());
    client.println("POST /bss/2/1/" + temp21);
    client.println(); //!!!!!!!important!!!!!!
    delay(500);

    String doorVal = isnan(digitalRead(door)) ? "NAN" : (String)digitalRead(door);
    Serial.println("POST /door/1/" + doorVal);
    client.println("POST /door/1/" + doorVal);
    client.println(); //!!!!!!!important!!!!!!
    delay(500);

    String alarmVal = isnan(digitalRead(alarm)) ? "NAN" : (String)digitalRead(alarm);
    Serial.println("POST /alarm/1/" + alarmVal);
    client.println("POST /alarm/1/" + alarmVal);
    client.println(); //!!!!!!!important!!!!!!
    delay(500);
  
    disconnectHubClient();
    delay(500);
  }
  else
  {
    Serial.println("Ethernet is not running");
    Serial.println();
    startEthernetClient();
  }
}

void setup()
{
  pinMode(door, INPUT);
  pinMode(alarm, INPUT);
  dht11.begin();
  dht21.begin();
  client.setConnectionTimeout(3000);

  Serial.begin(9600);
  while (!Serial)
  {
        // wait for serial port to connect. Needed for native USB port only
  }
  delay(3000);

  startEthernetClient();
  /*while(!startEthernetClient())
  {
    delay(1000);
  }*/
}
void loop()
{
    printDataFromHub();
    sendData();
    delay(58000);
}
