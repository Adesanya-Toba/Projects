/*
	Name:       Integrated_Code_Slave_ESP32.ino
	Created:	06/02/2020 09:26:17
	Author:     ADESANYA-TOBA\bruce2
*/

#include <Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include "PubSubClient.h"

const char* ssid = "*****"; // Your SSID (Name of your WiFi)
const char* password = "*******"; //Your Wifi password

char* topic = "channels/888134/publish/****************"; //channels/<channelID>/publish/API
char* server = "mqtt.thingspeak.com";

//Use WiFiClient class to create TCP connections
WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	// Not used.
}

Servo horizontal; //initialize horizontal servo object
Servo vertical; //Initialize vertical servo object

float pv_voltage_master;
float pv_voltage_slave;

int number3 = 3;
int number40 = 40;

// must match the controller struct
struct __attribute__((packed)) DataStruct {
	int elevation_angle;
	int azimuth_angle;
	float master_pv_voltage;
	float master_battery_voltage;
	char test_data[11] = "Test time";
};

DataStruct myData;

unsigned int temp_elevation = 0;
unsigned int temp_azimuth = 0;

#define LED 2
#define CHANNEL 1
#define SENDCHANNEL 1
#define WIFI_DEFAULT_CHANNEL 3

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 10000;

esp_now_peer_info_t peer;

void initESPNow();
void configDeviceAP();
void sendDataToMaster();
void addPeer(uint8_t* peerMacAddress);
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
//void onDataRecv(uint8_t* mac_addr, uint8_t* data, uint8_t data_len);


void setup()
{

	Serial.begin(115200);
	horizontal.attach(23, Servo::CHANNEL_NOT_ATTACHED, 5, 100, 1000, 2000);
	vertical.attach(22, Servo::CHANNEL_NOT_ATTACHED, 10, 150, 1000, 2000);
	pinMode(LED, OUTPUT);
	startMillis = millis();

	Serial.println();
	Serial.println("Starting EspnowSlave.ino");

	//Set device in AP mode to begin with
	WiFi.mode(WIFI_AP);

	// configure device AP mode
	configDeviceAP();

	// This is the mac address of the Slave in AP Mode
	Serial.print("Slave AP MAC: ");
	Serial.println(WiFi.softAPmacAddress());

	// Init ESPNow with a fallback logic
	initESPNow();

	// Once ESPNow is successfully Init, we will register for recv CB to get recv packer info.
	esp_now_register_send_cb(onDataSent);
	esp_now_register_recv_cb(onDataRecv);

	delay(50);

	// Connect to a WiFi access point
	wifiConnect();

	// This is the mac address of the Slave in Station Mode
	Serial.print("Slave STA MAC: ");
	Serial.println(WiFi.macAddress());

	// Set up MQTT for uploading data online
	String clientName = "IoT Solar Tracker";
	Serial.print("Connecting to ");
	Serial.print(server);
	Serial.print(" as ");
	Serial.println(clientName);

	if (client.connect((char*)clientName.c_str())) {
		Serial.println("Connected to MQTT broker");
		Serial.print("Topic is: ");
		Serial.println(topic);

		if (client.publish(topic, "hello from ESP8266")) {
			Serial.println("Publish ok");
		}
		else {
			Serial.println("Publish failed");
		}
	}
	else {
		Serial.println("MQTT connect failed");
		Serial.println("Will reset and try again...");
		abort();
	}

	Serial.println("End of setup - waiting for messages");

	//   Stationary (Non-Tracking Mode)
	/* Comment this section out to disable stationary (non-tracking) mode*/
	for (int e = 0; e < 170; e++)
	{
		vertical.write(e);
		delay(25);
	}
	delay(1500);
	for (int a = 0; a < 100; a++)
	{
		horizontal.write(a);
		delay(25);
	}

}

void loop()
{

	currentMillis = millis();
	pv_voltage_slave = analogRead(36);
	pv_voltage_slave = (200.0f / 100.0f) * 3.10f * pv_voltage_slave / 4095.0f;
	Serial.print("PV voltage: ");
	Serial.print(pv_voltage_slave, 2);
	Serial.println("V");
	Serial.println();

	Serial.println();
	Serial.print("Temp_elevation: "); Serial.println(temp_elevation);
	Serial.print("MyData.elevation_angle: "); Serial.println(myData.elevation_angle);
	Serial.println();

	Serial.println();
	Serial.print("Temp_azimuth: "); Serial.println(temp_azimuth);
	Serial.print("MyData.azimuth: "); Serial.println(myData.azimuth_angle);
	Serial.println();


	/* Uncomment this section to enable Tracking Mode*/
	//   Elevation Tracking
	if (temp_elevation < myData.elevation_angle)
	{
		if (myData.elevation_angle < 180)
		{
			//temp_elevation = myData.elevation_angle;
			for (int u = 0; u <= 100; u++)
			{
				temp_elevation = temp_elevation + 1;
				vertical.write(temp_elevation);
				delay(25);
				if (temp_elevation == myData.elevation_angle)
				{
					break;
				}
			}

			Serial.println();
			Serial.println("I am turning vertically up");
		}
	}


	if (temp_elevation > myData.elevation_angle)
	{
		//temp_elevation = myData.elevation_angle;

		for (int u = 0; u <= 100; u++)
		{
			temp_elevation = temp_elevation - 1;
			vertical.write(temp_elevation);
			delay(25);
			if (temp_elevation == myData.elevation_angle)
			{
				break;
			}
		}

		Serial.println();
		Serial.println("I am turning vertically down");


	}


	//    Azimuth Tracking
	if (temp_azimuth < 189 - myData.azimuth_angle)
	{
		for (int u = 0; u <= 100; u++)
		{
			temp_azimuth = temp_azimuth + 1;
			horizontal.write(temp_azimuth);
			delay(25);
			if (temp_azimuth == 189 - myData.azimuth_angle)
			{
				break;
			}
		}

		Serial.println();
		Serial.println("I am turning horizontally right");
	}

	if (temp_azimuth > 189 - myData.azimuth_angle)
	{
		for (int u = 0; u <= 100; u++)
		{
			temp_azimuth = temp_azimuth - 1;
			horizontal.write(temp_azimuth);
			delay(25);
			if (temp_azimuth == 189 - myData.azimuth_angle)
			{
				break;
			}
		}

		Serial.println();
		Serial.println("I am turning horizontally left");
	}



	Serial.print("elevation angle: "); Serial.println(myData.elevation_angle);
	Serial.println();
	delay(1000);

	Serial.print("azimuth angle: "); Serial.println(myData.azimuth_angle);
	Serial.println();

	if (currentMillis - startMillis >= period)
	{
		Serial.print("This should show up every 10 seconds: ");
		Serial.println(millis() / 1000);
		Serial.println();
		Serial.print("Sending data to cloud...");
		Serial.println();
		sendDataOnline();
		//sendDataToMaster();
		Serial.println();
		startMillis = currentMillis;
	}


	//ESP.restart();
	
}

// Init ESP Now with fallback
void initESPNow()
{
	WiFi.disconnect();
	if (esp_now_init() == ESP_OK) {
		Serial.println("ESPNow Init Success");
	}
	else
	{
		Serial.println("ESPNow Init Failed");
		ESP.restart();
	}
}

void configDeviceAP()
{
	String Prefix = "Slave:";
	String Mac = WiFi.macAddress();
	String SSID = Prefix + Mac;
	String Password = "123456789";
	bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
	if (!result)
	{
		Serial.println("AP Config failed.");
	}
	else
	{
		Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
	}
	delay(10);
}

// Add Master as a peer 
void addPeer(uint8_t *peerMacAddress)
{
	peer.channel = SENDCHANNEL;
	peer.ifidx = WIFI_IF_AP;
	peer.encrypt = 0;

	memcpy(peer.peer_addr, peerMacAddress, 6);
	esp_err_t addStatus = esp_now_add_peer(&peer);
	if (addStatus == ESP_OK)
	{
		// Pair success
		Serial.println("Pair success");
	}
	else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
	{
		// How did we get so far!!
		Serial.println("ESPNOW Not Init");
	}
	else if (addStatus == ESP_ERR_ESPNOW_ARG)
	{
		Serial.println("Add Peer - Invalid Argument");
	}
	else if (addStatus == ESP_ERR_ESPNOW_FULL)
	{
		Serial.println("Peer list full");
	}
	else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
	{
		Serial.println("Out of memory");
	}
	else if (addStatus == ESP_ERR_ESPNOW_EXIST)
	{
		Serial.println("Peer Exists");
	}
	else
	{
		Serial.println("Not sure what WENT WRONG");
	}
}

//Connect to a network to upload data online
void wifiConnect() {
	//WiFi.mode(WIFI_STA);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.print(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(250);
		Serial.print(".");
	}
	Serial.print("\nWiFi connected, IP address: ");
	Serial.println(WiFi.localIP());
	digitalWrite(LED, 1);
	delay(500);
	digitalWrite(LED, 0);
	delay(500);
	digitalWrite(LED, 1);
	delay(500);
	digitalWrite(LED, 0);
	delay(500);
}

void onDataRecv(const uint8_t* mac_addr, const uint8_t* data, int data_len)
{
	/*char macStr[18];
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	Serial.print("Last Packet Recv from: ");
	Serial.println(macStr);
	Serial.print("Last Packet Recv Data: ");
	Serial.println(*data);
	Serial.println("");*/


	memcpy(&myData, data, sizeof(myData));
	Serial.print("NewMsg from ");
	Serial.print("MacAddr: ");
	for (byte n = 0; n < 6; n++) {
		Serial.print(mac_addr[n], HEX);
	}
	//Serial.println(*data);
	Serial.print("  MsgLen ");
	Serial.print(data_len);
	Serial.print("  elevation_angle: ");
	Serial.print(myData.elevation_angle);
	Serial.print("  azimuth_angle: ");
	Serial.print(myData.azimuth_angle);
	Serial.print("  master_pv_voltage: ");
	Serial.print(myData.master_pv_voltage);
	Serial.print("  master_battery_voltage: ");
	Serial.print(myData.master_battery_voltage);
	digitalWrite(LED, 1);
	delay(250);
	digitalWrite(LED, 0);
	Serial.println();
}

// callback when data is sent from Master to Slave
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
	char macStr[18];
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	Serial.print("Last Packet Sent to: ");
	Serial.println(macStr);
	Serial.print("Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendDataOnline()
{
	Serial.println("Preparing to send data...");

	/* //Test data
	static int counter = 0;
	String payload = "field1=";
	payload += micros();
	payload += "&field2=";
	payload += counter;
	payload += "&field3=";
	payload += number3;
	payload += "&field4=";
	payload += number40;
	payload += "&status=MQTTPUBLISH";

	++counter;*/


	String payload = "field1=";
	payload += myData.elevation_angle;
	payload += "&field2=";
	payload += myData.azimuth_angle;
	payload += "&field3=";
	payload += myData.master_pv_voltage;
	payload += "&field4=";
	payload += pv_voltage_slave;
	payload += "&field5=";
	payload += myData.master_battery_voltage;
	payload += "&status=MQTTPUBLISH";

	if (client.connected()) {
		Serial.print("Sending payload: ");
		Serial.println(payload);

		if (client.publish(topic, (char*)payload.c_str())) {
			Serial.println("Publish ok");
		}
		else {
			Serial.println("Publish failed");
			Serial.println("Will reset and try again...");
			abort();
		}
	}

}

// send data to Master
void sendDataToMaster()
{
	const uint8_t* peer_addr = peer.peer_addr;

	// print for Master 
	Serial.printf("Sending: %s \n", myData.test_data);

	uint8_t bs[sizeof(myData)];
	memcpy(bs, &myData, sizeof(myData));


	esp_err_t result = esp_now_send(peer_addr, bs, sizeof(myData));
	Serial.print("Send Status: ");

	if (result == ESP_OK) {
		Serial.println("Success");
	}
	else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
		// How did we get so far!!
		Serial.println("ESPNOW not Init.");
	}
	else if (result == ESP_ERR_ESPNOW_ARG) {
		Serial.println("Invalid Argument");
	}
	else if (result == ESP_ERR_ESPNOW_INTERNAL) {
		Serial.println("Internal Error");
	}
	else if (result == ESP_ERR_ESPNOW_NO_MEM) {
		Serial.println("ESP_ERR_ESPNOW_NO_MEM");
	}
	else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
		Serial.println("Peer not found.");
	}
	else {
		Serial.println("Not sure what happened");
	}
	delay(100);

}