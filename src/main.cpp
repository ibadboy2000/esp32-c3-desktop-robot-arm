#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include "MotorControl.h"
#include "WebUI.h"

// 禁用欠压检测器，防止舵机/电机工作时电压跌落导致芯片复位
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ESP32-C3 Supermini 内置LED在GPIO8，低电平点亮
#define LED_PIN 8

// Ticker硬件定时器，用于配网模式下 LED 闪烁（不受主循环阻塞影响）
Ticker ledTicker;
void toggleLED() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

WebServer server(80);

uint64_t lastCmdSeq = 0;

// 处理首页请求
void handleRoot() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", web_ui_html);
}

// 处理控制API请求
void handleControl() {
    if (server.method() != HTTP_POST) {
        server.sendHeader("Connection", "close");
        server.send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
        return;
    }

    String body = server.arg("plain");
    Serial.printf("[API] 收到请求: %s\n", body.c_str());

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.printf("[API] JSON解析失败: %s\n", error.c_str());
        server.sendHeader("Connection", "close");
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    uint64_t seq = doc["seq"] | 0ULL;
    if (seq > 0) {
        if (seq <= lastCmdSeq) {
            server.sendHeader("Connection", "close");
            server.send(200, "application/json", "{\"status\":\"ignored_old_seq\"}");
            return;
        }
        lastCmdSeq = seq;
    }

    String device = doc["device"] | "";
    String action = doc["action"] | "";
    int speed = doc["speed"] | 255;
    unsigned long duration = doc["duration"] | 0;

    Serial.printf("[API] device=%s action=%s speed=%d duration=%lu\n",
                  device.c_str(), action.c_str(), speed, duration);

    if (device == "tt") {
        if (action == "up") motorControl.setTTMotor(1, speed, duration);
        else if (action == "down") motorControl.setTTMotor(-1, speed, duration);
        else motorControl.stopTTMotor();
    } else if (device == "servo1") {
        if (action == "left") motorControl.setServo1Direction(-1, speed, duration);
        else if (action == "right") motorControl.setServo1Direction(1, speed, duration);
        else motorControl.setServo1Direction(0);
    } else if (device == "servo2") {
        if (action == "left") motorControl.setServo2Direction(-1, speed, duration);
        else if (action == "right") motorControl.setServo2Direction(1, speed, duration);
        else motorControl.setServo2Direction(0);
    } else if (device == "servo3") {
        if (action == "left") motorControl.setServo3Direction(-1, speed);
        else if (action == "right") motorControl.setServo3Direction(1, speed);
        else if (action == "center") { motorControl.stopServo3(); motorControl.setServo3Angle(90); }
        else if (action == "setmax") { int maxA = doc["value"] | 160; motorControl.setServo3MaxAngle(maxA); }
        else motorControl.stopServo3();
    }

    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// 处理状态API请求
void handleStatus() {
    StaticJsonDocument<256> doc;
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["servo3_angle"] = motorControl.getServo3Angle();
    doc["servo3_max"] = motorControl.getServo3MaxAngle();
    doc["tt_dir"] = motorControl.ttDir;
    doc["s1_dir"] = motorControl.servo1Dir;
    doc["s2_dir"] = motorControl.servo2Dir;
    doc["s3_dir"] = motorControl.servo3Dir;
    
    String response;
    serializeJson(doc, response);
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", response);
}

// 处理未匹配的请求（如 /favicon.ico）
void handleNotFound() {
    server.sendHeader("Connection", "close");
    server.send(404, "text/plain", "Not Found");
}

void setup() {
    // 注: 恢复开启欠压检测器。
    // 如果供电不足，宁可让芯片复位，也不能死机导致需要断电重启。
    // 建议用户使用更强劲的供电（比如外接 5V 3A 电池）来彻底解决断电重启问题。

    // LED先点亮，证明芯片在运行
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LOW=亮

    // 短暂等待芯片稳定
    delay(500);

    Serial.begin(115200);
    // 移除 while(!Serial) 阻塞，否则在不连接电脑拔下USB时，程序会永远卡死在这里！
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32-C3 机械臂控制器启动中...");
    Serial.println("========================================");

    // LED灭掉，表示开始配网
    digitalWrite(LED_PIN, HIGH);

    Serial.println("[WiFi] 正在初始化WiFiManager...");

    WiFiManager wm;
    wm.setDebugOutput(true);

    uint32_t chipId = 0;
    for (int i = 0; i < 17; i += 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    String apName = "ESP32-RobotArm-" + String(chipId, HEX);
    Serial.printf("[WiFi] AP名称: %s\n", apName.c_str());

    wm.setConfigPortalTimeout(300);

    // 配网模式下用Ticker硬件定时器让LED闪烁（autoConnect是阻塞调用，loop()不会跑）
    wm.setAPCallback([](WiFiManager *myWiFiManager) {
        Serial.println("[WiFi] 已进入配网模式，LED开始闪烁...");
        Serial.println("[WiFi] 请连接热点并访问 192.168.4.1");
        ledTicker.attach(0.5, toggleLED); // 每0.5秒闪烁一次
    });

    Serial.println("[WiFi] 调用 autoConnect...");

    if (!wm.autoConnect(apName.c_str())) {
        Serial.println("[WiFi] 配网超时，重启...");
        ledTicker.detach();
        delay(3000);
        ESP.restart();
    }

    // WiFi连接成功，停止闪烁，LED常亮
    ledTicker.detach();
    digitalWrite(LED_PIN, LOW); // LED常亮
    Serial.println("[WiFi] ===== WiFi连接成功! =====");
    Serial.print("[WiFi] IP地址: ");
    Serial.println(WiFi.localIP());

    // 初始化电机
    Serial.println("[Motor] 初始化电机和舵机...");
    motorControl.init();

    // 配置Web服务器路由
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/control", HTTP_POST, handleControl);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.onNotFound(handleNotFound); // 处理favicon.ico等未知请求

    // 处理跨域预检请求
    server.on("/api/control", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.sendHeader("Connection", "close");
        server.send(204);
    });

    server.begin();

    Serial.println("[Web] ===== Web服务器已启动! =====");
    Serial.printf("[Web] 请访问 http://%s 控制机械臂\n", WiFi.localIP().toString().c_str());
}

void loop() {
    server.handleClient();
    motorControl.loop();

    // 配网模式LED闪烁（如果WiFi未连接，每500ms闪一次）
    static unsigned long lastBlink = 0;
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    }

    // 检测WiFi是否断开，自动重连
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 5000) {
        lastWifiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WiFi] 连接断开，尝试重连...");
            WiFi.reconnect();
        }
    }

    delay(2);
}
