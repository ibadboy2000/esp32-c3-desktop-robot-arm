#pragma once
#include <Arduino.h>

// 引脚定义
#define SERVO1_PIN 2    // 360度舵机1
#define SERVO2_PIN 3    // 360度舵机2
#define SERVO3_PIN 4    // 180度舵机(夹头)
#define MOTOR_IN1_PIN 0 // TT电机
#define MOTOR_IN2_PIN 1 // TT电机

// ESP32-C3 LEDC通道分配（共6个通道0-5，共4个定时器）
// ★★★ 关键：同一定时器的通道必须使用相同频率 ★★★
// 定时器映射: Ch0,Ch1→Timer0 | Ch2,Ch3→Timer1 | Ch4,Ch5→Timer2
// 通道0: Servo1 (50Hz)  ┐ Timer0 (50Hz)
// 通道1: Servo2 (50Hz)  ┘
// 通道2: Servo3 (50Hz)  ┐ Timer1 (50Hz)
// 通道3: (未使用)        ┘
// 通道4: TT电机IN1 (5kHz) ┐ Timer2 (5kHz)
// 通道5: TT电机IN2 (5kHz) ┘
#define SERVO1_CH 0
#define SERVO2_CH 1
#define SERVO3_CH 2
#define MOTOR_CH1 4
#define MOTOR_CH2 5

class MotorControl {
public:
    int ttDir = 0;
    int servo1Dir = 0;
    int servo2Dir = 0;
    int servo3Dir = 0;

    void init();
    void loop();

    // 360度舵机：dir=1正转, -1反转, 0停止; speed=0-255; duration_ms=运行时长
    void setServo1Direction(int dir, int speed = 255, unsigned long duration_ms = 0);
    void setServo2Direction(int dir, int speed = 255, unsigned long duration_ms = 0);
    void stopServo1();
    void stopServo2();

    // 180度舵机：设置绝对角度
    void setServo3Angle(int angle);
    int getServo3Angle();
    
    // 180度舵机最大角度限制（防止夹头掉落）
    void setServo3MaxAngle(int maxAngle);
    int getServo3MaxAngle();
    
    // 180度舵机平滑连续运动控制：dir=1(正向), -1(反向), 0(停止); speed=0-255
    void setServo3Direction(int dir, int speed = 255);
    void stopServo3();

    // TT电机：dir=1正转/-1反转/0停止, speed=0-255, duration_ms=运行时长(0=持续)
    void setTTMotor(int dir, int speed, unsigned long duration_ms);
    void stopTTMotor();

private:
    // 将角度转换为舵机PWM占空比（50Hz下，500-2500us脉宽）
    int angleToDuty(int angle);

    int currentServo3Angle = 90;
    int servo3MaxAngle = 160;  // 最大角度限制，默认160度防止夹头掉落
    
    unsigned long ttMotorStopTime = 0;
    bool ttMotorRunning = false;
    unsigned long ttBrakeStopTime = 0;

    unsigned long servo1StopTime = 0;
    bool servo1Running = false;

    unsigned long servo2StopTime = 0;
    bool servo2Running = false;

    bool servo3Running = false;
    int servo3Speed = 255;
    unsigned long lastServo3Update = 0;
};

extern MotorControl motorControl;
