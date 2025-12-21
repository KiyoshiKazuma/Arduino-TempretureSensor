
/*** DESCRIPTION ***/

/*** INCLUDE ***/
#include <Arduino.h>
#include "common.h"
#include "joystick.h"
#include "i2c_if.h"

/*** MACRO DEFINITIONS ***/
#define JOYSTICK_DEBUG_LEVEL 0

#define JOYSTICK_STATE_IDLE 0
#define JOYSTICK_STATE_READ_REQ 1
#define JOYSTICK_STATE_READ 2

#define JOYSTICK_ADDRESS 0x52

/*** TYPEDEF ***/
typedef struct
{
    U1 u1_state;
    U1 u1_xdata;
    U1 u1_ydata;
    U1 u1_buttondata;
    U1 au1_row_data[3];
}ST_JOYSTICK_CTRL;

/*** GLOBAL VARIABLES DEFINITION ***/
ST_JOYSTICK_CTRL st_g_joystick_ctrl;

/*** LOCAL FUNCTION DECLARE ***/

/*** EXTERNAL FUNCTION DEFINITION ***/

VD fn_joystick_init(VD){
    st_g_joystick_ctrl.u1_state = JOYSTICK_STATE_READ_REQ;
    st_g_joystick_ctrl.u1_xdata = 0;
    st_g_joystick_ctrl.u1_ydata = 0;
    st_g_joystick_ctrl.u1_buttondata = 0;
}

VD fn_joystick_cyc(VD){
    FG fg_t_function_return;

    #if JOYSTICK_DEBUG_LEVEL >= 2
    Serial.print("JOYSTICK TASK START:");
    Serial.println(st_g_joystick_ctrl.u1_state);
    #endif //debug

    switch(st_g_joystick_ctrl.u1_state)
    {
        case JOYSTICK_STATE_IDLE:

            break;

        case JOYSTICK_STATE_READ_REQ:
            fg_t_function_return = fg_i2c_if_request_rx(JOYSTICK_ADDRESS, st_g_joystick_ctrl.au1_row_data, 3U);
            if(fg_t_function_return){
                st_g_joystick_ctrl.u1_state = JOYSTICK_STATE_READ;
            }
            break;
        
        case JOYSTICK_STATE_READ:
            st_g_joystick_ctrl.u1_xdata = st_g_joystick_ctrl.au1_row_data[0];
            st_g_joystick_ctrl.u1_ydata = st_g_joystick_ctrl.au1_row_data[1];
            st_g_joystick_ctrl.u1_buttondata = st_g_joystick_ctrl.au1_row_data[2];
            st_g_joystick_ctrl.u1_state = JOYSTICK_STATE_READ_REQ;

            #if JOYSTICK_DEBUG_LEVEL >= 1
            Serial.print("x:y:button ");
            Serial.print(st_g_joystick_ctrl.u1_xdata);
            Serial.print(":");
            Serial.print(st_g_joystick_ctrl.u1_ydata);
            Serial.print(":");
            Serial.println(st_g_joystick_ctrl.u1_buttondata);
            #endif // debug
            break;
    }
 
}

U1 u1_joycon_x_data(VD){
    return st_g_joystick_ctrl.u1_xdata;
}

U1 u1_joycon_y_data(VD){
    return st_g_joystick_ctrl.u1_ydata;
}

U1 u1_joycon_botton_data(VD){
    return st_g_joystick_ctrl.u1_buttondata;
}


/*** LOCAL FUNCTION DEFINITION ***/

