String bootanim[] PROGMEM = {
  "SHOW:boot.pic",   // Load and show boot.pic
  "SCRIPT:boot.txt", // Load boot.txt as next script
  "QUIT"             // Quit if that fails
};

uint8_t bootanimindex = 0;
uint16_t animation_delay = 20;
bool bootanimRunning = true;

#define LABELS_COUNT 25
String label_names[LABELS_COUNT];
uint32_t label_positions[LABELS_COUNT];
uint8_t labels_next = 0;

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
    bool success = loadScriptFromSd(statement.substring(splitIndex + 1));
    if (success) {
      // reset some pointers
      labels_next = 0;
    }
  }
  if (command == "SLEEP") {
    sleep_command(statement.substring(splitIndex + 1));
  }
  if (command == "LABEL") {
    label_command(statement.substring(splitIndex + 1));
  }
  if (command == "GOTO") {
    goto_command(statement.substring(splitIndex + 1));
  }
  if (command == "QUIT") {
    bootanimRunning = false;
    return;
  }
}

void sleep_command(String duration) {
  int8_t unitposition = duration.indexOf("ms");
  if (unitposition < 0) {
    unitposition = duration.indexOf("s");
  }

  // If a unit was found...
  if (unitposition > 0) {
    // Split number and unit
    uint16_t duration_time = duration.substring(0, unitposition).toInt();
    String duration_unit = duration.substring(unitposition);

    // Execute delay
    if (duration_unit == "s") {
      delay(1000 * duration_time);
    }
    if (duration_unit == "ms") {
      delay(duration_time);
    }
  }
}

void label_command(String label) {
  if (labels_next >= LABELS_COUNT) {
    return;
  }

  label_names[labels_next] = label;
  label_positions[labels_next] = sd_scriptFilePosition();

#if DEBUG == 1
  Serial.print("label ");
  Serial.print(label);
  Serial.print(" at ");
  Serial.println(label_positions[labels_next]);
#endif

  labels_next++;
}

void goto_command(String label) {
  uint8_t i = 0;
  while (
    i < labels_next
  ) {
    if (label.equalsIgnoreCase(label_names[i])) {
      sd_scriptFileSeekPosition(label_positions[i]);
      return;
    }

    i++;
  }
}

