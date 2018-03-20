//Local MQTT demo with no authorization

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char* ssid = "ENTER-SSID";
const char* password = "ENTER-PASSKEY";
const char* mqtt_server = "ENTER-IP-ADDRESS";
#define mqtt_port 1883
#define MQTT_USER "surgemq"
#define MQTT_PASSWORD "verysecret"
#define MQTT_SERIAL_PUBLISH_CH "send"
#define MQTT_SERIAL_RECEIVER_CH "rec"
#define ID_MAP_LENGTH 5


//GPIO Config
/* LED pin */
byte ledPin = 14;
/* pin that is attached to interrupt */
byte interruptPin = 12;
/* hold the state of LED when toggling */
volatile byte state = LOW;


WiFiClient wifiClient;
/*id - GPIO
 * 1 - 4
 * 2 - 5
 * 3 - 16
 * 4 - 17
 */
int iomap[ID_MAP_LENGTH]={0,4,5,16,17};

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "surgemq";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("hello", "hello world 123");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte *payload, unsigned int length) {
  //print recevied messages on the serial console
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& message = jsonBuffer.parseObject((char *)payload);
    if (!message.success()) {
      Serial.println("JSON parse failed");  
      return;
    }
    // loop through each switch and swith IO sate
    JsonArray& switches=message["switches"];
    int i=0,id,io_status;
    for(i=0;i<switches.size();i++){
      id=switches[i]["id"];
      io_status=switches[i]["status"];
      digitalWrite(iomap[id],io_status);
    }
    
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  int i=0;
  for(i=0;i<ID_MAP_LENGTH;i++){
    pinMode(iomap[i],OUTPUT);
  }
//GPIO COnfig Start
pinMode(ledPin, OUTPUT);
  /* set the interrupt pin as input pullup*/
  pinMode(interruptPin, INPUT_PULLUP);
  /* attach interrupt to the pin
  function blink will be invoked when interrupt occurs
  interrupt occurs whenever the pin change value */
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, HIGH );

  //GPIO COnfig end
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

bool publish = false;
void loop() {
   if(publish)
   {
   client.publish("Thing", "D01,1");
   Serial.print("\nButtonPressed:");  
   publish = false;
   }
   client.loop();
 }

void blink() {
  publish = true;
  //client.publish("hello", "ButtonPressed");
  //Serial.print("ButtonPressed:");  
  
}
