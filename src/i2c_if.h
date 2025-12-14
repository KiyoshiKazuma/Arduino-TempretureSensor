#ifndef I2C_IF_H
#define I2C_IF_H

#include "common.h"


VD fn_i2c_if_init(VD);
VD fn_i2c_if_cyc(VD);

FG fg_i2c_if_request_tx(U1 u1_a_tgt_addr, const U1 *pau1_a_buf, U1 u1_a_len);
FG fg_i2c_if_request_rx(U1 u1_a_tgt_addr, U1 *pau1_a_buf,U1 u1_a_len);
FG fg_i2c_if_is_busy(VD);

#endif /* I2C_IF_H */