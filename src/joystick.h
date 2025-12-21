#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "common.h"

VD fn_joystick_init(VD);
VD fn_joystick_cyc(VD);

U1 u1_joycon_x_data(VD);
U1 u1_joycon_y_data(VD);
U1 u1_joycon_botton_data(VD);

#endif //JOYSTICK_H