/*
  Conti Alessandro - 10710583
  Casali Sara - 10727955
  Lazzeri Matteo - 10710962
*/


#include <WiFi.h>
#include <esp_now.h>


#define us_TO_s 1000000 //conversion factor for micro seconds to seconds
#define SOUND_SPEED 0.0343 //cm/us - it is used by ultrasonic distance sensor
#define CAR_DISTANCE 50.00 //cm - estimate the presence of a car in a parking spot
#define DUTY_CYCLE_PERIOD  38 //s - duty cycle period 


uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xFB, 0x90}; //receiver MAC address
esp_now_peer_info_t peerInfo; //saves the peer information 

const int trigPin = 5; //trig
const int echoPin = 27; //echo


//sending callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //print if the message was send correctly
  Serial.println("Send Status: " + String(status == ESP_NOW_SEND_SUCCESS ? "Ok" : "Error"));
}

//receiving Callback
void OnDataRecv(const uint8_t * mac, const uint8_t *data, int len) {
  char receivedString[len];
  memcpy(receivedString, data, len);
  //print the received message
  Serial.println("Message received: " + String(receivedString));
}

//enables wifi
void enableWiFi() {
  //setting the wifi in station mode
  WiFi.mode(WIFI_STA);
  //tx power property
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  //initialize ESP-NOW
  esp_now_init();
  //send callback - it is not necessary in the parking spot node
  esp_now_register_send_cb(OnDataSent);
  //receive callback - it is not necessary in the parking spot node 
  esp_now_register_recv_cb(OnDataRecv);

  //peer registration
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //save peer channel
  peerInfo.channel = 0;  
  //save if the peer is encrypted
  peerInfo.encrypt = false;
  //add peer
  esp_now_add_peer(&peerInfo);
}

//goes to deep sleep mode
void goToDeepSleep() {
  //configure wakeup timer
  esp_sleep_enable_timer_wakeup(DUTY_CYCLE_PERIOD * us_TO_s);

  //enter deep sleep with the configured wakeup options
  esp_deep_sleep_start();
}

//reads if the parking is free or not
String readOccupacy() {
  //clears trig pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(1);

  //sets the trig pin on High state for 1.micros
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //reads the echo pin --> Measure the high pulse length to get the distance
  long duration = pulseIn(echoPin, HIGH);

  //calculate distance - cm
  long distanceCm = duration * SOUND_SPEED/2;

  //return if the parking is FREE or not  
  return String((float) distanceCm < (float) CAR_DISTANCE ? "OCCUPIED" : "FREE");
}


void setup(){
  //start the serial communication
  Serial.begin(115200);

  //set HC-SR04 pin
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //save occupacy
  String message = readOccupacy();

  //enable wifi
  enableWiFi();

  //send the message
  esp_now_send(broadcastAddress, (uint8_t*)message.c_str(), message.length() + 1);  
  //delay to permit to receive the broadcast message
  delay(2);

  //go to deep sleep
  goToDeepSleep();
}

void loop(){
  //this function is empty because when the node wakes up after deep sleep state restarts from setup.
}
