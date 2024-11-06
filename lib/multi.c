/**
 * @file multi.c
 * @author JOSHUA OUNALOM
 * @brief Parallism System for TEAM 4B of the ENEL 300 Course.
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * Allows high accuracy preciseness of multi-task execution which is dependant on a 16 MHz
 * CPU clock and Type A Counter of the AVR128DB28.
 * 
 * Benchmarks:
 * Can sustainbly reach a execution speed of 38.432 kHZ or 26 µs of execution for only one legacy task.
 *      This interval is taken up by the background task of multi.c to perform other system functions.
 *      Other system functions such as
 *          Real Time Clock
 *          Task Counter
 * 
 * 
 * State Systems:
 *      There are two type of tasks the Parallism System can run.
 *          Legacy  = Infinite Execution (The simpliest and fastest)
 *          State   = Customizable Execution (Callbacks, Asynchronous Execution)
 *       
 *      The State System is a very versatile tool that is flexible to the needs of a task.
 *          It can perform callbacks on a completion of task.
 *          Nearly mimic the operation of normal code.
 *          Can disable or enable task
 */

#include "./multi.h"
#include <avr/interrupt.h>

#define INITIALIZE_TIMEFRAME(TIMEFRAME) \
    TIMEFRAME.nanoseconds = 0;          \
    TIMEFRAME.microseconds = 0;         \
    TIMEFRAME.milliseconds = 0;         \
    TIMEFRAME.seconds = 0              

int32_t soft_count = 0;

uint8_t number_of_tasks = 0;
struct ScheduledTask * tasks[15];
struct ScheduledTask * currentRanTask;

ISR(TCA0_OVF_vect){
    soft_count -= 0xFFFF;
    TCA0.SINGLE.INTFLAGS = 0b00000001;
}

struct ScheduledTask * create_task(struct ScheduledTask * newTask, void (*task)(TASK_FUNCTION_PARAMETERS), uint16_t millisecond_interval, uint16_t millisecond_delay){
    struct TimeFrame interval = {0, 0, millisecond_interval, 0};
    struct TimeFrame delta = {0, 0, millisecond_delay, 0};

    newTask->interval = interval;
    newTask->delta = delta;
    newTask->task = task;
    newTask->instruction_index = 1;
    newTask->state = ENABLED_EXECUTION_TASK | INFINITE_EXECUTION_TASK;
    newTask->execution_iteration = 0;
    newTask->execution_iteration_limit = 255;

    return newTask;
}

void add_task(struct ScheduledTask * task){
    tasks[number_of_tasks ++] = task;
}

struct TimeFrame * neg_wrap_time_frame(struct TimeFrame * timeFrame){
    if (timeFrame->nanoseconds < 0){
        timeFrame->microseconds -= 1;
        timeFrame->nanoseconds += 16;
    }
    if (timeFrame->microseconds < 0){
        timeFrame->milliseconds -= 1;
        timeFrame->microseconds += 1000;  
    }
    if (timeFrame->milliseconds < 0){
        timeFrame->seconds -= 1;
        timeFrame->milliseconds += 1000;
    }
    return timeFrame;
}

struct TimeFrame * wrap_time_frame(struct TimeFrame * timeFrame){
    
    if (timeFrame->nanoseconds >= 16){
        timeFrame->microseconds += 1;
        timeFrame->nanoseconds -= 16;
    }
    if (timeFrame->microseconds > 1000){
        timeFrame->milliseconds += 1;
        timeFrame->microseconds -= 1000;  
    }
    if (timeFrame->milliseconds > 1000){
        timeFrame->seconds += 1;
        timeFrame->milliseconds -= 1000;
    }
    
    return timeFrame;
}
struct TimeFrame* add_time_frame(struct TimeFrame * timeFrame, uint8_t nanoseconds_16, uint16_t microseconds){
    timeFrame->nanoseconds += nanoseconds_16;
    timeFrame->microseconds += microseconds;
        
    wrap_time_frame(timeFrame);
    
    return timeFrame;
}
int ms_compare_time_frame(struct TimeFrame * left, struct TimeFrame * right){
    uint32_t left_frame = ((uint32_t)left->milliseconds << 16) | (left->microseconds << 5) | (left->nanoseconds);
    uint32_t right_frame = ((uint32_t)right->milliseconds << 16) | (right->microseconds << 5) | (right->nanoseconds);
    
    return (left_frame < right_frame) * 2 - 1;
}

int s_compare_time_frame(struct TimeFrame * left, struct TimeFrame * right){
    // shitty way to do this. we are cutting seconds off for the sake of this to work. only 32 seconds interval is possible.
    if (left->seconds == right->seconds){
        uint32_t left_frame = ((uint32_t)left->milliseconds << 16) | (left->microseconds << 5) | (left->nanoseconds);
        uint32_t right_frame = ((uint32_t)right->milliseconds << 16)  | (right->microseconds << 5) | (right->nanoseconds);
        
        return (left_frame < right_frame) * 2 - 1;
    }else 
        return (left->seconds < right->seconds) * 2 - 1;
    
}

int imprecise_compare_time_frame(struct TimeFrame * left, struct TimeFrame * right){
    uint32_t left_frame = ((uint32_t)left->seconds << 16) | (left->microseconds << 5)  | (left->microseconds);
    uint32_t right_frame = ((uint32_t)right->seconds << 16) | (right->microseconds << 5)  | (right->microseconds);
    
    return (left_frame < right_frame) * 2 - 1;
}

void start_task(struct ScheduledTask * task){
    task->state &= ~FINISHED_EXECUTION_TASK;
    task->state |= ENABLED_EXECUTION_TASK;
    task->execution_iteration = 0;
    task->instruction_index = 1;
    INITIALIZE_TIMEFRAME((*task).delta);
}

void delay_s(uint16_t seconds_delay, uint8_t new_instruction_index){
    if (currentRanTask){
        INITIALIZE_TIMEFRAME(currentRanTask->interval);
        INITIALIZE_TIMEFRAME(currentRanTask->delta);
        currentRanTask->interval.seconds = seconds_delay;
        currentRanTask->instruction_index = new_instruction_index;
        currentRanTask->execution_iteration = 0;
        currentRanTask->execution_iteration_limit = 1;
    }
}
void delay_ms(uint16_t millisecond_delay, uint8_t new_instruction_index){
    if (currentRanTask){
        INITIALIZE_TIMEFRAME(currentRanTask->interval);
        INITIALIZE_TIMEFRAME(currentRanTask->delta);
        currentRanTask->interval.milliseconds = millisecond_delay;
        currentRanTask->instruction_index = new_instruction_index;
        currentRanTask->execution_iteration = 0;
        currentRanTask->execution_iteration_limit = 1;
    }
}
void delay_us(uint16_t microsecond_delay, uint8_t new_instruction_index){
    if (currentRanTask){
        INITIALIZE_TIMEFRAME(currentRanTask->interval);
        INITIALIZE_TIMEFRAME(currentRanTask->delta);
        currentRanTask->interval.microseconds = microsecond_delay;
        currentRanTask->instruction_index = new_instruction_index;
        currentRanTask->execution_iteration = 0;
        currentRanTask->execution_iteration_limit = 1;
    }
}

void run_mainthread(void){
    
    CCP = 0xd8;
    CLKCTRL.OSCHFCTRLA = 0b00011101;
    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){};
    
    SREG = 0b10000000;
            
    TCA0.SINGLE.CTRLA = 0b00000001;
    TCA0.SINGLE.CTRLB = 0b00000000;
    TCA0.SINGLE.INTCTRL = 0b00000001;

    while (1) {
        
        
        uint16_t count = TCA0.SINGLE.CNT;
        
        uint16_t delta_count = count - soft_count;
        soft_count = count;
        
        uint8_t timedelta_us = delta_count >> 4; 
        uint16_t timedelta_ns = delta_count & 0b1111;
        
        add_time_frame(&SystemElapsedTime, timedelta_ns, timedelta_us);
        
        
        for (uint8_t i = 0;i<number_of_tasks;i++){
            struct ScheduledTask * task = tasks[i];
            struct TimeFrame * interval = &(*task).interval;
            
            struct TimeFrame * delta = &(*task).delta;
            add_time_frame(delta, timedelta_ns, timedelta_us);
            
            uint8_t state = task->state;

            if (state & STATE_ENABLED_TASK){
                if (!(state & ENABLED_EXECUTION_TASK) ||
                    (!(task->state & INFINITE_EXECUTION_TASK) && task->execution_iteration >= task->execution_iteration_limit)){
                    continue;
                }
                if (s_compare_time_frame(interval, delta) == 1){
                    
                    
                    delta->nanoseconds -= interval->nanoseconds;
                    delta->microseconds -= interval->microseconds;
                    delta->milliseconds -= interval->milliseconds;
                    delta->seconds -= interval->seconds;
                    
                    currentRanTask = task;

                    neg_wrap_time_frame(delta);
                    
                    task->execution_iteration ++;
                    task->task(task, task->instruction_index, delta, &SystemElapsedTime);

                    currentRanTask = 0;

                    if (!(task->state & FINISHED_EXECUTION_TASK) && task->execution_iteration == task->execution_iteration_limit){
                        task->state |= FINISHED_EXECUTION_TASK;
                        if (task->state & CALLBACK_ENABLED_TASK){
                            start_task(task->callback_task);
                        }
                    }
                }
            }else{
                if (s_compare_time_frame(interval, delta) == 1){
                    
                    
                    delta->nanoseconds -= interval->nanoseconds;
                    delta->microseconds -= interval->microseconds;
                    delta->milliseconds -= interval->milliseconds;
                    delta->seconds -= interval->seconds;
                    
                    neg_wrap_time_frame(delta);
                    
                    task->execution_iteration ++;
                    task->task(task, 0, delta, &SystemElapsedTime);
                }
            }
        }
        
    }
}