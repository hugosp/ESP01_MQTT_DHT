#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
 
#define DHTPIN 2
#define DHTTYPE AM2301 // DHT 21 ?
 
const int lamp_pin = 0;
 
DHT dht(DHTPIN, DHTTYPE, 20);
 
const char* ssid =   "...........";
const char* password =   "...........";
const char* mqtt_server = "x.x.x.x"; // IP of broker
 
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int interval = 30000;
 
 
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
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
  Serial.println();
 
  if ((char)payload[0] == '1') {
    digitalWrite(lamp_pin, HIGH);
  } else {
    digitalWrite(lamp_pin, LOW);
  }
 
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("switches/esp01/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
 
 
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(lamp_pin, OUTPUT);
 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
 
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    int h = (int)dht.readHumidity();
    int t = (int)dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    snprintf (msg, 75, "%ld", t);
    client.publish("sensors/esp01/temperature",msg);
    Serial.print("DTH sensor sent Temperature : ");
    Serial.println(msg);
    snprintf (msg, 75, "%ld", h);
    client.publish("sensors/esp01/humidity",msg);
    Serial.print("DTH sensor sent Humidity : ");
    Serial.println(msg);
 
   
    digitalWrite(BUILTIN_LED, LOW);    
    delay(1000);
    digitalWrite(BUILTIN_LED, HIGH);
  }
}
