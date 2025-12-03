/*** include ***/
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "common.h"
#include "sensor.h"
 
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// 最大デバイス数 (必要に応じて変更してください)
#define MAX_DEVICES 10

// 検出されたデバイスのアドレスを保存する配列 (各アドレスは8バイト)
DeviceAddress tempSensorAddress[MAX_DEVICES];
U1 numberOfDevices; // 検出されたデバイスの総数

// タスク実行間隔 (ミリ秒)
#define SENSOR_TASK_DELAY_MS 1000

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int count = 0;

VD fn_sensor_init(VD){
  Serial.println("--- Sensor Initialization Start ---");

  // Dallas Temperatureライブラリの初期化
  sensors.begin();

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