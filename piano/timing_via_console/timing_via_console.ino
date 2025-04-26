const int bufferSize = 16;
int machineState = 0;

char incomingByte = 0;
char numberBuffer[bufferSize];

int serialParams[2];

void setup() {
  Serial.begin(9600);
}

void bufferAppend(char number) {
  for(int i=0; i<bufferSize; i++) {
    if (numberBuffer[i] == 0) {
      numberBuffer[i] = number;
      return;
    }
  }
}

void clearBuffer() {
  for(int i=0; i<bufferSize; i++) {
    numberBuffer[i] = 0;
  }
}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    if (machineState == 2) {
      machineState = 0;
    }
    // read the incoming byte:
    incomingByte = Serial.read();
    if (incomingByte != ' ' && incomingByte != 10) {
      bufferAppend(incomingByte);
      return;
    }
    serialParams[machineState] = atoi(numberBuffer);
    clearBuffer();

    machineState++;
  }

  if (machineState == 2 && serialParams[1] > 0) {
    digitalWrite(3, HIGH);
    delay(serialParams[0]);
    digitalWrite(3, LOW);
    delay(1000);
    serialParams[1]--;
  }
}
