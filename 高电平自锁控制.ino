const int inputPin = 16;//高电平只有16脚能用
const int outputPin = 2;

bool isLocked = false;

void setup() {
  pinMode(inputPin, INPUT_PULLDOWN_16);  // 将输入引脚设置为下拉模式.高电平只有16脚能用
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);  // 初始输出低电平
}

void loop() {
  int inputValue = digitalRead(inputPin);
  if (inputValue == HIGH) {  // 检测到高电平输入
    if (!isLocked) {
      digitalWrite(outputPin, HIGH);
      isLocked = true;
    } else {
      digitalWrite(outputPin, LOW);
      isLocked = false;
    }
    while (digitalRead(inputPin) == HIGH);  // 等待输入引脚恢复低电平
  }
}