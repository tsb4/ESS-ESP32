#include <Ultrasonic.h>
 

#define pino_trigger 4
#define pino_echo 5

int state = 0;
 

Ultrasonic ultrasonic(pino_trigger, pino_echo);
 
void setup()
{
  Serial.begin(9600);
  
}
 
void loop()
{
  if(state==0){
    //espera pelo comando para fazer a autenticação do lixo
    //Ao fazer, state=1
    state=1;
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
    state=0;
  }


  
}