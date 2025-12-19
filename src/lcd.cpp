/*** code temprate ***/

/*** DESCRIPTION ***/

/*** INCLUDE ***/
#include <Arduino.h>
#include <string.h>
#include "lcd.h"
#include "ascii_font.h"
#include "i2c_if.h"

/*** MACRO DEFINITIONS ***/
#define LCD_ADDRESS 0x3c
//#define LCD_ADDRESS 0x50

#define LCD_INIT_SEQUENCE_STEP (19U)
#define LCD_CHAR_NUM (256U)
#define LCD_MAX_WRITE_SIZE (16U)
#define LCD_PRINT_BUF_SIZE (32U)

#define LCD_STM_IDLE 0x00
#define LCD_STM_INIT 0x01
#define LCD_STM_CLEAR 0x02
#define LCD_STM_PRINT 0x03
#define LCD_STM_SET_CURSOR 0x04

/*** TYPE DEFINITION ***/
// typedef enum
// {
//     LCD_CMD_SET_CONTRAST = 0,
//     LCD_CMD_DISPLAY_ALL_ON_RESUME,
//     LCD_CMD_NORMAL_DISPLAY,
//     LCD_CMD_SET_DISPLAY_OFF,
//     LCD_CMD_SET_DISPLAY_ON,
//     LCD_CMD_DEACTIVATE_SCROLL,
//     LCD_CMD_ACTIVATE_SCROLL,
//     LCD_CMD_MEMORY_MODE,
//     LCD_CMD_SET_COL_ADDR,
//     LCD_CMD_SET_PAGE_ADDRESS,
//     LCD_CMD_SET_PAGE_START_ADDRESS,
//     LCD_CMD_SET_START_LINE,
//     LCD_CMD_SEG_REMAP,
//     LCD_CMD_SET_MULTIPLEX,
//     LCD_CMD_COM_SCAN_DEC,
//     LCD_CMD_SET_DISPLAY_OFFSET,
//     LCD_CMD_SET_COM_PINS,
//     LCD_CMD_SET_CLOCK_DIV,
//     LCD_CMD_SET_PRECHARGE,
//     LCD_CMD_SET_VCOM_DETECT,
//     LCD_CMD_CHARGE_PUMP
// } EN_LCD_CMD;

typedef struct
{
    U1 u1_len;
    U1 au1_cmd_buf[3];
}ST_LCD_CMD_SET;

typedef struct
{
    U1 state;

    U1 send_buf[LCD_MAX_WRITE_SIZE];

    U1 print_buf[LCD_PRINT_BUF_SIZE];
    U1 print_len;
    U1 print_index;

    U1 cursor_page;
    U1 cursor_column;
} ST_LCD_CTRL;

const ST_LCD_CMD_SET cst_lcd_set_contrast_control                           = {2, {0x81,  0xCF}};
const ST_LCD_CMD_SET cst_lcd_entire_display_on                              = {1, {0xA4}};
const ST_LCD_CMD_SET cst_lcd_set_normal_display                             = {1, {0xA6}};
const ST_LCD_CMD_SET cst_lcd_set_display_off                                = {1, {0xAE}};
const ST_LCD_CMD_SET cst_lcd_set_display_on                                 = {1, {0xAF}};
const ST_LCD_CMD_SET cst_lcd_deactivate_scroll                              = {1, {0x2E}};
const ST_LCD_CMD_SET cst_lcd_activate_scroll                                = {1, {0x2F}};
const ST_LCD_CMD_SET cst_lcd_set_memory_addressing_mode                     = {2, {0x20,  0x00}};
const ST_LCD_CMD_SET cst_lcd_set_column_address                             = {3, {0x21,  0x00,  0x7F}};
const ST_LCD_CMD_SET cst_lcd_page_address                                   = {3, {0x22,  0x00,  0x07}};
const ST_LCD_CMD_SET cst_lcd_set_page_start_addres                          = {1, {0xB0}};
const ST_LCD_CMD_SET cst_lcd_set_display_start_line                         = {1, {0x40}};
const ST_LCD_CMD_SET cst_lcd_set_segment_remap                              = {1, {0xA1}};
const ST_LCD_CMD_SET cst_lcd_set_multiplex_ratio                            = {2, {0xA8,  0x3F}};
const ST_LCD_CMD_SET cst_lcd_set_com_output_scan_direction                  = {1, {0xC8}};
const ST_LCD_CMD_SET cst_lcd_set_display_offset                             = {2, {0xD3,  0x00}};
const ST_LCD_CMD_SET cst_lcd_set_set_com_pins_hardware_configuration_offset = {2, {0xDA,  0x12}};
const ST_LCD_CMD_SET cst_lcd_set_display_clock                              = {2, {0xD5,  0x80}};
const ST_LCD_CMD_SET cst_lcd_set_precharge_period                           = {2, {0xD9,  0xF1}};
const ST_LCD_CMD_SET cst_lcd_set_vcomh_deselect_leve                        = {2, {0xDB,  0x40}};
const ST_LCD_CMD_SET cst_lcd_charge_pump_setting                            = {2, {0x8D,  0x14}};



const ST_LCD_CMD_SET * capst_lcd_init_cmd_list[] = {
    &cst_lcd_set_display_off,
    &cst_lcd_set_display_on,
    &cst_lcd_set_display_clock,
    &cst_lcd_set_multiplex_ratio,
    &cst_lcd_set_display_offset,
    &cst_lcd_set_display_start_line,
    &cst_lcd_charge_pump_setting,
    &cst_lcd_set_memory_addressing_mode,
    &cst_lcd_set_segment_remap,
    &cst_lcd_set_com_output_scan_direction,
    &cst_lcd_set_set_com_pins_hardware_configuration_offset,
    &cst_lcd_set_contrast_control,
    &cst_lcd_set_precharge_period,
    &cst_lcd_set_vcomh_deselect_leve,
    &cst_lcd_entire_display_on,
    &cst_lcd_set_normal_display,
    &cst_lcd_deactivate_scroll,
    &cst_lcd_page_address,
    &cst_lcd_set_column_address
};

/*** GLOBAL VARIABLES DEFINITION ***/
ST_LCD_CTRL st_g_lcd_ctrl;

// static U1 st_g_lcd_ctrl.send_buf[LCD_MAX_WRITE_SIZE];
// static U1 st_g_lcd_ctrl.print_buf[LCD_PRINT_BUF_SIZE];
// static U1 st_g_lcd_ctrl.print_len;
// static U1 st_g_lcd_ctrl.print_index;
// static U1 st_g_lcd_ctrl.state;
// static U1 st_g_lcd_ctrl.cursor_page;
// static U1 st_g_lcd_ctrl.cursor_columun;

/*** LOCAL FUNCTION DECLARE ***/
U1 u1_lcd_init_task(VD);
U1 u1_lcd_clear_task(VD);
U1 u1_lcd_print_task(VD);

U1 u1_lcd_write_char(U1 u1_a_char);

U1 u1_lcd_send_cmd_set(ST_LCD_CMD_SET *pst_a_cmd_set);
U1 u1_lcd_send_data(U1 *au1_a_data_buf, U1 au1_a_len);

// U1 u1_lcd_send_cmd(U1 *au1_a_data_buf, U1 au1_a_len);
// U1 u1_lcd_send_cmd_set(U1 u1_a_cmd);
// U1 u1_lcd_send_data(U1 *au1_a_data_buf, U1 au1_a_len);
// U1 u1_lcd_init_task(VD);
// U1 u1_lcd_set_cursor_task(VD);
// U1 u1_lcd_debug(VD);

/*** EXTERNAL FUNCTION DEFINITION ***/
VD fn_lcd_init(VD)
{
    U1 u1_t_send_buffer_index;
    U1 u1_t_print_buffer_index;

    st_g_lcd_ctrl.print_len = 0U;
    st_g_lcd_ctrl.print_index = 0U;
    st_g_lcd_ctrl.state  = LCD_STM_INIT;

    for(u1_t_send_buffer_index = 0; u1_t_send_buffer_index < LCD_MAX_WRITE_SIZE; u1_t_send_buffer_index++)
    {
        st_g_lcd_ctrl.send_buf[u1_t_send_buffer_index] = 0U;
    }

    for(u1_t_print_buffer_index = 0; u1_t_print_buffer_index < LCD_PRINT_BUF_SIZE; u1_t_print_buffer_index++)
    {
        st_g_lcd_ctrl.print_buf[u1_t_print_buffer_index] = 0U;
    }

}

VD fn_lcd_cyc(VD)
{
    U1 u1_t_func_result;
    U1 u1_t_start_state;

    u1_t_start_state = st_g_lcd_ctrl.state;

    switch (st_g_lcd_ctrl.state) {
        case LCD_STM_IDLE:
            break;
        case LCD_STM_INIT:
            /* do lcd init task */
            /* if lcd init task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_CLEAR)*/
            u1_t_func_result = u1_lcd_init_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                Serial.println("LCD Init Completed");
                st_g_lcd_ctrl.state = LCD_STM_CLEAR;
            }
            break;
        case LCD_STM_CLEAR:
            /* do lcd clear task */
            /* if lcd clear task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_DEBUG)*/
            u1_t_func_result = u1_lcd_clear_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                st_g_lcd_ctrl.state = LCD_STM_IDLE;
            }
            break;
        case LCD_STM_PRINT:
            /* do lcd print task */
            /* if lcd print task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_IDLE)*/
            u1_t_func_result = u1_lcd_print_task();
            if (u1_t_func_result == STD_RETURN_OK) {
                st_g_lcd_ctrl.state = LCD_STM_IDLE;
            }
            break;
        // case LCD_STM_SET_CURSOR:
            // /* do lcd set cursor task */
            // /* if lcd set cursor task completed (function return is STD_RETURN_OK) change stm next state(LCD_STM_IDLE)*/
            // u1_t_func_result = u1_lcd_set_cursor_task();
            // if (u1_t_func_result == STD_RETURN_OK) {
                // st_g_lcd_ctrl.state = LCD_STM_IDLE;
            // }
            // break;            
        default:
            st_g_lcd_ctrl.state = LCD_STM_IDLE;
            break;
    }

    if (u1_t_start_state != st_g_lcd_ctrl.state) {
        Serial.print("LCD State Changed: ");
        Serial.print(u1_t_start_state);
        Serial.print(" -> ");
        Serial.println(st_g_lcd_ctrl.state);
    }

    return;
}

/*!
@brief 文字列表示リクエスト処理
@param str 表示する文字列
@return STD_RETURN_OK:表示リクエスト完了
@return STD_RETURN_NG:表示リクエスト中
@details LCDに文字列を表示するためのリクエストを処理する。
*/
U1 u1_lcd_print_request(const char *str) {
    U1 u1_t_ret;
    U1 u1_t_len;

    if (st_g_lcd_ctrl.state == LCD_STM_IDLE) {
        /* copy string to print buffer */
        u1_t_len = strlen(str);
        if (u1_t_len < LCD_PRINT_BUF_SIZE) {
            /* if string length is less than print buffer size */
            /* set print length */
            st_g_lcd_ctrl.print_len = u1_t_len;
            /* copy string to print buffer */
            memcpy(st_g_lcd_ctrl.print_buf, str, u1_t_len);
            st_g_lcd_ctrl.print_buf[u1_t_len] = '\0'; // null-terminate the string
            /* reset print position */
            st_g_lcd_ctrl.print_index = 0U;
            /* set state machine to print */
            st_g_lcd_ctrl.state = LCD_STM_PRINT;
            u1_t_ret = STD_RETURN_OK;
        }
    }
    return u1_t_ret;
}

/*!
@brief LCDビジー状態判定
@param なし
@return false:非表示リクエスト中
@return true:表示リクエスト中
@details LCDのビジー状態を判定する。
*/
FG fg_lcd_is_busy(VD) {
    FG fg_t_ret;

    if (st_g_lcd_ctrl.state == LCD_STM_IDLE) {
        fg_t_ret = false;
    } else {
        fg_t_ret = true;
    }

    return fg_t_ret;
}


/*** LOCAL FUNCTION DEFINITION ***/

/*!
@brief 初期化タスクの処理
@param なし
@return STD_RETURN_OK:初期化完了
@return STD_RETURN_NG:初期化中
@details LCDの初期化コマンドを順次送信する。すべて送信が完了したらSTD_RETURN_OKを返す。
*/
U1 u1_lcd_init_task(VD)
{
    U1 u1_t_ret;
    U1 u1_t_func_result;
    static U1 u1_s_lcd_init_sequence = 0U;
    u1_t_ret = STD_RETURN_NG;

    if (u1_s_lcd_init_sequence < LCD_INIT_SEQUENCE_STEP) {
        u1_t_func_result = u1_lcd_send_cmd_set(capst_lcd_init_cmd_list[u1_s_lcd_init_sequence]);
        if (u1_t_func_result == STD_RETURN_OK) {
            u1_s_lcd_init_sequence++;
        }
    } else {
        u1_s_lcd_init_sequence = 0U;
        u1_t_ret = STD_RETURN_OK;
    }
    return u1_t_ret;
}

/*!
@brief 画面クリアタスクの処理
@param なし
@return STD_RETURN_OK:クリア完了
@return STD_RETURN_NG:クリア中
@details LCDページごとに0埋めする。すべてのページ処理が完了したらSTD_RETURN_OKを返す。
*/
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

U1 u1_lcd_print_task(VD) {
    U1 u1_t_ret;
    U1 u1_t_result;
    u1_t_ret = STD_RETURN_NG;

    /* if print position is less than print length */
    if (st_g_lcd_ctrl.print_index < st_g_lcd_ctrl.print_len) {
        /* write char to LCD */
        u1_t_result = u1_lcd_write_char(st_g_lcd_ctrl.print_buf[st_g_lcd_ctrl.print_index]);
        /* if write char is success increment print position */
        if (u1_t_result == STD_RETURN_OK) {
            st_g_lcd_ctrl.print_index++;
        }
    }/* if finish printing return OK */
    else {
        st_g_lcd_ctrl.print_index = 0U;
        st_g_lcd_ctrl.print_len = 0U;
        u1_t_ret = STD_RETURN_OK;
    }

    return u1_t_ret;
}

/*!
@brief 文字送信処理
@param u1_a_char 送信文字
@return STD_RETURN_OK:初期化完了
@return STD_RETURN_NG:初期化中
@details LCDに文字データを送信する。
*/
U1 u1_lcd_write_char(U1 u1_a_char) {
    U1 u1_t_ret;
    U1 u1_t_function_result;
    U1 au1_t_font[5];
    memcpy(au1_t_font, &(FONT_5x8[u1_a_char - 0x20]), 5U);

    u1_t_function_result = u1_lcd_send_data(au1_t_font, 5U);

    u1_t_ret = u1_t_function_result;

    return u1_t_ret;
}

/*!
@brief コマンドセット送信処理
@param なし
@return STD_RETURN_OK:初期化完了
@return STD_RETURN_NG:初期化中
@details LCDの初期化コマンドを順次送信する。1Byte目に0x00を設定し、2Byte目以降にコマンドセットを設定する。すべて送信が完了したらSTD_RETURN_OKを返す。
*/
U1 u1_lcd_send_cmd_set(ST_LCD_CMD_SET *pst_a_cmd_set) {
    U1 u1_t_len;
    U1 au1_t_buffer[4];
    U1 u1_t_ret;
    FG fg_t_func_result;

    u1_t_len = pst_a_cmd_set->u1_len + 1U; // コマンドバイト分+コントロールバイト分
    au1_t_buffer[0] = 0x00; // コントロールバイト(コマンド送信)
    memcpy(&au1_t_buffer[1], pst_a_cmd_set->au1_cmd_buf, pst_a_cmd_set->u1_len);

    fg_t_func_result = fg_i2c_if_request_tx(LCD_ADDRESS, au1_t_buffer, u1_t_len);

    if (fg_t_func_result == true) {
        u1_t_ret = STD_RETURN_OK;
    } else {
        u1_t_ret = STD_RETURN_NG;
    }

    return u1_t_ret;
}

/*!
@brief データセット送信処理
@param なし
@return STD_RETURN_OK:初期化完了
@return STD_RETURN_NG:初期化中
@details LCDにデータを送信する。1Byte目に0x40を設定し、2Byte目以降にデータを設定する。
*/
U1 u1_lcd_send_data(U1 *au1_a_data_buf, U1 au1_a_len) {
    U1 u1_t_len;
    FG fg_t_func_result;
    U1 u1_t_ret;

    u1_t_len = au1_a_len + 1;

    st_g_lcd_ctrl.send_buf[0] = 0x40;
    memcpy(&st_g_lcd_ctrl.send_buf[1], au1_a_data_buf, au1_a_len);

    fg_t_func_result = fg_i2c_if_request_tx(LCD_ADDRESS, st_g_lcd_ctrl.send_buf, u1_t_len);

    if (fg_t_func_result == true) {
        u1_t_ret = STD_RETURN_OK;
    } else {
        u1_t_ret = STD_RETURN_NG;
    }

    return u1_t_ret;
}



// U1 u1_lcd_print_data(U4 u4_a_data) {
//     U1 u1_t_ret = STD_RETURN_NG;
//     U1 au1_t_buffer[LCD_PRINT_BUF_SIZE];
//     U4 u4_t_data_tmp;
//     U1 u1_t_length;
//     U1 u1_t_cur;

//     /* detect data length */
//     u4_t_data_tmp = u4_a_data;
//     for (u1_t_length = 0; u1_t_length < LCD_PRINT_BUF_SIZE; u1_t_length++) {
//         u4_t_data_tmp /= 10;
//         if (u4_t_data_tmp < 1) {
//             break;
//         }
//     }

//     /* exchange to string */
//     u4_t_data_tmp = u4_a_data;
//     for (u1_t_cur = 0; u1_t_cur <= u1_t_length; u1_t_cur++) {
//         au1_t_buffer[u1_t_length-u1_t_cur] = u4_t_data_tmp % 10 + '0';
//         u4_t_data_tmp /= 10;
//     }
//     au1_t_buffer[u1_t_length + 1] = '\0';

//     u1_t_ret = u1_lcd_print(au1_t_buffer);

//     return u1_t_ret;
// }


// U1 u1_lcd_clear(VD) {
//     U1 u1_t_ret;

//     u1_t_ret = STD_RETURN_NG;
//     if (st_g_lcd_ctrl.state == LCD_STM_IDLE) {
//         st_g_lcd_ctrl.state = LCD_STM_CLEAR;
//         u1_t_ret = STD_RETURN_OK;
//     }
//     return u1_t_ret;
// }

// /*
// function name: u1_lcd_set_cursor
// description: request LCD set cursor
// parameters: none
// return value: STD_RETURN_OK if set cursor request is accepted, STD_RETURN_NG if set cursor request is rejected
// remarks: if LCD is busy with another task, set cursor request is rejected
//          if set cursor request is accepted, state machine is set to set cursor state
// */
// U1 u1_lcd_set_cursor(U1 u1_a_page, U1 u1_a_column){
//     U1 u1_t_ret;
//     u1_t_ret = STD_RETURN_NG;
//     if (st_g_lcd_ctrl.state == LCD_STM_IDLE) {
//         st_g_lcd_ctrl.state = LCD_STM_SET_CURSOR;
//         st_g_lcd_ctrl.cursor_page = u1_a_page;
//         st_g_lcd_ctrl.cursor_columun = u1_a_column;
//         u1_t_ret = STD_RETURN_OK;
//     }
//     return u1_t_ret;
// }

// /*
// function name: u1_lcd_get_state
// description: get current state of LCD
// parameters: none
// return value: LCD_STATE_IDLE if LCD is idle, LCD_STATE_BUSY if LCD is busy with another task
// remarks: this function is used to check if LCD is busy with another task
//          it is used to prevent sending commands to LCD while it is busy
//          this function does not change state of LCD
//  */

// U1 u1_lcd_get_state(VD) {
//     U1 u1_t_ret;
//     if (st_g_lcd_ctrl.state == LCD_STM_IDLE) {
//         u1_t_ret = LCD_STATE_IDLE;
//     } else {
//         u1_t_ret = LCD_STATE_BUSY;
//     }
//     return u1_t_ret;
// }

// /* definition of local function */




//     return u1_t_ret;
// }


// /*
// function name: u1_lcd_set_cursor_task
// description: request LCD set cursor
// parameters: none
// return value: STD_RETURN_OK if set cursor request is accepted, STD_RETURN_NG if set cursor request is rejected
// remarks: if LCD is busy with another task, set cursor request is rejected
//          if set cursor request is accepted, state machine is set to set cursor state
// */
// U1 u1_lcd_set_cursor_task(VD){
//     static U1 u1_s_sequence_num = 0U;
//     U1 u1_t_ret;
//     U1 u1_t_cmd;
//     /* if LCD is busy with another task, return NG */
//     u1_t_ret = STD_RETURN_NG;

//     /* set cursor command */
//     if(u1_s_sequence_num == 0U){
//         /* set page address */
//         u1_t_cmd = (0xB0 | (0x0F & st_g_lcd_ctrl.cursor_page));
//         u1_lcd_send_cmd(&u1_t_cmd, 1U);
//         u1_s_sequence_num++;
//     }else if(u1_s_sequence_num == 1U){
//         /* set column address */
//         /* lower 4 bits */
//         u1_t_cmd = (0x00 | (0x0F & st_g_lcd_ctrl.cursor_columun));
//         u1_lcd_send_cmd(&u1_t_cmd, 1U);
//         u1_s_sequence_num++;   
//     }else if(u1_s_sequence_num == 2U){
//         /* set column address */
//         /* upper 4 bits */
//         u1_t_cmd = (0x10 | (0x0F & (st_g_lcd_ctrl.cursor_columun >> 4U)));
//         u1_lcd_send_cmd(&u1_t_cmd, 1U);
//         u1_s_sequence_num++;
//     }else{
//         u1_s_sequence_num = 0U;
//         u1_t_ret = STD_RETURN_OK;
//     }
//     return u1_t_ret;
// }










