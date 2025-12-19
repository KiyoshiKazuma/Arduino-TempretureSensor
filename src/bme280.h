#ifndef BME280_H
#define BME280_H

VD fn_bme280_init(VD);
VD fn_bme280_cyc(VD);
S4 fn_bme280_get_temperature(VD);

#endif // BME280_H