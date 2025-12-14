#include <Arduino.h>
#include <string.h>
#include "lcd.h"
#include "ascii_font.h"
#include "i2c_if.h"

#define LCD_ADDRESS 0x3c
// #define LCD_ADDRESS             0x50
#define LCD_INIT_SEQUENCE_STEP (19U)
#define LCD_CHAR_NUM (256U)
#define LCD_MAX_WRITE_SIZE (16U)
#define LCD_PRINT_BUF_SIZE (32U)

#define LCD_STM_IDLE 0x00
#define LCD_STM_INIT 0x01
#define LCD_STM_CLEAR 0x02
#define LCD_STM_PRINT 0x03
#define LCD_STM_SET_CURSOR 0x04

#define LCD_CMD_SET_CONTRAST (0U)
#define LCD_CMD_DISPLAY_ALL_ON_RESUME (1U)
#define LCD_CMD_NORMAL_DISPLAY (2U)
#define LCD_CMD_SET_DISPLAY_OFF (3U)
#define LCD_CMD_SET_DISPLAY_ON (4U)
#define LCD_CMD_DEACTIVATE_SCROLL (5U)
#define LCD_CMD_ACTIVATE_SCROLL (6U)
#define LCD_CMD_MEMORY_MODE (7U)
#define LCD_CMD_SET_COL_ADDR (8U)
#define LCD_CMD_SET_PAGE_ADDRESS (9U)
#define LCD_CMD_SET_PAGE_START_ADDRESS (10U)
#define LCD_CMD_SET_START_LINE (11U)
#define LCD_CMD_SEG_REMAP (12U)
#define LCD_CMD_SET_MULTIPLEX (13U)
#define LCD_CMD_COM_SCAN_DEC (14U)
#define LCD_CMD_SET_DISPLAY_OFFSET (15U)
#define LCD_CMD_SET_COM_PINS (16U)
#define LCD_CMD_SET_CLOCK_DIV (17U)
#define LCD_CMD_SET_PRECHARGE (18U)
#define LCD_CMD_SET_VCOM_DETECT (19U)
#define LCD_CMD_CHARGE_PUMP (20U)

/* definition of global variable */
const U1 Cu1_lcd_init_task_CMD[] = {
    LCD_CMD_SET_DISPLAY_OFF,
    LCD_CMD_SET_DISPLAY_ON,
    LCD_CMD_SET_CLOCK_DIV,
    LCD_CMD_SET_MULTIPLEX,
    LCD_CMD_SET_DISPLAY_OFFSET,
    LCD_CMD_SET_START_LINE,
    LCD_CMD_CHARGE_PUMP,
    LCD_CMD_MEMORY_MODE,
    LCD_CMD_SEG_REMAP,
    LCD_CMD_COM_SCAN_DEC,
    LCD_CMD_SET_COM_PINS,
    LCD_CMD_SET_CONTRAST,
    LCD_CMD_SET_PRECHARGE,
    LCD_CMD_SET_VCOM_DETECT,
    LCD_CMD_DISPLAY_ALL_ON_RESUME,
    LCD_CMD_NORMAL_DISPLAY,
    LCD_CMD_DEACTIVATE_SCROLL,
    LCD_CMD_SET_PAGE_ADDRESS,
    LCD_CMD_SET_COL_ADDR
};

const U1 CU1_LCD_CMD_SET[] = {
    /*** 1. Fundamental Command Table ****/
    /* 0:Set Contrast Control */
    0x81, 0xCF,
    /* 1:Entire Display ON (Resume to RAM content display) */
    0xA4,
    /* 2:Set Normal/Inverse Display */
    0xA6, // Normal Display
    /* 4:Set Display OFF*/
    0xAE,
    /* 5:Set Display ON*/
    0xAF,

    /*** 2.Scrolling Command Table ***/
    /* 5:Deactivate scroll */
    0x2E,
    /* 6:Activate scroll */
    0x2F,

    /*** 3. Addressing Setting Command Table ***/
    /* 7:Set Memory Addressing Mode */
    0x20, 0x00, // Memory Addressing Mode (Horizontal addressing mode)
    /* 8:Set Column Address */
    0x21, 0x00, 0x7F, // Set Column Address (0-127)
    /* 9: Page Address */
    0x22, 0x00, 0x07, // Set Page Address (0-7)
    /* 10: Set Page Start Address*/
    0xB0, // Set GDDRAM Page Start Address PAGE0

    /*** 4.Hardware Configuration (Panel resolution & layout related) Command Table ***/
    /* 11:Set Display Start Line */
    0x40, // Set Display Start Line to 0
    /* 12:Set Segment Re-map*/
    0xA1, // column address 0 is mapped to SEG127 (display left to right)
    /* 13:Set Multiplex Ratio */
    0xA8, 0x3F, // Set Multiplex Ratio (0x3F = 64MUX)
    /* 14:Set COM Output Scan Direction*/
    0xC8, // COM Output Scan Direction (remapped mode)
    /* 15:Set Display Offset */
    0xD3, 0x00, // Set Display Offset 0
    /* 16:Set Set COM Pins Hardware Configuration Offset */
    0xDA, 0x12, // Set COM Pins hardware config

    /*** 5. Timing & Driving Scheme Setting Command Table ***/
    /* 17:Set Display Clock Divide Ratio/Oscillator Frequency */
    0xD5, 0x80,
    /* 18:Set Pre-charge Period */
    0xD9, 0xF1, // Set Pre-charge Period
    /* 19:Set VCOMH Deselect Leve */
    0xDB, 0x40, // Set VCOMH Deselect Level

    /*** Charge Pump Command Table ***/
    /* 20:Charge Pump Setting */
    0x8D, 0x14, // Enable charge pump
};

const U1 CU1_LCD_CMD_LEN[] = {
    2, 1, 1, 1, 1, 1, 1, 2, 3, 3, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2
};

const U1 CU1_LCD_CMD_CUR[] = {
    0, 2, 3, 4, 5, 6, 7, 8, 10, 13, 16, 17, 18, 19, 21, 22, 24, 26, 28, 30, 32
};

U1 au1_g_lcd_send_buf[LCD_MAX_WRITE_SIZE];
static U1 au1_g_lcd_print_buf[LCD_PRINT_BUF_SIZE];
static U1 u1_g_lcd_print_len;
static U1 u1_g_lcd_print_pos;
static U1 u1_g_lcd_stm = LCD_STM_INIT;
static U1 u1_g_lcd_cursor_page = 0U;
static U1 u1_g_lcd_cursor_column = 0U;
    
/* declaration of external function */

U1 u1_lcd_print_task(VD);
U1 u1_lcd_send_cmd(U1 *au1_a_data_buf, U1 au1_a_len);
U1 u1_lcd_send_cmd_set(U1 u1_a_cmd);
U1 u1_lcd_send_data(U1 *au1_a_data_buf, U1 au1_a_len);
U1 u1_lcd_init_task(VD);
U1 u1_lcd_clear_task(VD);
U1 u1_lcd_set_cursor_task(VD);
U1 u1_lcd_write_char(U1 u1_a_char);
U1 u1_lcd_debug(VD);

/* definition of external function */

VD fn_lcd_task(VD) {
    U1 u1_t_func_result;

    switch (u1_g_lcd_stm) {
        case LCD_STM_IDLE:
            break;
        case LCD_STM_INIT:
            /* do lcd init task */
            /* if lcd init task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_CLEAR)*/
            u1_t_func_result = u1_lcd_init_task();
            break;
        case LCD_STM_CLEAR:
            /* do lcd clear task */
            /* if lcd clear task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_DEBUG)*/
            u1_t_func_result = u1_lcd_clear_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                u1_g_lcd_stm = LCD_STM_IDLE;
            }
            break;
        case LCD_STM_PRINT:
            /* do lcd print task */
            /* if lcd print task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_IDLE)*/
            u1_t_func_result = u1_lcd_print_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                u1_g_lcd_stm = LCD_STM_IDLE;
            }
            break;
        case LCD_STM_SET_CURSOR:
            /* do lcd set cursor task */
            /* if lcd set cursor task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_IDLE)*/
            u1_t_func_result = u1_lcd_set_cursor_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                u1_g_lcd_stm = LCD_STM_IDLE;
            }
            break;
        default:
            u1_g_lcd_stm = LCD_STM_IDLE;
            break;
    }

    return;
}

/*
function name: u1_lcd_print
description: print requested string to LCD
parameters: str - string to print
return value: STD_RETURN_OK if print request is accepted, STD_RETURN_NG if print request is rejected
remarks: if LCD is busy with another task, print request is rejected
         if print request is accepted, string is copied to print buffer and state machine is set to print state
         string length must be less than LCD_PRINT_BUF_SIZE
 */
U1 u1_lcd_print(const char *str) {
    U1 u1_t_ret;
    U1 u1_t_len;

    u1_t_ret = STD_RETURN_NG;
    if (u1_g_lcd_stm == LCD_STM_IDLE) {
        /* copy string to print buffer */
        u1_t_len = strlen(str);
        if (u1_t_len < LCD_PRINT_BUF_SIZE) {
            /* if string length is less than print buffer size */
            /* set print length */
            u1_g_lcd_print_len = u1_t_len;
            /* copy string to print buffer */
            memcpy(au1_g_lcd_print_buf, str, u1_t_len);
            au1_g_lcd_print_buf[u1_t_len] = '\0'; // null-terminate the string
            /* reset print position */
            u1_g_lcd_print_pos = 0U;
            /* set state machine to print */
            u1_g_lcd_stm = LCD_STM_PRINT;
            u1_t_ret = STD_RETURN_OK;
        }
    }
    return u1_t_ret;
}

U1 u1_lcd_print_data(U4 u4_a_data) {
    U1 u1_t_ret = STD_RETURN_NG;
    U1 au1_t_buffer[LCD_PRINT_BUF_SIZE];
    U4 u4_t_data_tmp;
    U1 u1_t_length;
    U1 u1_t_cur;

    /* detect data length */
    u4_t_data_tmp = u4_a_data;
    for (u1_t_length = 0; u1_t_length < LCD_PRINT_BUF_SIZE; u1_t_length++) {
        u4_t_data_tmp /= 10;
        if (u4_t_data_tmp < 1) {
            break;
        }
    }

    /* exchange to string */
    u4_t_data_tmp = u4_a_data;
    for (u1_t_cur = 0; u1_t_cur <= u1_t_length; u1_t_cur++) {
        au1_t_buffer[u1_t_length-u1_t_cur] = u4_t_data_tmp % 10 + '0';
        u4_t_data_tmp /= 10;
    }
    au1_t_buffer[u1_t_length + 1] = '\0';

    u1_t_ret = u1_lcd_print(au1_t_buffer);

    return u1_t_ret;
}

/*
function name: u1_lcd_clear_task
description: request LCD clear
parameters: none
return value: STD_RETURN_OK if clear request is accepted, STD_RETURN_NG if clear request is rejected
remarks: if LCD is busy with another task, clear request is rejected
         if clear request is accepted, state machine is set to clear state
 */
U1 u1_lcd_clear(VD) {
    U1 u1_t_ret;

    u1_t_ret = STD_RETURN_NG;
    if (u1_g_lcd_stm == LCD_STM_IDLE) {
        u1_g_lcd_stm = LCD_STM_CLEAR;
        u1_t_ret = STD_RETURN_OK;
    }
    return u1_t_ret;
}

/*
function name: u1_lcd_set_cursor
description: request LCD set cursor
parameters: none
return value: STD_RETURN_OK if set cursor request is accepted, STD_RETURN_NG if set cursor request is rejected
remarks: if LCD is busy with another task, set cursor request is rejected
         if set cursor request is accepted, state machine is set to set cursor state
*/
U1 u1_lcd_set_cursor(U1 u1_a_page, U1 u1_a_column){
    U1 u1_t_ret;
    u1_t_ret = STD_RETURN_NG;
    if (u1_g_lcd_stm == LCD_STM_IDLE) {
        u1_g_lcd_stm = LCD_STM_SET_CURSOR;
        u1_g_lcd_cursor_page = u1_a_page;
        u1_g_lcd_cursor_column = u1_a_column;
        u1_t_ret = STD_RETURN_OK;
    }
    return u1_t_ret;
}

/*
function name: u1_lcd_get_state
description: get current state of LCD
parameters: none
return value: LCD_STATE_IDLE if LCD is idle, LCD_STATE_BUSY if LCD is busy with another task
remarks: this function is used to check if LCD is busy with another task
         it is used to prevent sending commands to LCD while it is busy
         this function does not change state of LCD
 */

U1 u1_lcd_get_state(VD) {
    U1 u1_t_ret;
    if (u1_g_lcd_stm == LCD_STM_IDLE) {
        u1_t_ret = LCD_STATE_IDLE;
    } else {
        u1_t_ret = LCD_STATE_BUSY;
    }
    return u1_t_ret;
}

/* definition of local function */


U1 u1_lcd_init_task(VD) {
    U1 u1_t_ret;
    U1 u1_t_func_result;
    static U1 u1_s_lcd_init_sequence = 0U;
    u1_t_ret = STD_RETURN_NG;

    if (u1_s_lcd_init_sequence < LCD_INIT_SEQUENCE_STEP) {
        u1_t_func_result = u1_lcd_send_cmd_set(Cu1_lcd_init_task_CMD[u1_s_lcd_init_sequence]);
        if (u1_t_func_result == STD_RETURN_OK) {
            u1_s_lcd_init_sequence++;
        }
    } else {
        u1_t_ret = STD_RETURN_OK;
    }

    return u1_t_ret;
}

U1 u1_lcd_clear_task(VD) {
    static U2 u2_s_sequence_num = 0U;
    U1 u1_t_ret;
    U1 u1_t_result;

    u1_t_ret = STD_RETURN_NG;

    /* repeat LCD_CHAR_NUM times sending space char */
    if (u2_s_sequence_num < LCD_CHAR_NUM) {
        u1_t_result = u1_lcd_write_char(' ');
        /* if sending a char is success sequence num increment */
        if (u1_t_result == STD_RETURN_OK) {
            u2_s_sequence_num++;
        }
    }/* if finish sending return OK */
    else {
        u2_s_sequence_num = 0U;
        u1_t_ret = STD_RETURN_OK;
    }
    return u1_t_ret;
}

/*
function name: u1_lcd_set_cursor_task
description: request LCD set cursor
parameters: none
return value: STD_RETURN_OK if set cursor request is accepted, STD_RETURN_NG if set cursor request is rejected
remarks: if LCD is busy with another task, set cursor request is rejected
         if set cursor request is accepted, state machine is set to set cursor state
*/
U1 u1_lcd_set_cursor_task(VD){
    static U1 u1_s_sequence_num = 0U;
    U1 u1_t_ret;
    U1 u1_t_cmd;
    /* if LCD is busy with another task, return NG */
    u1_t_ret = STD_RETURN_NG;

    /* set cursor command */
    if(u1_s_sequence_num == 0U){
        /* set page address */
        u1_t_cmd = (0xB0 | (0x0F & u1_g_lcd_cursor_page));
        u1_lcd_send_cmd(&u1_t_cmd, 1U);
        u1_s_sequence_num++;
    }else if(u1_s_sequence_num == 1U){
        /* set column address */
        /* lower 4 bits */
        u1_t_cmd = (0x00 | (0x0F & u1_g_lcd_cursor_column));
        u1_lcd_send_cmd(&u1_t_cmd, 1U);
        u1_s_sequence_num++;   
    }else if(u1_s_sequence_num == 2U){
        /* set column address */
        /* upper 4 bits */
        u1_t_cmd = (0x10 | (0x0F & (u1_g_lcd_cursor_column >> 4U)));
        u1_lcd_send_cmd(&u1_t_cmd, 1U);
        u1_s_sequence_num++;
    }else{
        u1_s_sequence_num = 0U;
        u1_t_ret = STD_RETURN_OK;
    }
    return u1_t_ret;
}

U1 u1_lcd_print_task(VD) {
    U1 u1_t_ret;
    U1 u1_t_result;
    u1_t_ret = STD_RETURN_NG;

    /* if print position is less than print length */
    if (u1_g_lcd_print_pos < u1_g_lcd_print_len) {
        /* write char to LCD */
        u1_t_result = u1_lcd_write_char(au1_g_lcd_print_buf[u1_g_lcd_print_pos]);
        /* if write char is success increment print position */
        if (u1_t_result == STD_RETURN_OK) {
            u1_g_lcd_print_pos++;
        }
    }/* if finish printing return OK */
    else {
        u1_t_ret = STD_RETURN_OK;
    }

    return u1_t_ret;
}

U1 u1_lcd_write_char(U1 u1_a_char) {
    U1 u1_t_ret;
    U1 u1_t_function_result;
    U1 *pau1_t_font = &(FONT_5x8[u1_a_char - 0x20]);

    u1_t_function_result = u1_lcd_send_data(pau1_t_font, 5U);

    u1_t_ret = u1_t_function_result;

    return u1_t_ret;
}

U1 u1_lcd_send_cmd_set(U1 u1_a_cmd) {
    U1 u1_t_len = CU1_LCD_CMD_LEN[u1_a_cmd] + 1;
    U1 u1_t_ret;
    FG fg_t_func_result;

    u1_t_ret = STD_RETURN_NG;

    au1_g_lcd_send_buf[0] = 0x00;
    memcpy(&au1_g_lcd_send_buf[1], &CU1_LCD_CMD_SET[CU1_LCD_CMD_CUR[u1_a_cmd]], u1_t_len);

    if (!I2C1_Host.IsBusy()) {
        fg_t_func_result = I2C1_Host.Write(LCD_ADDRESS, au1_g_lcd_send_buf, u1_t_len);

        if (fg_t_func_result) {
            u1_t_ret = STD_RETURN_OK;
        }
    }

    return u1_t_ret;
}

U1 u1_lcd_send_cmd(U1 *au1_a_data_buf, U1 au1_a_len) {
    U1 u1_t_len = au1_a_len + 1;
    U1 u1_t_ret;

    u1_t_ret = STD_RETURN_NG;

    au1_g_lcd_send_buf[0] = 0x00;
    memcpy(&au1_g_lcd_send_buf[1], au1_a_data_buf, au1_a_len);

    u1_t_ret = u1_i2c_if_i2c_enqueue(LCD_ADDRESS, au1_g_lcd_send_buf, u1_t_len);

    return u1_t_ret;
}

U1 u1_lcd_send_data(U1 *au1_a_data_buf, U1 au1_a_len) {
    U1 u1_t_len = au1_a_len + 1;
    U1 u1_t_ret;

    u1_t_ret = STD_RETURN_NG;

    au1_g_lcd_send_buf[0] = 0x40;
    memcpy(&au1_g_lcd_send_buf[1], au1_a_data_buf, au1_a_len);

    u1_t_ret = u1_i2c_if_i2c_enqueue(LCD_ADDRESS, au1_g_lcd_send_buf, u1_t_len);

    return u1_t_ret;
}
