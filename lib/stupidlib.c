#include "stupidlib.h"


void int_to_string(char * buffer, int16_t value){
    if (i16_sign(value) == -1){
        buffer[0] = '-';
    }else{
        buffer[0] = '+';
    }
    value = i16_abs(value);
    buffer[1] = 48 + (value / 100 % 10);
    buffer[2] = 48 + (value / 10 % 10);
    buffer[3] = 48 + (value % 10);
    buffer[4] = 0;
}

int16_t clamp(int16_t value, int16_t min, int16_t max){
    if (value < min){
        return min;
    }else if (value > max){
        return max;
    }
    return value;
}

int16_t i16_sign(int16_t value){
    if (value == 0){
        return 0;
    }else if (value < 0){
        return -1;
    }
    return 1;
}
int16_t i16_abs(int16_t value){
    if (i16_sign(value) == -1){
        return -1 * value;
    }
    return value;
}