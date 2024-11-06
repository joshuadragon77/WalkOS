#include <avr/io.h>

int16_t clamp(int16_t value, int16_t min, int16_t max);
int16_t i16_sign(int16_t value);
void int_to_string(char * string_buffer, int16_t value);
int16_t i16_abs(int16_t value);