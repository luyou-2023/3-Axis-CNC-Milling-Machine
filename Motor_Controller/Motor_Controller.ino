// Motor.h
// 这是控制步进电机的 Motor 类，包含了初始化、电机启动、停止、脉冲控制等功能

class Motor {
  private:
    int _dirPin;             // 控制电机方向的引脚
    int _stepPin;            // 控制电机步进的引脚
    int _waitTime;           // 每个脉冲之间的等待时间（微秒）
    long _lastPulse;         // 上次发送脉冲的时间（微秒）
    double _distancePerPulse; // 每个脉冲对应的距离（单位：毫米）
    double _pulsesRemaining;  // 剩余的脉冲数

  public:
    Motor();                // 构造函数，初始化电机
    void init(int dirPin, int stepPin, double distancePerPulse); // 初始化电机，设置方向和步进引脚
    void on();              // 启动电机
    void off();             // 停止电机
    void setWaitTime(int waitTime); // 设置脉冲之间的等待时间
    bool checkPulse();      // 检查是否可以发送下一个脉冲
    double travel(double deltaMM, double feedrate); // 计算电机需要运行的时间，并发出步进脉冲
    double getPulsesRemaining(); // 获取剩余的脉冲数
};

// Motor.cpp
// 这是 Motor 类的实现文件，定义了电机的各种操作方法

#include <Arduino.h>

Motor::Motor() {
  this->_dirPin = 0;       // 默认方向引脚为0
  this->_stepPin = 0;      // 默认步进引脚为0
  this->_waitTime = 1000;  // 默认等待时间为1000微秒
}

// 初始化函数，设置电机的方向引脚、步进引脚和每个脉冲对应的距离
void Motor::init(int dirPin, int stepPin, double distancePerPulse) {
  _dirPin = dirPin;                           // 设置方向引脚
  _stepPin = stepPin;                         // 设置步进引脚
  _distancePerPulse = distancePerPulse;       // 设置每个脉冲对应的距离
  pinMode(dirPin, OUTPUT);                    // 设置方向引脚为输出模式
  pinMode(stepPin, OUTPUT);                   // 设置步进引脚为输出模式
  digitalWrite(_dirPin, LOW);                 // 初始化时设置方向为低（反向）
  digitalWrite(_stepPin, LOW);                // 初始化时设置步进为低（无脉冲）
}

// 检查是否可以发送下一个脉冲
bool Motor::checkPulse() {
  if (_pulsesRemaining > 0) {             // 如果还有剩余脉冲
    if (micros() >= _lastPulse + _waitTime) {  // 如果当前时间已超过上次脉冲时间加上等待时间
      this->_lastPulse = micros();       // 更新上次脉冲的时间
      this->_pulsesRemaining = _pulsesRemaining - 1;  // 剩余脉冲数减1
      return true;                       // 返回true，表示可以发送脉冲
    } else {
      return false;                      // 如果等待时间没有到，返回false
    }
  } else {
    return false;                         // 如果没有剩余脉冲，返回false
  }
}

// 根据目标距离和进给速度计算电机所需的脉冲数，并计算脉冲间隔时间
double Motor::travel(double deltaMM, double feedrate) {
  if (deltaMM > 0) {  // 如果目标距离为正，设置方向为正向
    digitalWrite(_dirPin, HIGH);
  } else {  // 如果目标距离为负，设置方向为反向
    digitalWrite(_dirPin, LOW);
  }

  this->_pulsesRemaining = deltaMM / _distancePerPulse;  // 计算剩余脉冲数
  double pulsesPerSecond = feedrate / _distancePerPulse; // 计算每秒所需的脉冲数
  double microseconds = (_pulsesRemaining / pulsesPerSecond) * 1000000; // 计算总的时间（微秒）
  return (microseconds / _pulsesRemaining);  // 返回每个脉冲之间的时间
}

// 启动电机，设置方向引脚和步进引脚为高
void Motor::on() {
  digitalWrite(_dirPin, HIGH);  // 设置方向引脚为高
  digitalWrite(_stepPin, HIGH); // 设置步进引脚为高，启动电机
}

// 停止电机，设置方向引脚和步进引脚为低
void Motor::off() {
  digitalWrite(_dirPin, LOW);  // 设置方向引脚为低，停止电机
  digitalWrite(_stepPin, LOW); // 设置步进引脚为低，停止电机
}

// 设置脉冲之间的等待时间，用于控制电机的运动速度
void Motor::setWaitTime(int waitTime) {
  this->_waitTime = waitTime;  // 设置脉冲之间的等待时间
}

// 获取剩余的脉冲数
double Motor::getPulsesRemaining() {
  return _pulsesRemaining;  // 返回剩余的脉冲数
}
