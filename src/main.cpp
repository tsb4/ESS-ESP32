
#include "Ultrasonic.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include<string.h>
#include<time.h>
#include<ctime>

time_t tt;
 
 

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
  Serial.begin(9600);
  
  delay(4000);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
  digitalWrite(led_green, HIGH);
  digitalWrite(led_red, HIGH);
  delay(5000);
  digitalWrite(led_green, LOW); 
}
 
void loop()
{
  /* if(state==0){
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
    
  }*/ 
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
    state=1;
  }
  if(state==1){
    //Tenta mandar de volta a autenticação
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

      HTTPClient http;   
    
      http.begin("http://192.168.43.119:3000/confirmarQuentinha"); 
      //http.begin("https://cin.ufpe.br/~tsb4/temp/");  //Specify destination for HTTP request
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
        state = 1;
    
      }
    
      http.end();  //Free resources
    
    }else{
    
      Serial.println("Error in WiFi connection");   
    
    }
 
    delay(1000);
    
  }


  
}
