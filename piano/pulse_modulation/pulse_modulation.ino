int POT_PIN = A0;
int PWM_PIN = D9;

int last = 0;
int val = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);
}

void loop() {
  val = (analogRead(POT_PIN) >> 7) * 8; // 0 - 224

  if (val != last) {
    Serial.print("POT_Value:");
    Serial.println(val);
    analogWrite(PWM_PIN, val);
    last = val;
  }

  delay(100);
}