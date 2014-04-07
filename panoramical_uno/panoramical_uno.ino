// ----- LIBRARIES --------------------------------------------------------------------------------------------
#include <MIDI.h>

// ----- VARIABLES -------------------------------------------------------------------------------------------------
// Pin Mapping
const int topRow = A0;                            //This pin connects to the in/out of the top multiplexer
const int bottomRow = A1;                         //This pin connects to the in/out of the bottom multiplexer

// Multiplexer Specific Variables
const int bitPin[] = {5, 6, 7};                    //The multiplexer's three binary control inputs
const int numChannels = 18;                        //The number of channels available on each multiplexer

// Analog Inputs + Smoothing Variables
const int numReadings = 8;                        //The number of readings taken that are later averaged together
int readings[numChannels][numReadings];           //Two dimensional array for storing the readings to be averager per each analog channel
int index = 0;                                    //Index to keep track of number of readings taken
int total[numChannels];                           //Running total to be averaged
int average[numChannels];                         //Average value that will be used to do things with!
int lastAverage[numChannels];                     //Only take new reading if different from last reading

// MIDI CC Array
int midiCC[] = {20, 21, 22, 23, 24, 25, 26, 27, 30, 31, 32, 33, 34, 35, 36, 37, 28, 38};

// ----- FUNCTIONS -------------------------------------------------------------------------------------------------
// Read Multiplexer
int readMux(int readPin, int channel) {
  for(int x=0; x<3; x++) digitalWrite(bitPin[x], bitRead(channel, x));  //bitRead: my new favorite function
  return analogRead(readPin);                                           //all pins on teensy above 10 are analog.
}

// ----- SETUP -----------------------------------------------------------------------------------------------------
void setup() {
  MIDI.begin();
  Serial.begin(31250);
  for (int x=0; x<3; x++) pinMode(bitPin[x], OUTPUT);            //Initiate binary controllers as outputs
  for (int x=0; x<3; x++) digitalWrite(bitPin[x], LOW);
  pinMode(topRow, INPUT);
  pinMode(bottomRow, INPUT);
  pinMode(A2, INPUT);
  pinMode(A4, INPUT);
}

// ----- LOOP ------------------------------------------------------------------------------------------------------
void loop() {  
  for (int x=0; x<numChannels; x++) {      
      total[x] = total[x] - readings[x][index];  
      
      if (x == 16) readings[x][index] = analogRead(A2);
      else if (x == 17) readings[x][index] = analogRead(A4);
      else if (x < 8) readings[x][index] = readMux(topRow, x);
      else readings[x][index] = readMux(bottomRow, x-8); 
      
      total[x] += readings[x][index];
      average[x] = (total[x] / numReadings);
    
      if (average[x] > lastAverage[x] + 7 || average[x] < lastAverage[x] - 7) {
        int midiVal = map(average[x], 0, 1023, 0, 127);    
        MIDI.sendControlChange(midiCC[x], midiVal, 1);
        lastAverage[x] = average[x];
      }       
  }  
  index += 1;                                                   //Advance index to next position
  if (index >= numReadings) index = 0;
}
