//checks for new message to show
void showNewData(char message[number_units]) {
  if (strcmp(writtenLast, message) != 0) {
    strcpy(writtenLast, message);
    showMessage(message, flap_speed);
  }
  
}

//pushes message to units
void showMessage(char message[number_units], int flapSpeed) {
  // wait while display is still moving
  while (isDisplayMoving()) {
    Serial.println("moving");
    delay(500);
  }
  Serial.println("start to send");
  for (int i = 0; i < number_units; i++) {
    char currentLetter = toupper(message[i]);
    int currentLetterPosition = translateLettertoInt(currentLetter);
    writeToUnit(i, currentLetterPosition, flapSpeed);
  }
}

//translates char to letter position
int translateLettertoInt(char letterchar) {
  for (int i = 0; i < flapamount; i++) {
    if (letterchar == letters[i]) {
      return i;
    }
  }
}

//write letter position and speed in rpm to single unit
void writeToUnit(int address, int letter, int flapSpeed) {
  int sendArray[2] = {letter, flapSpeed}; //Array with values to send to unit

  Wire.beginTransmission(address);

  //Write values to send to slave in buffer
  for (int i = 0; i < sizeof sendArray / sizeof sendArray[0]; i++) {
    Wire.write(sendArray[i]);
  }
  Wire.endTransmission(); //send values to unit
}


//checks if unit in display is currently moving
bool isDisplayMoving() {
  //Request all units moving state and write to array
  for (int i = 0; i < number_units; i++) {
    displayState[i] = checkIfMoving(i);
    if (displayState[i] == 1) {
      return true;

      //if unit is not available through i2c
    } else if (displayState[i] == -1) {
      return true;
    }
  }
  return false;
}

//checks if single unit is moving
int checkIfMoving(int address) {
  int active;
  Wire.requestFrom(address, answersize, true);
  active = Wire.read();
  if (active == -1) {
    Wire.beginTransmission(address);
    Wire.endTransmission();
    //delay(5);
  }
  return active;
}
