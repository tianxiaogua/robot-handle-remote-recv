#include <stdio.h>
#include "main_app.h"


void app_main(void)
{
	init_app();

	start_app();

	while(1)
	{
		delay_ms(2000);
	}
}
