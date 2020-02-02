

/*******************************************************************************************************************************************************************************************************************************************
 * Title: Make Harvard 2020 Project
 * Description: 
 * Robot Name: MusicMachine
 * What does code do?: 
 * Hardware wanings:
 * Created by Leon Santen (leon.santen@icloud.com ), January 2020
 *******************************************************************************************************************************************************************************************************************************************
 */



//==========================================================================================================================================================================================================================================
// Load supporting Arduino Libraries
//==========================================================================================================================================================================================================================================
#include <Servo.h>                                //loading ServoMotors library
#include <HX711.h>                                //library for 24-bit load cell communication


//==========================================================================================================================================================================================================================================
// Create and initialize global variables, objects, and constants (containers for all data)
//==========================================================================================================================================================================================================================================
HX711 scale(5, 6);                      //initialize scale usually (sck, dt) --> however on current board, prints are wrong, therefore swapped

const int aliveLED = 13;                //create a name for "robot alive" blinky light pin 
const int eStopPin = 12;                //create a name for pin connected to ESTOP switch
boolean aliveLEDState = true;           //create a name for alive blinky light state to be used with timer
boolean ESTOP = true;                   //create a name for emergency stop of all motors
boolean realTimeRunStop = true;         //create a name for real time control loop flag
boolean debug = false;                  //create a name for debug boolean - when true, OCU is active; when false, OCU is deactivated and will directly go into "main" mode
String command = "move";                //create a String object name for operator command string
String loopError = "no error";          //create a String for the real time control loop error system
unsigned long oldLoopTime = 0;          //create a name for past loop time in milliseconds
unsigned long newLoopTime = 0;          //create a name for new loop time in milliseconds
unsigned long cycleTime = 0;            //create a name for elapsed loop cycle time
const long controlLoopInterval = 95;    //create a name for control loop cycle time in milliseconds
float calibration_factor = -385;        //this calibration factor is adjusted according to my load cell // -385 MakerHawk Digital Load Cell Weight Sensor HX711
float units;                            //units for 24-bit scale
float ounces;                           //unit for scale
int state = 1;                          //state of machine
int subState = 0;                       //substate of machine
int oldState = 0;
int oldSubState =0;


//==========================================================================================================================================================================================================================================
// Startup code to configure robot and pretest all robot functionality (to run once)
// and code to setup robot mission for launch
//==========================================================================================================================================================================================================================================
void setup() {
  // Step 1) Put your robot setup code here, to run once:
  pinMode(aliveLED, OUTPUT);            //initialize aliveLED pin as an output
  pinMode(eStopPin, INPUT_PULLUP);      //use internal pull-up on ESTOP switch input pin 
  Serial.begin(9600);                   //start serial communication
  scale.set_scale();                    //set scale object
  scale.tare();                         //Reset the scale to 0 
  long zero_factor = scale.read_average(); //Get a baseline reading
  if (debug == true){
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
    Serial.println(" Robot Controller Starting Up! Watch your little fingers! ");}
  

  // Step 2) Put your robot mission setup code here, to run once:
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
        if (debug == true){
          //Serial.println("Main robot ");
          //Serial.println("Type stop to stop robot");
        }
        int subState = weightFromScale();
        realTimeRunStop = true;                                   // don't exit loop after running once

        if (state != oldState or subState != oldSubState){    
          Serial.print(state);
          Serial.print(" ");
          Serial.println(subState);

          oldState = state;
          oldSubState = subState;
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
      
      else
      {
        Serial.println("**** WARNING **** Invalid Input, Robot Stopped, Please try again!");
        realTimeRunStop = false;
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
