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



在您将ESP8266配置好并上传了上述代码之后，您可以通过手机浏览器输入ESP8266的IP地址来访问控制页面。这个IP地址是ESP8266连接到您的WiFi网络后获得的，可以在串口监视器中看到，或者通过其他网络设备查看。
一旦您知道了ESP8266的IP地址，比如是192.168.1.100，那么您可以在手机浏览器中输入以下网址：
http://192.168.1.100/

这将打开一个简单的网页，上面有四个链接，分别用于控制两个继电器的开关状态：
•  打开继电器1（四路）：http://192.168.1.100/relay1?state=1
•  关闭继电器1（四路）：http://192.168.1.100/relay1?state=0
•  打开继电器2（五路）：http://192.168.1.100/relay2?state=1
•  关闭继电器2（五路）：http://192.168.1.100/relay2?state=0
您只需在浏览器中访问这些网址，就可以远程控制继电器的开关状态，并且这些状态会实时显示在连接到ESP8266的OLED屏幕上。
