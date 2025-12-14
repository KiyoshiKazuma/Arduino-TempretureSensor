#include <Arduino.h>
#include "common.h"
#include "i2c_if.h"
#include "i2c_hw.h"

#define U1_I2C_IF_BUFFER_MAX_SIZE 16

typedef enum
{
    I2C_STATE_IDLE = 0,
    I2C_STATE_START,
    I2C_STATE_SLA,
    I2C_STATE_TX,
    I2C_STATE_RX,
    I2C_STATE_STOP,
    I2C_STATE_DONE,
    I2C_STATE_ERROR
} EN_I2C_STATE;

typedef struct
{
    EN_I2C_STATE state;
    U1 u1_a_tgt_addr;
    U1 rw;

    U1 au1_tx_buf[U1_I2C_IF_BUFFER_MAX_SIZE];
    U1 tx_len;
    U1 tx_idx;

    U1 au1_rx_buf[U1_I2C_IF_BUFFER_MAX_SIZE];
    U1 rx_len;
    U1 rx_idx;
} ST_I2C_IF_CTRL;

static ST_I2C_IF_CTRL st_g_i2c_if_ctrl;

VD fn_i2c_if_init(VD)
{
    fn_i2c_hw_init();
    st_g_i2c_if_ctrl.state = I2C_STATE_IDLE;
}

/*!
@brief I2Cデータ送信要求
@param u1_a_tgt_addr 送信先スレーブアドレス
@param pau1_a_buf 送信データバッファ配列(ポインタ)
@param u1_a_len 送信データ長(バイト数)
@return 送信要求成功:true 失敗:false
@details ストップコンディションを送信する
*/
FG fg_i2c_if_request_tx(U1 u1_a_tgt_addr, const U1 *pau1_a_buf, U1 u1_a_len)
{
    // オート変数を定義
    U1 u1_t_index;

    // 引数チェック
    
    // NULLポインタチェック
    if(pau1_a_buf == NULL)
    {
        Serial.println("I2C IF TX Request Error: Null Pointer");
        return false;
    }

    // データ長チェック    
    if(u1_a_len > U1_I2C_IF_BUFFER_MAX_SIZE)
    {
        Serial.println("I2C IF TX Request Error: Data Length Exceeds Buffer Size");
        return false;
    }    
    
    // I2Cがビジー状態の場合、送信要求を拒否
    if (st_g_i2c_if_ctrl.state != I2C_STATE_IDLE) return false;

    // 送信先のアドレスを設定
    st_g_i2c_if_ctrl.u1_a_tgt_addr = u1_a_tgt_addr;
    
    // 書き込みモードを設定
    st_g_i2c_if_ctrl.rw = 0;

    // 送信データをバッファにコピー
    for(u1_t_index = 0; u1_t_index < u1_a_len && u1_t_index < U1_I2C_IF_BUFFER_MAX_SIZE; u1_t_index++)
    {
        st_g_i2c_if_ctrl.au1_tx_buf[u1_t_index] = pau1_a_buf[u1_t_index];
    }

    // 送信データ長を設定
    st_g_i2c_if_ctrl.tx_len = u1_a_len;

    // 送信データインデックスを初期化
    st_g_i2c_if_ctrl.tx_idx = 0;

    // I2C状態をスタート状態に設定
    st_g_i2c_if_ctrl.state = I2C_STATE_START;

    // 送信要求成功
    return true;
}

FG fg_i2c_if_request_rx(U1 u1_a_tgt_addr, U1 *pau1_a_buf,U1 u1_a_len)
{
    /* TBD */
    
    //if (st_g_i2c_if_ctrl.state != I2C_STATE_IDLE) return false;
//
    //st_g_i2c_if_ctrl.u1_a_tgt_addr = u1_a_tgt_addr;
    //st_g_i2c_if_ctrl.rw = 1;
    //st_g_i2c_if_ctrl.au1_rx_buf = pau1_a_buf;
    //st_g_i2c_if_ctrl.rx_len = u1_a_len;
    //
//
    //st_g_i2c_if_ctrl.rx_idx = 0;
    //st_g_i2c_if_ctrl.state = I2C_STATE_START;
    return true;
}

FG fg_i2c_if_is_busy(VD)
{
    return (st_g_i2c_if_ctrl.state != I2C_STATE_IDLE);
}

/*!
@brief I2C_IF周期処理
@param なし
@return なし
@details I2C_IFの周期処理。
*/
VD fn_i2c_if_cyc(VD)
{
    switch (st_g_i2c_if_ctrl.state)
    {
        case I2C_STATE_IDLE:
            break;

        case I2C_STATE_START:
            // I2Cスタートコンディション送信
            fn_i2c_hw_send_start();
            st_g_i2c_if_ctrl.state = I2C_STATE_SLA;
            break;

        case I2C_STATE_SLA:
            // スタートコンディション送信完了待ち
            if (!fg_i2c_hw_is_complete()) break;
            
            // スレーブアドレス送信
            fn_i2c_hw_write((st_g_i2c_if_ctrl.u1_a_tgt_addr << 1) | st_g_i2c_if_ctrl.rw);
            st_g_i2c_if_ctrl.state = (st_g_i2c_if_ctrl.rw == 0)
                                ? I2C_STATE_TX
                                : I2C_STATE_RX;
            break;

        case I2C_STATE_TX:
            // 前回の送信完了待ち
            if (!fg_i2c_hw_is_complete()) break;

            // 送信データが残っている場合は送信(インデックスを用いて順次送信)
            if (st_g_i2c_if_ctrl.tx_idx < st_g_i2c_if_ctrl.tx_len)
            {
                // 送信データ送信
                fn_i2c_hw_write(st_g_i2c_if_ctrl.au1_tx_buf[st_g_i2c_if_ctrl.tx_idx++]);
            }
            else
            {
                // 送信データが無くなった場合はストップ状態へ遷移
                st_g_i2c_if_ctrl.state = I2C_STATE_STOP;
            }
            break;

        case I2C_STATE_RX:
            if (!fg_i2c_hw_is_complete()) break;

            if (st_g_i2c_if_ctrl.rx_idx + 1 < st_g_i2c_if_ctrl.rx_len)
            {
                st_g_i2c_if_ctrl.au1_rx_buf[st_g_i2c_if_ctrl.rx_idx++] = fn_i2c_hw_read_ack();
            }
            else
            {
                st_g_i2c_if_ctrl.au1_rx_buf[st_g_i2c_if_ctrl.rx_idx++] = fn_i2c_hw_read_nack();
                st_g_i2c_if_ctrl.state = I2C_STATE_STOP;
            }
            break;

        case I2C_STATE_STOP:
            // ストップコンディション送信
            fn_i2c_hw_send_stop();
            st_g_i2c_if_ctrl.state = I2C_STATE_DONE;
            break;

        case I2C_STATE_DONE:
            st_g_i2c_if_ctrl.state = I2C_STATE_IDLE;
            break;

        case I2C_STATE_ERROR:
        default:
            fn_i2c_hw_send_stop();
            st_g_i2c_if_ctrl.state = I2C_STATE_IDLE;
            break;
    }
}