#include "MotorControl.h"

MotorControl motorControl;

// 将角度(0-180)转换为LEDC占空比
// 50Hz周期=20ms, 分辨率14bit=16384
// 500us对应0度: 500/20000*16384 = 409
// 2500us对应180度: 2500/20000*16384 = 2048
int MotorControl::angleToDuty(int angle) {
    return map(angle, 0, 180, 409, 2048);
}

void MotorControl::init() {
    Serial.println("[Motor] 开始初始化...");

    // 舵机使用50Hz PWM，14bit分辨率
    // 通道0: Servo1 (GPIO2)
    ledcSetup(SERVO1_CH, 50, 14);
    ledcAttachPin(SERVO1_PIN, SERVO1_CH);
    ledcWrite(SERVO1_CH, angleToDuty(90)); // 360度舵机停止
    Serial.println("[Motor] Servo1 初始化完成");

    // 通道1: Servo2 (GPIO3)
    ledcSetup(SERVO2_CH, 50, 14);
    ledcAttachPin(SERVO2_PIN, SERVO2_CH);
    ledcWrite(SERVO2_CH, angleToDuty(90)); // 360度舵机停止
    Serial.println("[Motor] Servo2 初始化完成");

    // 通道2: Servo3 (GPIO4)
    ledcSetup(SERVO3_CH, 50, 14);
    ledcAttachPin(SERVO3_PIN, SERVO3_CH);
    ledcWrite(SERVO3_CH, angleToDuty(90)); // 180度舵机归中
    Serial.println("[Motor] Servo3 初始化完成");

    // TT电机使用5kHz PWM，8bit分辨率
    // 通道4: Motor IN1 (GPIO0) — 与通道5共用Timer2
    ledcSetup(MOTOR_CH1, 5000, 8);
    ledcAttachPin(MOTOR_IN1_PIN, MOTOR_CH1);
    ledcWrite(MOTOR_CH1, 0);

    // 通道5: Motor IN2 (GPIO1)
    ledcSetup(MOTOR_CH2, 5000, 8);
    ledcAttachPin(MOTOR_IN2_PIN, MOTOR_CH2);
    ledcWrite(MOTOR_CH2, 0);
    Serial.println("[Motor] TT电机初始化完成");

    Serial.println("[Motor] 全部初始化完成!");
}

void MotorControl::loop() {
    unsigned long now = millis();
    
    if (ttBrakeStopTime > 0 && now >= ttBrakeStopTime) {
        ledcWrite(MOTOR_CH1, 0);
        ledcWrite(MOTOR_CH2, 0);
        ttBrakeStopTime = 0;
    }

    if (ttMotorRunning && ttMotorStopTime > 0 && now >= ttMotorStopTime) stopTTMotor();
    if (servo1Running && servo1StopTime > 0 && now >= servo1StopTime) stopServo1();
    if (servo2Running && servo2StopTime > 0 && now >= servo2StopTime) stopServo2();

    // 180度舵机平滑运动
    if (servo3Running && servo3Dir != 0) {
        // 固定的 20ms 更新周期 (对应50Hz PWM周期，防止更新过快导致PWM信号畸变)
        if (now - lastServo3Update >= 20) {
            // 速度 0-255 映射到每次跳变的角度步长 1 - 6 度
            int step = map(servo3Speed, 0, 255, 1, 6);
            setServo3Angle(currentServo3Angle + (servo3Dir * step));
            lastServo3Update = now;
        }
    }
}

void MotorControl::setServo1Direction(int dir, int speed, unsigned long duration_ms) {
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    
    int angleOffset = map(speed, 0, 255, 0, 90);
    int angle = 90;
    
    if (dir > 0) angle = 90 + angleOffset;
    else if (dir < 0) angle = 90 - angleOffset;
    
    ledcWrite(SERVO1_CH, angleToDuty(angle));
    
    if (dir != 0) {
        servo1Dir = dir;
        servo1Running = true;
        servo1StopTime = duration_ms > 0 ? millis() + duration_ms : 0;
    } else {
        stopServo1();
    }
}

void MotorControl::stopServo1() {
    ledcWrite(SERVO1_CH, angleToDuty(90));
    servo1Running = false;
    servo1Dir = 0;
}

void MotorControl::setServo2Direction(int dir, int speed, unsigned long duration_ms) {
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    
    int angleOffset = map(speed, 0, 255, 0, 90);
    int angle = 90;
    
    if (dir > 0) angle = 90 + angleOffset;
    else if (dir < 0) angle = 90 - angleOffset;
    
    ledcWrite(SERVO2_CH, angleToDuty(angle));
    
    if (dir != 0) {
        servo2Dir = dir;
        servo2Running = true;
        servo2StopTime = duration_ms > 0 ? millis() + duration_ms : 0;
    } else {
        stopServo2();
    }
}

void MotorControl::stopServo2() {
    ledcWrite(SERVO2_CH, angleToDuty(90));
    servo2Running = false;
    servo2Dir = 0;
}

void MotorControl::setServo3Angle(int angle) {
    if (angle < 0) angle = 0;
    if (angle > servo3MaxAngle) angle = servo3MaxAngle;
    ledcWrite(SERVO3_CH, angleToDuty(angle));
    currentServo3Angle = angle;
}

int MotorControl::getServo3Angle() {
    return currentServo3Angle;
}

void MotorControl::setServo3MaxAngle(int maxAngle) {
    if (maxAngle < 0) maxAngle = 0;
    if (maxAngle > 180) maxAngle = 180;
    servo3MaxAngle = maxAngle;
    // 如果当前角度超过新的最大角度，立即钳位
    if (currentServo3Angle > servo3MaxAngle) {
        setServo3Angle(servo3MaxAngle);
    }
    Serial.printf("[Motor] Servo3 最大角度设置为: %d\n", servo3MaxAngle);
}

int MotorControl::getServo3MaxAngle() {
    return servo3MaxAngle;
}

void MotorControl::setServo3Direction(int dir, int speed) {
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    
    if (dir != 0) {
        servo3Dir = dir;
        servo3Speed = speed;
        servo3Running = true;
    } else {
        stopServo3();
    }
}

void MotorControl::stopServo3() {
    servo3Running = false;
    servo3Dir = 0;
}

void MotorControl::setTTMotor(int dir, int speed, unsigned long duration_ms) {
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;

    if (dir > 0) {
        ledcWrite(MOTOR_CH1, speed);
        ledcWrite(MOTOR_CH2, 0);
    } else if (dir < 0) {
        ledcWrite(MOTOR_CH1, 0);
        ledcWrite(MOTOR_CH2, speed);
    } else {
        stopTTMotor();
        return;
    }

    ttDir = dir;
    ttMotorRunning = true;
    if (duration_ms > 0) {
        ttMotorStopTime = millis() + duration_ms;
    } else {
        ttMotorStopTime = 0;
    }
}

void MotorControl::stopTTMotor() {
    if (!ttMotorRunning && ttBrakeStopTime == 0) {
        ledcWrite(MOTOR_CH1, 0);
        ledcWrite(MOTOR_CH2, 0);
        return;
    }

    // 采用主动反接刹车 (Plug Braking) 防止滑行
    if (ttDir == 1) {
        ledcWrite(MOTOR_CH1, 0);
        ledcWrite(MOTOR_CH2, 255); // 反向全速
        ttBrakeStopTime = millis() + 40; 
    } else if (ttDir == -1) {
        ledcWrite(MOTOR_CH1, 255);
        ledcWrite(MOTOR_CH2, 0); // 反向全速
        ttBrakeStopTime = millis() + 40; 
    } else {
        ledcWrite(MOTOR_CH1, 0);
        ledcWrite(MOTOR_CH2, 0);
        ttBrakeStopTime = 0;
    }
    
    ttMotorRunning = false;
    ttDir = 0;
}
