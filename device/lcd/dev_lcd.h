#ifndef  __DEV_LCD_H__
#define  __DEV_LCD_H__

#include "drv_config.h"

#define  DEVICE_TFT_WIGHT  240
#define  DEVICE_TFT_HIGHT  240

//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 	 0x001F  
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x75C7
#define YELLOW        	 0xFFE0
#define BROWN 			     0XA145 //��ɫ
#define BRRED 			     0XFC07 //�غ�ɫ
#define GRAY  			     0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7CFC	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X7CFC //ǳ��ɫ
#define LGRAY 			     0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)


#define  DEV_LCD_DEFAULT_ASCII_FONT  {' ','.','/','0','1','2','3','4','5','6','7','8','9',':','A','V','m'}


enum
{
	ASCII_FONT_TYPE_1608 = 1 ,
	ASCII_FONT_TYPE_2412 ,
	ASCII_FONT_TYPE_3216 ,
	ASCII_FONT_TYPE_6432 
};

#define DEV_TFT_SCLK_CLR() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET)//CLK
#define DEV_TFT_SCLK_SET() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET)

#define DEV_TFT_SDIN_CLR() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET)//DIN
#define DEV_TFT_SDIN_SET() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET)

#define DEV_TFT_RST_CLR()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET)//RES
#define DEV_TFT_RST_SET()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET)

#define DEV_TFT_DC_CLR()   HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET)//DC
#define DEV_TFT_DC_SET()   HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET)
 		     
#define DEV_TFT_BLK_CLR()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET)//BLK
#define DEV_TFT_BLK_SET()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET)

void dev_lcd_hardware_init(void);

void dev_lcd_draw_window(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);

void dev_lcd_draw_point(uint16_t px,uint16_t py,uint16_t color);

void dev_lcd_draw_line(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);

void dev_lcd_draw_char(uint16_t px,uint16_t py,char chr,uint8_t type,uint16_t bkcolor,uint16_t frcolor);

void dev_lcd_draw_string(uint16_t px,uint16_t py,char *string,uint8_t type,uint16_t bkcolor,uint16_t frcolor);

#endif

