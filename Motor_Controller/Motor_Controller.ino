class Motor {
  private:
    int _dirPin;
    int _stepPin;
    int _waitTime;
    long _lastPulse;
    double _distancePerPulse;
    double _pulsesRemaining;

  public:
    Motor();
    void init(int dirPin, int stepPin, double distancePerPulse);
    void on();
    void off();
    void setWaitTime(int waitTime);
    bool checkPulse();
    double travel(double deltaMM, double feedrate);
    double getPulsesRemaining();
};

Motor::Motor(){
  this->_dirPin = 0;
  this->_stepPin = 0;
  this->_waitTime = 1000;
}

void Motor::init(int dirPin, int stepPin, double distancePerPulse) {
  _dirPin = dirPin;
  _stepPin = stepPin;
  _distancePerPulse = distancePerPulse;
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  digitalWrite(_dirPin, LOW);
  digitalWrite(_stepPin, LOW);
}

bool Motor::checkPulse() {
  if (_pulsesRemaining > 0) {
    if (micros() >= _lastPulse + _waitTime) {
      this->_lastPulse = micros();
      this->_pulsesRemaining = _pulsesRemaining - 1;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

double Motor::travel(double deltaMM, double feedrate) {
  if (deltaMM > 0) {
    digitalWrite(_dirPin, HIGH);
  } else {
    digitalWrite(_dirPin, LOW);
  }
  this->_pulsesRemaining = deltaMM / _distancePerPulse;
  double pulsesPerSecond = feedrate / _distancePerPulse;
  double microseconds = (_pulsesRemaining / pulsesPerSecond) * 1000000;
  return (microseconds / _pulsesRemaining);
}

void Motor::on() {
  digitalWrite(_dirPin, HIGH);
  digitalWrite(_stepPin, HIGH);
}

void Motor::off() {
  digitalWrite(_dirPin, LOW);
  digitalWrite(_stepPin, LOW);
}

void Motor::setWaitTime(int waitTime) {
  this->_waitTime = waitTime;
}

double Motor::getPulsesRemaining() {
  return _pulsesRemaining;
}
