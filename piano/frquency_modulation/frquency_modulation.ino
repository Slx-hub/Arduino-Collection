int POT_PIN = A0;
int PWM_PIN = D9;

int val = 0;
int freq = 10;
int arbitrary_value = 10000;
float duty = 0.3;

int cycle = 0;
int on = 0;
int current = 0;

void setup() {
  //Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);
}

void loop() {
  val = analogRead(POT_PIN) >> 7; // 0 - 28
  freq = pow(1 + val, 3); // 1 - 24389


  cycle = arbitrary_value / freq;
  on = cycle * duty;
  current++;
  if (current > cycle) {
    current = 0;
  }

  //Serial.print("POT_Value:");
  //Serial.print(val);
  //Serial.print(",PWM:");
  if (current < on) {
    //Serial.println(10);
    digitalWrite(PWM_PIN, HIGH);
  } else {
    //Serial.println(0);
    digitalWrite(PWM_PIN, LOW);
  }
}