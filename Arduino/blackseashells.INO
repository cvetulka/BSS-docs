/*
customer: Blacksea Shells
created 06 April 2021
author: Ts. Georgieva*/

#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#define DHT11TYPE DHT11
#define DHT21TYPE DHT21
#define DHT11PIN 2
#define DHT21PIN1 5
#define DHT21PIN2 6

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte hubIP[] = {78, 108, 247, 249};
int hubPort = 25000;
String hubClientID = "ff0e30a8-d56f-4ee6-98e5-abf14ed7339b";
int hubQueryID = 23;
EthernetClient client;

DHT dht11(DHT11PIN, DHT11TYPE);
DHT dht21_1(DHT21PIN1, DHT21TYPE);
DHT dht21_2(DHT21PIN2, DHT21TYPE);
DHT dhts[] = {dht11, dht21_1, dht21_2};
struct sensorsData
{
    String room = "";
    int sensorID = -1;
    float temperature = 0;
    float humidity = 0;
};

float humidity11;
float temperature11;
float humidity21_1;
float temperature21_1;
float humidity21_2;
float temperature21_2;
typedef struct sensorsData SensorsData;
SensorsData roomData[3] = {};

bool printWebData = false; //wheter to print received data from the remote server
unsigned long byteCount = 0;

void connectToHub()
{
    Serial.println("connecting...");
    if (client.connect(hubIP, hubPort))
    {
        Serial.print("connected to ");
        Serial.print(client.remoteIP());
        Serial.print("on port ");
        Serial.println(client.remotePort());
        postData();
    }
    else
    {
        Serial.println("connection failed");
    }
}

void postData()
{
    for (int i = 0; i < 3; i++)
    {
        Serial.print("{\"Room\":\"" + roomData[i].room + (String)(i + 1) +"\",);
        Serial.print(\"SensorID\":" + roomData[i].sensorID + ",");
        Serial.print("T\":" + roomData[i].temperature + ",");
        Serial.print("H\":" + roomData[i].humidity + ",");
        Serial.print("ID\":" + roomData[i].ID + ",");
        Serial.print("ID\":" + roomData[i].clientID + "\",");
        Serial.print("ClientName\":" + roomData[i].clientName + "\",");

        Serial.println("{\"Room\":" + roomData[i].room + "" + (String)(i + 1) + "\",\"T\":" + roomData[i].temperature + ",\"H\":" + roomData[i].humidity + ",\"Timestamp\":" + (String)roomData[i].timeStamp + ",\"ID\":23,\"ClientID\":\"e4d21c11-88c1-4ee1-aae0-a81849b8f8e7\",\"ClientName\":\"Black sea shell\"}");
        client.println("{\"Room\":\"Room " + (String)(i + 1) + "\",\"T\":" + roomData[i].temperature + ",\"H\":" + roomData[i].humidity + ",\"Timestamp\":" + (String)roomData[i].timeStamp + ",\"ID\":23,\"ClientID\":\"e4d21c11-88c1-4ee1-aae0-a81849b8f8e7\",\"ClientName\":\"Black sea shell\"}");
    }
    Serial.println();
}

bool startEthernetClient()
{
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        return false;
    }
    else
    {
        Serial.print("Arduino IP : ");
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

    while (!startEthernetClient())
    {
        delay(10000);
    }

    connectToHub();
}

void loop()
{
    int len = client.available();
    if (len > 0)
    {
        byte buffer[80];
        if (len > 80)
            len = 80;
        client.read(buffer, len);
        if (printWebData)
        {
            Serial.write(buffer, len); // show in the serial monitor (slows some boards)
        }
        byteCount = byteCount + len;
    }

    // if the server's disconnected, stop the client:
    if (!client.connected())
    {
        //Serial.println();
        //Serial.println("disconnecting.");
        //client.stop();
        connectToHub();
    }
    readDHT();
    delay(10000);
}

void readDHT()
{
    roomData[0].humidity = dht11.readHumidity();
    roomData[0].temperature = dht11.readTemperature();

    roomData[1].humidity = dht21_1.readHumidity();
    roomData[1].temperature = dht21_1.readTemperature();

    roomData[2].humidity = dht21_2.readHumidity();
    roomData[2].temperature = dht21_2.readTemperature();

    postData();
}