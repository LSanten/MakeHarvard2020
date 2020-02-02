#include <Adafruit_L3GD20_U.h>

Adafruit_L3GD20_Unified gyro();                   //gyro library I2C initialization


void setup() {
  // put your setup code here, to run once:
  bool begin(gyroRange_t L3DS20_RANGE_250DPS);
  // Try to initialise and warn if we couldn't detect the chip
    if (!gyro.begin(gyro.L3DS20_RANGE_250DPS))
  {
    Serial.println("Oops ... unable to initialize the L3GD20. Check your wiring!");
    while (1);
  }



}

void loop() {
  // put your main code here, to run repeatedly:

}
