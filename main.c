#include <avr/io.h>

#include "lib/multi.h"
#include "lib/walkos.h"
#include "lib/wireless.h"
#include "lib/lcddriver.h"

#define delay _delay_ms


int main(){
	wireless_initialize();
	lcd_initialize();
	boot_initialize();

	run_mainthread();
}