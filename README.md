功能设计：

1、串口透传：开发模式遥控器发送给接收器的数据被串口转发给开发设备，开发设备发送回来的串口数据可以通过接收器透传给手柄显示并且可以通过USB输出到电脑上变成无线串口；

2、手柄功能，支持无线接收器和有线手柄；

3、屏幕显示串口，设备信息，设置界面，出场校准；

4、保留开发接口，可以使用USB直接做开发；

技术细节

1、具有所有的完善的Xbox手柄的摇杆，模拟肩键没有，改为按键肩键

2、屏幕显示：设置不同的功能输出

3、屏幕显示串口打印的数据，需要特殊的通信协议

4、遥控功能，可以通过一个无线接收器输出遥控数据

5、屏幕显示电量，充电状态，玩儿游戏的时间以及连接不同的接收器的功能

6、屏幕显示设置，摇杆校准，

7、一键打开手柄，不适用开关，使用按键触发的方式；

8、长时间不操作手柄，手柄可以自动关机；

9、蜂鸣器用于报警；



# 参考：

[ESP32-S3-PIE 基于ESP32-S3的卡片随身终端 - 嘉立创EDA开源硬件平台 (oshwhub.com)](https://oshwhub.com/fanhuacloud/esp32-s3-pie)

[ESP32系列--第九篇 ADC的使用_esp32 adc-CSDN博客](https://blog.csdn.net/tianizimark/article/details/125348749)

[ESP32设备SPI主设备驱动_spi_device_queue_trans-CSDN博客](https://blog.csdn.net/zhejfl/article/details/85999816)

[ESP32-S3 引脚参考大全 – 凌顺实验室 (lingshunlab.com)](https://lingshunlab.com/book/esp32/esp32-s3-pin-reference)

[ESP32-S3-DevKitC-1 v1.1 - ESP32-S3 - — ESP-IDF 编程指南 latest 文档 (espressif.com)](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)

[ESP32-S3 USB & UART 下载总结_esp32s3 uart0下载-CSDN博客](https://blog.csdn.net/Marchtwentytwo/article/details/121169467)

[免外围电路ESP32/ESP8266系列单片机串口一键下载方案 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/544744217)

[利用CH340C制作MicroPython ESP8266,ESP32的下载器-改进型_ch340c esp32 下载器-CSDN博客](https://blog.csdn.net/zhuoqingjoking97298/article/details/115557204)

[ESP32-S3核心开发板 wifi蓝牙 DevKitC-1 WROOM-1乐鑫N8R2 N16R8-淘宝网 (taobao.com)](https://item.taobao.com/item.htm?abbucket=2&id=718248966902&ns=1&spm=a21n57.1.item.10.1535523c99XQFt)

[ESP32-S3核心板_免费高速下载|百度网盘-分享无限制 (baidu.com)](https://pan.baidu.com/s/1i7EL0OSNwKmKmA-fVP_Vmg?pwd=xzzp#list/path=%2FESP32-S3核心板%2F原理图和封装)

**模组内部的原理图**：[esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf (espressif.com.cn)](https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf)

# 接口问题

注意：这45个物理GPIO管脚的编号为：0~21、26~48。这些管脚既可作为输入又可作为输出管脚。



| 接口   | 推荐 IO MUX 管脚                               | 通过 GPIO 交换矩阵连接 |
| :----- | :--------------------------------------------- | :--------------------- |
| SPI0/1 | 见表 2-3 IO MUX 和 GPIO 管脚功能 说明 5c、5d – | –                      |
| SPI2   | 见表 2-3 IO MUX 和 GPIO 管脚功能 说明 5e       | 任意 IO 管脚           |
| SPI3   |                                                | 任意 IO 管脚           |

下面是ESP32-S3 UART的默认引脚。

|            | UART0 | UART1         | UART2  |
| :--------- | :---- | :------------ | :----- |
| 发送（TX） | IO43  | IO17 / 任意IO | 任意IO |
| 接收（RX） | IO44  | IO18 / 任意IO | 任意IO |

I2C总线分软件和硬件对象，硬件可以定义0和1，通过配置可以在任意引脚上实现I2C功能，以下是I2S默认引脚表：

|      | I2C(0) | I2C(1) |
| :--- | :----- | :----- |
| SCL  | IO0    | IO2    |
| SDA  | IO1    | IO3    |

| 1    | GPIO   | USB                  |
| ---- | ------ | -------------------- |
| 2    | GPIO19 | 与 USB_D- 接口相连。 |
| 3    | GPIO20 | 与 USB_D+ 接口相连。 |

![img](https://pic2.zhimg.com/80/v2-5cc9f64edddb2bfa1b581bf45c6210d9_720w.webp)

![img](https://pic3.zhimg.com/80/v2-a7cfe3f33b9819bc87ea8feccdb5d4ba_720w.webp)

# SPI问题

GPIO矩阵和IO_MUX
ESP32的大多数外设信号都直接连接到其专用的IO_MUX引脚。但是，也可以使用GPIO矩阵将信号转换到任何其他可用的引脚。如果至少一个信号通过GPIO矩阵转换，则所有信号都将通过GPIO矩阵转换。

GPIO矩阵引入了转换灵活性，但也带来了以下缺点：

增加了MISO信号的输入延迟，这更可能违反MISO设置时间。如果SPI需要高速运行，请使用专用的IO_MUX引脚。
如果使用IO_MUX引脚，则允许信号的时钟频率最多为40 MHz，而时钟频率最高为80 MHz。
SPI总线的IO_MUX引脚如下所示

引脚对应的

| GPIO   | SPI2         | IOMUX   |
| ------ | ------------ | ------- |
| CS0 *  | PIN15/GPIO10 | FSPICS0 |
| SCLK   | PIN17/GPIO12 | FSPICLK |
| MISO   | PIN18/GPIO13 | FSPIQ   |
| MOSI   | PIN16/GPIO11 | FSPID   |
| QUADWP | 2            | 22      |
| QUADHD | 4            | 21      |

仅连接到总线的第一个设备可以使用CS0引脚。
原文链接：https://blog.csdn.net/qq_36347513/article/details/117299126

![img](https://img-blog.csdnimg.cn/1e49eb2f2c4c41359839a452fba273bb.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bKU6Leva28=,size_19,color_FFFFFF,t_70,g_se,x_16)



# ADC引脚

[ESP32系列--第九篇 ADC的使用_esp32 adc-CSDN博客](https://blog.csdn.net/tianizimark/article/details/125348749)

![1](./DOCUMENT/picture/1.png)

ADC模块 能读取电压的范围（量程）有限，因此我们一般给某个 ADC 通道配置一定的衰减，使其读取更大的电压。但是，更大的量程会导致更小的精度。因此根据 ADC 的应用场景，选择适当的衰减级别十分必要。

ESP32 的每一个通道都有提供了4个级别的衰减等级，不同的衰减等级对于的量程在下表列出：注意，下表中的 “推荐范围” 并不是量程 ，而是在某衰减等级下测量最精确的推荐测量范围
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/m0_50064262/article/details/118817032



IO拓展

[IO扩展芯片PCA9557-CSDN博客](https://blog.csdn.net/shynanjushi/article/details/118766628)

[pca9557.pdf (ti.com.cn)](https://www.ti.com.cn/cn/lit/ds/symlink/pca9557.pdf?ts=1718589725951&ref_url=https%3A%2F%2Fwww.ti.com.cn%2Fproduct%2Fcn%2FPCA9557)



