#include "dev_console.h"
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
      rt_kprintf("Rtos tick index :%d\n",index++);
      vTaskDelay(1000);
    }
}

static void sys_init_thread(void* pvParameters){

		while(1)
    {
      elog_a("elog","Hello EasyLogger!");
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
	
	
	xTaskCreate( sys_init_thread,    		      /*   */
                 "vTaskLog",  		    /*     */
                 512,         		    /* word4 */
                 NULL,        		    /*   */
                 3,           		    /* */
                 &xHandleTaskLog );  /*   */
	
	
}




int main(void)
{
    board_init();
	  cm_backtrace_init("project", HARDWARE_VERSION, SOFTWARE_VERSION);
	    /* initialize EasyFlash and EasyLogger */
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
    AppTaskCreate();
    vTaskStartScheduler();
    while(1)
    {
		
    }

}

