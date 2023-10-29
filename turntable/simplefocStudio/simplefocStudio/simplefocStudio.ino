#include <SimpleFOC.h>

// init BLDC motor
BLDCMotor motor = BLDCMotor( 7, 10, 128 );
// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 8);
// instance of AS5600 sensor
MagneticSensorI2C encoder = MagneticSensorI2C(AS5600_I2C);

// include commander interface
Commander command = Commander(Serial);
void doMotor(char* cmd) { command.motor(&motor, cmd); }

void setup(){

  pinMode(12,OUTPUT); // Low current ground pin
  Serial.begin(115200);

  // add the motor to the commander interface
  // The letter (here 'M') you will provide to the SimpleFOCStudio
  command.add('M',doMotor,'motor');
  // tell the motor to use the monitoring
  motor.useMonitoring(Serial);

  encoder.init();
  motor.linkSensor(&encoder);

  driver.voltage_power_supply = 12; // [Volts]
  driver.init();
  motor.linkDriver(&driver);

  motor.init();
  motor.initFOC();

  Serial.println("Motor ready!");
  _delay(1000);
}
void loop(){
  motor.loopFOC();
  motor.move();

  // real-time monitoring calls
  motor.monitor();
  // real-time commander calls
  command.run();
}