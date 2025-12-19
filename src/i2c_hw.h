#ifndef I2C_HW_H
#define I2C_HW_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum
{
    I2C_HW_ST_START_OK,
    I2C_HW_ST_SLA_W_ACK,
    I2C_HW_ST_SLA_R_ACK,
    I2C_HW_ST_DATA_TX_ACK,
    I2C_HW_ST_DATA_RX_ACK,
    I2C_HW_ST_DATA_RX_NACK,
    I2C_HW_ST_ERROR
} EN_I2C_HW_STATUS;

VD fn_i2c_hw_init(VD);
VD fn_i2c_hw_send_start(VD);
VD fn_i2c_hw_send_stop(VD);
VD fn_i2c_hw_write(U1 u1_a_data);
U1 fn_i2c_hw_read_ack(VD);
U1 fn_i2c_hw_read_nack(VD);
FG fg_i2c_hw_is_complete(VD);
EN_I2C_HW_STATUS fn_i2c_hw_get_status(VD);


VD fn_i2c_hw_request_read_ack(VD);
VD fn_i2c_hw_request_read_nack(VD);
U1 fn_i2c_hw_get_data(VD);

#endif /* I2C_HW_H */

/*
□主要レジスタ
TWBR：ビットレート設定
TWSR：ステータス
TWCR：制御
TWDR：送受信データ
TWAR：自分がスレーブの時のアドレス（今回は未使用）

□TWCR：制御の各ビット
TWINT[7]: インターフェースの割り込みフラグ
    1: 処理完了
    0: 処理中
TWSTA[5]: スタートコンディション生成
    1: スタートコンディションを生成
    0: 通常動作
TWSTO[4]: ストップコンディション生成
    1: ストップコンディションを生成
    0: 通常動作 
TWEN[2]: TWIインターフェース有効化
    1: 有効化
    0: 無効化
□TWSR：ステータスの上位5ビット
0x08: スタートコンディション送信完了


*/