#include <Servo.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

int braille[26][6] = {
  {1, 0, 0, 0, 0, 0}, //a
  {1, 1, 0, 0, 0, 0}, //b
  {1, 0, 0, 1, 0, 0}, //c
  {1, 0, 0, 1, 1, 0}, //d
  {1, 0, 0, 0, 1, 0}, //e
  {1, 1, 0, 1, 0, 0}, //f
  {1, 1, 0, 1, 1, 0}, //g
  {1, 1, 0, 0, 1, 0}, //h
  {0, 1, 0, 1, 0, 0}, //i
  {0, 1, 0, 1, 1, 0}, //j
  {1, 0, 1, 0, 0, 0}, //k
  {1, 1, 1, 0, 0, 0}, //l
  {1, 0, 1, 1, 0, 0}, //m
  {1, 0, 1, 1, 1, 0}, //n
  {1, 0, 1, 0, 1, 0}, //o
  {1, 1, 1, 1, 0, 0}, //p
  {1, 1, 1, 1, 1, 0}, //q
  {1, 1, 1, 0, 1, 0}, //r
  {0, 1, 1, 1, 0, 0}, //s
  {0, 1, 1, 1, 1, 0}, //t
  {1, 0, 1, 0, 0, 1}, //u
  {1, 1, 1, 0, 0, 1}, //v
  {0, 1, 0, 1, 1, 1}, //w
  {1, 0, 1, 1, 0, 1}, //x
  {1, 0, 1, 1, 1, 1}, //y
  {1, 0, 1, 0, 1, 1}, //z
};

//array of pins connected to the servos in order
int controlPins[6] = {2, 3, 4, 5, 6, 7};
Servo servos[6];
int servo_positions[6];

// analog pins for servo-controlling buttons
int controlButtonPins[6] = {A0, A1, A2, A3, A4, A5};
  
// pin for the forward button
int buttonPinForward = 8;

// pin for backward button
int buttonPinBackward = 9;

// pin for mode change button
int buttonPinModeChange = 12;
  
// the alphabet
char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

// exercise mode button
bool exerciseMode = 0;

// initializing DFPlayer stuff
SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// to track current letter
int index = 0;

// variable to hold random number and letter
int randomIndex;
char randomLetter;

// states for the two buttons
int forwardButtonPrevState;
int forwardButtonState;
int backwardButtonPrevState;
int backwardButtonState;

void updateServos()
{
  Serial.println(alphabet[index]);
  // Move servos to represent the current braille character
  for (int i = 0; i < 6; i++) {
    int angle = braille[index][i] == 1 ? 0 : 90; // 0 degrees for raised, 90 for flat
    servos[i].write(angle);
    servo_positions[i] = angle;
    Serial.print(i);
    Serial.println(angle);
  }
}

void updateServosRandom()
{
  Serial.println(alphabet[randomIndex]);
  // Move servos to represent the current braille character
  for (int i = 0; i < 6; i++) {
    int angle = braille[randomIndex][i] == 1 ? 0 : 90; // 0 degrees for raised, 90 for flat
    servos[i].write(angle);
    servo_positions[i] = angle;
    Serial.print(i);
    Serial.println(angle);
  }
}


void playLetter() {
  // index starts from 0, file number starts at 1
  int fileNumber = index + 1;
  myDFPlayer.play(fileNumber); // Play the file
  Serial.print("Playing: ");
  Serial.println(alphabet[index]); // Print the letter being played
  Serial.println(fileNumber);
}


void playLetterRandom() {
  // index starts at 0 while file starts from 1, so + 1
  int fileNumber = randomIndex + 1;
  // play the file
  myDFPlayer.play(fileNumber); 
  // debug
  Serial.print("Playing: ");
  Serial.println(alphabet[randomIndex]);
  Serial.println(fileNumber);
}

void setup()
{ 
  // start mySerial and Serial
  mySerial.begin(9600);
  Serial.begin(9600);

  //setup servos and its buttons
  for (int i = 0; i < 6; i++) {
    servos[i].attach(controlPins[i]);
    servo_positions[i] = 0;
    pinMode(controlButtonPins[i], INPUT);
  }

  // make sure DFPlayer is working
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini not detected.");
    while (true); 
  }
  
  // volume (this rangeds from 0-30)
  myDFPlayer.volume(30);
  
  // setup buttons
  pinMode(buttonPinForward, INPUT);
  pinMode(buttonPinBackward, INPUT);
  pinMode(buttonPinModeChange, INPUT);
  
  // initialize the first servo position
  updateServos();
  forwardButtonPrevState = digitalRead(buttonPinForward);
  backwardButtonPrevState = digitalRead(buttonPinBackward);

  // read out "A", the first letter
  myDFPlayer.play(1);
}

void loop()
{	 
  // in another loop so I can break later
  while (true) {
    delay(150);
    // Read button states
    forwardButtonState = digitalRead(buttonPinForward);
    backwardButtonState = digitalRead(buttonPinBackward);

    // in learn mode
    if (!exerciseMode) {
      // Cycling forward on keydown
      if (forwardButtonState == HIGH && forwardButtonPrevState == LOW) {
        index++;
        if (index > 25) {
          index = 0; // Loop back to 'a'
        }
        Serial.println(alphabet[index]);
        updateServos();
        playLetter();
      }

      // Cycling backward on keydown
      if (backwardButtonState == HIGH && backwardButtonPrevState == LOW) {
        index--;
        if (index < 0) {
          index = 25; // Loop back to 'z'
        }
        Serial.println(alphabet[index]);
        updateServos();
        playLetter();
        //delay(1000);
      }

      // switching to exercise mopde
      if (digitalRead(buttonPinModeChange) == HIGH) {
        // turn exercise mode on
        exerciseMode = 1;
        // random seed based on microsecond
  		  randomSeed(micros());
        // reset all servos
        for (int i = 0; i < 6; i++) {
          servos[i].write(90);
          servo_positions[i] = 90;
        }
        // play sound "exercise mode"
        myDFPlayer.play(29);
        delay(2500);
        // prevent constant switching when holding button down
        while (digitalRead(buttonPinModeChange) == HIGH) {}
        // generating a random letter
  		  randomIndex = random(0, 26);  
  		  randomLetter = alphabet[randomIndex];
        playLetterRandom();
        // debug new letter on serial
        Serial.println("New letter");
        Serial.println(randomIndex);
        Serial.println(randomLetter);
        break;
      }
    }

    // for exercise mode
    if (exerciseMode) {
    // check each pin
      for (int i = 0; i <= 5; i++) {
        if (analogRead(controlButtonPins[i]) > 1010) { 
          // toggle the corresponding servo
          // this gets the new angle
          int new_angle = servo_positions[i] == 0 ? 90 : 0;
          // then apply the change
          servos[i].write(new_angle);
          servo_positions[i] = new_angle;
          // debugging
          Serial.print(i);
          Serial.println(new_angle);
          delay(300);
          break;
        }
      }

      // "confirm" button
      if (forwardButtonState == HIGH && forwardButtonPrevState == LOW) {
        // assume answer to be correct
        bool correct = true; 
        for (int i = 0; i < 6; i++) {
          // if anything is wrong
          if ((servo_positions[i] == 90 && braille[randomIndex][i] == 1) || 
            (servo_positions[i] == 0 && braille[randomIndex][i] == 0)) {
            correct = false; // mark the answer incorrect
           break; // break the loop, since 1 mistake already means it's incorrect
          }
        }

        // if correct, play the appropriate sound
        if (correct) {                                                                          
          myDFPlayer.play(27);
        } else {
          // otherwise, play the appropriate sound and cirrect the servo positions
          myDFPlayer.play(28);
          updateServosRandom(); 
        }

        // to handle the user holding down the forward button for no reason
        while (digitalRead(buttonPinForward) == HIGH){}

        delay(300);
        
        // wait until button is pressed again to continue
        while (digitalRead(buttonPinForward) == LOW){}

        
        // reset the servos
        for (int i = 0; i < 6; i++) {
          servos[i].write(90);
          servo_positions[i] = 90;
        }

        delay(300);
        
        // choose a new letter, read it out, debug in serial
        randomIndex = random(0, 26);  
  		  randomLetter = alphabet[randomIndex];
        playLetterRandom();
        Serial.println("New letter");
        Serial.println(randomIndex);
        Serial.println(randomLetter);
        // same thing, if the user just holds it for no reason it won't trigger multiple confirms
        while (digitalRead(buttonPinForward) == HIGH){}
        
        break;
      }

      // button to reset all servos
      if (backwardButtonState == HIGH && backwardButtonPrevState == LOW) {
        for (int i = 0; i < 6; i++) {
          servos[i].write(90);
          servo_positions[i] = 90;
        }
        delay(100);
        break;
      }

      // going back to learn mode
      else if (digitalRead(buttonPinModeChange) == HIGH) {
        // turn off exercise mode
        exerciseMode = 0;
        // play sound "learn mode"
        myDFPlayer.play(30);
        delay(2500);
        // set servos to the last letter before switching to test mode
        updateServos();
        // prevent holding trigerring multiple calls
        while (digitalRead(buttonPinModeChange) == HIGH) {}
        break;
      }
    }

    // Update previous button states
    forwardButtonPrevState = forwardButtonState;
    backwardButtonPrevState = backwardButtonState;
    }
}
                