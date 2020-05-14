#ifndef MOTORSTEUERUNG
#define MOTORSTEUERUNG

struct MotorCommands {
  enum Type {
    STOP,
    MOVE_UP,
    MOVE_DOWN
  };
};
typedef MotorCommands::Type MotorCommand;

struct MotorsteuerungSTATES {
  /*
      State         __Power__
  */
  enum Type {
    UNKNOWN,      /* OFF */
    STOPPED,      /* OFF */
    MOVING_UP,    /* ON  */
    MOVING_DOWN,  /* ON  */
    OPEN,         /* OFF */
    CLOSED        /* OFF */
  };
};
typedef MotorsteuerungSTATES::Type MotorsteuerungSTATE;

#define POWEROFF HIGH
#define POWERON LOW
#define DIRUP LOW
#define DIRDOWN HIGH
#define POSITION_UP 28
#define POSITION_DOWN 1

class Motorsteuerung
{
  public:
    Motorsteuerung(int powerPin, int dirPin);
    void setCommand(MotorCommand cmd);
    void tick();
    bool publishPosition() {
      return __positionDelta != 0;
    }
    int estimatedPosition() {
      return __estimatedPosition;
    }
  private:
    MotorsteuerungSTATE stop();
    MotorsteuerungSTATE up();
    MotorsteuerungSTATE down();
    uint8_t __powerPin;
    uint8_t __dirPin;
    MotorsteuerungSTATE __targetState;
    MotorsteuerungSTATE __currentState = MotorsteuerungSTATES::UNKNOWN;
    bool __hasKnownState = false;
    bool __powerPinState = HIGH;
    bool __dirPinState = HIGH;
    uint16_t __estimatedPosition = POSITION_UP;
    int8_t __positionDelta = 0;
};

Motorsteuerung::Motorsteuerung(int powerPin, int dirPin)
{
  __powerPin = powerPin;
  __dirPin = dirPin;
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, POWEROFF);
  pinMode(dirPin, OUTPUT);
}

void Motorsteuerung::setCommand(MotorCommand cmd)
{
  __targetState = cmd == MotorCommands::STOP
                  ? MotorsteuerungSTATE::STOPPED
                  : cmd == MotorCommands::MOVE_UP
                  ? MotorsteuerungSTATE::OPEN
                  : MotorsteuerungSTATE::CLOSED;
}

void Motorsteuerung::tick()
{
  // no further actions if no state change is required
  if (__targetState == __currentState) {
    return;
  }

  switch (__targetState)
  {
    case MotorsteuerungSTATE::STOPPED:
      __currentState = stop();
      break;
    case MotorsteuerungSTATE::OPEN:
      __currentState = up();
      break;
    case MotorsteuerungSTATE::CLOSED:
      __currentState = down();
      break;
    default:
      break;
  }

  digitalWrite(__powerPin, __powerPinState);
  digitalWrite(__dirPin, __dirPinState);

  __estimatedPosition += __positionDelta;

  Serial.print("Write Power ");
  Serial.print(__powerPinState == LOW ? "on" : "off");
  Serial.print(" direction ");
  Serial.print(__dirPinState == DIRUP ? "up" : "down");
  Serial.print(" position ");
  Serial.print(__estimatedPosition);
  Serial.println();
}

MotorsteuerungSTATE Motorsteuerung::stop() {
  Serial.println("M Stop");
  __powerPinState = POWEROFF;
  __positionDelta = 0;
  return MotorsteuerungSTATE::STOPPED;
}

MotorsteuerungSTATE Motorsteuerung::up() {
  Serial.println("M up");
  if (__currentState == MotorsteuerungSTATE::MOVING_DOWN) {
    return stop();
  }
  if (!__hasKnownState) {
    __estimatedPosition = POSITION_DOWN;
    __hasKnownState = true;
  }
  if (__currentState == MotorsteuerungSTATE::STOPPED) {
    __dirPinState = DIRUP;
    return MotorsteuerungSTATE::MOVING_UP;
  }
  if (__currentState == MotorsteuerungSTATE::MOVING_UP) {
    if (__estimatedPosition >= POSITION_UP) {
      __targetState = MotorsteuerungSTATE::STOPPED;
      return stop();
    }
    __powerPinState = POWERON;
    __positionDelta = 1;
    return MotorsteuerungSTATE::MOVING_UP;
  }
  return stop();
}

MotorsteuerungSTATE Motorsteuerung::down() {
  Serial.println("M down");
  if (__currentState == MotorsteuerungSTATE::MOVING_UP) {
    return stop();
  }
  if (!__hasKnownState) {
    __estimatedPosition = POSITION_UP;
    __hasKnownState = true;
  }
  if (__currentState == MotorsteuerungSTATE::STOPPED) {
    __dirPinState = DIRDOWN;
    return MotorsteuerungSTATE::MOVING_DOWN;
  }
  if (__currentState == MotorsteuerungSTATE::MOVING_DOWN) {
    if (__estimatedPosition <= POSITION_DOWN) {
      __targetState = MotorsteuerungSTATE::STOPPED;
      return stop();
    }
    __powerPinState = POWERON;
    __positionDelta = -1;
    return MotorsteuerungSTATE::MOVING_DOWN;
  }
  return stop();
}

#endif
