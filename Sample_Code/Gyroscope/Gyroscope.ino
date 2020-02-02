#include <Adafruit_L3GD20_U.h>

Adafruit_L3GD20(void);
Adafruit_L3GD20 gyro();

// Define the pins for SPI
#define GYRO_CS 4 // labeled CS
#define GYRO_DO 3 // labeled SA0
#define GYRO_DI A4  // labeled SDA
#define GYRO_CLK A5 // labeled SCL

Adafruit_L3GD20 gyro(GYRO_CS, GYRO_DO, GYRO_DI, GYRO_CLK);

void setup() {
  // put your setup code here, to run once:



}

void loop() {
  // put your main code here, to run repeatedly:

}
