#ifndef LCD_H
#define LCD_H

#include "common.h"


#define LCD_STATE_IDLE 0x00
#define LCD_STATE_BUSY 0x01


VD fn_lcd_cyc(VD);
VD fn_lcd_init(VD);
U1 u1_lcd_print_request(const char *str);
FG fg_lcd_is_busy(VD);
U1 u1_lcd_set_cursor(U1 u1_a_page, U1 u1_a_column);
// U1 u1_lcd_print(const char *str);
// U1 u1_lcd_print_data(U4 u4_a_data);
// U1 u1_lcd_clear(VD);
// U1 u1_lcd_set_cursor(U1 u1_a_page, U1 u1_a_column);
// U1 u1_lcd_get_state(VD);


#endif /* LCD_H */

