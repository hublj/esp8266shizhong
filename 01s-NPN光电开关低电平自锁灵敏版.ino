#include <ESP8266WiFi.h>

// 定义光电开关连接的引脚
const int photoelectricSwitchPin = 2;
// 定义输出控制的引脚
const int outputPin = 0;

// 防抖延时时间，单位：毫秒，可根据实际情况调整
const unsigned long debounceDelay = 20;
unsigned long lastDebounceTime = 0;
bool buttonState;
bool lastButtonState = HIGH;

// 用于记录当前的自锁状态，true为开启，false为关闭
bool selfLockState = false;

void setup() {
    // 初始化串口通信，波特率可根据需要设置
    Serial.begin(115200);

    // 设置光电开关引脚为输入模式
    pinMode(photoelectricSwitchPin, INPUT);
    // 设置输出引脚为输出模式
    pinMode(outputPin, OUTPUT);

    // 初始化输出引脚为低电平
    digitalWrite(outputPin, LOW);
}

void loop() {
    int reading = digitalRead(photoelectricSwitchPin);

    if (reading!= lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading!= buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                if (!selfLockState) {
                    Serial.println("第一次低电平，开启输出，自锁开启");
                    digitalWrite(outputPin, LOW);
                    selfLockState = true;
                } else {
                    Serial.println("第二次低电平，关闭输出，自锁关闭");
                    digitalWrite(outputPin, HIGH);
                    selfLockState = false;
                }
            }
        }
    }

    lastButtonState = reading;
}