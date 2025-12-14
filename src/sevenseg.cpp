/*** DESCRIPTION ***/

/*** INCLUDE ***/
#include <Arduino.h>
#include "common.h"
#include "sevenseg.h"

/*** MACRO DEFINITIONS ***/
#define SEGMENT_PIN_0 3
#define SEGMENT_PIN_1 4
#define SEGMENT_PIN_2 5
#define SEGMENT_PIN_3 6
#define SEGMENT_PIN_DIGIT 7

/*** GLOBAL VARIABLES DEFINITION ***/
U1 u1_g_dislay_number;
U1 u1_g_current_digit;

/*** LOCAL FUNCTION DECLARE ***/
VD fn_sevenseg_set_output_ports(U1 u1_a_number,U1 u1_a_digit);

/*** EXTERNAL FUNCTION DEFINITION ***/
VD fn_sevenseg_init(VD){
    pinMode(SEGMENT_PIN_0, OUTPUT);
    pinMode(SEGMENT_PIN_1, OUTPUT);
    pinMode(SEGMENT_PIN_2, OUTPUT);
    pinMode(SEGMENT_PIN_3, OUTPUT);
    pinMode(SEGMENT_PIN_DIGIT, OUTPUT);
    u1_g_dislay_number = 0;
    u1_g_current_digit = 0;
}

VD fn_sevenseg_cyc(VD){
    U1 u1_t_digit_1_value;
    U1 u1_t_digit_0_value;

    // 分解表示する数字を取得
    u1_t_digit_1_value = u1_g_dislay_number / 10;
    u1_t_digit_0_value = u1_g_dislay_number % 10;

    // 現在の桁に応じて表示する数字を切り替え
    if(u1_g_current_digit == 0){
        // 1の位を表示
        fn_sevenseg_set_output_ports(u1_t_digit_0_value, u1_g_current_digit);
        u1_g_current_digit = 1;
    } else {
        // 10の位を表示
        fn_sevenseg_set_output_ports(u1_t_digit_1_value, u1_g_current_digit);
        u1_g_current_digit = 0;
    }
}

VD fn_sevenseg_set_number(U1 u1_a_number){
    if(u1_a_number >99){
        u1_g_dislay_number = 99;
    }
    else{
        u1_g_dislay_number = u1_a_number;
    }
}


/*** LOCAL FUNCTION DEFINITION ***/

VD fn_sevenseg_set_output_ports(U1 u1_a_number, U1 u1_a_digit){
    FG fg_t_0bit_pin_state;
    FG fg_t_1bit_pin_state;
    FG fg_t_2bit_pin_state;
    FG fg_t_3bit_pin_state;
    FG fg_t_digit_pin_state;

    // 数字に応じたセグメントのON/OFFを決定
    fg_t_0bit_pin_state = u1_a_number & 0b00000001;
    fg_t_1bit_pin_state = u1_a_number & 0b00000010;
    fg_t_2bit_pin_state = u1_a_number & 0b00000100;
    fg_t_3bit_pin_state = u1_a_number & 0b00001000;
    fg_t_digit_pin_state = (u1_a_digit == 0) ? HIGH : LOW;

    // セグメントのON/OFFを設定
    digitalWrite(SEGMENT_PIN_0, fg_t_0bit_pin_state);
    digitalWrite(SEGMENT_PIN_1, fg_t_1bit_pin_state);
    digitalWrite(SEGMENT_PIN_2, fg_t_2bit_pin_state);
    digitalWrite(SEGMENT_PIN_3, fg_t_3bit_pin_state);
    digitalWrite(SEGMENT_PIN_DIGIT, fg_t_digit_pin_state);    

}