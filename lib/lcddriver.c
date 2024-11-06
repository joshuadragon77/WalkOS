/**
 * @file lcddriver.c
 * @author DARREN ERASMUS; JOSHUA OUNALOM
 * @brief LCD Driver designed and ported from the Arudino library to the AVR128DB28
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * Port:
 *      The driver was originally developed by Darren on an Arudino to develop and test functionality.
 *      It was later ported to the AVR and later to the AVR's Parallism System.
 * 
 * Optimized:
 *      The portability and modularity was lost due to heavy optimization to reduce the binary file size for quicker uploads.
 * 
 *      Marcos were used in place of functions that weren't neccessary to be functions.
 * 
 */

#include "lcddriver.h"
#include "multi.h"

#define PINGROUP PORTD
#define RS_PIN 1
#define RW_PIN 255
#define EN_PIN 2
#define D4_PIN 3
#define D5_PIN 4
#define D6_PIN 5
#define D7_PIN 6

#define COLS 16
#define ROWS 2
#define RESOLUTION 0

#define ADDITIONAL_COMMANDS 1


#define OUTPUT 1
#define LOW 0 
#define HIGH 1
#define pinMode(PINGROUP_PARAM, PIN, OUTPUT) PINGROUP_PARAM.DIRSET |= OUTPUT << PIN
#define digitalWrite(PINGROUP_PARAM, PIN, STATE) ((STATE) == 0 ? (PINGROUP_PARAM.OUT &= ~(1 << PIN)) : (PINGROUP_PARAM.OUT |= (1 << PIN)))

#define delay _delay_ms
#define delayMicroseconds(delay_us) _delay_us(delay_us)

#define U8_MAX_LCD_CHARS (uint8_t)100 /*! @remark arbitrary number for now */


uint8_t LCD_u8_displayFunction; // corresponds to Function Set Instruction
uint8_t LCD_u8_displayControl;  // corresponds to Display ON/OFF and Control Instruction
uint8_t LCD_u8_displayMode;     // corresponds to Cursor or Display Shift Instruction     

uint8_t LCD_au8_row_offsets[] = {0x00, 0x40, 0x00 + COLS, 0x40 + COLS};


struct ScheduledTask lcdTask;

uint8_t buffer[256];
uint8_t mode[256];
uint8_t read_head = 0;
uint8_t write_head = 0;

// set to a fixed pin arrangement and not dependant on LCD_au8_data_pins for optimization sake.
#define HELPER_LCD_write_four_bits(u8Value_) PINGROUP.OUT &= ~0b01111000; \
    PINGROUP.OUT |= (0b1111 & u8Value_) << 3

#define PRE_ASYNC_LCD_send_data_bits(u8Value_, u8Mode_, extensive_delay) \
    mode[write_head] = (u8Mode_ & 1) | ((extensive_delay & 1) << 1); \
    buffer[write_head ++] = u8Value_ 
#define ASYNC_LCD_command(u8Value_) PRE_ASYNC_LCD_send_data_bits(u8Value_, LOW, 0)
#define ASYNC_LCD_command_extensive_delay(u8Value_)PRE_ASYNC_LCD_send_data_bits(u8Value_, LOW, 1)
#define ASYNC_LCD_write(u8Value_) PRE_ASYNC_LCD_send_data_bits(u8Value_, HIGH, 0)

void ASYNC_LCD_task(TASK_FUNCTION_PARAMETERS) {
    switch(instruction_index){
        case 1:{
            delay_ms(50, 2);
            break;
        }
        case 2:{
            digitalWrite(PINGROUP, EN_PIN, LOW);
            digitalWrite(PINGROUP, RS_PIN, LOW);
            if (RW_PIN != 255) {
                digitalWrite(PINGROUP, RW_PIN, LOW);
            }
            if ( !(LCD_u8_displayFunction & U8_LCD_8BITMODE) ) {
                HELPER_LCD_write_four_bits(0x03);
                digitalWrite(PINGROUP, EN_PIN, HIGH);
                digitalWrite(PINGROUP, EN_PIN, LOW);
                return delay_ms(6, 3);
            }
            delay_us(1, 6); //smaller delay
            break;
        }
        case 3:{
            if (read_head < 3){
                read_head ++;
                HELPER_LCD_write_four_bits(0x03);
                digitalWrite(PINGROUP, EN_PIN, HIGH);
                digitalWrite(PINGROUP, EN_PIN, LOW);
                delay_ms(6, 3); //at least 4.1 ms
            }else{
                delay_us(150, 5); //smaller delay
            }
            break;
        }
        case 5:{
            HELPER_LCD_write_four_bits(0x02);
            digitalWrite(PINGROUP, EN_PIN, LOW);
            digitalWrite(PINGROUP, EN_PIN, HIGH);
            digitalWrite(PINGROUP, EN_PIN, LOW);
        }
        case 6:{
            ASYNC_LCD_command(U8_LCD_FUNCTIONSET | LCD_u8_displayFunction);
            LCD_u8_displayControl = U8_LCD_DISPLAYON | U8_LCD_CURSOROFF | U8_LCD_BLINKOFF;
            ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
            ASYNC_LCD_command(U8_LCD_CLEARDISPLAY);
            LCD_u8_displayMode = U8_LCD_SHIFTLEFT | U8_LCD_ENTRYDECREMENT; // 0x02 overall
            ASYNC_LCD_command(U8_LCD_ENTRYMODESET | LCD_u8_displayMode);
            delay_ms(1, 25);
            read_head = 0;
            break;
        }
        case 25:{
            if (read_head == write_head)
                return delay_ms(1, 25);

            uint8_t u8Value_ = buffer[read_head];//(buffer[read_head] & 15) | ((buffer[read_head + 1] & 15) << 4);
            uint8_t u8Mode_ = mode[read_head];
            uint8_t extensive_delay_mode = (u8Mode_ >> 1) & 1;
            read_head++;

            digitalWrite(PINGROUP, RS_PIN, u8Mode_ & 1);


            HELPER_LCD_write_four_bits(u8Value_ >> 4);
            
            digitalWrite(PINGROUP, EN_PIN, HIGH);
            digitalWrite(PINGROUP, EN_PIN, LOW);

            HELPER_LCD_write_four_bits(u8Value_);
            digitalWrite(PINGROUP, EN_PIN, HIGH);
            digitalWrite(PINGROUP, EN_PIN, LOW);
            
            if (extensive_delay_mode){
                delay_ms(1, 26);
            }else{
                delay_us(50, 25);
            }
            break;
        }
        case 26:{
            delay_us(400, 25);
        }
    }
}


// void ASYNC_LCD_command(const uint8_t u8Value_) {
//     PRE_ASYNC_LCD_send_data_bits(u8Value_, LOW, 0);
// }
// void ASYNC_LCD_command_extensive_delay(const uint8_t u8Value_) {
//     PRE_ASYNC_LCD_send_data_bits(u8Value_, LOW, 1);
// }

// void ASYNC_LCD_write(const uint8_t u8Value_) { 
//     PRE_ASYNC_LCD_send_data_bits(u8Value_, HIGH, 0);
// }

void ASNYC_LCD_clear() {
    ASYNC_LCD_command_extensive_delay(U8_LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    return;
}

void ASNYC_LCD_home() {
    ASYNC_LCD_command_extensive_delay(U8_LCD_RETURNHOME);  // set cursor position to zero ("00H")
    return;
}

void ASNYC_LCD_setCursor(uint8_t u8Col_, uint8_t u8Row_) {
    // both if statements below adjust the provided u8Row_ if too large based on supplied FW/HW facts.
    const size_t stMaxLines = sizeof(LCD_au8_row_offsets) / sizeof(*LCD_au8_row_offsets);
    if (u8Row_ >= stMaxLines) {
        u8Row_ = stMaxLines - 1;    // we count rows starting with 0
    }
    if (u8Row_ >= ROWS) {
        u8Row_ = ROWS - 1;    // we count rows starting with 0
    }

    ASYNC_LCD_command(U8_LCD_SETDDRAMADDR | (u8Col_ + LCD_au8_row_offsets[u8Row_])); // sets address counter
    return;
}

void ASNYC_LCD_setCGRAMCursor(uint8_t address) {
    ASYNC_LCD_command(U8_LCD_SETCGRAMADDR | (address  << 3)); // sets address counter
    return;
}

#if ADDITIONAL_COMMANDS

// Turn the display on/off
void ASNYC_LCD_displayOff(void) {
    LCD_u8_displayControl &= ~U8_LCD_DISPLAYON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}

void ASNYC_LCD_displayOn(void) {
    LCD_u8_displayControl |= U8_LCD_DISPLAYON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}


// Turns the underline cursor on/off
void ASNYC_LCD_cursorOff(void) {
    LCD_u8_displayControl &= ~U8_LCD_CURSORON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}
void ASNYC_LCD_cursorOn(void) {
    LCD_u8_displayControl |= U8_LCD_CURSORON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}


// Turn on and off the blinking cursor
void ASNYC_LCD_blinkOff(void) {
    LCD_u8_displayControl &= ~U8_LCD_BLINKON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}
void ASNYC_LCD_blinkOn(void) {
    LCD_u8_displayControl |= U8_LCD_BLINKON;
    ASYNC_LCD_command(U8_LCD_DISPLAYCONTROL | LCD_u8_displayControl);
    return;
}


// These commands scroll the display without changing the RAM
void ASNYC_LCD_scrollDisplayLeft(void) {
    ASYNC_LCD_command(U8_LCD_CURSORSHIFT | U8_LCD_DISPLAYMOVE | U8_LCD_MOVELEFT);
    return;
}
void ASNYC_LCD_scrollDisplayRight(void) {
    ASYNC_LCD_command(U8_LCD_CURSORSHIFT | U8_LCD_DISPLAYMOVE | U8_LCD_MOVERIGHT);
    return;
}


// This is for text that flows Left to Right
void ASNYC_LCD_leftToRight(void) {
    LCD_u8_displayMode |= U8_LCD_SHIFTLEFT;
    ASYNC_LCD_command(U8_LCD_ENTRYMODESET | LCD_u8_displayMode);
    return;
}

// This is for text that flows Right to Left
void ASNYC_LCD_rightToLeft(void) {
    LCD_u8_displayMode &= ~U8_LCD_SHIFTLEFT;
    ASYNC_LCD_command(U8_LCD_ENTRYMODESET | LCD_u8_displayMode);
    return;
}


// This will 'right justify' text from the cursor
void ASNYC_LCD_autoscrollOn(void) {
    LCD_u8_displayMode |= U8_LCD_ENTRYINCREMENT;
    ASYNC_LCD_command(U8_LCD_ENTRYMODESET | LCD_u8_displayMode);
    return;
}

// This will 'left justify' text from the cursor
void ASNYC_LCD_autoscrollOff(void) {
    LCD_u8_displayMode &= ~U8_LCD_ENTRYINCREMENT;
    ASYNC_LCD_command(U8_LCD_ENTRYMODESET | LCD_u8_displayMode);
    return;
}

#endif

//! @remarks REQUIRES: 
//!   - @param pu8MessageArray_ is a pointer to an array 
//!        of ASCII chars innitialized in a C string.
//!   - @param u8Size_ is the size of the array to be sent.
//! @returns remaining quantity of data unsent due to LCD character limit
uint8_t ASNYC_LCD_sendMessage(const char * pu8MessageArray_, const uint8_t u8Size_) {

    for (uint8_t i = 0; (pu8MessageArray_[i] != '\0') || (i == U8_MAX_LCD_CHARS); i++) {
        ASYNC_LCD_write(pu8MessageArray_[i]);
    }

    return (u8Size_ > U8_MAX_LCD_CHARS) ? (u8Size_ - U8_MAX_LCD_CHARS) : 0;
}

void ASYNC_LCD_assignCustomCharacter(const uint8_t * pu8DrawingArray_, uint8_t address){
    ASNYC_LCD_setCGRAMCursor(address);
    for (uint8_t i = 0;i<8; i++) {
        ASYNC_LCD_write(pu8DrawingArray_[i]);
    }
}

void lcd_initialize(){
    // LCD_init(RS_PIN, 255, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);


    LCD_u8_displayFunction = U8_LCD_4BITMODE | U8_LCD_1LINE | U8_LCD_5x8DOTS;

    (ROWS > 1) ? (LCD_u8_displayFunction |= U8_LCD_2LINE) : (LCD_u8_displayFunction |= U8_LCD_1LINE);

    if ((ROWS == 1) && (RESOLUTION != U8_LCD_5x8DOTS)) {
        LCD_u8_displayFunction |= U8_LCD_5x11DOTS;  // if the user wishes to select higher pixel vertical resolutions
    }

    // initialize pin directions.
    pinMode(PINGROUP,EN_PIN, OUTPUT);
    pinMode(PINGROUP,RS_PIN, OUTPUT);
    if (RW_PIN != 255) {
        pinMode(PINGROUP, RW_PIN, OUTPUT);
    } 

    //initialize data pins direction (only for 4 pin mode, but 8 bit mode could be implemented with & ternary)
    pinMode(PINGROUP, D4_PIN, OUTPUT);
    pinMode(PINGROUP, D5_PIN, OUTPUT);
    pinMode(PINGROUP, D6_PIN, OUTPUT);
    pinMode(PINGROUP, D7_PIN, OUTPUT);

    create_task(&lcdTask, ASYNC_LCD_task, 0, 0);
    lcdTask.state = ENABLED_EXECUTION_TASK | STATE_ENABLED_TASK;

    add_task(&lcdTask);

}