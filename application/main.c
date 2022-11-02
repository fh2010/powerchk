#include "dev_console.h"
#include "board.h"

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

static void vTaskLog(void *pvParameters)
{
    while(1)
    {
      rt_kprintf("TaskLog\n");
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
    //AppTaskCreate();
    vTaskStartScheduler();
    while(1)
    {
		
    }

}

