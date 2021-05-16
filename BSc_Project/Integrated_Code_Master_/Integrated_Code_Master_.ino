/*
	Name:       Integrated_Code_Master_.ino
	Created:	06/01/2020 12:46:29
	Author:     ADESANYA-TOBA\bruce2
*/

#include <Servo.h>
#include <esp_now.h>
#include <WiFi.h>

Servo horizontal; //initialize horizontal servo object
Servo vertical; //Initialize vertical servo object

// must match the controller struct
struct __attribute__((packed)) DataStruct {
	int elevation_angle;
	int azimuth_angle = 0;
	float master_pv_voltage;
	float master_battery_voltage;
	char test_data[11] = "Test time";
};

DataStruct myData;

#define CHANNEL 3
#define NUM_SLAVES 20                   // ESP-Now can handle a maximum of 20 slaves
#define PRINTSCANRESULTS 0

int slaveCount = 0;                     // Keeps count of no. of slaves with the defined prefix
esp_now_peer_info_t slaves[NUM_SLAVES]; // Stores the information of each of the slave that is added as a peer


int servoh = 0;
int servov = 0;
float batt_voltage, pv_voltage;

int servovLimitHigh = 120;
int servovLimitLow = 20;

int servohLimitHigh = 150;
int servohLimitLow = 20;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 5000;

void initESPNow();
void manageSlaves();
void scanForSlaves();
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
void onDataRecv(const uint8_t* mac_add, const uint8_t* data, int data_len);
void sendData();

void setup()
{
	Serial.begin(115200);
	horizontal.attach(23, Servo::CHANNEL_NOT_ATTACHED, 5, 170, 1000, 2000);
	vertical.attach(22, Servo::CHANNEL_NOT_ATTACHED, 10, 150, 1000, 2000);
	pinMode(2, OUTPUT);
	startMillis = millis();

	//Set device in STA mode to begin with
	WiFi.mode(WIFI_MODE_STA);

	// This is the mac address of the Master in Station Mode
	Serial.print("STA MAC: ");
	Serial.println(WiFi.macAddress());


	// Init ESPNow with a fallback logic
	initESPNow();

	// Once ESPNow is successfully Init, we will register for Send CB to
	// get the status of Trasnmitted packet
	esp_now_register_send_cb(onDataSent);
	esp_now_register_recv_cb(onDataRecv);

	scanForSlaves();
	manageSlaves();

	/*for (int i = 0; i <= 120; i++)
	{
		vertical.write(i);
		delay(50);
	}
	delay(500);

	for (int i = 0; i <= 150; i++)
	{
		horizontal.write(i);
		delay(50);
	}
	delay(500);*/
}

void loop()
{
	//start tracking algorithm
	int bottomRight = analogRead(35) - 500;
	int topRight = analogRead(34) - 500;
	int bottomLeft = analogRead(36) - 500;
	int topLeft = analogRead(39) - 500;
	batt_voltage = analogRead(32);
	pv_voltage = analogRead(33);

	int tolerance = 250;
	currentMillis = millis();

	int avt = (topRight + topLeft) / 2; //avearge value of top
	int avd = (bottomRight + bottomLeft) / 2;//avearge value of bottom
	int avl = (topLeft + bottomLeft) / 2;//avearge value of left
	int avr = (topRight + bottomRight) / 2;//avearge value of right

	int dvert = avt - avd;// vertical difference
	int dhorz = avl - avr;// horizontal difference

	if (-1 * tolerance > dvert || dvert > tolerance)
	{
		if (avt > avd + 100)
		{
			for (int u = 0; u <= 10; u++)
			{
				if (servov >= 0)
				{
					Serial.print("current servov: ");
					Serial.println(servov);
					servov = servov - 1;
					vertical.write(servov);
					delay(25);
				}
			}

			/*if (servov < servovLimitLow)
			{
				servov = servovLimitLow;
			}*/
		}

		else if (avt < avd - 100)
		{
			for (int i = 0; i <= 10; i++)
			{
				if (servov <= 150)
				{
					servov = servov + 1;
					vertical.write(servov);
					Serial.print("current servov: ");
					Serial.println(servov);
					delay(25);
				}
			}

			/*if (servov > servovLimitHigh)
			{
				servov = servovLimitHigh;
			}*/
		}
	}


	if (-1 * tolerance > dhorz || dhorz > tolerance)
	{
		if (avl > avr + 100)
		{
			for (int u = 0; u <= 10; u++)
			{
				if (servoh >= 0) {
					Serial.print("current servoh: ");
					Serial.println(servoh);
					servoh = servoh - 1;
					horizontal.write(servoh);
					delay(25);
				}
			}

			/*if  (servoh < servohLimitLow)
			{
				servoh = servohLimitLow;
			}*/
		}

		else if (avl < avr - 100)
		{
			for (int i = 0; i <= 10; i++)
			{
				if (servoh <= 170)
				{
					servoh = servoh + 1;
					horizontal.write(servoh);
					Serial.print("current servoh: ");
					Serial.println(servoh);
					delay(25);
				}
			}

			/*if (servoh > servohLimitHigh)
			{
				servoh = servohLimitHigh;
			}*/

			delay(150);
		}
	}
	/*Serial.println();
	Serial.print("Bottom-Right: ");
	Serial.println(bottomRight);
	Serial.print("Top-Right: ");
	Serial.println(topRight);
	Serial.print("Bottom-Left: ");
	Serial.println(bottomLeft);
	Serial.print("Top-Left: ");
	Serial.println(topLeft);
	
	Serial.println();
	Serial.print("Vertical diff: ");
	Serial.println(dvert);
	Serial.print("Horizontal diff: ");
	Serial.println(dhorz);

	Serial.println();
	Serial.print("average top: ");
	Serial.println(avt);
	Serial.print("average bottom: ");
	Serial.println(avd);
	Serial.print("average left: ");
	Serial.println(avl);
	Serial.print("average right: ");
	Serial.println(avr);
	*/

	//Serial.println();



	delay(50);



	if (currentMillis - startMillis >= period)
	{
		Serial.println();
		Serial.print("Printing every 5 seconds: ");
		Serial.println();
		Serial.print("Vertical Angle: ");
		Serial.println(servov);
		Serial.print("Read Vertical Angle: ");
		Serial.println(vertical.read());
		Serial.print("Horizontal Angle: ");
		Serial.println(servoh);
		Serial.print("Read Horizontal Angle: ");
		Serial.println(horizontal.read());

		Serial.println();
		batt_voltage = (127.0f / 100.0f) * 3.30f * analogRead(32) / 4095.0f;
		Serial.print("Battery voltage: ");
		Serial.print(batt_voltage, 2);
		Serial.println("V");
		Serial.println();

		pv_voltage = (200.0f / 100.0f) * 3.30f * pv_voltage / 4095.0f;
		Serial.print("PV voltage: ");
		Serial.print(pv_voltage, 2);
		Serial.println("V");
		Serial.println();

		myData.elevation_angle = servov;
		if (myData.elevation_angle < 0)
		{
			Serial.println(myData.elevation_angle);
			myData.elevation_angle = 0;
		}

		myData.azimuth_angle = servoh;
		
		if (myData.azimuth_angle < 0)
		{
			Serial.println(myData.azimuth_angle);
			myData.azimuth_angle = 0;
		}
		myData.master_pv_voltage = pv_voltage;
		myData.master_battery_voltage = batt_voltage;
		sendData();
		digitalWrite(2, 1);
		delay(250);
		digitalWrite(2, 0);
		Serial.println();

		startMillis = currentMillis;
	}

}

// Init ESP Now with fallback
void initESPNow()
{
	WiFi.disconnect();
	if (esp_now_init() == ESP_OK)
	{
		Serial.println("ESPNow Init Success");
	}
	else
	{
		Serial.println("ESPNow Init Failed");
		ESP.restart();
	}
}

// Scan for slaves in AP mode
void scanForSlaves()
{
	int8_t scanResults = WiFi.scanNetworks();

	//reset slaves
	memset(slaves, 0, sizeof(slaves));
	slaveCount = 0;
	Serial.println("");

	if (scanResults == 0)
	{
		Serial.println("No WiFi devices in AP Mode found");
	}
	else
	{
		Serial.print("Found ");
		Serial.print(scanResults);
		Serial.println(" devices ");

		for (int i = 0; i < scanResults; i++)
		{
			// Print SSID and RSSI for each device found
			String SSID = WiFi.SSID(i);
			int32_t RSSI = WiFi.RSSI(i);
			String BSSIDstr = WiFi.BSSIDstr(i);

			if (PRINTSCANRESULTS)
			{
				Serial.print(i + 1);
				Serial.print(": ");
				Serial.print(SSID);
				Serial.print(" [");
				Serial.print(BSSIDstr);
				Serial.print("]");
				Serial.print(" (");
				Serial.print(RSSI);
				Serial.print(")");
				Serial.println("");
			}

			delay(10);

			// Check if the current device starts with `Slave`
			if (SSID.indexOf("Slave") == 0)
			{
				// SSID of interest
				Serial.print(i + 1);
				Serial.print(": ");
				Serial.print(SSID);
				Serial.print(" [");
				Serial.print(BSSIDstr);
				Serial.print("]");
				Serial.print(" (");
				Serial.print(RSSI);
				Serial.print(")");
				Serial.println("");

				// Get BSSID => Mac Address of the Slave
				int mac[6];

				if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
				{
					for (int j = 0; j < 6; j++)
					{
						slaves[slaveCount].peer_addr[j] = (uint8_t)mac[j];
					}
				}

				slaves[slaveCount].channel = CHANNEL; // pick a channel
				slaves[slaveCount].encrypt = 0; // no encryption
				slaveCount++;
			}
		}
	}

	if (slaveCount > 0)
	{
		Serial.print(slaveCount); Serial.println(" Slave(s) found, processing..");
		digitalWrite(2, 1);
		delay(1500);
		digitalWrite(2, 0);
	}
	else
	{
		Serial.println("No Slave Found, trying again.");
	}

	// clean up ram
	WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
void manageSlaves()
{
	if (slaveCount > 0)
	{
		for (int i = 0; i < slaveCount; i++)
		{
			const esp_now_peer_info_t* peer = &slaves[i];
			const uint8_t* peer_addr = slaves[i].peer_addr;
			Serial.print("Processing: ");

			for (int j = 0; j < 6; j++)
			{
				Serial.print((uint8_t)slaves[i].peer_addr[j], HEX);
				if (j != 5)
				{
					Serial.print(":");
				}
			}

			Serial.print(" Status: ");

			// check if the peer exists
			bool exists = esp_now_is_peer_exist(peer_addr);
			if (exists)
			{
				// Slave already paired.
				Serial.println("Already Paired");
			}
			else
			{
				// Slave not paired, attempt pair
				esp_err_t addStatus = esp_now_add_peer(peer);
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
					Serial.println("Not sure what happened");
				}
				delay(1000);
			}
		}
	}
	else
	{
		// No slave found to process
		Serial.println("No Slave found to process");
	}
}

// send data
void sendData()
{
	for (int i = 0; i < slaveCount; i++)
	{

		const uint8_t* peer_addr = slaves[i].peer_addr;
		if (i == 0)
		{
			// print only for first slave
			Serial.printf("Sending: %i, %i, %f, %f\n", myData.elevation_angle, myData.azimuth_angle, myData.master_pv_voltage, myData.master_battery_voltage);

		}

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
}

// callback when data is sent from Master to Slave
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
	char macStr[18];
	int sendTime = millis();
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	Serial.print("Last Packet Sent to: ");
	Serial.println(macStr);
	Serial.print("Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback when data is received from Slave to Master
void onDataRecv(const uint8_t* mac_add, const uint8_t* data, int data_len)
{
	char macStr[18];
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac_add[0], mac_add[1], mac_add[2], mac_add[3], mac_add[4], mac_add[5]);

	Serial.print("Last Packet Recv from: ");
	Serial.println(macStr);
	Serial.print("Last Packet Recv Data: ");
	Serial.println(*data);
	Serial.println("");
}
