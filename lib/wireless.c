/**
 * @file wireless.c
 * @author JOSHUA OUNALOM
 * @brief The communication core of the Walkie Textie.
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * The Driver for handling 315MHz communication using the on board transmitter and receiver. There is no external hardware and it all was done lazily by the CPU. Not ideal.
 * The protocol designed and developed features a PWM signal generator and a decode to send messages in a low level fashion through the air.
 * 
 * Dependant on Parallism System to perform tasks in the background when the microchip does something else.
 */

#include "multi.h"


#include <string.h>
#include <avr/io.h>

#define PINGROUP PORTC

#define TRANSMITTER_PIN 1
#define RECEIVER_PIN 0
#define TEST_PIN 4


#if 1
    char * test_string = "Lorem Ipsum!\0";
#else
    char * test_string = "\0";
#endif

char transmitting_buffer[256];

char * transmitting_array;
uint8_t transmitting_array_length = 0;

uint8_t transmitting_selected_byte = 0;
uint8_t transmitting_selected_bit = 0;
uint8_t transmitting_selected_bit_fractional = 0;

void transmitter_clock(TASK_FUNCTION_PARAMETERS){
    char transmitting_byte = 0;
    if (transmitting_selected_byte >= transmitting_array_length + 1){
        return;
    }
    if (transmitting_selected_byte != 0){
        transmitting_byte = *(transmitting_array + transmitting_selected_byte - 1);
    }

    uint8_t bit = transmitting_byte >> (transmitting_selected_bit);


    uint8_t switch_point = 1;

    if (transmitting_selected_byte == 0){
        switch_point = 7;
    }else{
        if ((bit & 1)){
            switch_point = 6;
        }else{
            switch_point = 1;
        }
    }

    if (transmitting_selected_bit_fractional < switch_point){
        PINGROUP.OUT |= (uint8_t)(1 << TRANSMITTER_PIN);
    }else{
        PINGROUP.OUT &= ~(uint8_t)(1 << TRANSMITTER_PIN);
    }

    transmitting_selected_bit_fractional += 1;

    if (transmitting_selected_bit_fractional >= 8){
        transmitting_selected_bit_fractional = 0;

        transmitting_selected_bit += 1;
    }
    if (transmitting_selected_bit >= 8){
        transmitting_selected_bit = 0;
        transmitting_selected_byte += 1;
    }
}


char receiver_array[255];
uint8_t receiver_array_length = 0;

uint8_t ready_receiver = 0;

uint8_t duty_cycle_low = 0;
uint8_t duty_cycle_high = 0;
uint8_t duty_cycle_mode = 0;

uint8_t read_byte = 0;

uint8_t receiver_test_bit = 0;

uint8_t active_receiver = 0;
uint8_t receiver_byte = 0;
uint8_t received_bit = 0;

uint16_t counter_shit = 0;

void receiver_clock(TASK_FUNCTION_PARAMETERS){
    uint8_t input_signal = (PINGROUP.IN >> RECEIVER_PIN & 1) == 1;
    // if (counter_shit < 255 * 10){
    //     counter_shit ++;
    //     return;
    // }

    if (input_signal){
        if (duty_cycle_mode == 0){
            duty_cycle_mode = 1;

            uint8_t duty_cycle_payload = 256 * duty_cycle_high / (duty_cycle_high + duty_cycle_low);

            if (duty_cycle_payload){
                // strcpy(receiver_array, "good!  ");
                // receiver_array[6] = 49 + duty_cycle_payload;
                // receiver_array_length = 7;

            }

            if (ready_receiver == 0){
                if (duty_cycle_payload > 224 - 10 && duty_cycle_payload < 224 + 10){
                    receiver_test_bit += 1;

                    if (receiver_test_bit == 6){
                        for (int i = 0;i<255;i++){
                            receiver_array[i] = 0;
                        }
                        receiver_array_length = 0;
                        ready_receiver = 1;
                        receiver_test_bit = 0;
                    }
                    
                }else{
                    receiver_test_bit = 0;
                }
            }else{
                if (duty_cycle_payload < 200){
                    active_receiver = 1;
                }
                if (active_receiver){
                    uint8_t logical_bit_high = duty_cycle_payload >= 128;
                    receiver_byte = ((receiver_byte >> 1) & 0b01111111) | (logical_bit_high << 7);
                    received_bit += 1;

                    if (received_bit >= 8){
                        receiver_array[receiver_array_length ++] = receiver_byte;
                        if (receiver_byte == 0){
                            active_receiver = 0;
                            ready_receiver = 0;
                            counter_shit = 0;
                        }
                        receiver_byte = 0;
                        received_bit = 0;
                        active_receiver = 0;

                    }
                }
            }
            
            duty_cycle_high = 0;
            duty_cycle_low = 0;
        }
        duty_cycle_high += 1;
    }else{
        duty_cycle_mode = 0;
        duty_cycle_low += 1;
    }

    // if (strcmp(receiver_array, test_string) == 0 && ready_receiver == 0){
    // // if (input_bit){
    //     PINGROUP.OUT |= (uint8_t)(1 << TEST_PIN);
    // }else{
    //     PINGROUP.OUT &= ~(uint8_t)(1 << TEST_PIN);
    // }
}

void transmit_byte_array(char * byte_array, uint8_t byte_array_length){
    strcpy(transmitting_buffer, byte_array);
    transmitting_array = transmitting_buffer;
    transmitting_array_length = byte_array_length + 1;

    transmitting_selected_bit = 0;
    transmitting_selected_byte = 0;
    transmitting_selected_bit_fractional = 0;
}

void reset_clock(TASK_FUNCTION_PARAMETERS){
    transmitting_selected_bit = 0;
    transmitting_selected_byte = 0;
    transmitting_selected_bit_fractional = 0;
}

struct ScheduledTask transmissionClockTask;
struct ScheduledTask receiverClockTask;
struct ScheduledTask resetClockTask;


void wireless_initialize(){
    PINGROUP.DIRCLR = 1 << RECEIVER_PIN;
    PINGROUP.DIRSET |= 1 << TRANSMITTER_PIN;
    PINGROUP.DIRSET |= 1 << TEST_PIN;
    PINGROUP.OUT = 0b00000000;

    create_task(&transmissionClockTask,  transmitter_clock, 0, 0);
    create_task(&receiverClockTask, receiver_clock, 0, 0);
    create_task(&resetClockTask, reset_clock, 0, 0);

    receiverClockTask.delta.microseconds = 30;
    receiverClockTask.interval.microseconds = 30;
    transmissionClockTask.interval.microseconds = 800;


    transmitting_array = test_string;
    transmitting_array_length = 13;
    transmitting_selected_bit = 0;
    transmitting_selected_byte = 0;

    add_task(&transmissionClockTask);
    add_task(&receiverClockTask);
    // add_task(&resetClockTask);
}