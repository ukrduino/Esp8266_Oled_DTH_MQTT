#include <DHT.h>
#include <SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Credentials.h"

#define DHTPIN 2 // D4     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);

// Update these with values suitable for your network.

const char* mqtt_server = MQTT_SERVER_IP;
WiFiClient espClient;
PubSubClient client(espClient);
String outTemperature = "0";
String sensorTemperature = "0";
String sensorHumidity = "0";
unsigned long lastGetSensorData = 0;
int getSensorDataPeriod = 15000;
int currentScreen = 0;
int nextScreen = 1;
unsigned long lastScreenChange = 0;
int screenDisplayPeriod = 3000;


void setup_wifi() {
	delay(50);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println("");
	if (strcmp(topic, "ESP8266_1/showOutTemp") == 0) {
		char* buffer = (char*)payload;
		buffer[length] = '\0';
		outTemperature = String(buffer);
	}
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266_1")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			client.publish("ESP8266_1/status", "ESP8266_1 connected");
			// ... and resubscribe
			client.subscribe("ESP8266_1/showOutTemp");
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup() {
	Serial.begin(115200);
	dht.begin();
	// Setup wifi
	//setup_wifi();
	//client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
	//client.setCallback(callback);
	//// Initialising the UI will init the display too.
	display.init();
	display.flipScreenVertically();// If needed
	display.setFont(ArialMT_Plain_24);
}


void loop() {
	getDTHSensorData();
	//// process MQTT connection
	//if (!client.connected()) {
	//	reconnect();
	//}
	//client.loop();
	//// clear the display
	processDisplay();
}

void getDTHSensorData() {
	long now = millis();
	if (now - lastGetSensorData > getSensorDataPeriod) {
		lastGetSensorData = now;
		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		float h = dht.readHumidity();
		// Read temperature as Celsius (the default)
		float t = dht.readTemperature();
		// Read temperature as Fahrenheit (isFahrenheit = true)
		// Check if any reads failed and exit early (to try again).
		if (isnan(h) || isnan(t)) {
			Serial.println("Failed to read from DHT sensor!");
			return;
		}
		Serial.print("Humidity: ");
		Serial.print(h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		Serial.println(" *C ");
		sensorHumidity = String(h, 1);
		sensorTemperature = String(t, 1);
		//publishSensorData();
	}
}

void publishSensorData() {
	client.publish("ESP8266_1/temperature", String(sensorTemperature).c_str());
	client.publish("ESP8266_1/temperature", String(sensorHumidity).c_str());
}

void processDisplay() {
	long now = millis();
	if (now - lastScreenChange > screenDisplayPeriod) {
		lastScreenChange = now;
		switch (nextScreen) {
		case 0:
			// statements
			Serial.print(outTemperature);
			nextScreen = 1;
			showOutTemp();
			break;
		case 1:
			// statements
			Serial.print(sensorTemperature);
			nextScreen = 2;
			showInTemp();
			break;
		case 2:
			// statements
			Serial.print(sensorHumidity);
			nextScreen = 0;
			showInHumidity();
			break;
		default:
			// statements
			break;
		}
	}
}


void showOutTemp() {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(20, 0, "OUT Temperature");
	display.setFont(Monospaced_plain_32);
	display.drawString(20, 10, outTemperature);
	// write the buffer to the display
	display.display();
}

void showInTemp() {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(20, 0, "IN Temperature");
	display.setFont(Monospaced_plain_32);
	display.drawString(20, 10, sensorTemperature);
	// write the buffer to the display
	display.display();
}

void showInHumidity() {
	display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(20, 0, "IN Humidity");
	display.setFont(Monospaced_plain_32);
	display.drawString(20, 10, sensorHumidity);
	// write the buffer to the display
	display.display();
}

