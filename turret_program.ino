//turret_program
//Author: Craig Haber
//This program allows the Human Tracking Laser Turret to track colored targets that are predefined using the PixyMon software (i.e. shirt colors, colored tags)
//It is a modified version of the ccc_pantilt program developed by PixyMon with the following changes:
//  When the turret detects a target it will sound an alarm and shine lasers on it while tracking it.
//  When the turret is not tracking a target it continuously pans backa and forth to search for one.

//Include Pixy2 and PIDLoop libraries
#include <Pixy2.h>
#include <PIDLoop.h>

//Declare variables
Pixy2 pixy;
//proportional gain 500, integrail grain 0, derivative gain 500
PIDLoop panLoop(500, 0, 500, true); 
PIDLoop tiltLoop(500, 0, 500, true);
int loopCounter;
bool moveForward = true;
int audioPin = 9;
int laserPin = 10;

void setup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");

  //set pin 10 to control the output of the lasers
  pinMode(laserPin, OUTPUT);
  
  //Initialize the pixy object 
  pixy.init();
  
  // Use color connected components program for the pan/tilt to track 
  pixy.changeProg("color_connected_components");
  
}

void loop()
{  

  static int i = 0;
  int j;
  char buf[64]; 
  int32_t panOffset, tiltOffset;
  
  // get targets from Pixy
  pixy.ccc.getBlocks();

  //if camera finds a target
  if (pixy.ccc.numBlocks)
  { 
    //turn on the lasers       
    digitalWrite(laserPin, HIGH);
    i++;
    
    //sound the alarm
    if (i%40==0){
     tone(audioPin,700,1000);
    }
    if(i%40==20){
      tone(audioPin,400,1000);
    }
    
    if (i%60==0)
      Serial.println(i);   
    
    // blocks[0] is the biggest object the camera detects (they are automatically sorted) 
    //calculates the ditstance between the object and the center of the pixy to determine how much it should pan/tilt  
    panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
    tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  
  
    //feed loops calculation of pan/tilt offeset
    panLoop.update(panOffset);
    tiltLoop.update(tiltOffset);
  
    // set pan and tilt servos based on PIDLoop calculation
    pixy.setServos(panLoop.m_command, tiltLoop.m_command);
   

  }  
  else // no object detected, continually search
  {
    //turn lasers off
    digitalWrite(laserPin, LOW);

    //switch direction when pans maximum distance (1000)
    if (loopCounter == 1000){
      moveForward = false;
    }
    else if (loopCounter ==0){
      moveForward = true;
    }

    //determine how much the pan servo should move forward or backword
    if (moveForward){
      loopCounter+=5;
    }
    else{
      loopCounter-=5;
    }
     pixy.setServos(loopCounter,0);
    
    
    panLoop.reset();
    tiltLoop.reset();
   
  }
}
