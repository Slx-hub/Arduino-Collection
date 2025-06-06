int POT_PIN = A0;
int OUT1_PIN = D9;
int OUT2_PIN = D8;

int LED_PIN1 = D5;
int LED_PIN2 = D6;

int last = 0;
int val = 0;

int init_acc_time = 13;
int total_travel_time = 50;


void setup() {
  pinMode(OUT1_PIN, OUTPUT);
  pinMode(OUT2_PIN, OUTPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  digitalWrite(OUT1_PIN, LOW);
  digitalWrite(OUT2_PIN, LOW);
  
  Serial.begin(9600);
}

void command_init(int targetMask, int duration) {
  if (duration <= 0) {
    return;
  }
  if (targetMask % 2 == 1) {
    digitalWrite(OUT1_PIN, HIGH);
  }
  if (targetMask > 1) {
    digitalWrite(OUT2_PIN, HIGH);
  }
  digitalWrite(LED_PIN1, HIGH);
  delay(duration);
  digitalWrite(OUT1_PIN, LOW);
  digitalWrite(OUT2_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
}

void command_push(int targetMask, int duration) {
  if (duration <= 0) {
    return;
  }
  int idle_time = max(0, (total_travel_time - init_acc_time) - duration);
  if (targetMask % 2 == 1) {
    digitalWrite(OUT1_PIN, HIGH);
  }
  if (targetMask > 1) {
    digitalWrite(OUT2_PIN, HIGH);
  }
  digitalWrite(LED_PIN1, HIGH);
  delay(init_acc_time);
  digitalWrite(OUT1_PIN, LOW);
  digitalWrite(OUT2_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
  delay(idle_time);
  if (targetMask % 2 == 1) {
    digitalWrite(OUT1_PIN, HIGH);
  }
  if (targetMask > 1) {
    digitalWrite(OUT2_PIN, HIGH);
  }
  digitalWrite(LED_PIN1, HIGH);
  delay(max(total_travel_time - init_acc_time - idle_time, duration - idle_time));
  digitalWrite(OUT1_PIN, LOW);
  digitalWrite(OUT2_PIN, LOW);
  digitalWrite(LED_PIN1, LOW);
}

void command_hold(int targetMask, int force, int duration) {
  command_push(targetMask, force);
  duration = duration*1000;
  digitalWrite(LED_PIN2, HIGH);
  for(int i=0;i<duration;i++) {
    if (i%4>0) {
      if (targetMask % 2 == 1) {
        digitalWrite(OUT1_PIN, HIGH);
      }
      if (targetMask > 1) {
        digitalWrite(OUT2_PIN, HIGH);
      }
    } else {
      digitalWrite(OUT1_PIN, LOW);
      digitalWrite(OUT2_PIN, LOW);
    }
  }
  
  digitalWrite(OUT1_PIN, LOW);
  digitalWrite(OUT2_PIN, LOW);
  digitalWrite(LED_PIN2, LOW);
}

void command_init_demo(int targetMask) {
  int min = 25;
  int max = 50;
  if (targetMask % 2 == 1) {
    for (int i=min; i<max; i++) {
      command_init(1, i);
      delay(500);
    }
  }
  if (targetMask > 1) {
    for (int i=min; i<max; i++) {
      command_init(2, i);
      delay(500);
    }
  }
}

void command_demo(int targetMask) {
  int min = 15;
  int max = 40;
  if (targetMask % 2 == 1) {
    for (int i=min; i<max; i+=3) {
      command_hold(1, i, 200);
      delay(400);
    }
  }
  if (targetMask > 1) {
    for (int i=min; i<max; i+=3) {
      command_hold(2, i, 200);
      delay(400);
    }
  }
}

void loop() {
  if (Serial.available()) {
    int targetMask = 1;
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("M")) {
      command = command.substring(1);
      targetMask = 3;
    }

    if (command.startsWith("R")) {
      command = command.substring(1);
      targetMask = 2;
    }

    if (command.startsWith("INIT ")) {
      int duration = command.substring(5).toInt();
      command_init(targetMask, duration);
    }
    if (command.startsWith("PUSH ")) {
      int duration = command.substring(5).toInt();
      command_push(targetMask, duration);
    }
    if (command.startsWith("DEMO")) {
      command_demo(targetMask);
    }
    if (command.startsWith("INIT_DEMO")) {
      command_init_demo(targetMask);
    }
    if (command.startsWith("HOLD")) {
      int spaceIndex = command.indexOf(' ', 5);
      if (spaceIndex > 5) {
        int force = command.substring(5, spaceIndex).toInt();
        int duration = command.substring(spaceIndex + 1).toInt();
        command_hold(targetMask, force, duration);
      }
    }
  }
}