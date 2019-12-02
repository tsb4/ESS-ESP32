
#include "Ultrasonic.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include<string.h>
 
 

#define pino_trigger 4
#define pino_echo 5

const char* ssid = "yourNetworkName";
const char* password = "yourNetworkPassword";

int state = 0;
 

Ultrasonic ultrasonic(pino_trigger, pino_echo);
 
void setup()
{
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
    //espera pelo comando para fazer a autenticação do lixo
    //Ao fazer, state=1
    if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
      HTTPClient http;
  
      http.begin("http://jsonplaceholder.typicode.com/comments?id=10"); //Specify the URL
      int httpCode = http.GET();                                        //Make the request
  
      if (httpCode > 0) { //Check for the returning code
  
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        String teste = String("oi");
        if(payload.equals(teste)) state=1;
        else state = 0;

      }
  
      else {
        Serial.println("Error on HTTP request");
        state = 0;
      }
  
      http.end(); //Free the resources
    }
    
  } 
  if(state==1){
    float dist = 15.0;
    while (dist>10.0){
      long microsec = ultrasonic.timing();
      dist = ultrasonic.convert(microsec, Ultrasonic::CM);
      Serial.print("Dist: ");
      Serial.println(dist);
      delay(100);
    }
    Serial.println("Foi");
    state=2;
  }
  if(state==2){
    //Tenta mandar de volta a autenticação
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

      HTTPClient http;   
    
      http.begin("http://jsonplaceholder.typicode.com/posts");  //Specify destination for HTTP request
      http.addHeader("Content-Type", "text/plain");             //Specify content-type header
    
      int httpResponseCode = http.POST("POSTING from ESP32");   //Send the actual POST request
    
      if(httpResponseCode>0){
    
        String response = http.getString();                       //Get the response to the request
    
        Serial.println(httpResponseCode);   //Print return code
        Serial.println(response);           //Print request answer
        state=0;
    
      }else{
    
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        state = 2;
    
      }
    
      http.end();  //Free resources
    
    }else{
    
      Serial.println("Error in WiFi connection");   
    
    }
 
    delay(1000);
    
  }


  
}
