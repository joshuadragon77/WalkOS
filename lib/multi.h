#ifndef MULTI_H
#define MULTI_H  

#include <avr/io.h>

#define STATE_ENABLED_TASK 0b00000001
#define ENABLED_EXECUTION_TASK 0b00000010
#define INFINITE_EXECUTION_TASK 0b00000100
#define CALLBACK_ENABLED_TASK 0b00001000
#define FINISHED_EXECUTION_TASK 0b00010000

#define TASK_FUNCTION_PARAMETERS struct ScheduledTask *self, uint8_t instruction_index, struct TimeFrame *delta, struct TimeFrame *SystemElapsedTime

/**
 * @brief Represents a real time.
 * Is not a single value and is multiple properties describing time.
 */
struct TimeFrame{
    int8_t nanoseconds;
    int16_t microseconds;
    int16_t milliseconds;
    uint16_t seconds;
} SystemElapsedTime;

/**
 * @brief Represents a parallel task that the AVR CPU must perform.
 * You need to specify interval and execution offset and the function it will call when it is required to.
 */
struct ScheduledTask{
    /**
     * @brief The interval of execution.
     * Also used to define the executing frequency of the task.
     */
    struct TimeFrame interval;
    /**
     * @brief The delay of execution
     * Allows to specify the "phase" of execution.
     */
    struct TimeFrame delta;

    /**
     * @brief State bits that describe the role the task is assigned
     * 0th bit indicates enabled state
     * 1st bit indicates enabled execution
     * 2nd bit indicates infinite execution
     * 3rd bit indicates a callback execution
     * 4th bit indicates finished execution
     */
    uint8_t state;

    /**
     * @brief The number of iterations the task has been executed since a "task reset"
     * Only useful for "for loop" operations in an asynchronous sense.
     * 
     * Setting this value to zero performs a "task reset".
     */
    uint8_t execution_iteration;

    /**
     * @brief The maximum iterations the task can be performed. Represents the limit of a for loop.
     * Only useful for "for loop" operations in an asynchronous sense.
     */
    uint8_t execution_iteration_limit;


    /**
     * @brief A task that will be executed after this task has completed.
     */
    struct ScheduledTask * callback_task;

    /**
     * @brief Internal memory of execution state. Not intended for public use.
     * Note: The default execution state is 1.
    */
    uint8_t instruction_index;
    
    /**
     * @brief The task that the AVR CPU must execute.
     * 
     */
    void (*task)(TASK_FUNCTION_PARAMETERS);
};

/**
 * @brief Create a Scheduled Task object
 * By default, it is not state enabled. Therefore, it always enabled and loops.
 * You can set the delay and interval for seconds, microseconds by setting their individual property inside the ScheduledTask.
 * 
 * @param task The task that the AVR CPU must execute.
 * @param instruction_index The instruction index for a specialized execution strategy. Default is 1.
 * @param millisecond_interval The interval of execution in millisecond.
 * @param millisecond_delay The delay of execution in millisecond. 
 * @return struct ScheduledTask 
 */
struct ScheduledTask * create_task(struct ScheduledTask * newTask, void (*task)(TASK_FUNCTION_PARAMETERS), uint16_t millisecond_interval, uint16_t millisecond_delay);

/**
 * @brief Add the Scheduled Task to a list of parallel process.
 * 
 * The task that has been added first is given "top" priority to be executed. Any other task below are given least priority and may execute at undesirable intervals.
 * 
 * @param task 
 */
void add_task(struct ScheduledTask * task);

/**
 * @brief Cleanly starts a task.
 * Resets the executive iteration, resets its completion state, enables and resets its delta.
 * 
 * @param task 
 */
void start_task(struct ScheduledTask * task);

/**
 * @brief Delays a task by seconds. 
 * 
 * Intended for use in a specific use case. Allows the setting of a new execution state.
 * REQUIRES the task to be NON-LOOPED and is STATE-ENABLED.
 * 
 * @param seconds_delay 
 * @param new_instruction_index 
 */
void delay_s(uint16_t seconds_delay, uint8_t new_instruction_index);

/**
 * @brief Delays a task by milliseconds. 
 * 
 * Intended for use in a specific use case. Allows the setting of a new execution state.
 * REQUIRES the task to be NON-LOOPED and is STATE-ENABLED.
 * 
 * @param millisecond_delay 
 * @param new_instruction_index 
 */
void delay_ms(uint16_t millisecond_delay, uint8_t new_instruction_index);

/**
 * @brief Delays a task by microseconds. 
 * 
 * Intended for use in a specific use case. Allows the setting of a new execution state.
 * REQUIRES the task to be NON-LOOPED and is STATE-ENABLED.
 * 
 * @param microsecond_delay 
 * @param new_instruction_index 
 */
void delay_us(uint16_t microsecond_delay, uint8_t new_instruction_index);

/**
 * @brief Takes the main thread of the AVR CPU and use it for the parallel system. 
 * When this function is executed. All further synchronous main instructions will be blocked and will require a task to be continually executed.
 */
void run_mainthread(void);

#endif