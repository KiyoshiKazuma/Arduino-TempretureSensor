#ifndef SENSOR_H
#define SENSOR_H

#include "common.h"


VD fn_sensor_init(VD);
VD fn_sensor_cyc(VD);
VD fn_sensor_begin(VD);
VD fn_sensor_stop(VD);
U1 u1_sensor_get_temperature_value_int(VD);
U1 u1_sensor_get_temperature_value_frac(VD);
U1 u1_sensor_check_state(VD);

#endif // SENSOR_H
