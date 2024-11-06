#include <avr/io.h>

#define U8_LCD_INPUT 0
#define U8_LCD_OUTPUT 1
#define U8_LCD_LOW 0
#define U8_LCD_HIGH 1
#define R8_LCD_REGISTER_OF_CHOICE_OUTPUT PORTD.OUT
#define R8_LCD_REGISTER_OF_CHOICE_DIRECTION_SET PORTD.DIRSET

// Command Leading Bits (MSBs)
#define U8_LCD_CLEARDISPLAY   (uint8_t)0x01
#define U8_LCD_RETURNHOME     (uint8_t)0x02
#define U8_LCD_ENTRYMODESET   (uint8_t)0x04
#define U8_LCD_DISPLAYCONTROL (uint8_t)0x08
#define U8_LCD_CURSORSHIFT    (uint8_t)0x10
#define U8_LCD_FUNCTIONSET    (uint8_t)0x20
#define U8_LCD_SETCGRAMADDR   (uint8_t)0x40
#define U8_LCD_SETDDRAMADDR   (uint8_t)0x80

// Entry Mode Set Flags 
#define U8_LCD_SHIFTRIGHT     (uint8_t)0x00  /*! @remark may have switched these up -- will see if causes issues.  */
#define U8_LCD_SHIFTLEFT      (uint8_t)0x02
#define U8_LCD_ENTRYINCREMENT (uint8_t)0x01
#define U8_LCD_ENTRYDECREMENT (uint8_t)0x00

// Display On/Off Flags
#define U8_LCD_DISPLAYON      (uint8_t)0x04
#define U8_LCD_DISPLAYOFF     (uint8_t)0x00
#define U8_LCD_CURSORON       (uint8_t)0x02
#define U8_LCD_CURSOROFF      (uint8_t)0x00
#define U8_LCD_BLINKON        (uint8_t)0x01
#define U8_LCD_BLINKOFF       (uint8_t)0x00

// Display or Cursor Shift Flags
#define U8_LCD_DISPLAYMOVE    (uint8_t)0x08 // Note: if the display is set to move, the cursor shall move with it.
#define U8_LCD_CURSORMOVE     (uint8_t)0x00
#define U8_LCD_MOVERIGHT      (uint8_t)0x04
#define U8_LCD_MOVELEFT       (uint8_t)0x00

// Function Set Flags
#define U8_LCD_8BITMODE       (uint8_t)0x10
#define U8_LCD_4BITMODE       (uint8_t)0x00
#define U8_LCD_2LINE          (uint8_t)0x08
#define U8_LCD_1LINE          (uint8_t)0x00
#define U8_LCD_5x11DOTS       (uint8_t)0x04
#define U8_LCD_5x8DOTS        (uint8_t)0x00
// Initialization Fuctions:
//Useful Functions After LCD_init and LCD_begin are run:

// Useful Messaging Functions:

// PRIVATE, PROTECTED AND RESERVED FUNCTIONS TO IMPLEMENT LCD API - DO NOT USE:
void ASNYC_LCD_clear();
void ASNYC_LCD_home();
void ASNYC_LCD_setCursor(uint8_t u8Col_, uint8_t u8Row_);
void ASNYC_LCD_setCGRAMCursor(uint8_t address);
void ASNYC_LCD_displayOff(void);
void ASNYC_LCD_displayOn(void);
void ASNYC_LCD_cursorOff(void);
void ASNYC_LCD_cursorOn(void);
void ASNYC_LCD_blinkOff(void);
void ASNYC_LCD_blinkOn(void);
void ASNYC_LCD_scrollDisplayLeft(void);
void ASNYC_LCD_scrollDisplayRight(void);
void ASNYC_LCD_leftToRight(void);
void ASNYC_LCD_rightToLeft(void);
void ASNYC_LCD_autoscrollOn(void);
void ASNYC_LCD_autoscrollOff(void);
uint8_t ASNYC_LCD_sendMessage(const char *pu8MessageArray_, const uint8_t u8Size_);
void ASYNC_LCD_assignCustomCharacter(const uint8_t * pu8DrawingArray_, uint8_t address);

void lcd_initialize();
/*! @section END_DECLARATIONS --------------------------------- */

