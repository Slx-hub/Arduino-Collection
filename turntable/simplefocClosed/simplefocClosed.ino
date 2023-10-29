#include <SimpleFOC.h>

// init BLDC motor
BLDCMotor motor = BLDCMotor( 7, 10, 128 );
// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 8);
// instance of AS5600 sensor
MagneticSensorI2C encoder = MagneticSensorI2C(AS5600_I2C);

// instantiate the commander
Commander command = Commander(Serial);

void doVelocity(char* cmd) { motor.controller = MotionControlType::velocity; command.scalar(&motor.target, cmd); }
void doPosition(char* cmd) { motor.controller = MotionControlType::angle; command.scalar(&motor.target, cmd); }
void doMotor(char* cmd) { command.motor(&motor, cmd); }

void doLimitCurrent(char* cmd) { command.scalar(&motor.current_limit, cmd); }

void setup() {

  pinMode(12,OUTPUT); // Low current ground pin
  Serial.begin(115200);

  SimpleFOCDebug::enable(&Serial);

  encoder.init();
  motor.linkSensor(&encoder);

  driver.voltage_power_supply = 12; // [Volts]
  driver.init();
  motor.linkDriver(&driver);

  motor.current_limit = 0.1;   // [Amps]
  motor.controller = MotionControlType::velocity;

  motor.PID_velocity.P = 0.1;
  motor.PID_velocity.I = 30;
  motor.PID_velocity.output_ramp = 1000;

  // init motor hardware
  motor.init();
  motor.initFOC();

  // add target command T
  command.add('P', doPosition, "target position");
  command.add('V', doVelocity, "target velocity");
  command.add('C', doLimitCurrent, "current limit");
  command.add('M', doMotor ,"my motor");

  Serial.println("Motor ready!");
  _delay(1000);
}

void loop() {
  motor.loopFOC();
  motor.move();

  // user communication
  command.run();
}