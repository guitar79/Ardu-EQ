#include <Wire.h>
#define Clock_1 10
#define Start_Stop_1 12
#define CW_CCW_1 11
#define Clock_2 7
#define Start_Stop_2 9
#define CW_CCW_2 8
const int gamsokbi = 10; //감속비 : 1/10이면 10 입력
const int worm_gear_num = 144; //웜 기어 이빨 수
const float anglePerSteps = 1.8; //스텝 당 모터 회전각
const float stepsPerRevolution = worm_gear_num*360.0*gamsokbi/anglePerSteps; // 망원경이 한 바퀴 돌기 위한 스텝 수
const int speed_1 = 1000; //빨리 했을 때, 1000ms 당 개의 신호를 보낸다는 뜻 (루프가 60ms넘게 걸리기 때문에 80이상으로 해야 안전함)
const int speed_2 = 2000; //느리게 했을 때, 2000ms 당 개의 신호를 보낸다는 뜻
const int speed_earth = 299; // (int)(24*3600*1000/stepsPerRevolution); //지구의 자전에 의한 속도, int 형을 사용함에 따른 오차를 나중에 고려해야 한다//24시간-4분
int speed_now_1;
int speed_now_2;
int now1 = 0, previous1 = 0, now2 = 0, previous2 = 0;
long timenow;
int a[4] = {}, an1 = 0, an2 = 0;
long start_time_moter1;
long start_time_moter2;
long current_time1;
long current_time2;
void setup() {
Serial.begin(9600);
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
/*
setSyncProvider(RTC.get);
if(timeStatus()!=timeSet)
Serial.println("Unable to sync with the RTC");
else
Serial.println("RTC has set the system line");
*/
digitalWrite(Start_Stop_1, 1);//?
digitalWrite(Start_Stop_2, 1);//?
timenow = millis();
start_time_moter1 = millis();
start_time_moter2 = millis();
current_time1 = millis();
current_time2 = millis();
previous1 = 5;
previous2 = 5;
}

void prt1()
{
if(an1 == 1)
{ // 입력 신호가 하나인 경우, now 에 입력되고 있는 스위치 번호 입력되어 있음 1, 2, 3, 4
if(now1 == 1) Serial.println("\"Only switch number 1 is on (moter 1)\"");
else Serial.println("\"Only switch number 2 is on (moter 1)\"");
}
else if(an1 == 2) Serial.println("\"two switches are on (moter 1)\"");
else Serial.println("\"no switch is on (moter 1)\"");
}

void prt2()
{
if(an2 == 1)
{ // 입력 신호가 하나인 경우, now 에 입력되고 있는 스위치 번호 입력되어 있음 1, 2, 3, 4
if(now2 == 3) Serial.println("\"Only switch number 3 is on (moter 2)\"");
else Serial.println("\"Only switch number 4 is on (moter 2)\"");
}
else if(an2 == 2) Serial.println("\"two switches are on (moter 2)\"");
else Serial.println("\"no switch is on (moter 2)\"");
}
void f(int mode1, int mode2)
{
int time_flow_1 = millis() - start_time_moter1;
time_flow_1 = time_flow_1/500;
if(time_flow_1 > 3) time_flow_1 = 3;
speed_now_1 = 4 - time_flow_1;
int time_flow_2 = millis() - start_time_moter2;
time_flow_2 = time_flow_2/500;
if(time_flow_2 > 3) time_flow_2 = 3;
speed_now_2 = 4 - time_flow_2;
if(mode1 == 1 || mode1 == 2)
{
if(mode2 == 3 || mode2 == 4)
{

for(int i = 1 ; i <= 100; i++)
{
if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 1), digitalWrite(Clock_1, 0);
if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 1), digitalWrite(Clock_2, 0);
delay(1);
}

}
else
{
for(int i = 1 ; i <= 100; i++)
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
digitalWrite(Clock_1, 1), digitalWrite(Clock_1, 0);
delay(1);
}

if(mode2 == 3 || mode2 == 4)
{

for(int i = 1 ; i <= 100; i++)
{
if(i % speed_now_1 == 0 ) digitalWrite(Clock_1, 0), digitalWrite(Clock_1, 0);
if(i % speed_now_2 == 0 ) digitalWrite(Clock_2, 1), digitalWrite(Clock_2, 0);
delay(1);
}

}
else
{
for(int i = 1 ; i <= 100; i++)
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
Serial.println(millis());
//while(millis() < timenow + 1000) {}
//timenow = millis(); // 1000ms에 한 번씩 loop가 실행됨.
/////////////////////////////////////////////////////////////
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
prt1(); // 상태 출력 함수
if(now1 != previous1)
{
digitalWrite(Start_Stop_1, 0), delay(50), digitalWrite(Start_Stop_1, 1); // Stop 했다가 다시 Start
if(now1 == 1) digitalWrite(CW_CCW_1, 1); // 방향 설정
else if (now1 == 2) digitalWrite(CW_CCW_1, 0);
else digitalWrite(CW_CCW_1, 1); // 지구 자전 효과 보정 방향
current_time1 = millis();
start_time_moter1=millis();
}
previous1 = now1;
/////////////////////////////////////////////////////////////
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
prt2(); // 상태 출력 함수
if(now2 != previous2)
{
digitalWrite(Start_Stop_2, 0), delay(50), digitalWrite(Start_Stop_2, 1); // Stop 했다가 다시 Start
if(now2 == 3) digitalWrite(CW_CCW_2, 1); // 방향 설정
else if (now2 == 4) digitalWrite(CW_CCW_2, 0);
current_time2 = millis();
start_time_moter2=millis();
}
previous2 = now2;
f(now1, now2);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
