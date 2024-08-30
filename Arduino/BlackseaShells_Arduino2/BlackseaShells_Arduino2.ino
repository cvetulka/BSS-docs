/*
customer: Blacksea Shells Arduino_2
created 11 May 2021
author: Ts. Georgieva*/

#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

#define DHT21TYPE DHT21
#define DHT21PIN 2

DHT dht21_2(DHT21PIN, DHT21TYPE);
int alarm = 8;
int door = 9;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE9};
const IPAddress arduinoDNS(8, 8, 8, 8);
const IPAddress arduinoGateway(192, 168, 101, 1);
const IPAddress arduinoSubnet(255, 255, 255, 0);
const byte arduinoIP[] = {192,168,101,102};

byte hubIP[] = {78, 108, 247, 249};
//byte hubIP[] = {192,168,1,254};
//byte hubIP[] = {192,168,1,251};
int hubPort = 8000;
EthernetClient client;
bool printIncomingData = false; //wheter to print received data from the remote server

void connectHubClient()
{
    if (client.connect(hubIP, hubPort))
    {
        Serial.print("Ethernet client connected to ");
        Serial.print(client.remoteIP());
        Serial.print(" : ");
        Serial.println(client.remotePort());
    }
    else Serial.println("Ethernet client connection failed");
}

void disconnectHubClient()
{
  //if (client.connected())
  //{
    client.stop();
    Serial.println("Ethernet client disconnected");
    Serial.println();
  //}
}

bool startEthernetClient()
{
  Ethernet.begin(mac, arduinoIP, arduinoDNS, arduinoGateway, arduinoSubnet);
  delay(5000);
  Serial.println("Ethernet shield on " + Ethernet.localIP());

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
  connectHubClient();
  delay(500);

  String temp = isnan(dht21_2.readTemperature()) ? "NAN" : (String)dht21_2.readTemperature();
  Serial.println((String)dht21_2.readTemperature());
  client.println("POST /bss/2/2/" + temp);
  client.println(); //!!!!!!!important!!!!!!
  delay(500);

  String doorVal = isnan(digitalRead(door)) ? "NAN" : (String)digitalRead(door);
  Serial.println("POST /door/2/" + doorVal);
  client.println("POST /door/2/" + doorVal);
  client.println(); //!!!!!!!important!!!!!!
  delay(500);

  String alarmVal = isnan(digitalRead(alarm)) ? "NAN" : (String)digitalRead(alarm);
  Serial.println("POST /alarm/2/" + alarmVal);
  client.println("POST /alarm/2/" + alarmVal);
  client.println(); //!!!!!!!important!!!!!!
  delay(500);
  
  disconnectHubClient();
  delay(500);
}

void setup()
{
    pinMode(door, INPUT);
    pinMode(alarm, INPUT);
    dht21_2.begin();
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
