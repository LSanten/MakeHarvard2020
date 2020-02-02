

/*******************************************************************************************************************************************************************************************************************************************
 * Title: Make Harvard 2020 Project
 * Description: 
 * Robot Name: MusicMachine
 * What does code do?: Weight from scale changes frequency in MAX/MSP + LED Strip integration
 * Hardware wanings:
 * Created by Leon Santen (leon.santen@icloud.com ), January 2020
 *******************************************************************************************************************************************************************************************************************************************
 */



//==========================================================================================================================================================================================================================================
// Load supporting Arduino Libraries
//==========================================================================================================================================================================================================================================
#include <Servo.h>                                //loading ServoMotors library
#include <HX711.h>                                //library for 24-bit load cell communication
#include <Adafruit_NeoPixel.h>                    //library for Adafruit Neopixel LED Strip (GRB)


//==========================================================================================================================================================================================================================================
// Create and initialize global variables, objects, and constants (containers for all data)
//==========================================================================================================================================================================================================================================
#define LED_PIN    9                    //Adafruit LED Strip communication pin
#define LED_COUNT 10


HX711 scale(5, 6);                      //initialize scale usually (sck, dt) --> however on current board, prints are wrong, therefore swapped
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); //initialize adafruit neopixel led object


const int aliveLED = 13;                //create a name for "robot alive" blinky light pin 
const int eStopPin = 12;                //create a name for pin connected to ESTOP switch
const int trigPin = 11;                 //sonar trigger pin
const int echoPin = 10;                 //sonar echo pin
boolean aliveLEDState = true;           //create a name for alive blinky light state to be used with timer
boolean ESTOP = true;                   //create a name for emergency stop of all motors
boolean realTimeRunStop = true;         //create a name for real time control loop flag
boolean debug = false;                  //*****IMPORTANT***** ------  create a name for debug boolean - when true, OCU is active; when false, OCU is deactivated and will directly go into "main" mode
String command = "move";                //create a String object name for operator command string
String loopError = "no error";          //create a String for the real time control loop error system
unsigned long oldLoopTime = 0;          //create a name for past loop time in milliseconds
unsigned long newLoopTime = 0;          //create a name for new loop time in milliseconds
unsigned long cycleTime = 0;            //create a name for elapsed loop cycle time
const long controlLoopInterval = 95;    //create a name for control loop cycle time in milliseconds
float calibration_factor = -385;        //this calibration factor is adjusted according to my load cell // -385 MakerHawk Digital Load Cell Weight Sensor HX711
float units;                            //units for 24-bit scale
float ounces;                           //unit for scale
//STATES STATES STATES STATES STATES 
int state = 1;                          //state of machine
int subState = 0;                       //substate of machine
int oldState = 0;                       //old state
int oldSubState =0;                     //old substate
int volume = 0;                             //volume setting being sent to max
int oldVolume = 0;                          //old volume setting being sent to max
//STATES STATES STATES STATES STATES UP
int LEDColorNumber = 0;                 //led color number - starts at 0
uint32_t oldestZeroScale = 0;           //time when scale was (first of series) to measure for how long it was at 0 in order to turn off communication 
bool oldestZeroFlag = false;            //flag when oldest zero timer for scale is started
long sonarDuration;                     //duration of sonar echo
int sonarDistance;                      //distance of sonar in unknown units
bool songOnFlag = false;                //flag is true when song is sweeping up or up
uint32_t songOnTime = 0;                //time when song was on max volume



//==========================================================================================================================================================================================================================================
// Startup code to configure robot and pretest all robot functionality (to run once)
// and code to setup robot mission for launch
//==========================================================================================================================================================================================================================================
void setup() {
  // Step 1) Put your robot setup code here, to run once:
  pinMode(aliveLED, OUTPUT);                      //initialize aliveLED pin as an output
  pinMode(eStopPin, INPUT_PULLUP);                //use internal pull-up on ESTOP switch input pin 
  pinMode(trigPin, OUTPUT);                       // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);                        // Sets the echoPin as an Input
  Serial.begin(9600);                             //start serial communication
  scale.set_scale();                              //set scale object
  scale.tare();                                   //Reset the scale to 0 
  long zero_factor = scale.read_average();        //Get a baseline reading
  if (debug == true){
    Serial.print("Zero factor: ");                //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
    Serial.println(" Robot Controller Starting Up! Watch your little fingers! ");}
  strip.begin();                                  // begin adafruit neopixel LED strip
  strip.show();                                   // Initialize all pixels to 'off'
  

  // Step 2) Put your robot mission setup code here, to run once:
  uint32_t green = strip.Color(0,255,0);
  uint32_t off = strip.Color(0,0,0);
  strip.fill(green, 0, 10);
  strip.show();
  delay(700);
  strip.fill(off, 0, 10);
  strip.show();
  delay(700);
  strip.fill(green, 0, 10);
  strip.show();
  delay(700);
  strip.fill(off, 0, 10);
  strip.show();
}
//==========================================================================================================================================================================================================================================
// Flight code to run continuously until robot is powered down
//==========================================================================================================================================================================================================================================
void loop() {
  // Step 3) Put Operator-Input-to-Robot and Robot_Reports-Back-State code in non-real-time "outer" loop:
  //         Put real-time dependant sense-think-act control in the inner loop

  // GET Operator Control Unit (OCU) Input: ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-------
  if (debug == true){
    command = getOperatorInput();                         //get operator input from serial monitor
  }
  if (command == "stop") realTimeRunStop = false;       //skip real time inner loop
  else realTimeRunStop = true;                          //set loop flag to run = true

  // Step 4) Put your main flight code into "inner" soft-real-time while loop structure below, to run repeatedly,
  //         at a known fixed "real-time" periodic interval. This "soft real-time" loop timing structure, runs
  //         fast flight control code once every controlLoopInterval.

  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  while(realTimeRunStop == true) {

    // Check if operator inputs a command during real-time loop execution
    if (debug == false){
      command = "main";
    }
    else {
      if (Serial.available() > 0) {                             // check to see if operator typed at OCU
        realTimeRunStop = false;                                // if OCU input typed, stop control loop
        command = Serial.readString();                          // read command string to clear buffer
        break;                                                  // break out of real-time loop
      }
      else {realTimeRunStop = true;}                            // if no operator input, run real-time loop
    }


    // Real-Time clock control. Check to see if one clock cycle has elapsed before running this control code
    newLoopTime = millis();                                   // get current Arduino time (50 days till wrap)
                  
    if (newLoopTime - oldLoopTime >= controlLoopInterval) {   // if true run flight code
      //if (debug == true){Serial.println(newLoopTime - oldLoopTime);}      //// print loop time
      oldLoopTime = newLoopTime;                              // reset time stamp
      blinkAliveLED();                                        // toggle blinky alive light



    // SENSE sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense---sense-------------
    // TODO add sensor code here

    // THINK think---think---think---think---think---think---think---think---think---think---think---think---think-------------
    // Pick robot behavior based on operator input command typed at console
      if ( command == "stop") {
        Serial.println("Stop Robot");
        realTimeRunStop = false;                                  // exit real-time control loop
        break;
      }
      else if (command == "move") {
        Serial.println("Move robot ");
        Serial.println("Type stop to stop robot");
        realTimeRunStop = true;                                   // don't exit loop after running once
      }
      else if (command == "main") {
        int subState = weightFromScale();                         // set subState to scale reading
        setLEDStrip(units);                                       // call LED function with input from scale 
        realTimeRunStop = true;                                   // don't exit loop after running once

        //if (state != oldState or subState != oldSubState){ 
        if (units == 0 and oldestZeroFlag == false){                                      // if scale units values start to be 0, start flag and get time
          oldestZeroFlag = true;
          oldestZeroScale = millis();
        }
        
        if (units > 0 and oldestZeroFlag == true){oldestZeroFlag = false;}                //deactivate flag when values higher than 0

        int distance = getDistanceFromSonar();                         // call sonar function
        incrementVolumFromSonar(distance);                             // increment volume when person in range, slide volume down when far away
        
        if (millis() - oldestZeroScale < 1000 or state != oldState or subState != oldSubState or volume != oldVolume){ // only print states and volume when changed --> will keep sending zeros from scale for one more second to smoothen out signal in MAX
          
          Serial.print(state);
          Serial.print(" ");
          Serial.print(subState);
          Serial.print(" ");
          Serial.println(volume);

          oldState = state;
          oldSubState = subState;
          oldVolume = volume;
        }
        
        
        
      }
      else if (command == "scale") {
          scale.set_scale(calibration_factor); //Adjust to this calibration factor

          Serial.print("Reading: ");
          float weight = weightFromScale();
          Serial.print(weight);
          Serial.print(" grams"); 
          Serial.print(" calibration_factor: ");
          Serial.print(calibration_factor);
          Serial.println();                                // don't exit loop after running once
      }
      else if (command == "led") {
          setLEDStrip(100.5);
          Serial.println("LED Mode");
      }
      else if (command == "sonar") {
          int distance = getDistanceFromSonar();          // call sonar function
          Serial.print("sonarDistance: ");                // Prints the sonarDistance on the Serial Monitor
          Serial.println(distance);
          setLEDStrip(distance);
      }
      else if (command == "sonar-main") {                              // volume settings change if someone is in front of sensor
        int distance = getDistanceFromSonar();                         // call sonar function
        incrementVolumFromSonar(distance);                             // increment volume when person in range, slide volume down when far away
        Serial.println(volume);
        // function for volume incrementation

      }
      else
      {
        //Serial.println("**** WARNING **** Invalid Input, Robot Stopped, Please try again!");
        realTimeRunStop = true;
      }

    // ACT act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act---act-----------
       ESTOP = digitalRead(eStopPin);                                 // check ESTOP switch

    // Check to see if all code ran successfully on one real-time increment
    cycleTime = millis()-newLoopTime;                                 // calculate loop execution time
    if( cycleTime > controlLoopInterval + 10){                        // check cycle time and print warning when more than 10ms higher
      Serial.println("********************************************");
      Serial.println("error - real-time has failed, stop robot!");    // loop took too long to run
      Serial.print(" 1000 ms real-time loop took = ");              
      //Serial.println(cycleTime);                                      // print loop time
      Serial.println("********************************************");
      break;
    }
    } // end of "if (newLoopTime - oldLoopTime >= controlLoopInterval)" real-time loop structure
  } // end of "inner"   "while(realTimeRunStop == true)" real-time control loop
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******
  // real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******real-time-loop*******

  // SEND Robot State to Operator Control Unit (OCU) ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-----
    //Serial.println("  ");
    Serial.println("=======================================================================================");
    Serial.println("| Robot control loop stopping to wait for new command ");         // send robot status to operator
    if (ESTOP == true) Serial.println("| Robot motors E-Stopped by external switch"); // send E-Stop message to OCU



} // End of "outer" void loop()
//==========================================================================================================================================================================================================================================
// END OF Flight Code
//==========================================================================================================================================================================================================================================

//==========================================================================================================================================================================================================================================
//==========================================================================================================================================================================================================================================
// FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    FUNCTIONS    
// Functions for each section of above code
// Please note: Except for very simple cases, it would be better to place all of these functions in a 
// myRobotControlFunctions.h file and #include it at start of program to keep robot flight code brief

//-----------------------------------------------------------------------------------------------
// Functions for setup code
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Functions for flight code
//-----------------------------------------------------------------------------------------------

// Realtime loop functions loop---loop---loop---loop---loop---loop---loop---loop---loop---loop---

float weightFromScale(){
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  units   = scale.get_units(), 10;
  if (units < 0)
  {
    units = 0.00;
  }
  return units;
}

void blinkAliveLED(){
  // This function toggles state of aliveLED blinky light LED
  // if the LED is off turn it on and vice-versa:
    if (aliveLEDState == LOW) {
      aliveLEDState = HIGH;
    } else {
      aliveLEDState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(aliveLED, aliveLEDState);
}

void setLEDStrip(float scaleInput) {
  int input = scaleInput;                                   // convert float scale input to integer
  if (LEDColorNumber == 0){                                    // sweep through different colors for led strip
    if (scaleInput == 0){LEDColorNumber ++;}                   // increment LED color when not touched    
    int mapped_input = map(input, 0, 2500, 0, 255);            // map scale Input to LED brightness
    if (mapped_input > 255){mapped_input = 255;}               // set max value for LEDs to 255
    uint32_t magenta = strip.Color(mapped_input,mapped_input,0);
    strip.fill(magenta, 0, 10);
  }
  else if (LEDColorNumber == 1){
    if (scaleInput == 0){LEDColorNumber ++;}                   // increment LED color when not touched    
    int mapped_input = map(input, 0, 2500, 0, 255);            // map scale Input to LED brightness
    if (mapped_input > 255){mapped_input = 255;}               // set max value for LEDs to 255
    uint32_t magenta = strip.Color(mapped_input,0,0);
    strip.fill(magenta, 0, 10);
  }
  else if (LEDColorNumber == 2){
    if (scaleInput == 0){LEDColorNumber ++;}                   // increment LED color when not touched    
    int mapped_input = map(input, 0, 2500, 0, 255);            // map scale Input to LED brightness
    if (mapped_input > 255){mapped_input = 255;}               // set max value for LEDs to 255
    uint32_t magenta = strip.Color(0,0,mapped_input);
    strip.fill(magenta, 0, 10);
  }
  else if (LEDColorNumber == 3){
    if (scaleInput == 0){LEDColorNumber ++;}                   // increment LED color when not touched    
    int mapped_input = map(input, 0, 2500, 0, 255);            // map scale Input to LED brightness
    int mapped_input_2 = map(input, 0, 2500, 0, 200);
    if (mapped_input > 255){mapped_input = 255;}               // set max value for LEDs to 255
    uint32_t magenta = strip.Color(0,mapped_input_2,mapped_input);
    strip.fill(magenta, 0, 10);
  }
  else if (LEDColorNumber == 4){
    if (scaleInput == 0){LEDColorNumber ++;}                   // increment LED color when not touched    
    int mapped_input = map(input, 0, 2500, 0, 255);            // map scale Input to LED brightness
    int mapped_input_2 = map(input, 0, 2500, 0, 200);
    if (mapped_input > 255){mapped_input = 255;}               // set max value for LEDs to 255
    uint32_t magenta = strip.Color(mapped_input_2,mapped_input_2,mapped_input);
    strip.fill(magenta, 0, 10);
  }

  if (LEDColorNumber > 4){LEDColorNumber = 0;}
  
  strip.show();
}
int getDistanceFromSonar(){
  digitalWrite(trigPin, LOW);                     // Clears the trigPin
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);                    // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  sonarDuration = pulseIn(echoPin, HIGH);         // Reads the echoPin, returns the sound wave travel time in microseconds
  sonarDistance= sonarDuration*0.034/2;           // Calculating the sonarDistance
  return sonarDistance;   
}

int incrementVolumFromSonar(int distance){
  if (distance < 100 and songOnFlag == false){
    songOnFlag = true;
    songOnTime = millis();
  }
  if (songOnFlag == true and millis()-songOnTime < 5000){
    volume = volume + 5;
    if (volume > 100){volume = 100;}
  }
  if (millis()-songOnTime > 5000 and distance > 100){
    songOnFlag = false;
    volume = volume - 3;
    if (volume < 0){volume = 0;}
  }
  return volume;
}

// OCU functions ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu---ocu-----

String getOperatorInput() {
  // This function prints operator command options on the serial console and prompts
  // the operator to input desired robot command
  // Serial.println("   ");
  Serial.println("=======================================================================================");
  Serial.println("| Robot Behavior-Commands: move(moves robot), stop(e-stops motors), idle(robot idles) |");
  Serial.println("|                                                                                     |");
  Serial.println("|     Please type desired robot behavior in command line at top of this window        |");
  Serial.println("|     and then press SEND button.                                                     |");
  Serial.println("=======================================================================================");
  while (Serial.available()==0) {};                     // do nothinguntil operator input typed
  command = Serial.readString();                        // read command string
  Serial.print("| New Robot behavior command is: ");    // give command feedback to operator
  Serial.println(command);
  Serial.println("| Type 'stop' to stop control loop and wait for new command                           |");
  Serial.println("======================================================================================|");
  return command;
}

// SENSE functions sense---sense---sense---sense---sense---sense---sense---sense---sense---------
// place sense functions here

// THINK functions think---think---think---think---think---think---think---think---think---------
// place think functions here

// ACT functinos act---act---act---act---act---act---act---act---act---act---act---act---act-----
// place act functions here

// END of Functions
//===============================================================================================
// END of Robot Control Code
