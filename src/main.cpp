
#include "Ultrasonic.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include<string.h>
#include<time.h>
#include<ctime>
 
 

#define pino_trigger 4
#define pino_echo 5

#define led_red 19
#define led_green 23

//const char* ssid = "Moto G (5) 3330";
//const char* password = "viniciusserra";

const char* ssid = "AndroidAP442B";
const char* password = "uddx0298";

int state = 0;


 

Ultrasonic ultrasonic(pino_trigger, pino_echo);
 
void setup()
{
  
  
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  digitalWrite(led_green, HIGH);
  digitalWrite(led_red, HIGH);
  delay(5000);
  digitalWrite(led_red, LOW); 
  Serial.begin(9600);
  
  delay(4000);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
  
}
 
void loop()
{
  if(state==0){
    digitalWrite(led_red, HIGH);
    digitalWrite(led_green, LOW);
    float dist = 15.0;
    while (dist>10.0){
      long microsec = ultrasonic.timing();
      dist = ultrasonic.convert(microsec, Ultrasonic::CM);
      Serial.print("Dist: ");
      Serial.println(dist);
      delay(100);
    }
    Serial.println("Foi");
    digitalWrite(led_green, HIGH);
    state=1;
  }
  if(state==1){
    digitalWrite(led_red, LOW);
    delay(500);
    digitalWrite(led_red, HIGH);
    //Tenta mandar de volta a autenticação
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

      HTTPClient http;   
    
      http.begin("http://192.168.43.61:3000/confirmarQuentinha"); 
      http.addHeader("Content-Type", "text/plain");
    
      int httpResponseCode = http.POST("POSTING from ESP32");
    
      if(httpResponseCode>0){
    
        String response = http.getString();                      
    
        Serial.println(httpResponseCode);   
        Serial.println(response);           
        state=0;
        digitalWrite(led_red, LOW);
        delay(2000);
    
      }else{
    
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        state = 1;
    
      }
    
      http.end();  //Free resources
    
    }else{
    
      Serial.println("Error in WiFi connection");   
    
    }
 
    delay(1000);
    
  }


  
}
