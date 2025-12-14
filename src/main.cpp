#include <Arduino.h>

#include "common.h"
#include "i2c_if.h"
//#include "lcd.h"
//#include "sensor.h"
#include "sevenseg.h"

/*** MACRO DEFINITIONS ***/
#define U1_SENSOR_TASK 1
#define U1_LCD_TASK 2
#define U1_SEVENSEG_TASK 3
#define U1_I2C_TASK 4

//DEBUG LEVEL
//0: NO DEBUG
//1: BASIC DEBUG
//2: DETAILED DEBUG
#define DEBUG_LEVEL 0

/*** GLOBAL VARIABLES DEFINITION ***/

/*** LOCAL VARIABLES DEFINITION ***/
volatile unsigned long toggle_counter = 0;
FG fg_g_lcd_task_req = 0;
FG fg_g_sensor_task_req = 0;
FG fg_g_sevenseg_task_req = 0;
FG fg_g_i2c_task_req = 0;
FG fg_g_debug_req = 0;

U4 u4_g_startTime;
U4 u4_g_endTime;

/*** LOCAL FUNCTION DECLARE ***/
VD fn_main_task_start(U1 u1_task_id);
VD fn_main_task_end(U1 u1_task_id);

VD fn_main_debug_cyc(VD);

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
  fg_g_sensor_task_req = 0;
  fg_g_sevenseg_task_req = 0;
  fg_g_debug_req = 0;
  
  // 下記機能の初期化
  Serial.begin(9600);
  Serial.print("Software Version: ");
  Serial.println(AU1_SOFTVERSION);
  Serial.println("System Init Start");

  fn_sevenseg_init();
  fn_i2c_if_init();
  //fn_lcd_init();
  //fn_sensor_init();

  // Timer1の設定を呼び出し(計測開始)
  setupTimer1();
  
  Serial.println("System Init Complete");
}

void loop() {

//  if (fg_g_lcd_task_req == 1) {
//    fg_g_lcd_task_req = 0;
//    fn_main_task_start(U1_LCD_TASK);
//    fn_lcd_task();
//    fn_main_task_end(U1_LCD_TASK);
//  }
//  if (fg_g_sensor_task_req == 1) {
//    fg_g_sensor_task_req = 0;
//    fn_main_task_start(U1_SENSOR_TASK);
//    fn_sensor_cyc();
//    fn_main_task_end(U1_SENSOR_TASK);
//  }
  if(fg_g_sevenseg_task_req == 1){
    fg_g_sevenseg_task_req = 0;
    fn_main_task_start(U1_SEVENSEG_TASK);
    fn_sevenseg_cyc();
    fn_main_task_end(U1_SEVENSEG_TASK);
  }

  if(fg_g_i2c_task_req == 1){
    fg_g_i2c_task_req = 0;
    fn_main_task_start(U1_I2C_TASK);
    fn_i2c_if_cyc();
    fn_main_task_end(U1_I2C_TASK);
  }

  if(fg_g_debug_req == 1){
    fg_g_debug_req = 0;
    // debug process
    fn_main_debug_cyc();
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
    fg_g_sensor_task_req = 1;
    fg_g_debug_req = 1;
    toggle_counter = 0;  // カウンターをリセット
  }
  fg_g_sevenseg_task_req = 1;
  fg_g_i2c_task_req = 1;
}

VD fn_main_task_start(U1 u1_task_id) {
#if DEBUG_LEVEL >= 1
  if(u1_task_id == U1_LCD_TASK)
  {
      Serial.println("LCD Task Start");
  }
  else if(u1_task_id == U1_SENSOR_TASK)
  {
    Serial.println("Sensor Task Start");
  }
  else if(u1_task_id == U1_SEVENSEG_TASK)
  {
    Serial.println("Seven Segment Task Start");
  }
  else if(u1_task_id == U1_I2C_TASK)
  {
    Serial.println("I2C Task Start");
  }
  u4_g_startTime = micros();
#endif
}

VD fn_main_task_end(U1 u1_task_id) {
#if DEBUG_LEVEL >= 1
  u4_g_endTime = micros();
  if(u1_task_id == U1_LCD_TASK)
      Serial.print("LCD Task Execution Time: ");
  else if(u1_task_id == U1_SENSOR_TASK){
    Serial.print("Sensor Task Execution Time: ");
  }
  else if(u1_task_id == U1_SEVENSEG_TASK){
    Serial.print("Seven Segment Task Execution Time: ");
  }
  else if(u1_task_id == U1_I2C_TASK){
    Serial.print("I2C Task Execution Time: ");
  }

  Serial.print(u4_g_endTime - u4_g_startTime);
  Serial.println(" us");
#endif
}


VD fn_main_debug_cyc(VD){
  Serial.println("Debug Cycle Start");

  /* seven segment debug */
  static U1 u1_s_debug_sevenseg_cnt = 0;

  // set number to seven segment display
  Serial.print("Seven Segment Display Number: ");
  Serial.println(u1_s_debug_sevenseg_cnt);
  fn_sevenseg_set_number(u1_s_debug_sevenseg_cnt);

  u1_s_debug_sevenseg_cnt++;
  if(u1_s_debug_sevenseg_cnt >= 100){
    u1_s_debug_sevenseg_cnt = 0;
  }

  /* i2c debug */
  U1 u1_t_debug_i2c_tgt_addr = 0x50;
  U1 au1_t_debug_i2c_data[11] = {"Hello I2C"};
  U1 u1_t_debug_i2c_len=11;
  FG fg_t_debug_i2c_res;

  Serial.print("I2C TX Request to Address: 0x");
  Serial.println(u1_t_debug_i2c_tgt_addr, HEX);

  fg_t_debug_i2c_res = fg_i2c_if_request_tx(u1_t_debug_i2c_tgt_addr, au1_t_debug_i2c_data, u1_t_debug_i2c_len);

  if(fg_t_debug_i2c_res == true){
    Serial.println("I2C TX Request Success");
  }else{
    Serial.println("I2C TX Request Failed");
  }
  
  fg_t_debug_i2c_res = fg_i2c_if_request_tx(u1_t_debug_i2c_tgt_addr, au1_t_debug_i2c_data, u1_t_debug_i2c_len);

  if(fg_t_debug_i2c_res == true){
    Serial.println("I2C TX Request Success");
  }else{
    Serial.println("I2C TX Request Failed");
  }

}