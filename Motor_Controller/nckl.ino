int stepPerRev = 1000;
boolean inches = false;
boolean absolute = false;
boolean controlled = false;
boolean inMotion = false;
int feedrate = 1000;
int spindle = 1000;
double curX = 0;
double curY = 0;
double curZ = 0;
Motor x = Motor();
Motor y = Motor();
Motor z = Motor();

void setup() {
  Serial.begin(9600);
  x.init(8, 9, 0.0471); //dirPin, stepPin mmPerPulse
  y.init(11, 10, 0.0471);
  z.init(12, 7, 0.0471);
}

void loop() {
  if (Serial.available() > 0) {
    String line = Serial.readString();
    int g = parseNumber(line, "G", -1);
    feedrate = parseNumber(line, "F", 100);
    spindle = parseNumber(line, "S", 1000);
    if (g != -1) {
      switch (g) {
        case 0:
          controlled = false;
          break;
        case 1:
          controlled = true;
          break;
        case 2:
          arc(true, parseNumber(line, "X", 0), parseNumber(line, "Y", 0), parseNumber(line, "I", 0), parseNumber(line, "J", 0));
          break;
        case 3:
          arc(false, parseNumber(line, "X", 0), parseNumber(line, "Y", 0), parseNumber(line, "I", 0), parseNumber(line, "J", 0));
          break;
        case 17:
        case 18:
        case 19:
        case 20:
          inches = true;
          break;
        case 21:
          inches = false;
          break;
        case 28:
        case 40:
        case 43:
        case 49:
        case 54:
        case 80:
        case 90:
          absolute = true;
          break;
        case 91:
          absolute = false;
          break;
      }

      x.setWaitTime(x.travel(parseNumber(line, "X", 0), feedrate));
      y.setWaitTime(y.travel(parseNumber(line, "Y", 0), feedrate));
      z.setWaitTime(z.travel(parseNumber(line, "Z", 0), feedrate));
      // Serial.println("Recieved: " + line);
      // Serial.println("Current Position: (" + String(curX, 3) + ", " + String(curY, 3) + ", " + String(curZ, 3) + ")");
    }
  }
  if (x.checkPulse()) {
    x.on();
  } else {
    x.off();
  }
  if (y.checkPulse()) {
    y.on();
  } else {
    y.off();
  }
  if (z.checkPulse()) {
    z.on();
  } else {
    z.off();
  }
}

double atan3(float dy, float dx) {
  double a = atan2(dy, dx);
  if (a < 0) a = (PI * 2.0) + a;
  return a;
}

void arc(boolean clockwise, double xEnd, double yEnd, double xOffset, double yOffset) {
  double MM_PER_SEGMENT = 1;
  double atanOffset = 0;
  if (!clockwise) {
    atanOffset = PI;
  }
  double angleStart = atan3((curY - (curY + yOffset)), (curX - (curX + xOffset)));
  double angleEnd = atan2(yEnd - (curY + yOffset), xEnd - (curX + xOffset));
  double sweep = angleStart - angleEnd;

  Serial.println(angleStart);

  Serial.println(angleEnd);

  if (clockwise && sweep < 0) angleEnd += 2 * PI;
  else if (!clockwise && sweep > 0) angleStart += 2 * PI;

  sweep = angleStart - angleEnd;
  double radius = sqrt(sq(curY - (curY + yOffset)) + sq(curX - (curX + xOffset)));
  double len = abs(sweep) * radius;

  int segments = floor(len / MM_PER_SEGMENT);

  double percentComplete = 0;
  double currentAngle = 0;
  double nx, ny;
  for (int i = 0; i < segments; i++) {
    percentComplete = ((double)i) / ((double)segments);
    currentAngle = angleStart - (sweep * percentComplete);
    nx = curX + cos(currentAngle) * radius;
    ny = curY + sin(currentAngle) * radius;

    double maxPulsesRemaining = x.getPulsesRemaining();
    if (maxPulsesRemaining < y.getPulsesRemaining()) {
      maxPulsesRemaining = y.getPulsesRemaining();
    }
    //Serial.println(ny);
    x.setWaitTime(x.travel(nx - curX, feedrate));
    y.setWaitTime(y.travel(ny - curY, feedrate));
    while (maxPulsesRemaining > 0) {
      if (x.checkPulse()) {
        x.on();
      } else {
        x.off();
      }
      if (y.checkPulse()) {
        y.on();
      } else {
        y.off();
      }

      maxPulsesRemaining = x.getPulsesRemaining();
      if (maxPulsesRemaining < y.getPulsesRemaining()) {
        maxPulsesRemaining = y.getPulsesRemaining();
      }
      delayMicroseconds(10);
    }
    curX = nx;
    curY = ny;
  }
  //Make final line
}

int parseNumber(String line, String cmd, int defaultValue) {
  int start = line.indexOf(cmd);
  if (start == -1) {
    return defaultValue;
  }
  String number = line.substring(start + 1);
  number = number.substring(0, number.indexOf(" "));
  return number.toInt();
}
