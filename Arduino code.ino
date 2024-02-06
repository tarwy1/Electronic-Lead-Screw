#include "pins_arduino.h"

int gnd1 = 4; // topright
int gnd2 = 5; // top
int gnd3 = 6; // bottomright
int gnd4 = 7; // topleft
int gnd5 = 8; // middle
int gnd6 = 9; // decimalpoint
int gnd7 = 10; // bottomleft
int gnd8 = 11; // bottom
int enable = A0;
int A = A1;
int B = A2;
int C = A3;
uint16_t DisplayOne = 5678; // initialize Display numbers to 1234
uint16_t DisplayTwo = 1234;
int DisplayClockCount = 0; // initialize the clock pulse counter to 0
int DisplayClock = 2; 
int DisplayData = 12;
uint16_t InputData = 0; // initialize input data buffer to 0
int DisplayEnable = 3;
bool previous = false; // bool to check if enable pin has changed
int unitsled = 13;
int reverseled = A4;
int feedled = A5;
 
uint16_t unitsledstate = 0;
uint16_t forwardledstate = 0;
uint16_t threadledstate = 0;

void setup(){
  pinMode(gnd1, OUTPUT);
  pinMode(DisplayEnable, INPUT);
  pinMode(DisplayClock, INPUT);
  pinMode(DisplayData, INPUT);
  pinMode(gnd2, OUTPUT);
  pinMode(gnd3, OUTPUT);
  pinMode(gnd4, OUTPUT);
  pinMode(gnd5, OUTPUT);
  pinMode(gnd6, OUTPUT);
  pinMode(gnd7, OUTPUT);
  pinMode(gnd8, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  digitalWrite(gnd1, LOW);
  digitalWrite(gnd2, LOW);
  digitalWrite(gnd3, LOW);
  digitalWrite(gnd4, LOW);
  digitalWrite(gnd5, LOW);
  digitalWrite(gnd6, LOW);
  digitalWrite(gnd7, LOW);
  digitalWrite(gnd8, LOW);
  digitalWrite(enable, HIGH);
  attachInterrupt(digitalPinToInterrupt(DisplayEnable), enablerising, CHANGE); // attach an interrupt on change for enable pin
  attachInterrupt(digitalPinToInterrupt(DisplayClock), clockrising, RISING); // attach a rising edge interrupt to clock pin
  //Serial.println("Starting"+DisplayOne+DisplayTwo);
  pinMode(unitsled, OUTPUT);
  pinMode(reverseled, OUTPUT);
  pinMode(feedled, OUTPUT);
  
  digitalWrite(unitsled, 0);
  digitalWrite(reverseled, 0);
  digitalWrite(feedled, 0);
}

// enable pin interrupt
void enablerising(){ 
  if(previous==false&&DisplayClockCount!=15){
      DisplayClockCount = 0; // check if the enable signal has changed, set the clock count to 0
  }
}

// clock pin rising edge interrupt
void clockrising(){
  // if count==0, re-initialize input data, add the datapin value to it and bitshift 1 left
  // if count<15 add datapin value, bitshift, increment
  // if count>=15 reset count, add datapin value, set new display value
  if(DisplayClockCount == 0){ 
    InputData = 0;
    InputData += digitalRead(DisplayData);
    InputData <<= 1;
    DisplayClockCount++;  // increment count
  }
  else if(DisplayClockCount<15){
    InputData += digitalRead(DisplayData);
    InputData <<= 1;
    DisplayClockCount++;
  }
  else{
    DisplayClockCount = 0;
    InputData += digitalRead(DisplayData);
    if(InputData==6500){
      threadledstate = ~threadledstate & 0x0001;
      digitalWrite(feedled, threadledstate);
      //Serial.println("recived data 6500");
    }
    else if(InputData==6502){
      forwardledstate = ~forwardledstate & 0x0001;
      digitalWrite(reverseled, forwardledstate);
      //Serial.println("recived data 6502");
    }
    else if(InputData == 6504){
      unitsledstate = ~unitsledstate & 0x0001;
      digitalWrite(unitsled, unitsledstate);
      //Serial.println("recived data 6504");
    }
    else if(InputData>9999){
      DisplayTwo = InputData-9999;
      //Serial.println(InputData);
    }else{
      DisplayOne = InputData;
      //Serial.println(InputData);
    }
  }
}

void loop(){
  int temp = digitalRead(DisplayEnable);
  if(temp!=previous){
    previous = temp;
  }
  WriteFullNumber(DisplayOne, 1);
  WriteFullNumber(DisplayTwo, 0);
}
void Pos(int LED){
  digitalWrite(enable, HIGH);
  if(LED==1){
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
  }
  else if(LED==2){
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
  }
  else if(LED==3){
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
  }
  else if(LED==4){
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
  }
  else if(LED==5){
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
  }
  else if(LED==6){
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
  }
  else if(LED==7){
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
  }
  else if(LED==8){
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
  }
}
void reset(){
  digitalWrite(gnd1, LOW);
  digitalWrite(gnd2, LOW);
  digitalWrite(gnd3, LOW);
  digitalWrite(gnd4, LOW);
  digitalWrite(gnd5, LOW);
  digitalWrite(gnd6, LOW);
  digitalWrite(gnd7, LOW);
  digitalWrite(gnd8, LOW);
  digitalWrite(enable, LOW);
}
void gnd(int GROUND){
  digitalWrite(GROUND+3, HIGH);
}
void WriteNumber(int Number, int Digit){
  reset();
  switch(Number){
    case 0:
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(3);
      gnd(4);
      gnd(7);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 1:
      Pos(Digit);
      gnd(2);
      gnd(3);
      digitalWrite(enable, HIGH);
      break;
    case 2: //12578
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(5);
      gnd(7);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 3: // expt 467
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(3);
      gnd(5);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 4: //1345
      Pos(Digit);
      gnd(2);
      gnd(3);
      gnd(4);
      gnd(5);
      digitalWrite(enable, HIGH);
      break;
    case 5: // exp 167
      Pos(Digit);
      gnd(1);
      gnd(3);
      gnd(4);
      gnd(5);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 6: // exp 16
      Pos(Digit);
      gnd(1);
      gnd(3);
      gnd(4);
      gnd(5);
      gnd(7);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 7:
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(3);
      digitalWrite(enable, HIGH);
      break;
    case 8:
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(3);
      gnd(4);
      gnd(5);
      gnd(7);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 9:
      Pos(Digit);
      gnd(1);
      gnd(2);
      gnd(3);
      gnd(4);
      gnd(5);
      gnd(8);
      digitalWrite(enable, HIGH);
      break;
    case 10:   // DP
      Pos(Digit);
      gnd(6);
      digitalWrite(enable, HIGH);
      break;
  }
}
void WriteFullNumber(int number, int sevenSeg){
  String Numstring = String(number);
  if(Numstring.length()==3){
    Numstring = "0"+Numstring;
  }
  else if(Numstring.length()==2){
    Numstring = "00"+Numstring;
  }
  else if(Numstring.length()==1){
    Numstring = "000"+Numstring;
  }
  int numberarray[4];
  for(int i = Numstring.length()-1; i>=0; i--){
    numberarray[i] = Numstring.substring(i, i+1).toInt();
  }
  for(int i = 0; i<4; i++){
    WriteNumber(numberarray[i], (sevenSeg*4)+i+1);
    delayMicroseconds(250);
  }
}
