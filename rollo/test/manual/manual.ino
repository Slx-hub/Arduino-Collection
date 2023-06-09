#define EN_PIN           2 // Enable
#define DIR_PIN          3 // Direction
#define STEP_PIN         5 // Step
#define VOLTAGE          A5

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
}

void loop() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(500);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(500);
}
