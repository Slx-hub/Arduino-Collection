#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
 
int potpin = A0;  // analog pin used to connect the potentiometer
int datapin = 3; // pin to connect to the ESC
int val;    // variable to read the value from the analog pin 
 
void setup() 
{ 
  myservo.attach(datapin);
  pinMode(potpin, INPUT);
  Serial.begin(9600);
} 
 
void loop() 
{ 
  val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180) 

  Serial.println(val);
  Serial.print(" ");
  
  myservo.write(val);                  // sets the servo position according to the scaled value 
  delay(15);                           // waits for the servo to get there 
}
