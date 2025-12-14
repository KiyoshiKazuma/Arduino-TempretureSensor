/*** DESCRIPTION ***/

/*** INCLUDE ***/
#include <Arduino.h>
#include <OneWire.h>
// About OneWire.h : https://www.pjrc.com/teensy/td_libs_OneWire.html

#include "common.h"
#include "sensor.h"

/*** MACRO DEFINITIONS ***/
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// 最大デバイス数 (必要に応じて変更してください)
#define MAX_DEVICES 10

// タスク実行間隔 (ミリ秒)
#define SENSOR_TASK_DELAY_MS 1000

// sensor state machine
// 状態遷移定義
// | 状態名 | シンボル名 | 概要 |
// |--------|------------|------|
// | 停止状態 | ST_SENSOR_STOP | センサー動作停止。何もしない。 |
// | アドレス探索状態 | ST_SENSOR_SERCH_ADDRESS | 接続されているセンサーの数と各アドレスを取得する。 |
// | 温度変換要求状態 | ST_SENSOR_CONVERT_REQUEST | センサーに温度変換を要求する。 |
// | データ読み出し要求状態 | ST_SENSOR_READDATA_REQUEST | センサーに温度情報の読み出しを要求する。 |
// | データ読み出し状態 | ST_SENSOR_READDATA_READ | センサーから温度情報を読み出し、内部変数に保存する。 |

#define ST_SENSOR_STOP 0
#define ST_SENSOR_SERCH_ADDRESS 1
#define ST_SENSOR_CONVERT_REQUEST 2
#define ST_SENSOR_READDATA_REQUEST 4
#define ST_SENSOR_READDATA_READ 5

/*** GLOBAL VARIABLES DEFINITION ***/
static U1 au1_g_sensor_address[MAX_DEVICES][8];
static U1 u1_g_sensor_count;
static S2 s2_g_temperature;
static U1 u1_g_tempreture_int;
static U1 u1_g_tempreture_frac;
static U1 u1_g_sensor_state;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

/*** LOCAL FUNCTION DECLARE ***/

/*** EXTERNAL FUNCTION DEFINITION ***/


VD fn_sensor_init(VD)
{
    U1 u1_t_cnt1;
    U1 u1_t_cnt2;

    u1_g_sensor_state = ST_SENSOR_SERCH_ADDRESS;
    u1_g_sensor_count = 0;
    s2_g_temperature = 0;
    u1_g_tempreture_int = 0;
    u1_g_tempreture_frac = 0;

    for(u1_t_cnt1 = 0; u1_t_cnt1 < MAX_DEVICES; u1_t_cnt1++)
    {
        for(u1_t_cnt2 = 0; u1_t_cnt2 < 8; u1_t_cnt2++)
        {
            au1_g_sensor_address[u1_t_cnt1][u1_t_cnt2] = 0;
        }
    }
    Serial.println("sensor: initialized");
}

VD fn_sensor_begin(VD)
{
    u1_g_sensor_state = ST_SENSOR_CONVERT_REQUEST;
}

VD fn_sensor_stop(VD)
{
    u1_g_sensor_state = ST_SENSOR_STOP;
}

U1 u1_sensor_get_temperature_value_int(VD)
{
    return u1_g_tempreture_int;
}

U1 u1_sensor_get_temperature_value_frac(VD){
    return u1_g_tempreture_frac;
}

U1 u1_sensor_check_state(VD)
{
    return u1_g_sensor_state;
}

VD fn_sensor_cyc(VD)
{
    // auto variables definetion
//    U1 au1_t_data[8];
//    FG afg_t_write_read[8];
//    U1 u1_t_length = 0;
//    U1 u1_t_temperature_lsb, u1_t_temperature_msb;
    FG fg_t_return_onewire_search;
    U1 u1_t_cnt;

    switch (u1_g_sensor_state)
    {
    case ST_SENSOR_STOP:
        // Sensor is stopped, do nothing
        break;
    
    case ST_SENSOR_SERCH_ADDRESS:
        // search address
        fg_t_return_onewire_search = oneWire.search(au1_g_sensor_address[u1_g_sensor_count]);

        if(fg_t_return_onewire_search == true) // if device found
        {
            // count up device count
            u1_g_sensor_count++;
            if(u1_g_sensor_count >= MAX_DEVICES)
            {
                Serial.println("sensor warning: max device count reached");
                u1_g_sensor_state = ST_SENSOR_STOP;
            }
        }
        else // if no more device
        {
            //print sensor address list
            Serial.println("sensor: address search completed");
            Serial.print("sensor: device count = ");
            Serial.println(u1_g_sensor_count);

            for(u1_t_cnt = 0; u1_t_cnt < u1_g_sensor_count; u1_t_cnt++)
            {
                Serial.print("address ");;
                Serial.print(u1_t_cnt);
                Serial.print(": ");
                for(U1 u1_t_cnt2 = 0; u1_t_cnt2 < 8; u1_t_cnt2++)
                {
                    if(au1_g_sensor_address[u1_t_cnt][u1_t_cnt2] < 16)
                    {
                        Serial.print("0");
                    }
                    Serial.print(au1_g_sensor_address[u1_t_cnt][u1_t_cnt2], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }

            // move to next state
            u1_g_sensor_state = ST_SENSOR_CONVERT_REQUEST;
        }
/*

    case ST_SENSOR_CONVERT_REQUEST:
        // convert request
        au1_t_data[0] = 0xCC; // SKIP ROM
        afg_t_write_read[0] = ONEWIRE_WRITE;
        au1_t_data[1] = 0x44; // CONVERT T
        afg_t_write_read[1] = ONEWIRE_WRITE;
        u1_t_length = 2;

        if (u1_onewire_request(afg_t_write_read, au1_t_data, u1_t_length) == STD_RETURN_OK)
        {
            u1_g_sensor_state = ST_SENSOR_READDATA_REQUEST;
            u1_serial_print("sensor st ST_SENSOR_READDATA");
        }
        else
        {
            u1_serial_print("sensor error: request failed");
        }

        break;

    case ST_SENSOR_READDATA_REQUEST:
        // Read data request
        au1_t_data[0] = 0xCC; // SKIP ROM
        afg_t_write_read[0] = ONEWIRE_WRITE;
        au1_t_data[1] = 0xBE; // READ SCRATCHPAD
        afg_t_write_read[1] = ONEWIRE_WRITE;
        afg_t_write_read[2] = ONEWIRE_READ;
        afg_t_write_read[3] = ONEWIRE_READ;
        afg_t_write_read[4] = ONEWIRE_READ;
        afg_t_write_read[5] = ONEWIRE_READ;
        afg_t_write_read[6] = ONEWIRE_READ;
        afg_t_write_read[8] = ONEWIRE_READ;
        afg_t_write_read[8] = ONEWIRE_READ;
        afg_t_write_read[9] = ONEWIRE_READ;
        afg_t_write_read[10] = ONEWIRE_READ;
        u1_t_length = 11;

        if (u1_onewire_request(afg_t_write_read, au1_t_data, u1_t_length) == STD_RETURN_OK)
        {
            u1_g_sensor_state = ST_SENSOR_READDATA_READ;
            u1_serial_print("sensor st ST_SENSOR_READDATA_READ");
        }
        else
        {
            u1_serial_print("sensor error: request failed");
        }

        break;
    case ST_SENSOR_READDATA_READ:
        // check read data
        if (u1_onewire_get_state() == ST_TRANSACTION_COMPLATED)
        {
            u1_onewire_read_byte(&u1_t_temperature_lsb, 2);
            u1_onewire_read_byte(&u1_t_temperature_msb, 3);
            s2_g_temperature = (u1_t_temperature_msb << 8) | u1_t_temperature_lsb;
            u1_g_tempreture_int = (U1)(s2_g_temperature >> 4);
            u1_g_tempreture_frac = (U1)((U4)(s2_g_temperature & 0x000f)*10/16);
            
            u1_g_sensor_state = ST_SENSOR_CONVERT_REQUEST;
            u1_serial_print("sensor st ST_SENSOR_STOP");
        }
        else
        {
            u1_serial_print("sensor error: read failed");
        }
            */
    }

    // serial print sensor state
    Serial.print("sensor state: ");
    Serial.println(u1_g_sensor_state);
}
