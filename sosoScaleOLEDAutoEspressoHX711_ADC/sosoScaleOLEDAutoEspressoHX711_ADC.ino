/* next:
    descard stopwatch, cause it doesn't have a mark and doesn't know how long it has been running.

*/

#include "stdlib.h"
#include <Arduino.h>
#define GRIND_COFFEE_WEIGHT 20.0

/* should know: coffeegrindweight(w0) 20.0g
   always:stopwatch(t0) dripweight(w1)
   phaseone(t1) t2:phasetwo(t0-t1) r0:ratio("1:2",w1/w0)

*/
//const float calibrationValue = 1118.68; //白色39元称参数
const float calibrationValue = 1028.52; //黑色珠宝称参数

unsigned long t0 = 0;               //开始萃取打点
unsigned long t1 = 0;               //下液第一滴打点
unsigned long t2 = 0;               //下液结束打点
float w0 = 20.0; //咖啡粉重（g）
float w1 = 0.0;                 //下液重量（g）
float r0 = 0.0;   //粉水比 w1/w0
int tareCounter = 0; //不稳计数器
float aWeight = 0;          //稳定状态比对值（g）
float aWeightDiff = 0.05;    //稳定停止波动值（g）
float atWeight = 0;         //自动归零比对值（g）
float atWeightDiff = 0.3;   //自动归零波动值（g）
float asWeight = 0;         //下液停止比对值（g）
float asWeightDiff = 0.1;   //下液停止波动值（g）
float rawWeight = 0.0;      //原始读出值（g）
unsigned long autoTareMarker = 0;       //自动归零打点
unsigned long autoStopMarker = 0;       //下液停止打点
unsigned long scaleStableMarker = 0;    //稳定状态打点
unsigned long timeOutMarker = 0;        //超时打点
unsigned long t = 0;                  //最后一次重量输出打点
unsigned long oledRefreshMarker = 0;   //最后一次oled刷新打点

const int autoTareInterval = 500;       //自动归零检测间隔（毫秒）
const int autoStopInterval = 400;       //下液停止检测间隔（毫秒）
const int scaleStableInterval = 500;   //稳定状态监测间隔（毫秒）
const int timeOutInterval = 10000;      //超时检测间隔（毫秒）
int oledPrintInterval = 50;     //oled刷新间隔（毫秒）
const int serialPrintInterval = 100;  //称重输出间隔（毫秒）


//HX711 init
#include <HX711_ADC.h>
const int HX711_dout = 3;  //mcu > HX711 dout pin
const int HX711_sck = 5;   //mcu > HX711 sck pin//HX711 constructor:
HX711_ADC scale(HX711_dout, HX711_sck);


//oled init
int FONT_SIZE = 2;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Org_01.h"
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
const int yOffset = 7; //被遮盖的部分所占像素
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <StopWatch.h>
StopWatch stopWatch;
const char minsec[] = "00:00";
char *sec2minsec(int n) {
  int minute = 0;
  int second = 0;
  if (n < 99 * 60 + 60) {
    if (n >= 60) {
      minute = n / 60;
      n = n % 60;
    }
    second = n;
  } else {
    minute = 99;
    second = 59;
  }
  snprintf(minsec, 6, "%02d:%02d", minute, second);
  //  Serial.print("Serial.println :");
  //  Serial.print(minsec);
  return minsec;
}

void oled_init() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  delay(1000);
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(FONT_SIZE);
  display.setTextColor(WHITE);
  display.setFont(&Org_01);
  display.setCursor(20, 20 + yOffset);
  display.print(F("SOFRONIO"));
  display.display();
  delay(500);
  display.setCursor(30, 35 + yOffset);
  display.print(F("APOLLO"));
  display.display();
  delay(500);
  display.setCursor(35, 50 + yOffset);
  display.print(F("SCALE"));
  display.invertDisplay(0);
  display.display();
  Serial.println(F("display up"));
}

char *ltrim(char *s)
{
  while (isspace(*s)) s++;
  return s;
}

char *rtrim(char *s)
{
  char* back = s + strlen(s);
  while (isspace(*--back));
  *(back + 1) = '\0';
  return s;
}

char *trim(char *s)
{
  return rtrim(ltrim(s));
}

int alignmentRightOffset(char *input, int leftMargin, int rightMargin ) {
  //default value
  //leftMargin 0;
  //rightMargin 3;
  int xAdv_Font = 6;
  int xAdv_Dot = 2;
  int pixels = FONT_SIZE * ((strlen(input) - 1) * xAdv_Font + xAdv_Dot);
  int Cursor_x = 128 - pixels - leftMargin - rightMargin;
  return Cursor_x;
}

int alignmentRightOffset(int input, int leftMargin, int rightMargin ) {
  //default value
  //leftMargin 0;
  //rightMargin 3;
  String s = String(input);
  char const *pchar = s.c_str();
  int xAdv_Font = 6;
  int xAdv_Dot = 2;
  int pixels = FONT_SIZE * ((strlen(pchar) - 1) * xAdv_Font + xAdv_Dot);
  int Cursor_x = 128 - pixels - leftMargin - rightMargin;
  return Cursor_x;
}


void serialKeyboardControl() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    /* setSamplesInUse HX711_ADC的数据流滑动窗口平均值的样本数
        16为原默认值
        4为较好的效果
        这个样本数同时影响tare的效果，因为：
        在ADC采样的函数void HX711_ADC::conversion24bit()中

        if (doTare)
        {
          if (tareTimes < DATA_SET)
          {
            tareTimes++;         //<--------这行会积累tare所用采样直至达到SampleInUse
          }
          else
          {
            tareOffset = smoothedData();
            tareTimes = 0;
            doTare = 0;
            tareStatus = 1;
            convRslt++;
          }
        }
    */
    if (inByte == 'a') scale.setSamplesInUse(1);
    if (inByte == 's') scale.setSamplesInUse(2);//fast!
    if (inByte == 'd') scale.setSamplesInUse(4);//guuud!
    if (inByte == 'f') scale.setSamplesInUse(8);
    if (inByte == 'g') scale.setSamplesInUse(16);
    if (inByte == 'h') scale.setSamplesInUse(32);
    if (inByte == 'j') scale.setSamplesInUse(64);
    if (inByte == 'k') scale.setSamplesInUse(128);
    if (inByte == '0') oledPrintInterval = 0;
    if (inByte == '1') oledPrintInterval = 50;
    if (inByte == '2') oledPrintInterval = 100;
    if (inByte == '3') oledPrintInterval = 200;
    if (inByte == '4') oledPrintInterval = 300;
    if (inByte == '5') oledPrintInterval = 400;
    if (inByte == '6') oledPrintInterval = 500;
    if (inByte == '7') oledPrintInterval = 600;
    if (inByte == 't') scale.tareNoDelay();               //tare
    if (inByte == '[') stopWatch.start();
    if (inByte == ']') stopWatch.stop();
    if (inByte == '\\') stopWatch.reset();
    if (inByte == 'z') {
      //resetall
      stopWatch.stop();
      stopWatch.reset();
      scale.tareNoDelay();
      t0 = 0;             //开始萃取打点
      t1 = 0;             //下液第一滴打点
      t2 = 0;             //下液结束打点
      w0 = 20.0; //咖啡粉重（g）
      w1 = 0.0;                 //下液重量（g）
      tareCounter = 0; //不稳计数器
      aWeight = 0;          //稳定状态比对值（g）
      aWeightDiff = 0.05;    //稳定停止波动值（g）
      atWeight = 0;         //自动归零比对值（g）
      atWeightDiff = 0.3;   //自动归零波动值（g）
      asWeight = 0;         //下液停止比对值（g）
      asWeightDiff = 0.1;   //下液停止波动值（g）
      rawWeight = 0.0;      //原始读出值（g）
      autoTareMarker = 0;       //自动归零打点
      autoStopMarker = 0;       //下液停止打点
      scaleStableMarker = 0;    //稳定状态打点
      timeOutMarker = 0;        //超时打点
      t = 0;                  //最后一次重量输出打点
      oledRefreshMarker = 0;   //最后一次oled刷新打点
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println(F("soso Apollo scale"));
  oled_init();

  scale.begin();
  // calibration value
  unsigned long stabilizingtime = 2000;  // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                  //set this to false if you don't want tare to be performed in the next step
  scale.start(stabilizingtime, _tare);
  scale.setCalFactor(calibrationValue);  // set calibration value (float)
  scale.setSamplesInUse(4);
  //  if (scale.getTareTimeoutFlag()) {
  //    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  //    while (1)
  //      ;
  //  } else {
  //    scale.setCalFactor(calibrationValue);  // set calibration value (float)
  //  }
  Serial.print(F("scale calibrated, sps="));
  Serial.println(scale.getSPS());
  stopWatch.setResolution(StopWatch::SECONDS);
  //给stopWatch.elapsed()赋值
  stopWatch.start();
  stopWatch.reset();
  Serial.println(F("setup done"));
}

void loop() {
  static boolean newDataReady = 0;
  static boolean scaleStable = 0;
  serialKeyboardControl();

  //获得原始数据与oled显示数据

  char coffee[10];
  if (scale.update()) newDataReady = true;
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      rawWeight = scale.getData();
      newDataReady = 0;
      t = millis();
      //-0.0 -> 0.0 正负符号稳定
      if (rawWeight > -0.15 && rawWeight < 0)
        rawWeight = 0.0;
      dtostrf(rawWeight, 8, 1, coffee);
      Serial.print(rawWeight);
      Serial.print(" ");

      if (millis() > scaleStableMarker + scaleStableInterval)
      {
        //重量稳定判断
        scaleStableMarker = millis();
        if (abs(aWeight - rawWeight) < aWeightDiff)
        {
          scaleStable = true;
          Serial.print(F("稳定 aWeight "));
          Serial.print(aWeight);
          Serial.print(F(" rawWeight "));
          Serial.print(rawWeight);
          Serial.print(F(" 差绝对值"));
          Serial.print(abs(aWeight - rawWeight));
          Serial.print(F(" 设定差"));
          Serial.println(aWeightDiff);
          aWeight = rawWeight;

          //稳定后
          //自动归零
          if (millis() > autoTareMarker + autoTareInterval)
          {
            //    Serial.print(F("逝去时间"));
            //    Serial.println(stopWatch.elapsed());
            if (t0 > 0 && tareCounter > 3)
            {

              if (t2 == 0)
              {
                t2 = millis(); //萃取完成打点
                stopWatch.stop();
                //萃取完成 单次翻转屏幕 单次固定液重
                Serial.println(F("萃取完成 单次翻转屏幕 单次固定液重"));
                display.invertDisplay(1);
                display.display();
                delay(500);
                display.invertDisplay(0);
                display.display();
                w1 = rawWeight;
              }

            }
            if (stopWatch.elapsed() == 0)
            {
              //时钟为0
              autoTareMarker = millis();
              if (rawWeight > 0.5 )
              {
                //oled重量归零
                display.invertDisplay(1);
                display.display();
                scale.tare();
                //开始计时
                display.invertDisplay(0);
                display.display();
                stopWatch.start();
                scaleStable = false;
                t0 = millis();
                Serial.println(F("正归零 开始计时 取消稳定"));
              }
              //时钟为零，负重量稳定后归零，时钟不变
              if (rawWeight < -0.5)
              {
                //负重量 归零
                scale.tare();
                Serial.println(F("负归零 不计时"));
              }
            }
            atWeight = rawWeight;
          }
        }
        else
        {
          scaleStable = false;
          if (t0 > 0)
          {
            //过滤tare环节的不稳
            if (tareCounter <= 3)
              tareCounter ++; ///tare后 遇到前三次不稳 视为稳定
            else
            {
              //tareCounter > 3 //视为开始萃取
              //萃取开始 下液重量开始计算
              w1 = rawWeight;
              t1 = millis();//第一滴下液
            }
          }
          Serial.print(F("不稳 aWeight"));
          Serial.print(aWeight);
          Serial.print(F(" rawWeight "));
          Serial.print(rawWeight);
          Serial.print(F(" 差绝对值"));
          Serial.print(abs(aWeight - rawWeight));
          Serial.print(F(" 设定差"));
          Serial.println(aWeightDiff);
          aWeight = rawWeight;
          //不稳 为负 停止计时
          //stopWatch.stop();
        }
      }

      //非阻塞tare状态检查 getTareStatus()只有完成后才会true一次 调用后即为false
      if (scale.getTareStatus() == true) {
        Serial.println(F("TareNoDelay complete"));
      }
      /*
            //自动归零
            if (millis() > autoTareMarker + autoTareInterval)
            {
              //    Serial.print(F("逝去时间"));
              //    Serial.println(stopWatch.elapsed());
              if (stopWatch.elapsed() == 0)
              {
                //时钟为0
                autoTareMarker = millis();
                if (scaleStable)
                {
                  //重量稳定
                  if (rawWeight > 0.5 )
                  {
                    //正重量
                    //oled重量归零
                    display.clearDisplay();
                    int xAdv_Font = 6;
                    int xAdv_Dot = 2;
                    int Right_Margin = 3;
                    int String_Length = FONT_SIZE * ((strlen("0.0") - 1) * xAdv_Font + xAdv_Dot);
                    int int_Cursor_x = 128 - String_Length - Right_Margin;
                    display.setCursor(int_Cursor_x , 10);
                    display.print("0.0");
                    //          display.setCursor(0, 10);
                    //          display.print("00:00");
                    display.display();
                    //重量归零
                    scale.tare();
                    //开始计时
                    stopWatch.start();
                    scaleStable = false;
                    Serial.println(F("正归零 开始计时 取消稳定"));
                  }
                  //时钟为零，负重量稳定后归零，时钟不变
                  if (rawWeight < -0.5)
                  {
                    //负重量
                    ////oled重量归零
                    display.clearDisplay();
                    int xAdv_Font = 6;
                    int xAdv_Dot = 2;
                    int Right_Margin = 3;
                    int String_Length = FONT_SIZE * ((strlen("0.0") - 1) * xAdv_Font + xAdv_Dot);
                    int int_Cursor_x = 128 - String_Length - Right_Margin;
                    display.setCursor(int_Cursor_x , 10);
                    display.print("0.0");
                    //          display.setCursor(0, 10);
                    //          display.print("00:00");
                    display.display();
                    //重量归零
                    scale.tare();
                    Serial.println(F("负归零 不计时"));
                  }
                }
              }
              atWeight = rawWeight;
            }

            //自动停止计时
            if (millis() > autoStopMarker + autoStopInterval)
            {
              if (stopWatch.isRunning())
              {
                //计时中
                autoStopMarker = millis();
                if (scaleStable)
                {
                  //重量稳定
                  if (abs(rawWeight) > 0.5)
                  {
                    //液体超过0.5g 非tare状态 时钟停止
                    stopWatch.stop();
                    Serial.print(rawWeight);
                    Serial.println(F("萃取结束 计时停止"));
                  }
                  if (rawWeight < -0.5)
                  {
                    //拿起杯子 重量为负 计时停止
                    stopWatch.stop();
                    Serial.print(rawWeight);
                    Serial.println(F("杯子拿走 计时停止"));
                  }
                }
              }
              asWeight = rawWeight;
            }
      */
    }
  }
  //  //超时 大重量归零
  //  if (scaleStable && millis() > timeOutMarker + timeOutInterval && abs(rawWeight) > 0.1)
  //  {
  //    timeOutMarker = millis();
  //    //任意时刻 稳定超过timeOutInterval秒 全部复位
  //    ////oled双归零
  //    display.clearDisplay();
  //    int xAdv_Font = 6;
  //    int xAdv_Dot = 2;
  //    int Right_Margin = 3;
  //    int String_Length = FONT_SIZE * ((strlen("0.0") - 1) * xAdv_Font + xAdv_Dot);
  //    int int_Cursor_x = 128 - String_Length - Right_Margin;
  //    display.setCursor(int_Cursor_x , 10);
  //    display.print("0.0");
  //    display.setCursor(0, 10);
  //    display.print("00:00");
  //    display.display();
  //    stopWatch.reset();
  //    scale.tareNoDelay();
  //    Serial.println(F("超时"));
  //  }


  if (millis() > oledRefreshMarker + oledPrintInterval)
  {
    //oled刷新
    oledRefreshMarker = millis();
    display.clearDisplay();
    //strncat(coffee, "g", 1); add suffix of gram
    //align to the right
    //font.h should be changed for even width. e.g.[1][.][-]etc.


    int xAdv_Font = 6;
    int xAdv_Dot = 2;
    int Right_Margin = 3;
    int String_Length = 0;
    int int_Cursor_x = 3;
    int int_Cursor_y = 0;
    int simbols = 0;

    //第1行
    int_Cursor_y = 10 + yOffset;
    FONT_SIZE = 1;
    display.setTextSize(FONT_SIZE);
    display.setCursor(0, int_Cursor_y);
    display.print("TIMER");
    simbols = 0;
    String_Length = FONT_SIZE * ((strlen("WEIGHT") - simbols) * xAdv_Font + xAdv_Dot * simbols); //dot
    int_Cursor_x = 128 - String_Length - Right_Margin;
    display.setCursor(int_Cursor_x, int_Cursor_y);
    display.print("WEIGHT");

    //第2行
    int_Cursor_y = 22 + yOffset;
    FONT_SIZE = 2;
    display.setTextSize(FONT_SIZE);
    xAdv_Font = 6;
    xAdv_Dot = 2;
    Right_Margin = 3;
    String_Length = FONT_SIZE * ((strlen(coffee) - 1) * xAdv_Font + xAdv_Dot);
    int_Cursor_x = 128 - String_Length - Right_Margin;
    display.setCursor(int_Cursor_x , int_Cursor_y);
    display.print(coffee);
    display.setCursor(0, int_Cursor_y);
    display.print(sec2minsec(stopWatch.elapsed()));
    //    int x = alignmentRightOffset(sec2minsec(stopWatch.elapsed()), 0, 3);
    //    display.setCursor(x, 40);

    //第3行
    int_Cursor_y = 47 + yOffset;
    char ratio[10];
    char weight0[10];
    if (rawWeight >= 0)
      r0 = w1 / w0;
    else
      r0 = 0;
    dtostrf(r0, 8, 1, ratio);
    dtostrf(w0, 8, 1, weight0);
    char result[30];
    sprintf(result, "%s", trim(weight0));
    display.setCursor(0, int_Cursor_y);
    display.print(trim(result));
    sprintf(result, "1:%s", trim(ratio));
    //20.0 1:2.0 3个标点
    simbols = 2;
    String_Length = FONT_SIZE * ((strlen(trim(result)) - simbols) * xAdv_Font + xAdv_Dot * simbols); //dot
    int_Cursor_x = 128 - String_Length - Right_Margin;
    display.setCursor(int_Cursor_x, int_Cursor_y);
    display.print(trim(result));

    //第4行
    int_Cursor_y = 56 + yOffset;
    FONT_SIZE = 1;
    display.setTextSize(FONT_SIZE);
    display.setCursor(0, int_Cursor_y);
    display.print("COFFEE");
    simbols = 0;
    String_Length = FONT_SIZE * ((strlen("RATIO") - simbols) * xAdv_Font + xAdv_Dot * simbols); //dot
    int_Cursor_x = 128 - String_Length - Right_Margin;
    display.setCursor(int_Cursor_x, int_Cursor_y);
    display.print("RATIO");

    display.display();
    FONT_SIZE = 2;
    display.setTextSize(FONT_SIZE);
    //Serial.println(coffee);
  }
}
