#include <avr/io.h>

int wireless_initialize();

char receiver_array[255];
uint8_t receiver_array_length;
uint8_t ready_receiver;

char * transmitting_array;
uint8_t transmitting_array_length;

uint8_t transmitting_selected_byte;
uint8_t transmitting_selected_bit;
uint8_t transmitting_selected_bit_fractional;

void transmit_byte_array(char * byte_array, uint8_t byte_array_length);