#include <avr/io.h>

void boot_initialize();
void text_input();
void perform_message_transmission(char * input, uint8_t success);