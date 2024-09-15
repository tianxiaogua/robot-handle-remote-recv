#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "driver_gpio.h"
#include "driver_tool.h"

#include "pca9557.h"

#define USE_HORIZONTAL 2  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 135
#define LCD_H 240

#else
#define LCD_W 240
#define LCD_H 135
#endif

#include "driver_spi.h"
//-----------------LCD端口定义---------------- 

//#define LCD_RES_Clr()  GPIO_SetPin(LCD_SPI_REST, GPIO_LOW)//RES
//#define LCD_RES_Set()  GPIO_SetPin(LCD_SPI_REST,GPIO_HIGH)
#define LCD_RES_Clr()  set_pca9557_io_low(5) //RES
#define LCD_RES_Set()  set_pca9557_io_high(5)

#define LCD_DC_Clr()   GPIO_SetPin(LCD_SPI_DC,GPIO_LOW)//DC
#define LCD_DC_Set()   GPIO_SetPin(LCD_SPI_DC,GPIO_HIGH)
 		     
//#define LCD_CS_Clr()   GPIO_SetPin(LCD_SPI_CS,GPIO_LOW)//CS
//#define LCD_CS_Set()   GPIO_SetPin(LCD_SPI_CS,GPIO_HIGH)
#define LCD_CS_Clr()  set_pca9557_io_low(0)//CS
#define LCD_CS_Set()  set_pca9557_io_high(0)

//#define LCD_BLK_Clr()  GPIO_SetPin(LCD_SPI_BCK,GPIO_LOW)//BLK
//#define LCD_BLK_Set()  GPIO_SetPin(LCD_SPI_BCK,GPIO_HIGH)
#define LCD_BLK_Clr()  set_pca9557_io_low(4)//BLK
#define LCD_BLK_Set()  set_pca9557_io_high(4)


void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif




