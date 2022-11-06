#define  LOG_TAG   "UI"
#include "dev_lcd.h"
#include <elog.h>
#include "FreeRTOS.h"
#include "task.h"
#include <FreeRTOSConfig.h>

static TaskHandle_t xHandleTaskUi = NULL;

#define TASK_UI_STATIC_STACK_SIZE    1024


static void ui_main_page_display(void)
{
	dev_lcd_draw_window(0,0,240,94,BLACK);
	dev_lcd_draw_window(0,94,240,111,WHITE);
	for(uint8_t i=0;i<5;i++)
	{
		if(i==0)
		{
			for(uint8_t j=0;j<24;j++)
			{
				dev_lcd_draw_point(0,4*j,CYAN);
			}
			dev_lcd_draw_char(0,95,'0',ASCII_FONT_TYPE_1608,WHITE,BLACK);
		}
		else
		{
			if(i==4)
			{
				dev_lcd_draw_char(i*60-8,95,'0'+i,ASCII_FONT_TYPE_1608,WHITE,BLACK);
			}
			else
			{
				dev_lcd_draw_char(i*60-4,95,'0'+i,ASCII_FONT_TYPE_1608,WHITE,BLACK);
			}
			for(uint8_t j=0;j<24;j++)
			{
				dev_lcd_draw_point(i*60-1,4*j,CYAN);
			}
		}
	}
	dev_lcd_draw_line(0,94,240,94,GREEN);
	for(uint8_t i=0;i<60;i++)
	{
		dev_lcd_draw_point(4*i,0,CYAN);
		dev_lcd_draw_point(4*i,23,CYAN);
		dev_lcd_draw_point(4*i,46,CYAN);
		dev_lcd_draw_point(4*i,70,CYAN);
	}
	dev_lcd_draw_string(0,111,"A:",ASCII_FONT_TYPE_6432,BLUE,RED);
	dev_lcd_draw_string(64,111,"    ",ASCII_FONT_TYPE_6432,LGRAY,RED);
	dev_lcd_draw_char(192,138,'m',ASCII_FONT_TYPE_3216,LGRAY,BLUE);
	dev_lcd_draw_char(208,111,'A',ASCII_FONT_TYPE_6432,LGRAY,BLACK);
	dev_lcd_draw_window(192,111,208,111+32,LGRAY);


	dev_lcd_draw_line(0,175,240,175,BLACK);
	dev_lcd_draw_string(0,176,"V:",ASCII_FONT_TYPE_6432,BLUE,RED);
	dev_lcd_draw_string(64,176,"    ",ASCII_FONT_TYPE_6432,LGRAY,RED);
	dev_lcd_draw_window(192,176,208,176+68,LGRAY);
	dev_lcd_draw_char(208,176,'V',ASCII_FONT_TYPE_6432,LGRAY,BLACK);
}





static void vTaskUI_Handler(void* pvParameters)
{
	dev_lcd_hardware_init();
	ui_main_page_display();
	while(1)
	{
	  elog_i("tag","vTaskLog");
	  vTaskDelay(500);
	}
}



void ui_init(void)
{
	BaseType_t CreatRes = xTaskCreate( vTaskUI_Handler, "vTaskShow",TASK_UI_STATIC_STACK_SIZE,NULL,2,&xHandleTaskUi ); 
	if(CreatRes != pdTRUE)
	{
		log_e("UI task create fail");
		return;
	}
}

