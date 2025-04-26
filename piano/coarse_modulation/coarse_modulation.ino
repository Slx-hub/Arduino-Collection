int POT_PIN = A0;
int OUT_PIN = D9;

int LED_PIN1 = D5;
int LED_PIN2 = D6;

int last = 0;
int val = 0;

int init_acc_time = 13;
int total_travel_time = 50;


void setup() {
  Serial.begin(9600);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  digitalWrite(OUT_PIN, LOW);
}

void command_init(int duration) {
  if (duration <= 0) {
    return;
  }
  digitalWrite(OUT_PIN, HIGH);
  digitalWrite(LED_PIN1, HIGH);
  delay(duration);
  digitalWrite(OUT_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
}

void command_push(int duration) {
  if (duration <= 0) {
    return;
  }
  int idle_time = max(0, (total_travel_time - init_acc_time) - duration);
  digitalWrite(OUT_PIN, HIGH);
  digitalWrite(LED_PIN1, HIGH);
  delay(init_acc_time);
  digitalWrite(OUT_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
  delay(idle_time);
  digitalWrite(OUT_PIN, HIGH);
  digitalWrite(LED_PIN1, HIGH);
  delay(max(total_travel_time - init_acc_time - idle_time, duration - idle_time));
  digitalWrite(OUT_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
}

void command_hold(int force, int duration) {
  command_push(force);
  duration = duration*1000;
  digitalWrite(LED_PIN2, HIGH);
  for(int i=0;i<duration;i++) {
    if (i%2==0) {
      digitalWrite(OUT_PIN, HIGH);
    } else {
      digitalWrite(OUT_PIN, LOW);
    }
  }
  digitalWrite(LED_PIN2, LOW);
}

void command_init_demo() {
  for (int i=10; i<30; i++) {
    command_init(i);
    delay(500);
  }
}

void command_demo() {
  for (int i=5; i<15; i++) {
    command_hold(i, 200);
    delay(400);
  }
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("INIT ")) {
      int duration = command.substring(5).toInt();
      command_init(duration);
    }
    if (command.startsWith("PUSH ")) {
      int duration = command.substring(5).toInt();
      command_push(duration);
    }
    if (command.startsWith("DEMO")) {
      command_demo();
    }
    if (command.startsWith("INIT_DEMO")) {
      command_init_demo();
    }
    if (command.startsWith("HOLD")) {
      int spaceIndex = command.indexOf(' ', 5);
      if (spaceIndex > 5) {
        int force = command.substring(5, spaceIndex).toInt();
        int duration = command.substring(spaceIndex + 1).toInt();
        command_hold(force, duration);
      }
    }
  }
}