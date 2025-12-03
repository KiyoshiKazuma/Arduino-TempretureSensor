/*** DESCRIPTION ***/

/*** INCLUDE ***/
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "common.h"
#include "sensor.h"

/*** MACRO DEFINITIONS ***/
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// 最大デバイス数 (必要に応じて変更してください)
#define MAX_DEVICES 10

// タスク実行間隔 (ミリ秒)
#define SENSOR_TASK_DELAY_MS 1000

// STM状態定義
#define STM_STATE_INIT 0
#define STM_STATE_RUNNING 1
#define STM_STATE_IDLE 2

/*** GLOBAL VARIABLES DEFINITION ***/

/*** LOCAL VARIABLES DEFINITION ***/
// 検出されたデバイスのアドレスを保存する配列 (各アドレスは8バイト)
DeviceAddress ast_g_sensor_temp_sensor_address[MAX_DEVICES];

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire st_g_sensor_onewire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature st_g_sensor_sensors(&st_g_sensor_onewire);

U1 u1_g_sensor_number_of_devices; // 検出されたデバイスの総数
U1 u1_g_sensor_stm_state; // STM状態変数

/*** LOCAL FUNCTION DECLARE ***/

/*** EXTERNAL FUNCTION DEFINITION ***/
VD fn_sensor_init(VD){
    Serial.println("--- Sensor Initialization Start ---");
    u1_g_sensor_stm_state = STM_STATE_INIT; 

    // Dallas Temperatureライブラリの初期化
    st_g_sensor_sensors.begin();
}

VD fn_sensor_task(VD){
    Serial.println("Sensor Task RUNNING...");

    switch(u1_g_sensor_stm_state){
        case STM_STATE_INIT:
            // 接続されているデバイス数を取得
            u1_g_sensor_number_of_devices = st_g_sensor_sensors.getDeviceCount();
            Serial.print("Found ");
            Serial.print(u1_g_sensor_number_of_devices, DEC);
            Serial.println(" devices.");
            u1_g_sensor_stm_state = STM_STATE_IDLE;
            break;

        case STM_STATE_RUNNING:
            //do nothing
            break;

        case STM_STATE_IDLE:
            //do nothing
            break;
    }
}

/*** LOCAL FUNCTION DEFINITION ***/



/* old code



  // 接続されているデバイス数を取得
  numberOfDevices = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  if (numberOfDevices > MAX_DEVICES) {
        Serial.println("Warning: Device count exceeds MAX_DEVICES. Some devices ignored.");
  }

  // 検出されたすべてのデバイスのアドレスを取得し、保存する
    for (U1 i = 0; i < numberOfDevices && i < MAX_DEVICES; i++) {
        if (sensors.getAddress(tempSensorAddress[i], i)) {
            Serial.print("Device ");
            Serial.print(i);
            Serial.print(" Address: ");
            // アドレスをシリアルに出力する関数
            for (U1 j = 0; j < 8; j++) {
                Serial.print(tempSensorAddress[i][j], HEX);
                Serial.print(" ");
            }
            Serial.println();
        } else {
            Serial.print("Could not find address for Device #");
            Serial.println(i);
        }
    }
  Serial.println("--- Sensor Initialization Complete ---");
}

VD fn_sensor_task(VD){
// タスクのメインループ
  Serial.println("Sensor Task RUNNING...");
  //DEBUG
  U4 u4_t_startTime = xTaskGetTickCount();

  // 全デバイスに対して温度測定を要求
  sensors.requestTemperatures();

  // 検出されたすべてのデバイスの温度を取得し、出力する
  for (U1 i = 0; i < numberOfDevices && i < MAX_DEVICES; i++) {
      // アドレスを指定して温度を取得
      float temperatureC = sensors.getTempC(tempSensorAddress[i]);

      Serial.print("Device ");
      Serial.print(i);
      Serial.print(" (");

      // 簡易的なアドレス表示 (最後のバイト)
      Serial.print(tempSensorAddress[i][7], HEX);
      Serial.print("): ");

      if (temperatureC != DEVICE_DISCONNECTED_C) {
          Serial.print(temperatureC);
          Serial.println(" *C");
      } else {
          Serial.println("Error: Device disconnected or read failure.");
      }
  }
  Serial.println("--------------------");

  //DEBUG
  U4 u4_t_endTime = xTaskGetTickCount();
  Serial.print("Sensor Task Execution Time: ");
  Serial.print((u4_t_endTime - u4_t_startTime)*portTICK_PERIOD_MS);
  Serial.println(" ms");


}
*/