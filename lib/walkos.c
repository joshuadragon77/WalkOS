/**
 * @file walkos.c
 * @author JOSHUA OUNALOM; Sara Rutherford
 * @brief 
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * The Main Operating System for our Walkie Textie. Utilizes the Parallism System to perform multi-tasking task.
 * 
 * Is in charge on graphical operation and user input controls.
 * 
 */
#include "walkos.h"

#include <string.h>

#include "wireless.h"
#include "multi.h"
#include "lcddriver.h"
#include "stupidlib.h"

#define KEY_ACTIVITY_SLIDER(LEFT_KEY, RIGHT_KEY) \
if (key_activity & LEFT_KEY){ \
    counter_1 -= 1; \
}else if(key_activity & RIGHT_KEY){ \
    counter_1 += 1; \
}else{ \
    counter_1 -= i16_sign(counter_1); \
} \
counter_1 = clamp(counter_1, -15, 15); \
ASNYC_LCD_setCursor(0, 1); \
if (counter_1 > 0){ \
    ASNYC_LCD_rightToLeft(); \
    ASNYC_LCD_setCursor(14, 1); \
 \
    for (int i = 0;i<counter_1;i++){ \
        ASNYC_LCD_sendMessage("\05", 1); \
    } \
}else{ \
    ASNYC_LCD_leftToRight(); \
    ASNYC_LCD_setCursor(1, 1); \
 \
    for (int i = 0;i<-1 * counter_1;i++){ \
        ASNYC_LCD_sendMessage("\05", 1); \
    } \
}

int16_t counter_1 = 0;
int8_t counter_2 = 0;
int8_t counter_3 = 0;
int8_t counter_4 = 0;
char string_buffer[16];

char name[8] = "bob";
char channel[8] = "1";


char alert_buffer_top[16];
char alert_buffer_bottom[16];

uint8_t key_activity = 0;

struct AbstractUI{
    char * name;
    uint8_t menu_mode;

    struct AbstractUI ** options;
    uint8_t current_option_selection;
    uint8_t number_of_options;

    struct AbstractUI * exitToMenu;
    void (* callback)(struct AbstractUI *);
};

struct AbstractUI mainMenu;
struct AbstractUI settingsMenu;
struct AbstractUI transmitMenu;
struct AbstractUI exitOption;

struct AbstractUI usernameChangeMenu;
struct AbstractUI channelChangeMenu;

struct AbstractUI * mainMenuOptions[] = {&transmitMenu, &settingsMenu};
struct AbstractUI * settingsMenuOptions[] = {&usernameChangeMenu, &channelChangeMenu, &exitOption};
struct AbstractUI * userChangeMenuOptions[] = {&exitOption};
struct AbstractUI * channelChangeMenuOptions[] = {&exitOption};

struct AbstractUI * currentMenuSelection = &mainMenu;

struct ScheduledTask operatingSystemUITask;
struct ScheduledTask operatingSystemInputTask;

char typed_characters[256];
uint8_t cursor_index = 0;
uint8_t on_type_delay_index = 0;
uint8_t alternative_key_delay = 0;

uint8_t char_selection_index = 0;
uint8_t char_map_selection_index = 0;
void (* text_submission_callback)(char * input, uint8_t success);


void operating_system_ui_task(TASK_FUNCTION_PARAMETERS){
    switch(instruction_index){
        case 0:{
            uint8_t enter[] = {0x1F,0x01,0x01,0x05,0x0D,0x1F,0x0C,0x04};
            ASYNC_LCD_assignCustomCharacter(enter, 1);
            
            uint8_t logo[] = {0x19,0x14,0x18,0x10,0x05,0x07,0x05,0x10};
            ASYNC_LCD_assignCustomCharacter(logo, 2);

            uint8_t left[] = {0x00,0x04,0x0C,0x1F,0x1F,0x0C,0x04,0x00};
            ASYNC_LCD_assignCustomCharacter(left, 3);

            uint8_t right[] = {0x00,0x04,0x06,0x1F,0x1F,0x06,0x04,0x00};
            ASYNC_LCD_assignCustomCharacter(right, 4);

            uint8_t line[] = {0x00,0x00,0x00,0x1F,0x1F,0x00,0x00,0x00};
            ASYNC_LCD_assignCustomCharacter(line, 5);

            uint8_t delete[] = {0x00,0x11,0x0E,0x04,0x04,0x0E,0x11,0x00};
            ASYNC_LCD_assignCustomCharacter(delete, 6);
        }
#if 1
        case 1:{
            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            counter_1 += 1;
            strcpy(string_buffer, "Walkie Textie");
            string_buffer[counter_1] = 0;
            ASNYC_LCD_sendMessage(string_buffer, 16);
            if (counter_1 < 16){
                delay_ms(50, 1);
            }else{
                delay_ms(25, 2);
                counter_1 = 0;
            }
            break;
        }
        case 2:{
            ASNYC_LCD_setCursor(15, 1);
            ASNYC_LCD_rightToLeft();
            counter_1 += 1;
            //"PH Mifflin Inc"
            //"cnI nilffiM HP"
            strcpy(string_buffer, "cnI nilffiM HP\2");
            string_buffer[counter_1] = 0;
            ASNYC_LCD_sendMessage(string_buffer, 16);
            if (counter_1 < 16){
                delay_ms(50, 2);
            }else
                delay_s(2, 3);
            break;
        }
        case 3:{
            ASNYC_LCD_clear();
            ASNYC_LCD_leftToRight();
            ASNYC_LCD_sendMessage("\2 Platyhorse", 16);
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("Mifflin Inc", 16);
            delay_s(2, 4);
            counter_1 = 0;
            break;
        }
#endif
        case 4:{
            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            struct AbstractUI * selectedOption = *(currentMenuSelection->options + currentMenuSelection->current_option_selection);

            if (counter_1 != 0){
                ASNYC_LCD_setCursor(0, 0);
                ASNYC_LCD_sendMessage("                ", 15);
                ASNYC_LCD_setCursor(0, 0);
                ASNYC_LCD_sendMessage(selectedOption->name, 15);
            }else{
                ASNYC_LCD_sendMessage(currentMenuSelection->name, 10);
            }


            // int_to_string(string_buffer, counter_1);
            // ASNYC_LCD_sendMessage(string_buffer, 4);
            

            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("\03 ", 1);

            ASNYC_LCD_sendMessage(selectedOption->name, 10);

            ASNYC_LCD_setCursor(15, 1);
            ASNYC_LCD_sendMessage("\04", 1);


            if (key_activity & 1){
                counter_1 -= 1;
            }else if(key_activity & 2){
                counter_1 += 1;
            }else{
                counter_1 -= i16_sign(counter_1);
            }

            counter_1 = clamp(counter_1, -15, 15);

            if (counter_3 != 0){
                ASNYC_LCD_setCursor(0, 0);
                ASNYC_LCD_sendMessage("                ", 15);
                ASNYC_LCD_setCursor(0, 0);
                if (counter_3 < 0){
                    counter_3 += 1;
                    for (int i = 15;i>0;i--){
                        string_buffer[i] = string_buffer[i - 1];   
                        if (!string_buffer[i]){
                            string_buffer[i] = ' ';
                        }
                    }
                    if (-counter_3 > strlen(selectedOption->name)){
                        string_buffer[0] = ' ';
                    }else
                        string_buffer[0] = *(selectedOption->name - counter_3);
                    ASNYC_LCD_sendMessage(string_buffer, 15);
                }else{
                    counter_3 -= 1;
                    if (15 - counter_3 > strlen(selectedOption->name)){
                        string_buffer[0] = ' ';
                    }else
                        string_buffer[15] = *(selectedOption->name + (15 - counter_3));
                    for (int i = 0;i<15;i++){
                        string_buffer[i] = string_buffer[i + 1];   
                        if (!string_buffer[i]){
                            string_buffer[i] = ' ';
                        }
                    }
                    ASNYC_LCD_sendMessage(string_buffer, 15);
                }
            }else
                if (i16_abs(counter_1) == 15){
                    counter_2 += 1;
                    if (counter_2 >= 10){
                        strcpy(string_buffer, selectedOption->name);

                        if (counter_1 > 0){
                            currentMenuSelection->current_option_selection += 1;
                            if (currentMenuSelection->current_option_selection >= currentMenuSelection->number_of_options){
                                currentMenuSelection->current_option_selection = 0;
                            }
                            counter_3 = -16;
                        }else{
                            if (currentMenuSelection->current_option_selection == 0){
                                currentMenuSelection->current_option_selection = currentMenuSelection->number_of_options - 1;
                            }else
                                currentMenuSelection->current_option_selection -= 1;
                            counter_3 = 16;
                        }
                        counter_2 = 0;
                    }
                }else{
                    counter_2 = 0;
                }

            if (counter_1 > 0){
                ASNYC_LCD_rightToLeft();
                ASNYC_LCD_setCursor(14, 1);

                for (int i = 0;i<counter_1;i++){
                    ASNYC_LCD_sendMessage("\05", 1);
                }
            }else{
                ASNYC_LCD_leftToRight();
                ASNYC_LCD_setCursor(1, 1);

                for (int i = 0;i<-1 * counter_1;i++){
                    ASNYC_LCD_sendMessage("\05", 1);
                }
            }
            ASNYC_LCD_setCursor(15, 0);
            ASNYC_LCD_sendMessage("\02", 1);

            delay_ms(30, 4);
            break;
        }
        case 5:{
            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            ASNYC_LCD_setCursor(0, 0);
            ASNYC_LCD_sendMessage(alert_buffer_top, 16);
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage(alert_buffer_bottom, 16);
            delay_s(4, 4);
            break;
        }
        case 6:{

            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            ASNYC_LCD_setCursor(0, 0);
            ASNYC_LCD_sendMessage(receiver_array, 16);
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("\06", 1);
            ASNYC_LCD_setCursor(2, 1);

            if (ready_receiver){
                for (uint8_t i =0;i<14;i++){
                    if ((i + counter_4) & 1 != 0){
                        ASNYC_LCD_sendMessage("\05", 1);
                    }else{
                        ASNYC_LCD_sendMessage(" ", 1);
                    }
                }
                counter_4 ++;
            }else{
                ASNYC_LCD_sendMessage("Ready", 1);
            }
            ASNYC_LCD_setCursor(15, 1);
            ASNYC_LCD_sendMessage("\01", 1);
            KEY_ACTIVITY_SLIDER(64, 128);
            if (counter_1 == -15 || counter_1 == 15){
                alternative_key_delay += 1;

                if (alternative_key_delay >= 33){
                    if (counter_1 == 15){
                        text_input();
                        alternative_key_delay = 0;
                        counter_1 = 0;
                        text_submission_callback = perform_message_transmission;
                        delay_ms(100, 7);
                    }else{
                        currentMenuSelection = &mainMenu;
                        delay_ms(100, 4);
                    }
                    break;
                }
            }
            delay_ms(30, 6);
            break;
        }
        case 7:{
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("\03 Enter Text   \01", 16);
            KEY_ACTIVITY_SLIDER(64, 128);
            if (counter_1 == -15 || counter_1 == 15){
                alternative_key_delay += 1;

                if (alternative_key_delay >= 33){

                    if (counter_1 == -15 && cursor_index > 0){
                        alternative_key_delay = 15;   
                        cursor_index -= 1;
                        typed_characters[cursor_index] = 0;
                    }
                    if (counter_1 == 15){
                        alternative_key_delay = 0;   
                        delay_ms(1, 4);
                        ASNYC_LCD_blinkOff();
                        text_submission_callback(typed_characters, 1);
                        break;
                    }
                    if (counter_1 == -15 && cursor_index == 0 && alternative_key_delay >= 66){
                        alternative_key_delay = 0;  
                        delay_ms(1, 4);
                        ASNYC_LCD_blinkOff();
                        text_submission_callback(typed_characters, 0);
                        break;
                    }
                }
            }
            ASNYC_LCD_setCursor(0, 0);
            ASNYC_LCD_sendMessage(">               ", 1);
            ASNYC_LCD_setCursor(1, 0);
            ASNYC_LCD_sendMessage(typed_characters, cursor_index + 1);
            ASNYC_LCD_setCursor(cursor_index + 1, 0);
            delay_ms(30, 7);

            break;
        }
        case 8:{
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("\03 Enter Text   \01", 16);
            KEY_ACTIVITY_SLIDER(64, 128);
            ASNYC_LCD_setCursor(0, 0);
            ASNYC_LCD_sendMessage(">               ", 1);
            ASNYC_LCD_setCursor(1, 0);
            ASNYC_LCD_sendMessage(typed_characters, cursor_index + 1);
            ASNYC_LCD_setCursor(cursor_index + 1, 0);
            
            on_type_delay_index += 1;
            if (on_type_delay_index < 32){
                delay_ms(30, 8);
            }else
                delay_ms(30, 9);
            break;
        }
        case 9:{
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage("\03 Enter Text   \01", 16);
            KEY_ACTIVITY_SLIDER(64, 128);
            delay_ms(30, 7);
            ASNYC_LCD_blinkOn();
            cursor_index += 1;
            char_selection_index = 0;
            break;
        }
        case 10:{
            char shit[5];
            int_to_string(shit, key_activity);
            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            ASNYC_LCD_sendMessage(shit, 5);
            delay_ms(50, 10);
            break;
        }
        case 11:{
            ASNYC_LCD_clear();
            ASNYC_LCD_home();
            ASNYC_LCD_setCursor(0, 0);
            ASNYC_LCD_sendMessage(alert_buffer_top, 16);
            ASNYC_LCD_setCursor(0, 1);
            ASNYC_LCD_sendMessage(alert_buffer_bottom, 16);
            delay_ms(100, 6);
            break;
        }
    }
}

uint8_t debounce_key_type = 0;

void text_input(){
    debounce_key_type = 0;
    operatingSystemUITask.instruction_index = 7;
    ASNYC_LCD_clear();
    ASNYC_LCD_home();
    ASNYC_LCD_setCursor(0, 1);
    ASNYC_LCD_sendMessage("\03 Enter Text   \01", 16);
    ASNYC_LCD_blinkOn();
    for (uint8_t i = 0;i<255;i++){
        typed_characters[i] = 0;
    }
    cursor_index = 0;
}

void apply_placeholder(char * placeholder_text){
    strcpy(typed_characters, placeholder_text);
    cursor_index = strlen(placeholder_text);
}


void operating_system_input_task(TASK_FUNCTION_PARAMETERS){

    if (key_activity != PORTA.IN){
        uint8_t current_key_activity = PORTA.IN;

        // uint8_t left = PORTA.IN & 1;
        // uint8_t right = PORTA.IN & 2;
        // uint8_t enter = PORTA.IN & 4;
        // uint8_t back = PORTA.IN & 8;

        if (operatingSystemUITask.instruction_index == 4 && PORTA.IN & 128){
            struct AbstractUI * selectedOption = *(currentMenuSelection->options + currentMenuSelection->current_option_selection);
            switch(selectedOption->menu_mode){
                case 0:{
                    currentMenuSelection = *(currentMenuSelection->options + currentMenuSelection->current_option_selection);
                    break;
                }
                case 2:{
                    selectedOption->callback(selectedOption);
                }
                case 1:{
                    currentMenuSelection = currentMenuSelection->exitToMenu;
                    break;
                }
            }
        }
        if ((operatingSystemUITask.instruction_index == 7 ||
            operatingSystemUITask.instruction_index == 8 ||
            operatingSystemUITask.instruction_index == 9)){
            if (!(counter_1 == -15 || counter_1 == 15 ) && debounce_key_type > 4){
                alternative_key_delay = 0;
                if (key_activity != 0 && current_key_activity == 0){
                    char key_map[6];
                    uint8_t key_map_length = 5;
                    uint8_t current_char_map_selection_index = 0;
                    ASNYC_LCD_blinkOff();
                    switch(key_activity){
                        case 1:{
                            current_char_map_selection_index = 0;
                            strcpy(key_map, "1abcd\0");
                            break;
                        }
                        case 2:{
                            current_char_map_selection_index = 1;
                            strcpy(key_map, "2efgh\0");
                            break;
                        }
                        case 4:{
                            current_char_map_selection_index = 2;
                            strcpy(key_map, "3ijkl\0");
                            break;
                        }
                        case 8:{
                            current_char_map_selection_index = 3;
                            strcpy(key_map, "4mnop\0");
                            break;
                        }
                        case 16:{
                            current_char_map_selection_index = 4;
                            strcpy(key_map, "5qrst\0");
                            break;
                        }
                        case 32:{
                            current_char_map_selection_index = 5;
                            strcpy(key_map, "6uvwx\0");
                            break;
                        }
                        case 64:{
                            current_char_map_selection_index = 6;
                            strcpy(key_map, "78yz \0");
                            break;
                        }
                        case 128:{
                            current_char_map_selection_index = 7;
                            key_map_length = 2;
                            strcpy(key_map, "90\0");
                            break;
                        }
                    }


                    if (current_char_map_selection_index != char_map_selection_index){
                        if (operatingSystemUITask.instruction_index == 7){
                            char_map_selection_index = current_char_map_selection_index;
                            char_selection_index = 0;
                        }else{
                            cursor_index += 1;
                            char_map_selection_index = current_char_map_selection_index;
                            char_selection_index = 0;
                        }
                    }else{
                        char_selection_index += 1;
                        if (char_selection_index == key_map_length){
                            char_selection_index = 0;
                        }
                    }

                    typed_characters[cursor_index] = key_map[char_selection_index];
                    operatingSystemUITask.instruction_index = 8;
                    on_type_delay_index = 0;
                }
            }else if(debounce_key_type <= 4){
                debounce_key_type ++;
            }
        };
        key_activity = PORTA.IN;
    }

}

void perform_message_transmission(char * input, uint8_t success){
    if (success){
        strcpy(alert_buffer_top, "Transmitting...");
        transmit_byte_array(input, 10);
    }else{
        strcpy(alert_buffer_top, "Message not sent");
        strcpy(alert_buffer_bottom, "cool");
    }
    delay_ms(100, 11);
}
void perform_name_change(char * input, uint8_t success){
    if (success){

        for (int i = 0;i<8;i++){
            name[i] = *(input + i);
        }

        strcpy(alert_buffer_top, "You changed");
        strcpy(alert_buffer_bottom, "name to ");
        strcat(alert_buffer_bottom, name);

    }else{
        strcpy(alert_buffer_top, "Name Change");
        strcpy(alert_buffer_bottom, "Cancelled.");
    }
    operatingSystemUITask.instruction_index = 5;
}
void name_change_option(struct AbstractUI * selectedOption){
    text_input();
    apply_placeholder(name);
    text_submission_callback = perform_name_change;
}


void perform_channel_change(char * input, uint8_t success){
    if (success){

        for (int i = 0;i<8;i++){
            channel[i] = *(input + i);
        }

        strcpy(alert_buffer_top, "Channel is set");
        strcpy(alert_buffer_bottom, "to ");
        strcat(alert_buffer_bottom, channel);

    }else{
        strcpy(alert_buffer_top, "Channel Change");
        strcpy(alert_buffer_bottom, "Cancelled.");
    }
    operatingSystemUITask.instruction_index = 5;
}
void channel_change_option(struct AbstractUI * selectedOption){
    text_input();
    apply_placeholder(channel);
    text_submission_callback = perform_channel_change;
}

void perform_transmission(struct AbstractUI * selectedOption){
    operatingSystemUITask.instruction_index = 6;
}


void boot_initialize(){

    PORTA.DIRCLR = 0b11111111;

    create_task(&operatingSystemUITask, operating_system_ui_task, 250, 0);
    operatingSystemUITask.state = ENABLED_EXECUTION_TASK | STATE_ENABLED_TASK;
    operatingSystemUITask.instruction_index = 0;

    create_task(&operatingSystemInputTask, operating_system_input_task, 50, 0);

    add_task(&operatingSystemUITask);
    add_task(&operatingSystemInputTask);

    mainMenu.name = "Main Menu";
    mainMenu.number_of_options = 2;
    mainMenu.current_option_selection = 0;
    mainMenu.menu_mode = 0;

    settingsMenu.name = "Settings";
    settingsMenu.number_of_options = 3;
    settingsMenu.menu_mode = 0;
    settingsMenu.current_option_selection = 0;
    settingsMenu.exitToMenu = &mainMenu;

    usernameChangeMenu.name = "Name Change";
    usernameChangeMenu.number_of_options = 1;
    usernameChangeMenu.menu_mode = 2;
    usernameChangeMenu.callback = name_change_option;
    usernameChangeMenu.current_option_selection = 0;
    usernameChangeMenu.exitToMenu = &settingsMenu;


    channelChangeMenu.name = "Channel Change";
    channelChangeMenu.number_of_options = 1;
    channelChangeMenu.menu_mode = 2;
    channelChangeMenu.callback = channel_change_option;
    channelChangeMenu.current_option_selection = 0;
    channelChangeMenu.exitToMenu = &settingsMenu;


    transmitMenu.name = "TX/RX";
    transmitMenu.number_of_options = 0;
    transmitMenu.menu_mode = 2;
    transmitMenu.current_option_selection = 0;
    transmitMenu.exitToMenu = &mainMenu;
    transmitMenu.callback = perform_transmission;


    exitOption.name = "Return";
    exitOption.number_of_options = 0;
    exitOption.menu_mode = 1;

    mainMenu.options = mainMenuOptions;
    settingsMenu.options = settingsMenuOptions;
    usernameChangeMenu.options = userChangeMenuOptions;
    channelChangeMenu.options = channelChangeMenuOptions;

}