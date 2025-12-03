#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "common.h"
#include "lcd.h"

#define SCREEN_WIDTH 128  // OLEDの横幅（ピクセル）
#define SCREEN_HEIGHT 64  // OLEDの縦幅（ピクセル）

// I2C接続を使用する場合
#define OLED_RESET -1  // リセットピンは不要なので-1
// 0x3C は一般的なI2Cアドレスですが、モジュールによっては 0x3D の場合もあります。
#define SCREEN_ADDRESS 0x3C

// STM状態定義
#define STM_STATE_INIT 0
#define STM_STATE_CLEAR 1
#define STM_STATE_RUNNING 2
#define STM_STATE_IDLE 3
#define STM_STATE_ERROR 4

U1 u1_g_lcd_state;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

VD fn_lcd_init(VD) {
  u1_g_lcd_state = STM_STATE_INIT;
  Serial.begin(9600);
  Serial.println("LCD Init Function Called");
}

VD fn_lcd_task(VD) {
  Serial.print("LCD Task Running...\n");
  switch (u1_g_lcd_state) {
    case STM_STATE_INIT:
      if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306の割り当てに失敗しました"));
      } else {
        Serial.println(F("SSD1306の割り当てに成功しました"));
        u1_g_lcd_state = STM_STATE_CLEAR;
      }
      break;
    case STM_STATE_CLEAR:
      // 画面全体をクリア
      display.clearDisplay();
      display.display();
      u1_g_lcd_state = STM_STATE_RUNNING;
      break;
    case STM_STATE_RUNNING:
      // テキスト色を設定 (白)
      display.setTextColor(SSD1306_WHITE);
      // テキストの大きさを設定 (1x=標準サイズ、2x=2倍サイズ)
      display.setTextSize(1);
      // カーソル位置を設定 (左上隅)
      display.setCursor(0, 0);
      // 1行目のテキストを表示
      display.println("SSD1306 OLED Test");

      // テキストサイズを変更して2行目を表示
      display.println("Hello!");

      display.print(AU1_SOFTVERSION);

      // 描画した内容を物理ディスプレイに表示
      display.display();
      u1_g_lcd_state = STM_STATE_IDLE;
      break;
    case STM_STATE_IDLE:
      // 何もしないで待機
      break;
  }
}