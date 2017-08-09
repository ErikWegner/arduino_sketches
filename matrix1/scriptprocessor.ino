String bootanim[] PROGMEM = {
  "SHOW:boot.pic",   // Load and show boot.pic
  "SCRIPT:boot.txt", // Load boot.txt as next script
  "QUIT"             // Quit if that fails
};

uint8_t bootanimindex = 0;
uint16_t animation_delay = 20;
bool bootanimRunning = true;

/* Intermediate function to walk through the bootanim array */
inline void scriptprocessor_executeNextStatement() {
  String statement = 0;
  if (bootanimRunning == false) {
    statement = sd_loadNextLine();
    if (!statement) {
      return;
    }
  } else {
    statement = bootanim[bootanimindex];
    bootanimindex++;
  }

  if (statement.length() > 1) {
    scriptprocessor_execute(statement);
  }
  
  delay(animation_delay);
}

void scriptprocessor_execute(String statement) {
#if DEBUG == 1
  Serial.print("scriptprocessor_execute(");
  Serial.print(statement);
  Serial.println(")");
#endif

  uint8_t splitIndex = statement.indexOf(':');
  String command = statement.substring(0, splitIndex);
  if (command == "SHOW") {
    loadAndShowImageFromSd(statement.substring(splitIndex + 1));
    return;
  }
  if (command == "SCRIPT") {
    loadScriptFromSd(statement.substring(splitIndex + 1));
  }
  if (command == "QUIT") {
    bootanimRunning = false;
    return;
  }
}


