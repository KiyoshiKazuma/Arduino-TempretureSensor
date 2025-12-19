#include <Arduino.h>
#include "common.h"
#include "i2c_if.h"
#include "bme280.h"

//　デバッグレベル
#define BME280_DEBUG_LEVEL 0

// I2Cスレーブアドレス
#define BME280_SLAVE_ADDR   0x76

// レジスタアドレス
#define BME280_REG_ID           0xD0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7

// 補正パラメータの開始アドレスと長さ
#define BME280_REG_CALIB_T_P_START  0x88 // T & P Calib Data (26 Bytes)
#define BME280_LEN_CALIB_T_P        26
#define BME280_REG_CALIB_H1         0xA1 // H1 Calib Data (1 Byte)
#define BME280_LEN_CALIB_H1         1
#define BME280_REG_CALIB_H2_H6_START 0xE1 // H2-H6 Calib Data (7 Bytes)
#define BME280_LEN_CALIB_H2_H6      7

// 測定値レジスタの長さ (Press(3) + Temp(3) + Hum(2) = 8 Bytes)
#define BME280_LEN_MEASUREMENT      8

// 状態遷移の定義
#define BME280_STATE_INIT 0    
#define BME280_STATE_IDLE 1    
// 補正パラメータ読み出しフェーズ
#define BME280_STATE_CALIB_T_P_REQ 2         // 温度・気圧補正パラメータ読み出し要求
#define BME280_STATE_CALIB_T_P_WAIT 3        // 待機
#define BME280_STATE_CALIB_H1_REQ 4          // 湿度補正パラメータH1読み出し要求
#define BME280_STATE_CALIB_H1_WAIT 5         // 待機
#define BME280_STATE_CALIB_H2_H6_REQ 6       // 湿度補正パラメータH2-H6読み出し要求
#define BME280_STATE_CALIB_H2_H6_WAIT 7      // 待機
#define BME280_STATE_CALIB_DONE 8            // 完了    
// 制御レジスタ書き込みフェーズ (初期設定)
#define BME280_STATE_CTRL_HUM_REQ 9         // ctrl_hum書き込み要求
#define BME280_STATE_CTRL_HUM_WAIT 10   
#define BME280_STATE_CONFIG_REQ 11           // config書き込み要求
#define BME280_STATE_CONFIG_WAIT 12   
#define BME280_STATE_CTRL_MEAS_REQ 13        // ctrl_meas書き込み要求 (Normal mode開始)
#define BME280_STATE_CTRL_MEAS_WAIT 14       // 測定値読み出しフェーズ
#define BME280_STATE_MEAS_REQ 15             // 測定値読み出し要求
#define BME280_STATE_MEAS_WAIT 16   
#define BME280_STATE_ERROR 17   

// 制御構造体
typedef struct
{
    U1 state;

    // I2C通信バッファ (最大長 + 1(レジスタアドレス用))
    U1 au1_i2c_tx_buf[BME280_LEN_MEASUREMENT + 1];
    U1 au1_i2c_rx_buf[BME280_LEN_CALIB_T_P]; // 最大の長さに合わせる
    
    // 補正パラメータ格納用
    U1 au1_calib_t_p[BME280_LEN_CALIB_T_P];
    U1 au1_calib_h[BME280_LEN_CALIB_H1 + BME280_LEN_CALIB_H2_H6];

    // 測定生データ格納用
    U1 au1_raw_measurement[BME280_LEN_MEASUREMENT];

} ST_BME280_CTRL;

// グローバル制御構造体
static ST_BME280_CTRL st_g_bme280_ctrl;

VD fn_bme280_state_change(VD);

// BME280モジュールの初期化
VD fn_bme280_init(VD)
{
    st_g_bme280_ctrl.state = BME280_STATE_INIT;
}

// BME280の状態遷移を処理する周期関数
VD fn_bme280_cyc(VD)
{
    U1 u1_t_start_state;

    u1_t_start_state = st_g_bme280_ctrl.state;

    switch (st_g_bme280_ctrl.state)
    {
        case BME280_STATE_INIT:
            // I2C通信が空くまで待つ
            if (fg_i2c_if_is_busy()) break;

            // 初期設定の開始 (補正パラメータ読み出しから)
            st_g_bme280_ctrl.state = BME280_STATE_CALIB_T_P_REQ;
            

            break;

        // --- 補正パラメータ読み出しフェーズ ---

        case BME280_STATE_CALIB_T_P_REQ:
            // Register Address + Read (リピートスタート)
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, (const U1[]){BME280_REG_CALIB_T_P_START}, 1))
            {
                // 送信要求成功後、I2C完了待ちへ
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_T_P_WAIT;
                
            }
            break;
            
        case BME280_STATE_CALIB_T_P_WAIT:
            if (fg_i2c_if_is_busy()) break; // I2C送信(アドレス)完了待ち
            
            // I2C送信が完了したら、受信要求
            if (fg_i2c_if_request_rx(BME280_SLAVE_ADDR, st_g_bme280_ctrl.au1_calib_t_p, BME280_LEN_CALIB_T_P))
            {
                
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_H1_REQ;
                
            }
            break;

        case BME280_STATE_CALIB_H1_REQ:
            if (fg_i2c_if_is_busy()) break;
            
            // Register Address + Read (リピートスタート)
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, (const U1[]){BME280_REG_CALIB_H1}, 1))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_H1_WAIT;
                
            }
            break;
            
        case BME280_STATE_CALIB_H1_WAIT:
            if (fg_i2c_if_is_busy()) break;
            
            // 受信要求
            if (fg_i2c_if_request_rx(BME280_SLAVE_ADDR, &st_g_bme280_ctrl.au1_calib_h[0], BME280_LEN_CALIB_H1))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_H2_H6_REQ;
                
            }
            break;

        case BME280_STATE_CALIB_H2_H6_REQ:
            if (fg_i2c_if_is_busy()) break;
            
            // Register Address + Read (リピートスタート)
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, (const U1[]){BME280_REG_CALIB_H2_H6_START}, 1))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_H2_H6_WAIT;
                
            }
            break;
            
        case BME280_STATE_CALIB_H2_H6_WAIT:
            if (fg_i2c_if_is_busy()) break;
            
            // 受信要求 (H1の次のインデックスから格納)
            if (fg_i2c_if_request_rx(BME280_SLAVE_ADDR, &st_g_bme280_ctrl.au1_calib_h[BME280_LEN_CALIB_H1], BME280_LEN_CALIB_H2_H6))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CALIB_DONE;
                
            }
            break;

        case BME280_STATE_CALIB_DONE:
            // 補正パラメータの読み出し完了 -> 初期設定へ
            st_g_bme280_ctrl.state = BME280_STATE_CTRL_HUM_REQ;
            
            break;

        // --- 制御レジスタ書き込みフェーズ ---

        case BME280_STATE_CTRL_HUM_REQ:
            if (fg_i2c_if_is_busy()) break;
            // ctrl_hum = 0x01 (osrs_h x1)
            st_g_bme280_ctrl.au1_i2c_tx_buf[0] = BME280_REG_CTRL_HUM;
            st_g_bme280_ctrl.au1_i2c_tx_buf[1] = 0x01; 
            
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, st_g_bme280_ctrl.au1_i2c_tx_buf, 2))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CTRL_HUM_WAIT;
                
            }
            break;
            
        case BME280_STATE_CTRL_HUM_WAIT:
            if (fg_i2c_if_is_busy()) break;
            st_g_bme280_ctrl.state = BME280_STATE_CONFIG_REQ;
            
            break;
            
        case BME280_STATE_CONFIG_REQ:
            if (fg_i2c_if_is_busy()) break;
            // config = 0x00 (t_sb=0.5ms, filter=off)
            st_g_bme280_ctrl.au1_i2c_tx_buf[0] = BME280_REG_CONFIG;
            st_g_bme280_ctrl.au1_i2c_tx_buf[1] = 0x00;
            
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, st_g_bme280_ctrl.au1_i2c_tx_buf, 2))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CONFIG_WAIT;
                
            }
            break;
            
        case BME280_STATE_CONFIG_WAIT:
            if (fg_i2c_if_is_busy()) break;
            st_g_bme280_ctrl.state = BME280_STATE_CTRL_MEAS_REQ;
            
            break;
            
        case BME280_STATE_CTRL_MEAS_REQ:
            if (fg_i2c_if_is_busy()) break;
            // ctrl_meas = 0x25 (osrs_t=x1, osrs_p=x1, Normal mode)
            st_g_bme280_ctrl.au1_i2c_tx_buf[0] = BME280_REG_CTRL_MEAS;
            st_g_bme280_ctrl.au1_i2c_tx_buf[1] = 0x25; 
            
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, st_g_bme280_ctrl.au1_i2c_tx_buf, 2))
            {
                st_g_bme280_ctrl.state = BME280_STATE_CTRL_MEAS_WAIT;
                
            }
            break;
            
        case BME280_STATE_CTRL_MEAS_WAIT:
            if (fg_i2c_if_is_busy()) break;
            // 初期設定完了 -> 定期的な測定値読み出しへ
            st_g_bme280_ctrl.state = BME280_STATE_MEAS_REQ;
            
            break;

        // --- 測定値読み出しフェーズ ---
        
        case BME280_STATE_MEAS_REQ:
            if (fg_i2c_if_is_busy()) break;
            
            // レジスタアドレス送信 (0xF7: PRESS_MSB)
            if (fg_i2c_if_request_tx(BME280_SLAVE_ADDR, (const U1[]){BME280_REG_PRESS_MSB}, 1))
            {
                st_g_bme280_ctrl.state = BME280_STATE_MEAS_WAIT;
                
            }
            break;
            
        case BME280_STATE_MEAS_WAIT:
            if (fg_i2c_if_is_busy()) break;
            
            // 測定値8バイト受信要求
            if (fg_i2c_if_request_rx(BME280_SLAVE_ADDR, st_g_bme280_ctrl.au1_raw_measurement, BME280_LEN_MEASUREMENT))
            {
                // 受信要求が成功したら、次の周期で再度MEAS_REQへ遷移
                // データの格納は完了しているので、次のI2C通信のためにIDLEへ
                st_g_bme280_ctrl.state = BME280_STATE_IDLE; 

                #if BME280_DEBUG_LEVEL >= 1
                Serial.print("BME280 MEASUREMENT DATA : ");
                for(int i = 0; i < BME280_LEN_MEASUREMENT; i++){
                    Serial.print(st_g_bme280_ctrl.au1_raw_measurement[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                #endif //debug
                
            }
            break;

        // --- 共通状態 ---
        case BME280_STATE_IDLE:
            // 一定時間後に測定値の読み出しを再開
            // ここでは簡易的に、次の周期タスクで即座に再開としています
            st_g_bme280_ctrl.state = BME280_STATE_CTRL_MEAS_REQ;
            
            break;
            
        case BME280_STATE_ERROR:
        default:
            // エラー処理 (ここでは初期化状態に戻す)
            st_g_bme280_ctrl.state = BME280_STATE_INIT;
            break;
    }

    if(u1_t_start_state != st_g_bme280_ctrl.state)
    {
        fn_bme280_state_change();
    }
}

VD fn_bme280_state_change(VD){
#if BME280_DEBUG_LEVEL >= 2
    Serial.print("BME280 State Change to : ");
    Serial.println(st_g_bme280_ctrl.state);
#endif
}

/**
 * @brief  BME280 温度物理量取得関数
 * @return S4 摂氏温度（単位：0.01℃ / 例：2505 は 25.05℃ を表す）
 * ※データが未取得の場合は 0 を返します。
 */
S4 fn_bme280_get_temperature(VD)
{
    S4 s4_t_adc;
    S4 s4_t_var1, s4_t_var2, s4_t_fine, s4_t_temp;
    U2 u2_t_dig_t1;
    S2 s2_t_dig_t2, s2_t_dig_t3;

    /* 1. 測定が完了しているか（IDLE状態以降か）確認 */
    if (st_g_bme280_ctrl.state < BME280_STATE_IDLE)
    {
        return 0;
    }

    /* 2. 温度ADC値の抽出 (20bit) */
    /* au1_raw_measurement[0-2]は気圧、[3-5]が温度 */
    s4_t_adc = (S4)((((U4)st_g_bme280_ctrl.au1_raw_measurement[3]) << 12) |
                   (((U4)st_g_bme280_ctrl.au1_raw_measurement[4]) << 4)  |
                   (((U4)st_g_bme280_ctrl.au1_raw_measurement[5]) >> 4));

    /* 3. 補正パラメータの抽出 (Little Endian) */
    u2_t_dig_t1 = (U2)((st_g_bme280_ctrl.au1_calib_t_p[1] << 8) | st_g_bme280_ctrl.au1_calib_t_p[0]);
    s2_t_dig_t2 = (S2)((st_g_bme280_ctrl.au1_calib_t_p[3] << 8) | st_g_bme280_ctrl.au1_calib_t_p[2]);
    s2_t_dig_t3 = (S2)((st_g_bme280_ctrl.au1_calib_t_p[5] << 8) | st_g_bme280_ctrl.au1_calib_t_p[4]);

    /* 4. データシート記載の補正演算 (32bit版) */
    s4_t_var1 = ((((s4_t_adc >> 3) - ((S4)u2_t_dig_t1 << 1))) * ((S4)s2_t_dig_t2)) >> 11;

    s4_t_var2 = (((((s4_t_adc >> 4) - ((S4)u2_t_dig_t1)) * ((s4_t_adc >> 4) - ((S4)u2_t_dig_t1))) >> 12) * ((S4)s2_t_dig_t3)) >> 14;

    s4_t_fine = s4_t_var1 + s4_t_var2;
    s4_t_temp = (s4_t_fine * 5 + 128) >> 8;

    return s4_t_temp;
}
