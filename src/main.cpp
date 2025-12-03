#include <Arduino.h>

#include "common.h"
#include "lcd.h"

/*** MACRO DEFINITIONS ***/

/*** GLOBAL VARIABLES DEFINITION ***/

/*** LOCAL VARIABLES DEFINITION ***/
volatile unsigned long toggle_counter = 0;
FG fg_g_lcd_task_req = 0;

/*** LOCAL FUNCTION DECLARE ***/


/*** EXTERNAL FUNCTION DEFINITION ***/


/*** LOCAL FUNCTION DEFINITION ***/
// Timer1の設定関数
void setupTimer1() {
  // Timer1を停止
  TCCR1A = 0;
  TCCR1B = 0;

  // Timer1をCTCモードに設定（WGM13:0 = 0100）
  // WGM12をセット
  TCCR1B |= (1 << WGM12);

  // 比較一致レジスタA (OCR1A) に 249 を設定
  // 16MHz / 64 / (249 + 1) = 1000Hz (1ms)
  OCR1A = 249;

  // プリスケーラを64に設定し、Timer1を開始（CS11とCS10をセット）
  // CS12=0, CS11=1, CS10=1 -> プリスケーラ64
  TCCR1B |= (1 << CS11) | (1 << CS10);

  // Timer1 比較一致A (OCIE1A) 割り込みを有効化
  TIMSK1 |= (1 << OCIE1A);

  // 全体の割り込みを有効化（setup()関数外で自動的に有効化されますが、念のため）
  // sei();
}

void setup() {
  // グローバル変数の初期化
  toggle_counter = 0;
  fg_g_lcd_task_req = 0;
  
  // 下記機能の初期化
  Serial.begin(9600);
  fn_lcd_init();

  // Timer1の設定を呼び出し(計測開始)
  setupTimer1();
}

void loop() {
  if (fg_g_lcd_task_req == 1) {
    fg_g_lcd_task_req = 0;
    fn_lcd_task();
  }
}

// Timer1 比較一致A 割り込みサービスルーチン (ISR)
// 1msごとに実行されます
ISR(TIMER1_COMPA_vect) {
  // 割り込み処理：
  // 1msごとにカウンターをインクリメント
  toggle_counter++;

  if (toggle_counter >= 1000) {
    fg_g_lcd_task_req = 1;
    toggle_counter = 0;  // カウンターをリセット
  }
}