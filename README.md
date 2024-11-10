# esp8266shizhong
天气时钟
ESP8266 开发板网络时钟和天气 OLED 显示
oled-ssd1315

更新 低电平自锁模式，输出高电平，
esp8266高电平自锁模式，只能gpio16输出才能实现高电平自锁模式

定时开关控制增加了IP显示功能，实现更好的查找地址




显示log图像

delay(2000); // 暂停2秒


  display.display();
  delay(2000); // 暂停2秒
  display.clearDisplay();


          display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Initializing...");
        display.display();
        delay(2000); // 暂停2秒
        display.clearDisplay();
