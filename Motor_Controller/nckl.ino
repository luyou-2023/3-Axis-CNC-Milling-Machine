// 定义一些全局变量，初始化参数
int stepPerRev = 1000;         // 每转步数
boolean inches = false;        // 是否使用英寸（默认为假）
boolean absolute = false;      // 是否使用绝对坐标（默认为假）
boolean controlled = false;    // 电机控制状态（默认为不控制）
boolean inMotion = false;      // 电机是否在运动中（默认为假）
int feedrate = 1000;           // 默认进给速度（毫米/分钟）
int spindle = 1000;            // 默认主轴速度（单位可能是RPM）
double curX = 0;               // 当前X轴位置
double curY = 0;               // 当前Y轴位置
double curZ = 0;               // 当前Z轴位置
Motor x = Motor();             // X轴电机实例
Motor y = Motor();             // Y轴电机实例
Motor z = Motor();             // Z轴电机实例

void setup() {
  // 初始化串口通信
  Serial.begin(9600);
  // 初始化X、Y、Z轴电机
  x.init(8, 9, 0.0471); // X轴的方向引脚为8，步进引脚为9，每步对应的距离为0.0471毫米
  y.init(11, 10, 0.0471); // Y轴的方向引脚为11，步进引脚为10，每步对应的距离为0.0471毫米
  z.init(12, 7, 0.0471);  // Z轴的方向引脚为12，步进引脚为7，每步对应的距离为0.0471毫米
}

void loop() {
  // 如果串口有数据可读
  if (Serial.available() > 0) {
    String line = Serial.readString();  // 读取一行数据
    int g = parseNumber(line, "G", -1); // 解析G指令
    feedrate = parseNumber(line, "F", 100);  // 解析进给速度（F指令）
    spindle = parseNumber(line, "S", 1000); // 解析主轴速度（S指令）

    // 如果G指令有效，根据不同的G指令执行相应的动作
    if (g != -1) {
      switch (g) {
        case 0: // G0：快速定位
          controlled = false;  // 不控制电机
          break;
        case 1: // G1：线性插补
          controlled = true;   // 控制电机
          break;
        case 2: // G2：顺时针圆弧插补
          arc(true, parseNumber(line, "X", 0), parseNumber(line, "Y", 0), parseNumber(line, "I", 0), parseNumber(line, "J", 0));
          break;
        case 3: // G3：逆时针圆弧插补
          arc(false, parseNumber(line, "X", 0), parseNumber(line, "Y", 0), parseNumber(line, "I", 0), parseNumber(line, "J", 0));
          break;
        case 17:
        case 18:
        case 19:
        case 20:
          inches = true;  // 切换为英寸模式
          break;
        case 21:
          inches = false; // 切换为毫米模式
          break;
        case 28:
        case 40:
        case 43:
        case 49:
        case 54:
        case 80:
        case 90:
          absolute = true; // 启用绝对坐标
          break;
        case 91:
          absolute = false; // 启用相对坐标
          break;
      }

      // 更新X、Y、Z轴电机的脉冲等待时间
      x.setWaitTime(x.travel(parseNumber(line, "X", 0), feedrate));
      y.setWaitTime(y.travel(parseNumber(line, "Y", 0), feedrate));
      z.setWaitTime(z.travel(parseNumber(line, "Z", 0), feedrate));
    }
  }

  // 检查并控制电机是否运动
  if (x.checkPulse()) {
    x.on();  // 如果有脉冲，启动X轴电机
  } else {
    x.off(); // 如果没有脉冲，停止X轴电机
  }
  if (y.checkPulse()) {
    y.on();  // 如果有脉冲，启动Y轴电机
  } else {
    y.off(); // 如果没有脉冲，停止Y轴电机
  }
  if (z.checkPulse()) {
    z.on();  // 如果有脉冲，启动Z轴电机
  } else {
    z.off(); // 如果没有脉冲，停止Z轴电机
  }
}

// atan3 函数：用于计算角度，确保结果为正值
double atan3(float dy, float dx) {
  double a = atan2(dy, dx);
  if (a < 0) a = (PI * 2.0) + a; // 如果角度为负，转换为正角度
  return a;
}

// arc 函数：计算圆弧插补路径，生成多个小段
void arc(boolean clockwise, double xEnd, double yEnd, double xOffset, double yOffset) {
  double MM_PER_SEGMENT = 1;  // 每个圆弧段的长度（毫米）
  double atanOffset = 0;
  if (!clockwise) {
    atanOffset = PI;  // 如果是逆时针圆弧，调整角度偏移
  }
  double angleStart = atan3((curY - (curY + yOffset)), (curX - (curX + xOffset)));
  double angleEnd = atan2(yEnd - (curY + yOffset), xEnd - (curX + xOffset));
  double sweep = angleStart - angleEnd;

  // 如果圆弧的角度跨度为负，则调整角度
  if (clockwise && sweep < 0) angleEnd += 2 * PI;
  else if (!clockwise && sweep > 0) angleStart += 2 * PI;

  sweep = angleStart - angleEnd;
  double radius = sqrt(sq(curY - (curY + yOffset)) + sq(curX - (curX + xOffset))); // 计算圆弧半径
  double len = abs(sweep) * radius;  // 计算圆弧的总长度

  int segments = floor(len / MM_PER_SEGMENT);  // 计算需要多少小段来逼近圆弧

  double percentComplete = 0;
  double currentAngle = 0;
  double nx, ny;
  for (int i = 0; i < segments; i++) {
    percentComplete = ((double)i) / ((double)segments);  // 当前段的百分比
    currentAngle = angleStart - (sweep * percentComplete);  // 计算当前角度
    nx = curX + cos(currentAngle) * radius;  // 计算当前X坐标
    ny = curY + sin(currentAngle) * radius;  // 计算当前Y坐标

    double maxPulsesRemaining = x.getPulsesRemaining();
    if (maxPulsesRemaining < y.getPulsesRemaining()) {
      maxPulsesRemaining = y.getPulsesRemaining();  // 获取X、Y电机中剩余脉冲数最多的一个
    }

    // 更新X、Y电机的脉冲等待时间
    x.setWaitTime(x.travel(nx - curX, feedrate));
    y.setWaitTime(y.travel(ny - curY, feedrate));

    // 等待所有脉冲完成后再进行下一步
    while (maxPulsesRemaining > 0) {
      if (x.checkPulse()) {
        x.on();  // 启动X轴电机
      } else {
        x.off(); // 停止X轴电机
      }
      if (y.checkPulse()) {
        y.on();  // 启动Y轴电机
      } else {
        y.off(); // 停止Y轴电机
      }

      maxPulsesRemaining = x.getPulsesRemaining();
      if (maxPulsesRemaining < y.getPulsesRemaining()) {
        maxPulsesRemaining = y.getPulsesRemaining();
      }
      delayMicroseconds(10);  // 稍微延时，确保脉冲稳定
    }

    curX = nx;  // 更新当前X位置
    curY = ny;  // 更新当前Y位置
  }
  // 完成圆弧段的移动
}

// 解析指令中的数字，若找不到则返回默认值
int parseNumber(String line, String cmd, int defaultValue) {
  int start = line.indexOf(cmd);  // 查找命令所在位置
  if (start == -1) {
    return defaultValue;  // 如果未找到命令，则返回默认值
  }
  String number = line.substring(start + 1);  // 获取数字部分
  number = number.substring(0, number.indexOf(" "));  // 提取数字（去掉后面的空格）
  return number.toInt();  // 转换为整数并返回
}
