#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "milligram_min_css.h"
#include "connexion.h"

#define PIN_RED_LED 5
#define PIN_GREEN_LED 4


const char index_html[] PROGMEM = R"*****(
<!doctype html>
<html lang="fr">
	<head>
			<meta charset="utf-8">
			<title>LED driver</title>
			<meta name="viewport" content="width=device-width, initial-scale=1.0, minimal-ui">
			<link rel="stylesheet" href="/milligram.min.css">
	</head>
	<body>
		<h1>LED state</h1>
		<h2 id="LEDState">LED %LED%</h2>
		<h1>LED command</h1>
		<div class="container">
			<div class="row">
				<div class="column column-25 column-offset-20">
					<button class="button" onclick=CallServer("/switchLedOn",ProcessResponse)>LED On</button>
				</div>
				<div class="column column-25 column-offset-10">
					<button class="button button-outline" onclick=CallServer("/switchLedOff",ProcessResponse)>LED Off</button>
				</div>
			</div>
		</div>
		<script>
			function CallServer(url, cFunction) {
				var xhttp = new XMLHttpRequest();
				xhttp.onreadystatechange = function() {
					if (this.readyState === 4 && this.status === 200) { 
						cFunction(this);
					}
				};
				xhttp.open("GET",url, true);
				xhttp.send();
			}
			function ProcessResponse(xhttp) {
				document.getElementById("LEDState").innerHTML = "LED " + ((xhttp.responseText == "1")?"On":"Off");
			}
		</script>		
	</body>
</html>
)*****";


ESP8266WebServer serverWeb(80);

void onConnected(const WiFiEventStationModeConnected& event);
void onGotIP(const WiFiEventStationModeGotIP& event);

void switchLedOn() {
	digitalWrite(PIN_GREEN_LED,HIGH);
	//webServer.send(200,"text/html","Led On");
	//handleRoot()
	//webServer.send(200,"text/plain","On");
  serverWeb.send(200,"text/plain","1");
}
void switchLedOff() {
	digitalWrite(PIN_GREEN_LED,LOW);
	//webServer.send(200,"text/html","Led Off")
	//handleRoot();
	//webServer.send(200,"text/plain","Off");
	serverWeb.send(200,"text/plain","0");
}

void handleRoot() {
	String temp(reinterpret_cast<const __FlashStringHelper *>(index_html));
	if (digitalRead(PIN_GREEN_LED) == HIGH) 
		temp.replace("%LED%","On");
	else
		temp.replace("%LED%","Off");
	serverWeb.send(200,"text/html",temp);
}

// API
// ?cmd=getLED
// ?cmd=setLED&value=ON
void handleAPI_html() {
	String response;
	if (serverWeb.args() > 0) {
		if (serverWeb.argName(0) == "cmd") {
			if (serverWeb.args() == 1) {
				if (serverWeb.arg(0) == "getLED") {
					if (digitalRead(PIN_GREEN_LED) == LOW) response = "0"; else response="1";
				}
				else response = "Unknown command";
			}
			else if (serverWeb.args() == 2) {
				if (serverWeb.arg(0) == "setLED") {
					if (serverWeb.argName(1) == "value") {
						if (serverWeb.arg(1) == "ON") {
							digitalWrite(PIN_GREEN_LED, HIGH);
							response = "1";
						}
						else if (serverWeb.arg(1) == "OFF") {
							digitalWrite(PIN_GREEN_LED, LOW);
							response = "0";
						}
						else response = "Unknown value";
					} 
					else response = "Incorrect second parameter";
				} 
				else response = "Unknown command";
			}
			else response = "Unknown command or wrong parameter number";
		}
		else response = "Unknown prarmeter";
		serverWeb.send(200,"text/plain", response);
	}
	else {
		response = "API call without parameter ...";
		serverWeb.send(200,"text/plain",response);
	}
}

void setup() {
	Serial.begin(115200L);
	Serial.println("");
	
	pinMode(PIN_RED_LED, OUTPUT);
	pinMode(PIN_GREEN_LED, OUTPUT);
	
	WiFi.mode(WIFI_STA);
	
	WiFi.softAP("LED Object");
	// 192.168.4.1
	
	WiFi.begin(SSID, PASSWORD);
	
	static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
	static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);
	
	serverWeb.on("/switchLedOn", switchLedOn);
	serverWeb.on("/switchLedOff", switchLedOff);
	serverWeb.on("/", handleRoot);
	serverWeb.on("/index_html", handleRoot);
	serverWeb.on("/milligram.min.css", [] {
		serverWeb.send(200, "text/css", MILLIGRAM_MIN_CSS);
	});
	serverWeb.on("/api.html",handleAPI_html);
	serverWeb.begin();
	
	
}

void loop() {
	if (WiFi.isConnected()) {
		digitalWrite(PIN_RED_LED, HIGH);
		serverWeb.handleClient();
	} else {
		digitalWrite(PIN_RED_LED, LOW);
	}
}

void onConnected(const WiFiEventStationModeConnected& event) {
	Serial.println("WiFi is connected");
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
		Serial.println("IP Address: " + WiFi.localIP().toString());
		Serial.println("IP Gateway: " + WiFi.gatewayIP().toString());
		Serial.println("IP DNS : " + WiFi.dnsIP().toString());
		Serial.print("Receiving power : ");
		Serial.println();
		Serial.println(WiFi.RSSI());
}
	
