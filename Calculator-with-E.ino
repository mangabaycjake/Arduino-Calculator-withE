  //Created by Christian Jake Mangabay
  
  //INSTRUCTIONS
    /*
      Hold the button for secondary keys
        Minus -> Negative
        Multiply -> E
        Equals -> Point
        Backspace -> Clear All 
    */
  //INCLUSIONS
    #include <LiquidCrystal_I2C.h>
    #include <Wire.h>

    LiquidCrystal_I2C lcd(0x27, 16, 2);

  //Pins
    const int KeyPp1 = 3;
    const int KeyPp2 = 4;
    const int KeyPp3 = 5;
    const int KeyPp4 = 6;
    const int KeyPp5 = 7;  
    const int KeyPp6 = 8;
    const int KeyPp7 = 9;
    const int KeyPp8 = 10;
  
  //Characters
    const int E_ = 69;
    const int posi_ = 43;
    const int neg_ = 45;
    const int dot_ = 46;
    const int blank_ = 32;
  
  //Buttons

  //Adjustable Variables
    const int _MaxSize = 32; //be divisible by 8
    const int LongPressTime = 70;
    const int EmptyChar = 35;

  //Global Variable Declarations
    int CmdIndex = 16;
    int prevCmdIndex = 16;
    int OperatorIndex = 0;
    int LongPressStack = 0;
    int CurPlace= 0;
    int WholePlaceA = 0;
    int WholePlaceB = 0;
    int WholePlaceY = 0;
    int EPlaceA = 0;
    int EPlaceB = 0;
    int DecimalPlaceA = 0;
    int DecimalPlaceB = 0;
    int NumberEA = 0;
    int NumberEB = 0;
    int Inequality = 0;

    bool isNegativeEA = 0;
    bool isNegativeEB = 0;
    bool isOutput = 0;
    bool previsOutput = 0;

    const int MaxSize = _MaxSize + 1;

    int NumberA[MaxSize];
    int NumberB[MaxSize];
    int Answer[_MaxSize*2+1];

    int DisplayNumA[16];
    int DisplayNumB[16];

    float Percentage;
  void setup() {
    
    //LCD
    lcd.begin(16, 2);
    lcd.setBacklight(1);
    lcd.print("Setting up...");
    Serial.begin(9600);

    //4X4 MATRIX
    pinMode(KeyPp1, OUTPUT);
    pinMode(KeyPp2, OUTPUT);
    pinMode(KeyPp3, OUTPUT);
    pinMode(KeyPp4, OUTPUT);
    pinMode(KeyPp5, INPUT);
    pinMode(KeyPp6, INPUT);
    pinMode(KeyPp7, INPUT);
    pinMode(KeyPp8, INPUT);

    lcd.clear();
  
    ClearArray();
  }

  //------------------------------------------------------------------------------------

  void loop() {
    
    //Determine the Command Index 
    WhatButton();

    //When Button is pressed but not yet released
    if (CmdIndex == prevCmdIndex && CmdIndex < 16){
      
        LongPressStack += 1;  //The time of long press is stacking so that at indicated Long Press Time, it registers as a long press 
        if (CmdIndex == 10 || CmdIndex == 13){LongPressStack = 0;} //No long press alternatives on these buttons
        if (prevCmdIndex > 9  && LongPressStack == LongPressTime){ //Indicates Long Press reached
            Serial.println("Long Press");
        }
    //When Button is released after press
    }else if(prevCmdIndex <16 && CmdIndex == 16){ //16 means released, <16 means the previous Index is not 16

    isOutput = 0; //indicates this is an input

          //Numeric Commands
          if (prevCmdIndex < 10){
            if (previsOutput == 1){ClearArray();} //Previously, the number displayed in bottom is answer, so clear it first
            LongPressStack = 0;           //resetes the long-press stacks
            CurPlace += 1;                //cursor location for number
            NumberA[CurPlace] = prevCmdIndex;   //Command Index 0-9 corresponds to numerics 0-9, respectively
          }

          //Operation Commands are pressed - no long press
          else if (prevCmdIndex < 14  && LongPressStack < LongPressTime){ //no long press
            LongPressStack = 0;

              //When No number in both top and bottom
              if (NumberA[1] == EmptyChar && NumberB[1] == EmptyChar){
                //do nothing  if no number is displayed in both top and bottom
              }
              //When No Number at Bottom but there is at Top
              else if(NumberA[1] == EmptyChar && NumberB[1] != EmptyChar){
                OperatorIndex = prevCmdIndex - 9; //Determine the Operation
              }
              //When there is number at bottom none on top
              else if(NumberA[1] != EmptyChar && NumberB[1] == EmptyChar){
                PushUp();     //Move the number upwards
                OperatorIndex = prevCmdIndex - 9;   //determine operation
              }
              //When there are numbers both on top and bottom
              else{
                Operate();      //Determine the right operation
                RefreshDisplay(); //Show the answer at the bottom
                PushUp();       //Then immedeiately bring upwards
                OperatorIndex = prevCmdIndex - 9;   //determine the next operation
                isOutput = 0;   //what to be written next is an input
                SerialCheck();  //show the inputs in serial monitor
            }
          }

          //Backspace is pressed
          else if(prevCmdIndex == 15 && LongPressStack < LongPressTime){
            LongPressStack = 0;
            if (previsOutput == 1){ClearArray();} //new input if the previous content is an answer
            NumberA[CurPlace] = EmptyChar;
            if (CurPlace == 0){   //if the cursor is already at left-most digit
              NumberA[0]=posi_;   //make the number positive
              CurPlace += 1;      //neutralize the movement of cursor so that it will remain on index 1 of array (0 is for sign)
            }
            CurPlace -= 1;       //after clearing the current slot, move the cursor backwards
          }

          //Equals is pressed
          else if(prevCmdIndex == 14  && LongPressStack < LongPressTime){
            LongPressStack = 0;
            if (NumberA[1] != EmptyChar && NumberB[1] != EmptyChar){ //both top and bottom have values
              Operate();             //determine the right operation
            }else if(NumberA[1] != EmptyChar){//see guide#7 on if bottom have value but top is blank
              PushUp();             //move the number upwards
              OperatorIndex = 1;    //do supposed addition
            }
            SerialAnswer();       //check the answer in serial monitor
          }


          //These are the Long Press Commands
          else if(prevCmdIndex > 9  && LongPressStack >= LongPressTime){
            LongPressStack = 0;

            if (previsOutput == 1){ClearArray();} //if new input, clear first

            switch (prevCmdIndex){  //determine what is the command index
              
              case 14://Decimal Point is pressed
                if (DecimalPlaceA>0){ //if there is already decimal point
                  break;              //do nothing
                }  
                CurPlace += 1;        //move the cursor rightwards
                NumberA[CurPlace] = dot_;   //place a dot
                break;
              case 11://Negative button
                if (CurPlace == 0){     //can only make the number negative at beginning
                  NumberA[0] = neg_;    //put a negative sign
                }else if(NumberA[CurPlace]==E_){  //can also make the exponent negative only after "E"
                  CurPlace += 1;        //move cursor leftwards
                  NumberA[CurPlace] = neg_; //place negative sign after "E"
                }
                break;
              case 12://x10 N or E Button
                CurPlace += 1;      //move the cursor
                NumberA[CurPlace] = E_; //put "E"
                break;
              case 15://CLR (clear all) is pressed
                  lcd.clear();
                  ClearArray();    //reset
                break;
            }
          }
          AssessNum();      //Determine the attributes of the inputs such as the number of decimal places etc.
          SerialCheck();    //Show the inputs in the serial monitor
          RefreshDisplay(); //Show the updates in LCD
    }
    prevCmdIndex = CmdIndex;    //register the current command to be previous input before having new command
    previsOutput = isOutput;    //same in determination if previous value of the number is already an output
    delay(5);                   //for more stabilization
  }
  //END LOOP
  //----------------------------------------------------------------------------------

void WhatButton(){
//This function determines the pressed button and outputs the assigned command index for the buttons

    CmdIndex = 16;  //instead of using "else", the command index is 16 if not altered (nothing is pressed)
    for (int i = 0; i < 4;i++){
      digitalWrite(i+KeyPp1, HIGH); //set the first to last row high one at a time
      for (int j = 0; j < 4; j++){
        if (digitalRead(j + KeyPp5)==HIGH){
          CmdIndex = i*3 + 1 + j;   //determine which column will be high because of each row
        }                           //this is only applicable for numerics (col 0-2, row 0-2)
      }  
      if (digitalRead(KeyPp8)==HIGH){ //at last column, the operation commands (10-13) are found
        CmdIndex = 10 + i;            //operation is determined based on row
      }
      digitalWrite(i+KeyPp1, LOW);  //return to low so that only one row is HIGH at a time 
    }
    digitalWrite(KeyPp4, HIGH);     //for the last row
    if (digitalRead(KeyPp5)==HIGH) {  //first column indicates 14 or (equals or dot)
      CmdIndex = 14;
    }else if (digitalRead(KeyPp6)==HIGH) {
      CmdIndex = 0;                   //col 2 --> numeric 0
    }else if(digitalRead(KeyPp7)==HIGH) {
      CmdIndex = 15;                  //col 3 --> backspace or clr
    }else if(digitalRead(KeyPp8)==HIGH) {
      CmdIndex = 13;                  //col 4 --> division
    }
    digitalWrite(6, LOW);
  }

void ClearArray(){
//clar the contents of array

    for (int i=0; i<MaxSize; i++){    //The default character for Inputs is '#'
      NumberA[i]=EmptyChar;
      NumberB[i]=EmptyChar;
    }
    for (int i = 1; i<_MaxSize*2+1; i++){ //The default character for answer is 0
      Answer[i] = 0;
    }
    for (int i = 0; i < 16; i++) {DisplayNumA[i]= blank_;}  //The default character for Display is Blank
    for (int i = 0; i < 16; i++) {DisplayNumB[i]= blank_;}

    //Set the sign and attributes to default
    NumberA[0] = posi_;
    NumberB[0] = posi_;
    Answer[0] = posi_;
    Answer[MaxSize] = dot_;// decimal point
    CurPlace= 0;
    WholePlaceA = 0;
    WholePlaceB = 0;
    DecimalPlaceA = 0;
    DecimalPlaceB = 0;
    OperatorIndex = 0;
    RefreshDisplay();
}

//Monitoring of Input numbers via Serial Monitor
void SerialCheck(){
    Serial.print(char(NumberA[0]));
    Serial.print("         ");
    Serial.print(OperatorIndex);
    Serial.print("  ");
    Serial.println(char(NumberB[0]));
    for (int i = 0; i < MaxSize/8; i++){
        for (int j=0; j<8;j++){
          if (NumberA[8*i+j+1]>9){
            Serial.print(char(NumberA[8*i+j+1]));
          }else{
            Serial.print(NumberA[8*i+j+1]);
          }    
        }
        Serial.print("     ");
        for (int j=0; j<7;j++){
          if (NumberB[8*i+j+1]>9){
            Serial.print(char(NumberB[8*i+j+1]));
          }else{
            Serial.print(NumberB[8*i+j+1]);
          }
        }
        if (NumberB[8*i+7+1]>9){
          Serial.println(char(NumberB[8*i+7+1]));
        }else{
          Serial.println(NumberB[8*i+7+1]);
        }
    }
    
    Serial.println(" ");
    Serial.print("WP: ");
    Serial.print(WholePlaceA);
    Serial.print(" ");
    Serial.print("DP: ");
    Serial.print(DecimalPlaceA);
    Serial.print(" ");
    Serial.print("EA: ");
    Serial.print(NumberEA);
    Serial.print(" ");
    Serial.print("WP: ");
    Serial.print(WholePlaceB);
    Serial.print(" ");
    Serial.print("DP: ");
    Serial.print(DecimalPlaceB);
    Serial.print(" ");
    Serial.print("EB: ");
    Serial.print(NumberEB);
    Serial.println("  ");
    Serial.println("------------------");
}


void SerialAnswer(){
//Monitoring of the answer via Serial Monitor
    
    Serial.println(char(Answer[0]));
    for (int i = 0; i < (_MaxSize * 2)/16; i++){
      for (int j = 1; j < 9; j++){
        Serial.print(Answer[i*8+j]);
      }
      Serial.print("     ");
      for (int j = 1; j < 8; j++){
        if (Answer[i*8+j+_MaxSize]==dot_){
          Serial.print(char(Answer[i*8+j+_MaxSize]));
        }else{
          Serial.print(Answer[i*8+j+_MaxSize]);
        }
      }
      Serial.println(Answer[i*8+8+_MaxSize]);
    }
    Serial.println("------------------------");   
}

void Operate(){
//Main entrance before the actual operation


  //Check the characters
  //Check if decimal moving is within the declared array size (the resulting decimal places and whole places should not be more than capacity)
    if (isNegativeEA == 0 && WholePlaceA + NumberEA + 1 >= _MaxSize){ 
      ErrorPrompt(1); //Math Error, can not be solved due to limitation
    }
    if (isNegativeEB == 0 && WholePlaceB + NumberEB + 1 >= _MaxSize){
      ErrorPrompt(1); //Math Error, can not be solved due to limitation
    }
    //check for doubled E
    int Duplicate = -1; //next is 0, no duplication, 
    for (int x : NumberA){if (x == E_){Duplicate += 1;}}
    if (Duplicate>0){ErrorPrompt(2);} //if more than 0, there is duplication of 'E', Syntax Error
    Duplicate = -1;
    for (int x : NumberB){if (x == E_){Duplicate += 1;}}
    if (Duplicate>0){ErrorPrompt(2);}

    //Check for double dots. Same as previous but checks for dots instead
    Duplicate = -1;
    for (int x : NumberA){if (x == dot_){Duplicate += 1;}}
    if (Duplicate>0){ErrorPrompt(2);}
    Duplicate = -1;
    for (int x : NumberB){if (x == dot_){Duplicate += 1;}}
    if (Duplicate>0){ErrorPrompt(2);}
    
    //CHeck for E without multiplier/base. There should be a number before the 'E'
    if (NumberA[1] == E_){ErrorPrompt(2);}  //so E should not be the first character
    if (NumberB[1] == E_){ErrorPrompt(2);}

  //If the input characters are valid,
  CurPlace = 0;
  isOutput = 1; //the next value on bottom is output
  DetermineInequality();  //determine if bottom or top is larger in absolute
  ClearAnswer();          //clear the previous array for answer

  switch (OperatorIndex){//determine the index of operation
  
    case 1: //If initially supposed to be ADDITION

      if (NumberA[0]==posi_ && NumberB[0] == posi_){ //if both inputs are positive,
        Answer[0] = posi_;      //answer is positive
        Addition();             //just add
      }
      else if(NumberA[0]==neg_ && NumberB[0] == neg_){ //if both are negative,
        Answer[0] = neg_; //answer is negative
        Addition();       //do addition on absolute values
      }
      else if(Inequality==1){   //if the bottom number is larger and either is negative,
        Answer[0] = NumberA[0]; //answer takes the sign of the larger number (bottom)
        ExchangeAB();           //Put the larger number (bottom) to the top to become subtrahend
        Subtraction();          //Then do subtraction instead
      }
      else if(Inequality==0){   //if numbers are equal
        Answer[0] = posi_;      //expected to be 0 so positive
        Subtraction();          //subtract to get 0
      }
      else{                       //if the top is larger,
        Answer[0] = NumberB[0];  //answer takes the sign of the top (larger)
        Subtraction();            //do subtraction of the absolute
      }   
        
      break;

    case 2: //If initially supposed to be SUBTRACTION

      if (NumberA[0]==neg_ && NumberB[0] == posi_){ //if (+) - (-)
        Answer[0] = posi_;  //answer is positive
        Addition();         //do addition instead
      }
      else if(NumberA[0]==posi_ && NumberB[0] == neg_){ //if (-) - (+)
        Answer[0] = neg_;   //answer is negative
        Addition();         //do addition
      }
      //if (+) - (+) or (-) - (-), depends on what is larger
      else if(Inequality==1){   //if bottom is larger,
        Answer[0] = 44- (NumberA[0] - 44); //negate the sign of top
        ExchangeAB();           //bring the larger to top
        Subtraction();          //do subtraction
      }
      else if(Inequality==0){   //if equal,
        Answer[0] = posi_;      //answer is 0(+)
        Subtraction();
      }
      else{                     //if top is larger,
        Answer[0] = NumberA[0]; //sign depends on the sign of the top
        Subtraction();          //do subtraction
      }

      break;

    case 3: //if Multiplication,
      if (NumberA[0]==NumberB[0]){  //if Same sign
        Answer[0] = posi_; //positive
      }else{
        Answer[0] = neg_; //negative
      }
      Multiplication();
      break;
    case 4: //Div
      if (NumberA[0]==NumberB[0]){  //if Same sign
        Answer[0] = posi_; //positive
      }else{
        Answer[0] = neg_; //negative
      }
      Division();
      break;
  }
  AnswerToNumA(); //put the answer to bottom
  for (int i = 0; i < MaxSize; i++){ //error if any element is beyond the defined numbers and characters. Usually happens on supposed overflow
    if (NumberA[i] > 9 && NumberA[i] != dot_ && NumberA[i] != neg_ && NumberA[i] != posi_ && NumberA[i] != EmptyChar){
      ErrorPrompt(1);
    } 
  }
}

  void Addition(){

    AddDecimal();

    int DecimalPlaceY = 0;
    if (DecimalPlaceA > DecimalPlaceB){
      DecimalPlaceY = DecimalPlaceA;
    }else{
      DecimalPlaceY = DecimalPlaceB;
    }

    int AddendA=0;
    int AddendB=0;
    int iA=0;
    int iB=0;
    int Carry = 0;

    for (int i = MaxSize + DecimalPlaceY; i > 0; i--){
      
      if (i==MaxSize){ //decimal point
        i -= 1;
      }
      iA = i - _MaxSize + WholePlaceA;
      iB = i - _MaxSize + WholePlaceB;
      if ((iA)>0 && NumberA[iA]!=EmptyChar){  //on-range of NumA and is not blank
        AddendA = NumberA[iA];
      }else{//out of size of NumberA,or #
        AddendA = 0;
      }
      if ((iB)>0 && NumberB[iB]!=EmptyChar){ //on-range of NumB and is not blank
        AddendB = NumberB[iB];
      }else{//out of size of NumberB,or #
        AddendB = 0;
      }
      Answer[i] = AddendA + AddendB + Carry;  //elemetary line addition
      Carry = 0;      //carry is used up
      if (Answer[i] > 9){ // needs carry
        Answer[i] = Answer[i]-10;
        Carry = 1;
      }
    }
  }// End Addition()

  void Subtraction(){

    int DecimalPlaceY = 0;
    if (DecimalPlaceA > DecimalPlaceB){
      DecimalPlaceY = DecimalPlaceA;
    }else{
      DecimalPlaceY = DecimalPlaceB;
    }

    int Subtrahend=0;
    int Minuend=0;
    int iA=0;
    int iB=0;
    int Borrow = 0;
    int Borrow2 = 0;

    for (int i = MaxSize + DecimalPlaceY; i > 0; i--){
      
      if (i==MaxSize){ //decimal point, skip
        i -= 1;
      }
      iA = i - _MaxSize + WholePlaceA;
      iB = i - _MaxSize + WholePlaceB;
      if ((iA)>0 && NumberA[iA]!=EmptyChar){  //on-range of NumA and is not blank
        Subtrahend = NumberA[iA];
      }else{//out of size of NumberA,or #
        Subtrahend = 0;
      }
      if ((iB)>0 && NumberB[iB]!=EmptyChar){ //on-range of NumB and is not blank
        Minuend = NumberB[iB];
      }else{//out of size of NumberB,or #
        Minuend = 0;
      }
      //if previously borrowed
      if (Borrow == 10){
        Minuend -= 1;   //borrow was subtracted
        Borrow = 0;     //borrow cleared
      }
      if (Subtrahend > Minuend){
        Borrow = 10;    //needs borrow
      }
      Answer[i] = Borrow + Minuend - Subtrahend;
    }
  }//End Subtraction()

void Multiplication(){  //re document this

  //takes long...might be misinterpretted as hangging so blink
  const int LoadingBlinkSize = 15000;
  int LoadingBlink;


  //since the Multiplicand will be added to itself for Multiplier times, multiplier (NumA) should have the lower digit count
  if ( WholePlaceA+DecimalPlaceA > WholePlaceB+DecimalPlaceB){
    ExchangeAB();
    RemoveZero();
  }
  //POSITIVE (to the right) for Multiplier
    for (int i = WholePlaceA + 1; i < MaxSize; i++){
          if(i <= WholePlaceA + DecimalPlaceA){   //move characters to the left
            NumberA[i] = NumberA[i+1];
          }  
          if(i <= WholePlaceA + DecimalPlaceA + 2){     //put the necessary 0's
            if (NumberA[i]==EmptyChar){
              NumberA[i]=0;
            }
          }
    }
    NumberA[WholePlaceA + DecimalPlaceA + 1] = dot_; //put decimal place at right place

    //Negative (to the left) for Multiplicand
        int prevDecimalPlaceA = 0;
        if (WholePlaceB < DecimalPlaceA + 1){//make sure there is space for moving decimal to the left
          for (int i = _MaxSize; i> DecimalPlaceA - WholePlaceB + 1; i--){
            NumberB[i] = NumberB[i - (DecimalPlaceA - WholePlaceB + 1)];//shift to right
          }
          for (int i = 1; i <= DecimalPlaceA - WholePlaceB + 1; i++){  //set the former numbers to 0
            NumberB[i] = 0;
          }
          prevDecimalPlaceA = DecimalPlaceA;
          AssessNum();
          DecimalPlaceA = prevDecimalPlaceA;
        }
        for (int i = WholePlaceB + 1; i > WholePlaceB + 1 - DecimalPlaceA; i--){
          NumberB[i] = NumberB[i-1];
        }
        NumberB[WholePlaceB + 1 - DecimalPlaceA] = dot_;  
        AssessNum();

  //Transfer B to Answer
  for (int i = MaxSize + DecimalPlaceB; i > 0; i--){ 
    if (i==MaxSize){ //decimal point, skip
      i -= 1;
    }
    Answer[i] = NumberB[i - _MaxSize + WholePlaceB];  //correspondence
    if(i - _MaxSize + WholePlaceB == 1){break;}
  }
  
  Percentage = 0;
  for (int i = 1; i < 7; i++){
    Percentage = Percentage*10 + NumberA[i];
  }


  while (1){

    //use NumberA as counter down
	  NumberA[WholePlaceA] -= 1; 
  	for (int i = 0; i < WholePlaceA - 1; i++){
      if (NumberA[WholePlaceA - i] == -1){
        NumberA[WholePlaceA - i] = 9;
        NumberA[WholePlaceA - i - 1] -= 1;    
      }
    }
    //Stopping Condition, completion of countdown if all NumberA is 0
    bool exiter = 1;
    for (int i = 1; i <= WholePlaceA; i++){      
      if (NumberA[i]!=0){
        exiter = 0;
      }    
    }
    if (exiter == 1){
      break;
    }
    
    //Addition to itself of every element in Answer[]
    int Carry = 0;
    for (int i = MaxSize + DecimalPlaceB; i > 0; i--){
      if (i==MaxSize){ //decimal point, skip
        i -= 1;
      }
      int Addend;
      if (i - _MaxSize + WholePlaceB > 0 && NumberB[i - _MaxSize + WholePlaceB] != EmptyChar){
        Addend = NumberB[i - _MaxSize + WholePlaceB];
      }else{
        Addend = 0;
      }   
      Answer[i] = Answer[i] + Addend + Carry;
		  Carry = 0;
		  if (Answer[i] > 9){
        Carry = 1;
        Answer[i] -= 10;
      }
    }
    LoadingBlink += 1;
    if (LoadingBlink==LoadingBlinkSize){
      PleaseWait();
      LoadingBlink = 0;
    }
  }
}//end multiplication

void Division(){


    int Dividend[MaxSize];
    int Divisor[MaxSize];
    int AnswerI[MaxSize];
    
    //clear Dividend and Divisor
    for (int i = 0; i < MaxSize; i++){
      Dividend[i] = 0;
      Divisor[i] = 35;
      AnswerI[i] = 0;
    }

    //Remove decimal point from NumberB (dividend source)
    for (int i = 1; i <= WholePlaceB; i++){
      Dividend[i-1] = NumberB[i]; 
    }
    for (int i = WholePlaceB + 2; i < MaxSize; i++){
      Dividend[i-2] = NumberB[i];
      if (Dividend[i-2] == EmptyChar){Dividend[i-2] = 0;}
    }

    //Remove decimal point from NumberA (divisor source)
    for (int i = 1; i <= WholePlaceA; i++){
      Divisor[i-1] = NumberA[i]; 
    }
    for (int i = WholePlaceA + 2; i < MaxSize; i++){
      Divisor[i-2] = NumberA[i];
    }

    //div/0
    if (Divisor[0]==0 && Divisor[1]==0 && Divisor[2] == EmptyChar){
      ErrorPrompt(1);
      return;
    }

    //Remove unnecessary zeros after the decimal
    for (int i = _MaxSize; i > WholePlaceA - 1; i--){
      if (Divisor[i] != 0 && Divisor[i] != EmptyChar){break;}
      Divisor[i] = EmptyChar;
    }

    //Align Divisor to the right
    for (int i = 0; i < MaxSize; i++){
      for (int i = _MaxSize; i > 0; i--){
        if (Divisor[i]!= EmptyChar){break;}
        Divisor[i] = Divisor[i - 1];
        Divisor[i-1] = EmptyChar;
      }
    }

    // Clean EmptyChar
    for (int i = 0; i < MaxSize; i++){
      if (Divisor[i] != EmptyChar){break;}
      Divisor[i] = 0;
    }

    //Division

    int Temp[MaxSize];
    //Clear Temp
    for (int i = 0; i < MaxSize; i++){Temp[i]=0;}

    //First Drop
    Temp[_MaxSize] = Dividend[_MaxSize];

    for (int j = 0; j < MaxSize; j++){

        
        bool Sub = 1; //default should be Sub as if tied until the end, sub
        
        //Divisor > Temp        first to have larger digit means larger
        for (int i = 0; i < MaxSize; i++){
          if (Divisor[i] > Temp[i]){
            Sub = 0;
          break;
          }
          if (Divisor[i] < Temp[i]){
            Sub = 1;
          break;
          }
        }
        if (Sub == 0){
          //Drop
          for (int i = 0; i < _MaxSize; i++){
            Temp[i] = Temp[i+1];    //shift previous values to left
          }
          Temp[_MaxSize] = Dividend[j];  //bring down the digit from dividend
        }        
        //Sub
        if (Sub == 1){
          for (int i = MaxSize; i >= 0; i-- ){
            if (Temp[i] < Divisor[i]){  //needs borrow
              Temp[i-1] -= 1;
              Temp[i] += 10;
            }
            Temp[i] -= Divisor[i];            
          }
          AnswerI[j] += 1;
          j -= 1;
        }
      //Serial.println("");
     // for (int x : Temp){Serial.print(x);}
    }
//Serial.println("");
//for (int x : AnswerI){Serial.print(x);}
//Serial.println("");
  int WholePlaceY = DecimalPlaceA + WholePlaceB;

  if (DecimalPlaceA == 1 && NumberA[WholePlaceA + 2] == 0){
    WholePlaceY -= 1;
  }
  //Place Whole Place for Answer
  for (int i = WholePlaceY; i >= 0; i--){  
    Answer[MaxSize + i - WholePlaceY - 1] = AnswerI[i];
  }
  //Place Decimal Place for Answer
  for (int i = WholePlaceY + 1; i < MaxSize; i++){
    if (MaxSize + i - WholePlaceY + 1 == _MaxSize*2){break;}
    Answer[MaxSize + i - WholePlaceY] = AnswerI[i];
  }

}//end division   


  
void AssessNum(){
//Determine the digits on the left and right of the decimal point

    int Placer = 0;
    int S = 0;
    isNegativeEA = 0;
    isNegativeEB = 0;
    EPlaceA = 0;
    EPlaceB = 0;
    NumberEA=0;
    NumberEB=0;
    
    WholePlaceA = 0;
    WholePlaceB = 0;
    DecimalPlaceA = 0;
    DecimalPlaceB = 0;
    
    //NUM A
      while (S<1){    //target: before the decimal point, 
        Placer += 1;                //next element
        if (NumberA[Placer]<10){    //whole numbers are counted from numerics
          WholePlaceA += 1;
        }else if(NumberA[Placer]==dot_){  //if next is decimal point, next S
          S = 1;                  
        }else if(NumberA[Placer]==E_){ //if next is E, skip the next S
          S = 2;
        }else if(NumberA[Placer]==EmptyChar){ //if nothing next, skip both
          S = 3;
        }
      }
      while (S<2){    //target: after the decimal point and before E
        Placer += 1;                //next element
        if (NumberA[Placer]<10){    //decimal numbers are counted from numerics
          DecimalPlaceA += 1;
        }else if(NumberA[Placer]==dot_){  //if next is decimal point, means double decimal point
          //INVALID
        }else if(NumberA[Placer]==E_){ //if next is E, next S
          S = 2;
        }else if(NumberA[Placer]==EmptyChar){ //if nothing next, skip 
          S = 3;
        }
      }
      while (S<3){    //target: after E
        isNegativeEA = 0;
        Placer += 1;
        if(NumberA[Placer]==neg_){  //detects negative sign
          isNegativeEA = 1;         //means the exponent is negative
        }
        if (NumberA[Placer]<10){    //decimal
          EPlaceA += 1;     //counts the number of digits of exponent
          NumberEA = NumberEA*10 + NumberA[Placer]; //records the exponent as an integer (limited)
        }else if(NumberA[Placer]==dot_){  //if next is decimal point, invalid
          //INVALID
        }else if(NumberA[Placer]==E_){ //if next is E, double E, invalid
          //INVALID
        }else if(NumberA[Placer]==EmptyChar){ //if nothing next, exit
          S = 3;
        }
      }
      //NUM B
      //Same as NumberA (Bottom Number)
      S=0;
      Placer=0;
      while (S<1){    //before decimal point
        Placer += 1;
        if (NumberB[Placer]<10){    //whole
          WholePlaceB += 1;
        }else if(NumberB[Placer]==dot_){  //if next is decimal
          S = 1;
        }else if(NumberB[Placer]==E_){ //if next is E
          S = 2;
        }else if(NumberB[Placer]==EmptyChar){ //if nothing next
          S = 3;
        }
      }
      while (S<2){    //before E
        Placer += 1;
        if (NumberB[Placer]<10){    //decimal
          DecimalPlaceB += 1;
        }else if(NumberB[Placer]==dot_){  //if next is decimal
          //INVALID
        }else if(NumberB[Placer]==E_){ //if next is E
          S = 2;
        }else if(NumberB[Placer]==EmptyChar){ //if nothing next
          S = 3;
        }
      }
      while (S<3){    //after E
        isNegativeEB = 0;
        Placer += 1;
        if(NumberB[Placer]==neg_){
          isNegativeEB = 1;
        }
        if (NumberB[Placer]<10){    //decimal
          EPlaceB += 1;
          NumberEB = NumberEB*10 + NumberB[Placer];
        }else if(NumberB[Placer]==dot_){  //if next is decimal
          //INVALID
        }else if(NumberB[Placer]==E_){ //if next is E
          //INVALID
        }else if(NumberB[Placer]==EmptyChar){ //if nothing next
          S = 3;
        }
      }
  }

void PushUp(){
//This transfers the exact number and its exact attributes to the top row
  for (int i=0; i<MaxSize; i++){  //each element
    NumberB[i] = NumberA[i];    //copy the correspinding element from A to B (bottom to top)
    NumberA[i] = EmptyChar;    //then clear A
  }
  for (int i = 0; i < 16; i++){
    DisplayNumB[i] = DisplayNumA[i];  //copy the display
  }
  isNegativeEB = isNegativeEA; //copy the negative attribute of exponent
  NumberA[0] = posi_;         //reset A to positive
  CurPlace = 0;               //back the cursor to the beginning
}

  
void EvaluateE(){
//Rewrite numbers with E to form (wwww.ddddEee --> wwwwwww.ddd or 0.000dddddd)

    AddDecimal(); //make sure there is a decimal in the number for proper sequence of reading
    //NumberA
      //Determine if E is negative
      if (NumberA[WholePlaceA + 1 + DecimalPlaceA + 1 + 1] == neg_){  //if the next character after 'E' is '-',
        isNegativeEA = 1;           //exponent is negative
      }else{
        isNegativeEA = 0;           //else, positive
      }
      //Remove E and NumberEA
        for (int i = WholePlaceA + DecimalPlaceA + 2; i < MaxSize; i++){  //since the exponent and sign is known, 
          NumberA[i] = EmptyChar;   //they can be removed and converted into '#'
        }
    
      //for Positive Exponents
      if (isNegativeEA==0 && EPlaceA > 0){
        
        //Move Decimal point to the right by Exponent places
        for (int i = WholePlaceA + 1; i < MaxSize; i++){ //starting from the digit after the decimal point, 
          if(i <= WholePlaceA + NumberEA){   //up to the (exponent) places, e.g.
            NumberA[i] = NumberA[i+1];        //move characters to the left
          }  
          if(i <= WholePlaceA + NumberEA + 2){  // if in case that the decimal point is moved to places that are empty,
            if (NumberA[i]==EmptyChar){         //replace
              NumberA[i]=0;                     //replace '#' with 0
            }
          }
        }
        NumberA[WholePlaceA + NumberEA + 1] = dot_; //put decimal place at right place (Whole Places + exponent)
    
      //Negative E 
      }else if (EPlaceA > 0){   //if not positive (isNegative != 0). This portion adds necessary 0' before the number before moving decimal
        int prevNumberEA = 0;   
        if (WholePlaceA < NumberEA + 1){ //if the exponent is larger than the number of whole number places,
          for (int i = _MaxSize; i> NumberEA - WholePlaceA + 1; i--){
            NumberA[i] = NumberA[i - (NumberEA - WholePlaceA + 1)];//shift the characters to right (places of movement equals the lacking)
          }
          for (int i = 1; i <= NumberEA - WholePlaceA + 1; i++){  
            NumberA[i] = 0;       //set the added numbers to 0
          }
          prevNumberEA = NumberEA;  //record the previous exponent
          AssessNum();              //recount the number of decimal and whole number places
          NumberEA = prevNumberEA;  //since the exponent is removed and recounted, it should be back to previous
        }
        for (int i = WholePlaceA + 1; i > WholePlaceA + 1 - NumberEA; i--){
          NumberA[i] = NumberA[i-1];  //move the characters from the decimal point to the right
        }
        NumberA[WholePlaceA + 1 - NumberEA] = dot_;       //add decimal point exponent places before the previous location
      }


    //NumberB - Same as number A
      //Determine if E is negative
      if (NumberB[WholePlaceB + 1 + DecimalPlaceB + 1 + 1] == neg_){
        isNegativeEB = 1;
      }else{
        isNegativeEB = 0;
      }
      //Remove E and NumberEB
        for (int i = WholePlaceB + DecimalPlaceB + 2; i < MaxSize; i++){
          NumberB[i] = EmptyChar;
        }
    
      //Positive EB
      if (isNegativeEB==0 && EPlaceB > 0){

        //Move Decimal
        for (int i = WholePlaceB + 1; i < MaxSize; i++){
          if(i <= WholePlaceB + NumberEB){   //move characters to the left
            NumberB[i] = NumberB[i+1];
          }   
          if(i <= WholePlaceB + NumberEB + 2){     //put the necessary 0's
            if (NumberB[i]==EmptyChar){
              NumberB[i]=0;
            }
          }
        }
        NumberB[WholePlaceB + NumberEB + 1] = dot_; //put decimal place at right place
    
      //Negative EB
      }else if (EPlaceB > 0){
        int prevNumberEB = 0;
        if (WholePlaceB < NumberEB + 1){//make sure there is space for moving decimal to the left
          for (int i = _MaxSize; i> NumberEB - WholePlaceB + 1; i--){
            NumberB[i] = NumberB[i - (NumberEB - WholePlaceB + 1)];//shift to right
          }
          for (int i = 1; i <= NumberEB - WholePlaceB + 1; i++){  //set the former numbers to 0
            NumberB[i] = 0;
          }
          prevNumberEB = NumberEB;
          AssessNum();
          NumberEB = prevNumberEB;
        }
        for (int i = WholePlaceB + 1; i > WholePlaceB + 1 - NumberEB; i--){
          NumberB[i] = NumberB[i-1];
        }
        NumberB[WholePlaceB + 1 - NumberEB] = dot_;  
      }
    AddDecimal(); //after the movement, there can be no zeroes after the decimal point
  }


void AddDecimal(){
//If there is no decimal point, add a decimal point and 0 after it. It is because some functions rely on this structure

  AssessNum(); //assess the attributes

  //add decimal point to NumberA
      if (DecimalPlaceA==0 && NumberA[1]!=EmptyChar){ //if there is no decimal point, and is not empty,
        for (int i = _MaxSize; i > WholePlaceA + 2; i--){ //for all the characters to the right of the last whole number digit,
          NumberA[i] = NumberA[i-2];  //move characters to the right in 2 places to fit a dot and a 0 
        }
        NumberA[WholePlaceA + 1] = dot_; //put decimal point
        NumberA[WholePlaceA + 2] = 0;     //put zero
        AssessNum();
      }
  //add decimal point to NumberB  - same as NumberA
      if (DecimalPlaceB==0 && NumberB[1]!=EmptyChar){
        for (int i = _MaxSize; i > WholePlaceB + 2; i--){
          NumberB[i] = NumberB[i-2];
        }
        NumberB[WholePlaceB + 1] = dot_;
        NumberB[WholePlaceB + 2] = 0;
        AssessNum();
      }
}// END ADD DECIMAL

void DetermineInequality(){
//This determines which number is larger

  EvaluateE();  //first, transform numbers with E to its general form (ww.ddddd)
  RemoveZero(); //remove unnecessary zeros
  Inequality = 0; //if not altered later, it means they are equal

  if (WholePlaceA > WholePlaceB){ //if A has more whole number places than B, A is larger
    Inequality = 1;
  }else if (WholePlaceA < WholePlaceB){ //else if the other way around, B is larger
    Inequality = 2;
  }else{  //if equal, 

    for (int i = 1; i < MaxSize; i++){ //compare the corresponding elements
      int TempA = NumberA[i];
      int TempB = NumberB[i];
      if (TempA == EmptyChar){TempA = 0;} //if the element is '#', consider it as 0
      if (TempB == EmptyChar){TempB = 0;}
      if (TempA > TempB){   //if A is the first one to have a larger element, 
        Inequality = 1;      //A is larger
        break;
      }else if (TempB > TempA){ //in the other way around, 
        Inequality = 2;       //B is larger
        break;
      }
    }
  }   
}

void ExchangeAB(){
//This exchanges all the elements of A and B, which also exchanges their attributes
    int Temp[MaxSize];
    for (int i = 0; i < MaxSize; i++){
      Temp[i] = NumberB[i];       //temporarily transfer B to temporary array
      NumberB[i] = NumberA[i];    //transfer A to B
      NumberA[i] = Temp[i];       //from the temporary array which contains the original B, transfer to A
    }
    AssessNum();  //re-assess the numbers for their attributes
}

void RemoveZero(){
// This removes the unecessary zeroes from the left(0011.2) and right(11.20000)  

  int ZeroCount = 0; //counts the zeroes
  AddDecimal(); //first, transform to standard
  //Count the number of Left Zeros in A
  for (int i = 1; i < WholePlaceA -1; i++){ //exclude the last digit before decimal so it won't affect if (0.ddd)
    if (NumberA[i] == 0) {  //if 0
      ZeroCount += 1;       //count
    }else{
      break;
    }
  }
  //move to left
  for (int i = 1; i < MaxSize-ZeroCount; i++){ 
    NumberA[i] = NumberA[i + ZeroCount];  //move the characters to the left by "counted zeroes" places
  }
  for (int i = MaxSize-ZeroCount; i < MaxSize; i++){ 
    NumberA[i] = EmptyChar;  //clear the old positions of numbers from the right
  }
  ZeroCount = 0;
  //Count zero for B - Same as A
  for (int i = 1; i < WholePlaceB - 1; i++){
    if (NumberB[i] == 0) {
      ZeroCount += 1;
    }else{
      break;
    }
  }
  //Move to left
  for (int i = 1; i < MaxSize-ZeroCount; i++){
    NumberB[i] = NumberB[i + ZeroCount];
  }
  for (int i = MaxSize-ZeroCount; i < MaxSize; i++){
    NumberB[i] = EmptyChar;  //clean the remain
  }
  AssessNum(); //re-assess attributes

  // For right zeros (0.dddd0000000000), only applicable if on decimal places
  //Count the number of Right-Zeros of A
  for (int i = _MaxSize; i > WholePlaceA + 2; i--){ //exclude the last digit before decimal for (wwww.0)
    if (NumberA[i] == 0 || NumberA[i] == EmptyChar) {    //if it faces 0's or empty,
      NumberA[i] = EmptyChar; //clear
    }else{
      break;    //if there is another character, it's significant so stop
    }
  }
  //Count the number of Right Zeros B - Same as A
  for (int i = _MaxSize; i > WholePlaceB + 2; i--){ //exclude the last digit before decimal
    if (NumberB[i] == 0 || NumberB[i] == EmptyChar) {    //include blanks
      NumberB[i] = EmptyChar;
    }else{
      break;    //if there is another character, it's significant, stop
    }
  }
  AssessNum(); //re-assess attributes
}

void AnswerToNumA(){
//Transfer the answer from array Answer[] to array NumberA[] (bottom-row number)

  int ZerosLeft = 0;
  int ZerosRight = 0;
  NumberA[0] = Answer[0];
  //Count unnecessary zeroes from left, exclude the last digit before decimal point
  for (int i = 1; i < _MaxSize; i++){ //Answer do not have empty chars, only 0's
    if (Answer[i] == 0){
      ZerosLeft += 1;
    }else{
      break;
    }
  }
  //Count unnecessary zeroes from right of decimal point, exclude the last before
  for (int i = _MaxSize*2; i > MaxSize + 1; i--){
    if (Answer[i] == 0){
      ZerosRight += 1;
    }else{
      break;
    }}
  //clear A
  for (int i = 1; i < MaxSize; i++){
    NumberA[i] = EmptyChar;} 
  //Move the Answer to Number A starting from after the unnecessary left-zeroes up to before the unnecessary right-zeroes
  for (int i = ZerosLeft + 1; i <= _MaxSize*2 - ZerosRight; i++){
    NumberA[i - ZerosLeft] = Answer[i];}
    NumberA[0] = Answer[0];
  //clear numB - need to clear it because the calcu displays the answer
  for (int i=1; i<MaxSize; i++){  
    NumberB[i] = EmptyChar;    
  }
  NumberB[0] = posi_; //reset B to positive
  //clear displayB
  for (int i = 0; i < 16; i++) {DisplayNumB[i] = blank_;}
  AssessNum(); 
}

void ClearAnswer(){
//This clears the array Answer into its initial values
  for (int i = 1; i <= _MaxSize*2; i++){
    Answer[i] = 0;
  }
  Answer[MaxSize]=dot_; //decimal point
  //no need to clear sign since it's always indicated before operation
}

void RefreshDisplay(){
//This facilitates what is displayed in the LCD

  int Placer = 0;
  lcd.clear(); //clear first

  //for Number A
  
  for (int i = 0; i < 16; i++) {DisplayNumA[i] = blank_;  }      //clear display array for A

  //Inputting
  if (isOutput == 0){ //This means we are inputting, not from answer
   
    Placer = WholePlaceA + DecimalPlaceA + EPlaceA + isNegativeEA;  //initial character count
    for (int i = 1; i < MaxSize; i++){
      if (NumberA[i] == E_ || NumberA[i] == neg_ || NumberA[i] == dot_){
        Placer += 1;        //slot for each special character
      }
    }
    
    if (NumberA[0]==neg_) {                     //if A is negative,
      lcd.setCursor(15 - Placer,1);             
      DisplayNumA[15 - Placer] = neg_;        //put negative sign in array
      lcd.print("-");                         //shortcut for putting negativ sign
    }
    for (int i = 16-Placer; i < 16; i++){DisplayNumA[i] = NumberA[i - 15 + Placer]; } // The succeeding display is the succeeding A elements
  }
  //Output  - the calcu is displaying an output from answer
  else{
    OutputDisplay();  //call a particular function for this
  }

  //print DisplayNumA to LCD
  for (int i = 0 ; i < 16; i++){
    lcd.setCursor(i,1);
    if (DisplayNumA[i] > 9){
      lcd.print(char(DisplayNumA[i]));    //print signs and special characters from ASCII
    }else{
      lcd.print(DisplayNumA[i]);          //print numerics
    }    
  }
  
  //B
  //B is just from what was previously displayed in NumA
  lcd.setCursor(0,0);
  for (int x : DisplayNumB){
    if (x > 9){
      lcd.print(char(x)); //if not numerics, print ASCII equivalents
    }
    else{
      lcd.print(x); //if numerics, print sstraight
    }
  }
}

void OutputDisplay(){  
//assigned for displaying answers
//note that at this point NumA is in general form +-123.456 without E
    
    int NegSlot = 0;  //reserved slot for negative sign

    //Determine if answer is negative
    if (NumberA[0]==neg_) {                     //Negative Sign
      NegSlot = 1;      //reserve slot for negative sign
    }

    //Determine the number of zeroes from the right of decimal point, going to right, where there is a non-zero digit next
    //application: Negative exponent
      int NegExponent = 0;
      for (int i = MaxSize + 1; i < _MaxSize * 2 + 1; i++){ //from after the decimal point up to the 2nd to the last digit       
        if (Answer[i]!=0 && Answer[i+1]!=EmptyChar){break;} //if the end is empty, skip
        if (i == _MaxSize*2 - 1 && Answer[_MaxSize*2] == 0){  //this means all these zeroes are unnecessary
          NegExponent = 0;  //so there is no actually decimal place
          break;
        }
        NegExponent += 1; //count zeroes in between the decimal point and non-zero digit
      }      

    //CATERGORY 1: Needs positive scientific notation
    if (WholePlaceA + NegSlot > 15 ){ //this activates when the number of whole number places is greater than 15 (not fitting the screen)
        if (NegSlot==1){
          DisplayNumA[0] = neg_;  //if negative number, put negative
        }
          DisplayNumA[NegSlot] = NumberA[1]; //the first digit is the first digit of the answer
          DisplayNumA[NegSlot + 1] = dot_;  //Then, decimal point

        //determine the number of digits for exponent, limit of 999 exponent
        int ELength = 0;
        int Exponent[3]; //stores the exponent in array form to be written later
        if (WholePlaceA-1>=1){ //if the number of whole number places >= 1
          ELength = 1;  //exponent is only one digit
          Exponent[1] = ((WholePlaceA - 1) % 10); //remainder after dividing the places by 10 to determine E
        }if (WholePlaceA-1>=10){  //if the number of whole number places >= 10
          ELength = 2;  //2 digit exponent
          Exponent[2] = (((WholePlaceA-1) % 100) - ((WholePlaceA-1) % 10))/10; //the first digit is the multiple of 10 
          Exponent[1] = ((WholePlaceA-1) % 10); //and next is the multiple of 1
        }
        if (WholePlaceA-1>=100){ //if the number of whole number places >= 100
          ELength = 3;  //3 digits
          Exponent[3] = (((WholePlaceA-1) % 1000) - ((WholePlaceA-1) % 100))/100; //the first digit is the multiple of 100
          Exponent[2] = (((WholePlaceA-1) % 100) - ((WholePlaceA-1) % 10))/10; //the first digit is the multiple of 10
          Exponent[1] = ((WholePlaceA-1) % 10); //the first digit is the multiple of 1
        }

        //Place the base
        for (int i = NegSlot + 2; i < 16-ELength-2;i++){ //reserve slots for the 'E', possible negative sign, and the number of digits of E
          DisplayNumA[i] = NumberA[i-NegSlot]; //write the base number
        }

        //RoundOff
        if (NumberA[16-ELength-1] > 4){
          DisplayNumA[16-ELength-2] = NumberA[16-ELength-2-NegSlot] + 1; //if the last digit is > 4, round up
        }else{
          DisplayNumA[16-ELength-2] = NumberA[16-ELength-2-NegSlot]; //else, no round
        }
        //Place E
        DisplayNumA[15-ELength] = E_;
        //Place exponent
        for (int i = ELength; i > 0; i--){
         DisplayNumA[16-i] = Exponent[i];
        } 
        //Clean Middle Zeroes
        while (1){//basically copy then del --> move
          if (DisplayNumA[14-ELength] != 0 && DisplayNumA[14-ELength] != dot_){break;} //continue delete if 0 or .
          for (int i = 14-ELength; i > 0; i--){
            DisplayNumA[i] = DisplayNumA[i-1]; //move the base to the right as long as the right most digit is 0
            DisplayNumA[i-1] = blank_;      //set the previous location to blank
          }
        }
    }
    //CATEGORY 2: Needs negative scientific notation when zeros is >= 5 or (E-6)
    else if(WholePlaceA == 1 && Answer[MaxSize-1] == 0 && NegExponent > 5){

      //Place negative if needed 
      if (NegSlot==1){
        DisplayNumA[0] = neg_;
      }
        DisplayNumA[NegSlot] = Answer[MaxSize + NegExponent]; //place the 1st digit
        DisplayNumA[NegSlot + 1] = dot_; //place the decimal point
      
      //determine the number of digits for exponent, limit of 999 exponent
        int ELength = 0;
        int Exponent[3];
        //The digits for the exponent is based on the number of zeroes between the decimal point and non-zero digit
        if (NegExponent>=1){ //same as in category 1, 1-digit exponent
          ELength = 1;
          Exponent[1] = (NegExponent % 10); 
        }if (NegExponent>=10){ //same as in category 1, 2-digit exponent
          ELength = 2;
          Exponent[2] = ((NegExponent % 100) - (NegExponent % 10))/10;
          Exponent[1] = (NegExponent % 10);
        }
        if (NegExponent>=100){ //same as in category 1, 3-digit exponent
          ELength = 3;
          Exponent[3] = ((NegExponent % 1000) - (NegExponent % 100))/100;
          Exponent[2] = ((NegExponent %  100) - (NegExponent % 10))/10;
          Exponent[1] = (NegExponent % 10);
        }
        //Same as in category 1
        //Place the base
        for (int i = NegSlot + 2; i < 15-ELength-2; i++){
          DisplayNumA[i] = Answer[MaxSize + i - NegSlot + NegExponent - 1];
        }
        //RoundOff the last digit but one more slot that is for the negative sign after the 'E'
        if (Answer[MaxSize + 13 - ELength - NegSlot + NegExponent] > 4){
          DisplayNumA[13-ELength] = Answer[MaxSize + 12 - ELength - NegSlot + NegExponent] + 1;
        }else{
          DisplayNumA[13-ELength] = Answer[MaxSize + 12 - ELength - NegSlot + NegExponent];
        }
        //Place E
        DisplayNumA[14-ELength] = E_;
        //Place -
        DisplayNumA[15-ELength] = neg_;
        //Place exponent
        for (int i = ELength; i > 0; i--){
         DisplayNumA[16-i] = Exponent[i];
        }
        //Clean Middle Zeroes
        while (1){
          if (DisplayNumA[14-ELength] != 0 && DisplayNumA[14-ELength] != dot_){break;} //continue delete if 0 or .
          for (int i = 14-ELength; i > 0; i--){
            DisplayNumA[i] = DisplayNumA[i-1];
            DisplayNumA[i-1] = blank_;      //basically copy then del --> move
          }
        }
    }
    //CATEGORY 3: All characters can fit and has no decimal (nnnn.0)
    else if(DecimalPlaceA==1 && Answer[MaxSize+1]==0 && WholePlaceA + NegSlot < 16){  //No decimal, all can fit
      if (NegSlot==1){
        DisplayNumA[15-WholePlaceA] = neg_; //if negative, place negative sign
      }    
      for (int i = 16 - WholePlaceA + NegSlot; i <= 16; i++){ //reduce and move 1 slot if negative
        DisplayNumA[i-NegSlot] = NumberA[i + WholePlaceA - 15 - NegSlot]; //copy paste until before '.'
      }
    }
    //CATEGORY 4: All characters can fit and has significant decimal places
    else if(DecimalPlaceA > 0 && WholePlaceA+DecimalPlaceA + NegSlot < 16){          
      if (NegSlot==1){
        DisplayNumA[16-WholePlaceA-DecimalPlaceA] = neg_; //if negative, place negative sign
      }
      for (int i = 16 - WholePlaceA - DecimalPlaceA - 1 + NegSlot; i <= 16; i++){ //reduce and move 1 slot if negative
        DisplayNumA[i-NegSlot] = NumberA[i + WholePlaceA + DecimalPlaceA + 1 - 15 - NegSlot]; //copy paste from end to finish
      }
    }
    //CATEGORY 5: Characters can not fit but some decimals are included in the screen
    else if(WholePlaceA<16 && WholePlaceA + DecimalPlaceA + NegSlot + 1> 16){ //can happen if Whole Number Places is less than 15
      if (NegSlot==1){
        DisplayNumA[0] = neg_;  //for negative sign
      }
      for (int i = NegSlot; i < 15; i++){
        DisplayNumA[i] = NumberA[i + 1 - NegSlot]; //copy paste until before the last digit
      }
      //Round off the last digit
      if (NumberA[15 + 1 - NegSlot] > 4){
        DisplayNumA[15] = NumberA[15 + 1- NegSlot] + 1;
      }else{
        DisplayNumA[15] = NumberA[15 + 1 - NegSlot];
      }
    }
}

void ErrorPrompt(int ErrorIndex){
  
  lcd.clear();

  switch (ErrorIndex){
    case 1: //Math errpr
      lcd.print("Math Error");
      break;
    case 2: //Syntax Error
      lcd.print("Syntax Error");
      break;
  }
  while(1){ 
    WhatButton();
    //wait for a button press   
    if (CmdIndex != 16){
      lcd.clear();  //then reset
      ClearArray();   
      break;
    }
  }
}

void PleaseWait(){
//displays when operation is taking longer
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Taking Longer...");
  lcd.setCursor(0,1);
  float _Percent = 0;
  for (int i = 1; i < 7; i++){
    if (NumberA[i] == 35){
      _Percent = _Percent*10 + 0;
    }else{
      _Percent = _Percent*10 + NumberA[i];
    }
  }
  lcd.print(100 - 100*(_Percent / Percentage));
  lcd.print("%");
} 