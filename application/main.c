#include "dev_console.h"
#include "board.h"

int main(void)
{
	board_init();
	dev_console_set_device();
	while(1)
	{
		
	}

}

