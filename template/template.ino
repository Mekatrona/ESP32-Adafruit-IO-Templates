#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>

const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttUser = "username";
const char* mqttPassword = "password";
const char* clientId = "ESP32-XXX";

String feedId = "sensor";
String publishFeedUrl = ""; // Adafruit IO feed structure. Will update after user input!


#include <Ticker.h>
Ticker ticker;
int LED_PIN = 2;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void tick()
{
  //toggle state
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));     // set pin to the opposite state
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  ticker.attach(0.6, tick);
  WiFiManager wm;

  //reset settings - for testing
  wm.resetSettings();

  // MQTT input fields for the manager
  WiFiManagerParameter customNodeId("id", "Node ID", clientId, 40);
  wm.addParameter(&customNodeId);
  clientId = customNodeId.getValue();

  WiFiManagerParameter customMqttServer("server", "MQTT Server", mqttServer, 40);
  wm.addParameter(&customMqttServer);
  mqttServer = customMqttServer.getValue();

  WiFiManagerParameter customMqttUser("user", "User", mqttUser, 40);
  wm.addParameter(&customMqttUser);
  mqttUser = customMqttUser.getValue();

  WiFiManagerParameter customMqttPassword("password", "Password", mqttPassword, 40);
  wm.addParameter(&customMqttPassword);
  mqttPassword = customMqttPassword.getValue();

 
  
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect("ESP32 Node","MyEspIsFantastic")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }

  Serial.println("Connected to Wifi");
  ticker.detach();
  digitalWrite(LED_PIN, LOW);


  // Setup MQTT connection
  mqttClient.setServer(mqttServer, mqttPort);

  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");

    boolean connectionSucceeded = false;
    if (strlen(mqttPassword) == 0) {
      connectionSucceeded = mqttClient.connect(String(clientId).c_str());
    } else {
      connectionSucceeded = mqttClient.connect(String(clientId).c_str(), mqttUser, mqttPassword );
    }
    
    if (connectionSucceeded) {
      Serial.print("Connected to ");
      Serial.println(mqttServer);

      // Adafruit IO feed url structure
      publishFeedUrl = "";
      publishFeedUrl += mqttUser;
      publishFeedUrl += "/feeds/";
        
    } else {
      Serial.print("Failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

long lastRequest = millis();
void loop() {

  int sensorValue = 123;
  
  // Repetedly publish sensor values every X ms
  long now = millis();
  long publishTime = 10000;  // 10s
  if (now - lastRequest >= publishTime) {

    char value[100];
    sprintf(value, "%d", sensorValue);
    mqttClient.publish((char*)publishFeedUrl.c_str(), value);
    lastRequest = now;
    Serial.print("Published ");
    Serial.println(value);
  }

}
