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
const int outputPinForGroup1 = 0;
const int outputPinForGroup2 = 2;

// 定义 OLED 的 I2C 引脚
const int sclPin = 1;
const int sdaPin = 3;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

ESP8266WebServer server(80);

// 标志位，用于判断是否设置了定时
bool isTimed = false;

// 标志位，用于判断是否需要更新开关状态
bool settingsChanged = false;
bool isSwitchOnForGroup1 = false;
bool isSwitchOnForGroup2 = false;

// 声明并初始化 lastCheckTime
unsigned long lastCheckTime = 0;

// 存储定时的开始小时和分钟
int startHourForGroup1 = 0;
int startMinuteForGroup1 = 0;
// 存储定时的结束小时和分钟
int endHourForGroup1 = 0;
int endMinuteForGroup1 = 0;
// 存储第二个定时的开始小时和分钟
int startHourForGroup2 = 0;
int startMinuteForGroup2 = 0;
// 存储第二个定时的结束小时和分钟
int endHourForGroup2 = 0;
int endMinuteForGroup2 = 0;

// 存储 IP 地址的字符串
String ipAddressStr;

// 用于滚动的字符串
String scrollText = "You have more beauty to achieve. yaoyang production";
int scrollPosition = 0;

// 用于存储定时显示字符串，以滚动形式展示
String timingDisplay = "";

// 定义一个结构体来存储时间信息
struct TimeInfo {
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
};

void handleRoot();
void handleSetTime();
void handleCancelTiming();
void updateOLED();

// 获取当前时间并返回结构体的函数
TimeInfo getCurrentTime() {
    time_t epochTime = timeClient.getEpochTime();
    struct tm *timeInfo;
    timeInfo = localtime(&epochTime);
    TimeInfo currentTime;
    currentTime.hour = timeInfo->tm_hour;
    currentTime.minute = timeInfo->tm_min;
    currentTime.second = timeInfo->tm_sec;
    currentTime.day = timeInfo->tm_mday;
    currentTime.month = timeInfo->tm_mon;
    currentTime.year = timeInfo->tm_year + 1900;
    return currentTime;
}

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
        startHourForGroup1 = server.arg("startHourForGroup1").toInt();
        startMinuteForGroup1 = server.arg("startMinuteForGroup1").toInt();
        endHourForGroup1 = server.arg("endHourForGroup1").toInt();
        endMinuteForGroup1 = server.arg("endMinuteForGroup1").toInt();
        startHourForGroup2 = server.arg("startHourForGroup2").toInt();
        startMinuteForGroup2 = server.arg("startMinuteForGroup2").toInt();
        endHourForGroup2 = server.arg("endHourForGroup2").toInt();
        endMinuteForGroup2 = server.arg("endMinuteForGroup2").toInt();
        isTimed = true;
        settingsChanged = true;
        Serial.println("Set startHourForGroup1: " + String(startHourForGroup1));
        Serial.println("Set startMinuteForGroup1: " + String(startMinuteForGroup1));
        Serial.println("Set endHourForGroup1: " + String(endHourForGroup1));
        Serial.println("Set endMinuteForGroup1: " + String(endMinuteForGroup1));
        Serial.println("Set startHourForGroup2: " + String(startHourForGroup2));
        Serial.println("Set startMinuteForGroup2: " + String(startMinuteForGroup2));
        Serial.println("Set endHourForGroup2: " + String(endHourForGroup2));
        Serial.println("Set endMinuteForGroup2: " + String(endMinuteForGroup2));
        // 更新定时显示字符串
        timingDisplay = String(startHourForGroup1) + ":" + String(startMinuteForGroup1) + " - " + String(endHourForGroup1) + ":" + String(endMinuteForGroup1) +
                        "  |  " + String(startHourForGroup2) + ":" + String(startMinuteForGroup2) + " - " + String(endHourForGroup2) + ":" + String(endMinuteForGroup2);
        server.send(200, "text/html; charset=utf-8", "设置成功！");
    } else if (server.method() == HTTP_GET) {
        String html = "<html><head><meta charset=\"UTF-8\"></head><body>";
        html += "<h1>设置定时时间</h1>";
        html += "<form method='post' action='/setTime'>";
        html += "第一组开启小时：<input type='number' name='startHourForGroup1' min='0' max='23' value='0'><br>";
        html += "第一组开启分钟：<input type='number' name='startMinuteForGroup1' min='0' max='59' value='0'><br>";
        html += "第一组关闭小时：<input type='number' name='endHourForGroup1' min='0' max='23' value='0'><br>";
        html += "第一组关闭分钟：<input type='number' name='endMinuteForGroup1' min='0' max='59' value='0'><br>";
        html += "第二组开启小时：<input type='number' name='startHourForGroup2' min='0' max='23' value='0'><br>";
        html += "第二组开启分钟：<input type='number' name='startMinuteForGroup2' min='0' max='59' value='0'><br>";
        html += "第二组关闭小时：<input type='number' name='endHourForGroup2' min='0' max='23' value='0'><br>";
        html += "第二组关闭分钟：<input type='number' name='endMinuteForGroup2' min='0' max='59' value='0'><br>";
        html += "<input type='submit' value='设置定时'>";
        html += "</form>";
        html += "</body></html>";
        server.send(200, "text/html; charset=utf-8", html);
    }
}

void handleCancelTiming() {
    isTimed = false;
    timingDisplay = "";
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
    pinMode(outputPinForGroup1, OUTPUT);
    digitalWrite(outputPinForGroup1, LOW);
    pinMode(outputPinForGroup2, OUTPUT);
    digitalWrite(outputPinForGroup2, LOW);

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
    ipAddressStr = "IP:" + String(localIP[0]) + "." +
                   String(localIP[1]) + "." +
                   String(localIP[2]) + "." +
                   String(localIP[3]);
}

// 判断是否在定时时间段内，考虑跨越到第二天的情况
bool isInOnPeriod(int startHour, int startMinute, int endHour, int endMinute) {
    TimeInfo currentTime = getCurrentTime();
    int currentTotalMinutes = currentTime.hour * 60 + currentTime.minute;
    int startTotalMinutes = startHour * 60 + startMinute;
    int endTotalMinutes = endHour * 60 + endMinute;
    if (endTotalMinutes < startTotalMinutes) {
        // 跨越到第二天的情况
        return currentTotalMinutes >= startTotalMinutes || currentTotalMinutes < endTotalMinutes;
    } else {
        // 同一天的情况
        return currentTotalMinutes >= startTotalMinutes && currentTotalMinutes < endTotalMinutes;
    }
}

void loop() {
    timeClient.update();
    TimeInfo currentTime = getCurrentTime();

    // 如果设置了定时，判断是否在定时时间段内
    if (isTimed) {
        bool isInOnPeriodForGroup1 = isInOnPeriod(startHourForGroup1, startMinuteForGroup1, endHourForGroup1, endMinuteForGroup1);
        bool isInOnPeriodForGroup2 = isInOnPeriod(startHourForGroup2, startMinuteForGroup2, endHourForGroup2, endMinuteForGroup2);
        // 根据状态设置输出引脚
        if (isInOnPeriodForGroup1) {
            isSwitchOnForGroup1 = true;
            digitalWrite(outputPinForGroup1, HIGH);
            digitalWrite(outputPinForGroup2, LOW);
        } else if (isInOnPeriodForGroup2) {
            isSwitchOnForGroup2 = true;
            digitalWrite(outputPinForGroup1, LOW);
            digitalWrite(outputPinForGroup2, HIGH);
        } else {
            isSwitchOnForGroup1 = false;
            isSwitchOnForGroup2 = false;
            digitalWrite(outputPinForGroup1, LOW);
            digitalWrite(outputPinForGroup2, LOW);
        }
    } else {
        // 未定时状态下，开关状态可以根据其他逻辑设置，或者保持默认状态
        isSwitchOnForGroup1 = false;
        isSwitchOnForGroup2 = false;
        digitalWrite(outputPinForGroup1, LOW);
        digitalWrite(outputPinForGroup2, LOW);
    }

    // 在 OLED 屏幕上显示日期、时间、IP 地址和特定字符串
    display.clearDisplay();
    // 固定显示日期时间在最上方
    display.setCursor(0, 0);
    display.println(String(currentTime.year) + "-" + String(currentTime.month + 1) + "-" + String(currentTime.day) + " " + String(currentTime.hour) + ":" + String(currentTime.minute) + ":" + String(currentTime.second));

    // 检查是否需要更新 IP 地址显示
    unsigned long currentTimeMillis = millis();
    if (currentTimeMillis - lastCheckTime >= 60000) {
        IPAddress localIP = WiFi.localIP();
        ipAddressStr = "IP:" + String(localIP[0]) + "." +
                       String(localIP[1]) + "." +
                       String(localIP[2]) + "." +
                       String(localIP[3]);
        lastCheckTime = currentTimeMillis;
    }

    // 固定显示 IP 地址
    display.setCursor(0, 10);
    display.println(ipAddressStr);

    // 显示开关状态在正数第三行和第四行
    display.setCursor(0, 20);
    display.print(isSwitchOnForGroup1? "timing 1: ON  " : "timing 1: OFF ");
    display.setCursor(0, 30);
    display.print(isSwitchOnForGroup2? "timing 2: ON" : "timing 2: OFF");

    // 根据定时状态显示定时时间或固定的提示信息
    if (isTimed) {
        // 倒数第四个显示第一组定时时间
        display.setCursor(0, SCREEN_HEIGHT - 24);
        display.println(String(startHourForGroup1) + ":" + String(startMinuteForGroup1) + " - " + String(endHourForGroup1) + ":" + String(endMinuteForGroup1));

        // 倒数第三个显示第二组定时时间
        display.setCursor(0, SCREEN_HEIGHT - 16);
        display.println(String(startHourForGroup2) + ":" + String(startMinuteForGroup2) + " - " + String(endHourForGroup2) + ":" + String(endMinuteForGroup2));
    } else {
        // 倒数第四行固定显示提示信息
        display.setCursor(0, SCREEN_HEIGHT - 24);
        display.println("Timer switch V2.3.3");
    }

    // 倒数第二行显示滚动的底部文本
    display.setCursor(0, SCREEN_HEIGHT - 8);
    display.print(scrollText.substring(scrollPosition));
    scrollPosition++;
    if (scrollPosition >= scrollText.length()) {
        scrollPosition = 0;
    }

    display.display();

    server.handleClient();
    delay(1000);
}

void updateOLED() {
    extern bool isInOnPeriodForGroup1;
    extern bool isInOnPeriodForGroup2;

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
    // 固定显示日期时间在最上方
    display.setCursor(0, 0);
    display.println(String(currentYear) + "-" + String(currentMonth) + "-" + String(currentDay) + " " + String(currentHour) + ":" + String(currentMinute) + ":" + String(currentSecond));

       // 检查是否需要更新 IP 地址显示
    unsigned long currentTimeMillis = millis();
    if (currentTimeMillis - lastCheckTime >= 60000) {
        IPAddress localIP = WiFi.localIP();
        ipAddressStr = "IP:" + String(localIP[0]) + "." +
                       String(localIP[1]) + "." +
                       String(localIP[2]) + "." +
                       String(localIP[3]);
        lastCheckTime = currentTimeMillis;
    }

    // 固定显示 IP 地址
    display.setCursor(0, 10);
    display.println(ipAddressStr);

    // 显示开关状态在正数第三行和第四行
    display.setCursor(0, 20);
    display.print(isSwitchOnForGroup1? "timing 1: ON  " : "timing 1: OFF ");
    display.setCursor(0, 30);
    display.print(isSwitchOnForGroup2? "timing 2: ON" : "timing 2: OFF");

// 根据定时状态显示定时时间或固定的提示信息
if (isTimed) {
    // 倒数第四个显示第一组定时时间
    display.setCursor(0, SCREEN_HEIGHT - 24);
    display.println(String(startHourForGroup1) + ":" + String(startMinuteForGroup1) + " - " + String(endHourForGroup1) + ":" + String(endMinuteForGroup1));

    // 倒数第三个显示第二组定时时间
    display.setCursor(0, SCREEN_HEIGHT - 16);
    display.println(String(startHourForGroup2) + ":" + String(startMinuteForGroup2) + " - " + String(endHourForGroup2) + ":" + String(endMinuteForGroup2));

    // 显示当前开关状态
    display.setCursor(0, SCREEN_HEIGHT - 12);
    if (isSwitchOnForGroup1 || isSwitchOnForGroup2) {
        display.println("Switch Status: ON");
    } else {
        display.println("Switch Status: OFF");
    }

    // 计算并显示距离下一次定时切换的时间
    TimeInfo currentTime = getCurrentTime();
    int currentTotalMinutes = currentTime.hour * 60 + currentTime.minute;
    int nextSwitchMinutes = 0;
    bool isGroup1Next = false;
    bool isGroup2Next = false;

    if (isInOnPeriodForGroup1) {
        int endTotalMinutesGroup1 = endHourForGroup1 * 60 + endMinuteForGroup1;
        if (currentTotalMinutes < endTotalMinutesGroup1) {
            nextSwitchMinutes = endTotalMinutesGroup1 - currentTotalMinutes;
            isGroup1Next = true;
        }
    } else if (isInOnPeriodForGroup2) {
        int endTotalMinutesGroup2 = endHourForGroup2 * 60 + endMinuteForGroup2;
        if (currentTotalMinutes < endTotalMinutesGroup2) {
            nextSwitchMinutes = endTotalMinutesGroup2 - currentTotalMinutes;
            isGroup2Next = true;
        }
    } else {
        int startTotalMinutesGroup1 = startHourForGroup1 * 60 + startMinuteForGroup1;
        int startTotalMinutesGroup2 = startHourForGroup2 * 60 + startMinuteForGroup2;
        if (currentTotalMinutes < startTotalMinutesGroup1 && currentTotalMinutes < startTotalMinutesGroup2) {
            if (startTotalMinutesGroup1 < startTotalMinutesGroup2) {
                nextSwitchMinutes = startTotalMinutesGroup1 - currentTotalMinutes;
                isGroup1Next = true;
            } else {
                nextSwitchMinutes = startTotalMinutesGroup2 - currentTotalMinutes;
                isGroup2Next = true;
            }
        }
    }

    int hours = nextSwitchMinutes / 60;
    int minutes = nextSwitchMinutes % 60;
    display.setCursor(0, SCREEN_HEIGHT - 8);
    if (nextSwitchMinutes > 0) {
        display.print("Next Switch in: " + String(hours) + "h " + String(minutes) + "m");
    } else {
        display.print("Switching soon...");
    }
} else {
    // 倒数第四行固定显示提示信息
    display.setCursor(0, SCREEN_HEIGHT - 24);
    display.println("Timer switch V2.3.3");
}
// 倒数第二行显示滚动的底部文本
display.setCursor (0, SCREEN_HEIGHT - 8);
display.print (scrollText.substring (scrollPosition));
scrollPosition++;
if (scrollPosition >= scrollText.length ()) {
scrollPosition = 0;
}
display.display();
}