#include <SimpleFOC.h>

// init BLDC motor
BLDCMotor motor = BLDCMotor( 7, 10, 128 );
// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 8);
// Magnetic sensor instance
MagneticSensorPWM AS5x = MagneticSensorPWM(5, 115, 4351, 128, 4223);

// instantiate the commander
//Commander command = Commander(Serial);
//void doTarget(char* cmd) { command.scalar(&motor.target, cmd); }
//void doLimitCurrent(char* cmd) { command.scalar(&motor.current_limit, cmd); }

void setup() {

  pinMode(12,OUTPUT); // Low current ground pin
  Serial.begin(115200);

  SimpleFOCDebug::enable(&Serial);

  AS5x.init();
  motor.linkSensor(&AS5x);

  driver.voltage_power_supply = 12; // [Volts]
  driver.init();
  motor.linkDriver(&driver);

  motor.current_limit = 0.1;   // [Amps]
  motor.controller = MotionControlType::velocity;

  // init motor hardware
  motor.init();
  motor.initFOC();

  // add target command T
  //command.add('T', doTarget, "target velocity");
  //command.add('C', doLimitCurrent, "current limit");

  Serial.println("Motor ready!");
  Serial.println("Set target velocity [rad/s]");
  _delay(1000);
}

void loop() {
  motor.loopFOC();
  motor.move();

  // user communication
  //command.run();
}