#include <AWS_IOT.h>
#include <WiFi.h>

AWS_IOT hornbill;

char WIFI_SSID[] = "WIFI-SSID";
char WIFI_PASSWORD[] = "PASSWORD";
char HOST_ADDRESS[] = "iot.ap-southeast-1.amazonaws.com";
char CLIENT_ID[] = "ThingName";
char TOPIC_NAME[] = "iotworkshop";


int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}

//GPIO Config
/* LED pin */
byte ledPin = 14;
/* pin that is attached to interrupt */
byte interruptPin = 12;
/* hold the state of LED when toggling */
volatile byte state = LOW;

bool publish = false;
void blink() {
  publish = true;
}


void setup() {

  //GPIO COnfig Start
  pinMode(ledPin, OUTPUT);
  /* set the interrupt pin as input pullup*/
  pinMode(interruptPin, INPUT_PULLUP);
  /* attach interrupt to the pin
    function blink will be invoked when interrupt occurs
    interrupt occurs whenever the pin change value */
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, HIGH );

  //GPIO COnfig end


  Serial.begin(115200);
  delay(2000);

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("Connected to wifi");

  if (hornbill.connect(HOST_ADDRESS, CLIENT_ID) == 0)
  {
    Serial.println("Connected to AWS");
    delay(1000);

    if (0 == hornbill.subscribe(TOPIC_NAME, mySubCallBackHandler))
    {
      Serial.println("Subscribe Successfull");
    }
    else
    {
      Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
      while (1);
    }
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }

  delay(2000);

}
void loop() {

  if (msgReceived == 1)
  {
    msgReceived = 0;
    Serial.print("\nReceived Message:");
    Serial.println(rcvdPayload);
  }

  if (publish)
  {
    sprintf(payload, "{\"CD\":\"espresso,%d\"}", msgCount++);
    if (hornbill.publish(TOPIC_NAME, payload) == 0)
    {
      Serial.print("\nPublish Message:");
      Serial.println(payload);
    }
    else
    {
      Serial.println("Publish failed");
    }
    Serial.print("\nButtonPressed:");
    publish = false;
  }
  vTaskDelay(1000 / portTICK_RATE_MS);
}
