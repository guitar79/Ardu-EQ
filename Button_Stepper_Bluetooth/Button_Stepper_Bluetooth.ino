#include <Wire.h>
#include <SoftwareSerial.h>
#define Clock_1 10
#define Start_Stop_1 12
#define CW_CCW_1 11
#define Clock_2 7
#define Start_Stop_2 9
#define CW_CCW_2 8
#define AnalogLimit 50
SoftwareSerial btSerial(2,13); //btSerial(arduino_rxPin,arduino_txPin)
const int microstep=4; //드라이버에서의 마이크로 스텝이 1/4라는 뜻
const int gamsokbi = 10; //감속비 : 1/10이면 10 입력
const int worm_gear_num = 144; //웜 기어 이빨 수
const float anglePerSteps = 1.8; //스텝 당 모터 회전각
const float stepsPerRevolution = worm_gear_num*360.0*gamsokbi/anglePerSteps; // 망원경이 한 바퀴 돌기 위한 스텝 수
const int speed_1 = 1000; //빨리 했을 때, 1000ms 당 개의 신호를 보낸다는 뜻 (루프가 60ms넘게 걸리기 때문에 80이상으로 해야 안전함)
const int speed_2 = 2000; //느리게 했을 때, 2000ms 당 개의 신호를 보낸다는 뜻
const int speed_earth = 299;// (int)(24*3600*1000/stepsPerRevolution); //지구의 자전에 의한 속도, int 형을 사용함에 따른 오차를 나중에 고려해야 한다//24시간-4분
const long earth_fix = 1654;//실제 계산값보다 조금 빠른 주기로 신호를 입력하므로 보정해줘야 함.
long fix_variable = 0;
const int NUM=150;
int speed_now_1;
int speed_now_2;
long now1 = 0, previous1 = 0, now2 = 0, previous2 = 0;
long timenow;
int a[4] = {}, an1 = 0, an2 = 0;
long start_time_motor1;
long start_time_motor2;
long current_time1;
long current_time2;
char com;
int check[5];
void setup()
{
    Serial.begin(9600);
    btSerial.begin(9600);
//모터1(적경)
    pinMode(Clock_1, 1); // Clock
    pinMode(Start_Stop_1, 1); // Start/ Stop
    pinMode(CW_CCW_1, 1); // CW/ CCW
//모터2(적위)
    pinMode(Clock_2, 1); // Clock
    pinMode(Start_Stop_2, 1); // Start/ Stop
    pinMode(CW_CCW_2, 1); // CW/ CCW
//입력
    pinMode(3,INPUT); // 1번 스위치 : 모터 1 정방향 회전
    pinMode(4,INPUT); // 2번 스위치 : 모터 1 역방향 회전
    pinMode(5,INPUT); // 3번 스위치 : 모터 2 정방향 회전
    pinMode(6,INPUT); // 4번 스위치 : 모터 2 역방향 회전
    pinMode(2,INPUT); // 핸드컨트롤러로 할지 블루투스를 통한 앱으로 작동할지 결정하는 스위치
    pinMode(A0,INPUT); //스위치를 담당하는 Analog핀
    digitalWrite(Start_Stop_1, 1);//?
    digitalWrite(Start_Stop_2, 1);//?
    timenow = millis();
    start_time_motor1 = millis();
    start_time_motor2 = millis();
    current_time1 = millis();
    current_time2 = millis();
    previous1 = 5;
    previous2 = 5;
}
void f(int mode1, int mode2)
{
    long time_flow_1 = millis() - start_time_motor1;
    time_flow_1 = time_flow_1/500;
    if(time_flow_1 > 3) time_flow_1 = 3;
    speed_now_1 = 4 - time_flow_1;
    long time_flow_2 = millis() - start_time_motor2;
    time_flow_2 = time_flow_2/500;
    if(time_flow_2 > 3) time_flow_2 = 3;
    speed_now_2 = 4 - time_flow_2;
    if(mode1 == 1 || mode1 == 2)
    {
        if(mode2 == 3 || mode2 == 4)
        {

            for(int i = 1 ; i <= NUM; i++)
            {
                if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 1), digitalWrite(Clock_1, 0);
                if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 1), digitalWrite(Clock_2, 0);
                delay(1);
            }
        }
        else
        {
            for(int i = 1 ; i <= NUM; i++)
            {
                if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 1), digitalWrite(Clock_1, 0);
                if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 0), digitalWrite(Clock_2, 0);
                delay(1);
            }
        }
    }
    else
    {
        if(millis() > current_time1 + speed_earth) // 적경 모터에 입력 없을 경우 (mode == 0) 지구 자전 효과 고려
        {
            current_time1 = millis();
            fix_variable++;
            if(fix_variable % earth_fix == 0)
            {
              fix_variable = 0;
              delay(microstep);
            }
            else
            {
              for(int i=0;i<microstep;i++){
                digitalWrite(Clock_1, 1), digitalWrite(Clock_1, 0);
                delay(1);
              }
            }     
        }
        if(mode2 == 3 || mode2 == 4)
        {
            for(int i = 1 ; i <= NUM; i++)
            {
                if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 0), digitalWrite(Clock_1, 0);
                if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 1), digitalWrite(Clock_2, 0);
                delay(1);
            }
        }
        else
        {
            for(int i = 1 ; i <= NUM; i++)
            {
                if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 0), digitalWrite(Clock_1, 0);
                if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 0), digitalWrite(Clock_2, 0);
                delay(1);
            }

        }
    }
}
void loop()
{
//핸드 컨트롤러
    if(analogRead(A0)<AnalogLimit){
      //모터 1
      now1 = 0;
      an1 = 0;
      for(int i = 1; i <= 2; i++)
      {
        a[i]=digitalRead(i+2);
        a[i]=1-a[i];
        if(a[i] == 1) now1 = i, an1++;
      }
      if(an1 == 2) now1 = 0;
      //모터 2
      now2 = 0;
      an2 = 0;
      for(int i = 3; i <= 4; i++)
      {
        a[i]=digitalRead(i+2);
        a[i]=1-a[i];
        if(a[i] == 1) now2 = i, an2++;
      }
      if(an2 == 2) now2 = 0;
    }
//스마트폰(by Bluetooth)
    if(analogRead(A0)>AnalogLimit){
      while(btSerial.available()){
        com=btSerial.read();
        switch(com){
          case 'a':
            check[1]=1;break;
          case 'b':
            check[1]=0;break;
          case 'c':
            check[2]=1;break;
          case 'd':
            check[2]=0;break;
          case 'e':
            check[3]=1;break;
          case 'f':
            check[3]=0;break;
          case 'g':
            check[4]=1;break;
          case 'h':
            check[4]=0;break;
        }
      }
      now1=0;now2=0;
      if((check[1]==1&&check[2]==1)||(check[3]==1&&check[4]==1)){
        now1=0;now2=0;
      }
      else{
        if(check[1]==1){
          now1=1;
        }
        else if(check[2]==1){
          now1=2;
        }
        else{
          now1=0;
        }
        
        if(check[3]==1){
          now2=3;
        }
        else if(check[4]==1){
          now2=4;
        }
        else{
          now2=0;
        }
      }
    }
    if(now1 != previous1)
    {
        digitalWrite(Start_Stop_1, 0), delay(50), digitalWrite(Start_Stop_1, 1); // Stop 했다가 다시 Start
        if(now1 == 1) digitalWrite(CW_CCW_1, 0); // 방향 설정
        else if (now1 == 2) digitalWrite(CW_CCW_1, 1);
        else digitalWrite(CW_CCW_1, 0); // 지구 자전 효과 보정 방향
        current_time1 = millis();
        start_time_motor1=millis();
    }
    previous1 = now1;
    if(now2 != previous2)
    {
        digitalWrite(Start_Stop_2, 0), delay(50), digitalWrite(Start_Stop_2, 1); // Stop 했다가 다시 Start
        if(now2 == 3) digitalWrite(CW_CCW_2, 0); // 방향 설정
        else if (now2 == 4) digitalWrite(CW_CCW_2, 1);
        current_time2 = millis();
        start_time_motor2=millis();
    }
    previous2 = now2;
    f(now1, now2);
}

