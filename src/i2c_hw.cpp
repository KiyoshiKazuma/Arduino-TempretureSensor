#include <avr/io.h>
#include "i2c_hw.h"
#include "common.h"


/*!
@brief I2Cハードウェア初期化
@param なし
@return なし
@details I2Cインターフェースの初期化を行う
*/
VD fn_i2c_hw_init(VD)
{
    TWSR = 0x00;      /* prescaler = 1 */
    TWBR = 72;        /* 100kHz @16MHz */
    TWCR = (1 << TWEN);
}

/*!
@brief I2Cスタートコンディション送信
@param なし
@return なし
@details スタートコンディションを送信する
*/
VD fn_i2c_hw_send_start(VD)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
}

/*!
@brief I2Cストップコンディション送信
@param なし
@return なし
@details ストップコンディションを送信する
*/
VD fn_i2c_hw_send_stop(VD)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

/*!
@brief I2Cデータ送信
@param u1_a_data 送信データ(1バイト)
@return なし
@details 送信データ(1バイト)をI2Cバスに送信する
*/
VD fn_i2c_hw_write(U1 u1_a_data)
{
    TWDR = u1_a_data;
    TWCR = (1<<TWINT)|(1<<TWEN);
}

/*!
@brief I2Cデータ受信(ACK応答)
@param なし
@return なし
@details 送信データ(1バイト)をI2Cバスに送信する
*/
U1 fn_i2c_hw_read_ack(VD)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    return TWDR;
}

U1 fn_i2c_hw_read_nack(VD)
{
    TWCR = (1<<TWINT)|(1<<TWEN);
    return TWDR;
}

FG fg_i2c_hw_is_complete(VD)
{
    return (TWCR & (1<<TWINT)) != 0;
}

EN_I2C_HW_STATUS fn_i2c_hw_get_status(VD)
{
    switch (TWSR & 0xF8)
    {
        case 0x08: return I2C_HW_ST_START_OK;
        case 0x18: return I2C_HW_ST_SLA_W_ACK;
        case 0x40: return I2C_HW_ST_SLA_R_ACK;
        case 0x28: return I2C_HW_ST_DATA_TX_ACK;
        case 0x50: return I2C_HW_ST_DATA_RX_ACK;
        case 0x58: return I2C_HW_ST_DATA_RX_NACK;
        default:   return I2C_HW_ST_ERROR;
    }
}
