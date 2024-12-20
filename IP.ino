#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ctime>

// 设置 OLED 屏幕的宽度和高度
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 设置 NTP 服务器和时区
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 8);

// 定义输出引脚
const int outputPin = 2;

// 定义 OLED 的 I2C 引脚
const int sclPin = 1;
const int sdaPin = 3;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

ESP8266WebServer server(80);

// 标志位，用于判断是否设置了定时
bool isTimed = false;

// 标志位，用于判断是否需要更新开关状态
bool settingsChanged = false;
bool isSwitchOn = false;

// 声明并初始化 lastCheckTime
unsigned long lastCheckTime = 0;

// 存储定时的开始小时和分钟
int startHour = 0;
int startMinute = 0;
// 存储定时的结束小时和分钟
int endHour = 0;
int endMinute = 0;

// 存储 IP 地址的字符串
String ipAddressStr;

void handleRoot();
void handleSetTime();
void handleCancelTiming();
void updateOLED(const char* status);

void handleRoot() {
    String html = "<html><head><meta charset=\"UTF-8\"></head><body>";
    html += "<h1>设置定时开关</h1>";
    if (isTimed) {
        html += "<p>当前处于定时状态。</p>";
        html += "<a href='/cancelTiming'>取消定时</a>";
    } else {
        html += "<p>当前处于未定时状态。</p>";
        html += "<a href='/setTime'>设置定时</a>";
    }
    html += "</body></html>";
    server.send(200, "text/html; charset=utf-8", html);
}

void handleSetTime() {
    if (server.method() == HTTP_POST) {
        Serial.println("Received POST request for setting time.");
        startHour = server.arg("startHour").toInt();
        startMinute = server.arg("startMinute").toInt();
        endHour = server.arg("endHour").toInt();
        endMinute = server.arg("endMinute").toInt();
        isTimed = true;
        settingsChanged = true;
        Serial.println("Set startHour: " + String(startHour));
        Serial.println("Set startMinute: " + String(startMinute));
        Serial.println("Set endHour: " + String(endHour));
        Serial.println("Set endMinute: " + String(endMinute));
        server.send(200, "text/html; charset=utf-8", "设置成功！");
    } else if (server.method() == HTTP_GET) {
        String html = "<html><head><meta charset=\"UTF-8\"></head><body>";
        html += "<h1>设置定时时间</h1>";
        html += "<form method='post' action='/setTime'>";
        html += "开启小时：<input type='number' name='startHour' min='0' max='23' value='0'><br>";
        html += "开启分钟：<input type='number' name='startMinute' min='0' max='59' value='0'><br>";
        html += "关闭小时：<input type='number' name='endHour' min='0' max='23' value='0'><br>";
        html += "关闭分钟：<input type='number' name='endMinute' min='0' max='59' value='0'><br>";
        html += "<input type='submit' value='设置定时'>";
        html += "</form>";
        html += "</body></html>";
        server.send(200, "text/html; charset=utf-8", html);
    }
}

void handleCancelTiming() {
    isTimed = false;
    server.send(200, "text/html; charset=utf-8", "定时已取消！");
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("2.4G", "8888888888");
    while (WiFi.status()!= WL_CONNECTED) {
        delay(1000);
        Serial.println("连接 WiFi 中...");
    }
    Serial.println("已连接到 WiFi");

    // 初始化输出引脚为低电平
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, LOW);

    // 设置 I2C 引脚
    Wire.begin(sdaPin, sclPin);

    // 初始化 OLED 屏幕
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 分配失败"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    // 初始化 NTP 客户端
    timeClient.begin();

    server.on("/", handleRoot);
    server.on("/setTime", handleSetTime);
    server.on("/cancelTiming", handleCancelTiming);
    server.begin();

    // 获取并保存 IP 地址
    IPAddress localIP = WiFi.localIP();
    ipAddressStr = String(localIP[0]) + "." +
                   String(localIP[1]) + "." +
                   String(localIP[2]) + "." +
                   String(localIP[3]);
}

void loop() {
    timeClient.update();

    // 获取时间戳并转换为 time_t 类型
    time_t epochTime = timeClient.getEpochTime();
    struct tm *timeInfo;
    timeInfo = localtime(&epochTime);

    // 获取日期和时间信息
    int currentHour = timeInfo->tm_hour;
    int currentMinute = timeInfo->tm_min;
    int currentSecond = timeInfo->tm_sec;
    int currentDay = timeInfo->tm_mday;
    int currentMonth = timeInfo->tm_mon;
    int currentYear = timeInfo->tm_year + 1900;

    // 如果设置了定时，判断是否在定时时间段内
    if (isTimed) {
        bool isInOnPeriod = false;
        // 修改后的时间判断条件
        if ((currentHour > startHour || (currentHour == startHour && currentMinute >= startMinute)) &&
            (currentHour < endHour || (currentHour == endHour && currentMinute < endMinute))) {
            isInOnPeriod = true;
        }

        // 根据状态设置输出引脚
        if (isInOnPeriod) {
            isSwitchOn = true;
            digitalWrite(outputPin, HIGH);
        } else {
            isSwitchOn = false;
            digitalWrite(outputPin, LOW);
            updateOLED("Off");
        }
    } else {
        // 未定时状态下，开关状态可以根据其他逻辑设置，或者保持默认状态
        isSwitchOn = false;
        digitalWrite(outputPin, LOW);
    }

    // 在 OLED 屏幕上显示日期、时间、开关状态和 IP 地址
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(String(currentYear) + "-" + String(currentMonth + 1) + "-" + String(currentDay));
    display.println(String(currentHour) + ":" + String(currentMinute) + ":" + String(currentSecond));
    int pinState = digitalRead(outputPin);
    String switchStatus = (pinState == HIGH? "On" : "Off");
    if (isTimed) {
        display.println("Switch Status: Timed - " + switchStatus);
    } else {
        display.println("Switch Status: Not Timed - " + switchStatus);
    }
    display.println("IP Address: " + ipAddressStr);
    display.display();

    server.handleClient();
    delay(1000);
}

void updateOLED(const char* status) {
    // 获取时间戳并转换为 time_t 类型
    time_t epochTime = timeClient.getEpochTime();
    struct tm *timeInfo;
    timeInfo = localtime(&epochTime);

    int currentYear = timeInfo->tm_year + 1900;
    int currentMonth = timeInfo->tm_mon + 1;
    int currentDay = timeInfo->tm_mday;
    int currentHour = timeInfo->tm_hour;
    int currentMinute = timeInfo->tm_min;
    int currentSecond = timeInfo->tm_sec;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(String(currentYear) + "-" + String(currentMonth) + "-" + String(currentDay));
    display.println(String(currentHour) + ":" + String(currentMinute) + ":" + String(currentSecond));
    display.println("Switch Status: " + String(status));
    display.println("IP Address: " + ipAddressStr);
    display.display();
}