#define  LOG_TAG   "main"
#include "dev_console.h"
#include "dev_lcd.h"
#include "board.h"
#include <elog.h>
#include <cm_backtrace.h>

#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.1.0"


static TaskHandle_t xHandleTaskShow = NULL;
static TaskHandle_t xHandleTaskLog = NULL;

static void vTaskShowRTOS(void *pvParameters)
{
    uint32_t index = 0;
    while(1)
    {
      log_w("Rtos tick index :%d",index++);
      vTaskDelay(1000);
    }
}

static void vTaskLog(void* pvParameters){

		uint16_t index = 0;
		char show[6];
		sprintf(show,"V:%04d",index);
		dev_lcd_hardware_init();
		dev_lcd_draw_string(10,10,"123456AVm",ASCII_FONT_TYPE_1608,WHITE,RED);
		dev_lcd_draw_string(10,40,"123456AVm",ASCII_FONT_TYPE_2412,WHITE,RED);
		dev_lcd_draw_string(0,112,"A:1234",ASCII_FONT_TYPE_6432,WHITE,RED);
		dev_lcd_draw_string(0,176,show,ASCII_FONT_TYPE_6432,WHITE,RED);
		while(1)
    {
      elog_i("tag","vTaskLog");
			dev_lcd_draw_string(0,176,show,ASCII_FONT_TYPE_6432,WHITE,RED);
			sprintf(show,"V:%04d",index++);
      vTaskDelay(500);
    }
}

static void AppTaskCreate (void)
{

    xTaskCreate( vTaskShowRTOS,   	      /*   */
                 "vTaskShow",     	  /*     */
                 512,               	/* word4 */
                 NULL,              	/*   */
                 2,                 	/* */
                 &xHandleTaskShow );  /*   */
	
	
	xTaskCreate( vTaskLog,    		      /*   */
                 "vTaskLog",  		    /*     */
                 512,         		    /* word4 */
                 NULL,        		    /*   */
                 3,           		    /* */
                 &xHandleTaskLog );  /*   */
	
	
}




int main(void)
{
    board_init();
	  //cm_backtrace_init("project", HARDWARE_VERSION, SOFTWARE_VERSION);
    if ((elog_init() == ELOG_NO_ERR)) {
        /* set enabled format */
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
        /* set EasyLogger assert hook */
        //elog_assert_set_hook(elog_user_assert_hook);
        /* start EasyLogger */
        elog_start();
        /* set hardware exception hook */
        //rt_hw_exception_install(exception_hook);
        /* set RT-Thread assert hook */
        //rt_assert_set_hook(rtt_user_assert_hook);

    }
	ui_init();
    //AppTaskCreate();
    vTaskStartScheduler();
    while(1)
    {
		
    }

}

