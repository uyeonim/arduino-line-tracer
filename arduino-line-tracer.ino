// 급커브 전용 - 역방향 차동 회전 + 교차점 직진 + 좌우 비대칭 튜닝
// ===== 교차로 강제 직진 강화 + 역회전 킥스타트 버전 =====

// ===== 핀 설정 =====
int RightMotor_E_pin = 5;
int LeftMotor_E_pin  = 6;
int RightMotor_1_pin = 8;
int RightMotor_2_pin = 9;
int LeftMotor_3_pin  = 10;
int LeftMotor_4_pin  = 11;
int L_Line = A5;
int C_Line = A4;
int R_Line = A3;

// ===== 속도 =====
int base_Speed = 130;
int turn_Speed_L = 200;
int turn_Speed_R = 220;
int blind_time_L = 300;
int blind_time_R = 300;
int forward_boost_time = 60;
int cross_detect_speed = 80;       // 교차로 감지 구간 감속값

// ===== 킥스타트 설정 =====
int kickstart_pwm  = 255;
int kickstart_time = 40;
bool rightKickDone = false;
bool leftKickDone  = false;
unsigned long rightKickStart = 0;
unsigned long leftKickStart  = 0;

// ===== 상태 변수 =====
int SL=1, SC=1, SR=1;
int turnState = 0;
unsigned long pivotStartTime   = 0;
unsigned long forwardStartTime = 0;

// ===== 연속 감지 카운터 (회전용) =====
int centerConfirmCount = 0;
int centerConfirmThreshold = 12;

// ===== 교차로 전용 변수 =====
int crossConfirmCount = 0;
const int crossConfirmThreshold  = 4;  // 2 유지
bool inCrossing = false;
unsigned long crossStartTime = 0;
const int cross_straight_time = 370;

// =========================================
void setup() {
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(LeftMotor_E_pin,  OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin,  OUTPUT);
  pinMode(LeftMotor_4_pin,  OUTPUT);
  Serial.begin(9600);
}

// =========================================
void loop() {
  int L = digitalRead(L_Line);
  int C = digitalRead(C_Line);
  int R = digitalRead(R_Line);

  // ===== [최우선 1] 교차로 강제 직진 통과 중 =====
  if(inCrossing){
    drive_pwm(base_Speed, base_Speed);
    if(millis() - crossStartTime > cross_straight_time){
      inCrossing = false;
      crossConfirmCount = 0;
      turnState = 0;
    }
    return;
  }

  // ===== [최우선 2] 교차로 연속 감지 (감속 추가) =====
  if(L==LOW && C==LOW && R==LOW){
    drive_pwm(cross_detect_speed, cross_detect_speed);  // 감속으로 놓치지 않게
    crossConfirmCount++;
    if(crossConfirmCount >= crossConfirmThreshold){
      inCrossing = true;
      crossStartTime = millis();
      turnState = 0;
      centerConfirmCount = 0;
      crossConfirmCount  = 0;
    }
    SL=L; SC=C; SR=R;
    return;
  } else {
    crossConfirmCount = 0;
  }

  // ===== 회전 후 직진 안정화 =====
  if(turnState == 4){
    drive_pwm(base_Speed, base_Speed);
    if(millis() - forwardStartTime > forward_boost_time){
      turnState = 0;
    }
    return;
  }

  // ===== 우 pivot =====
  if(turnState == 1){
    pivot_right_run();
    if(millis() - pivotStartTime > blind_time_R){
      if(C == LOW){
        centerConfirmCount++;
      } else {
        centerConfirmCount = 0;
      }
      if(centerConfirmCount >= centerConfirmThreshold){
        centerConfirmCount = 0;
        turnState = 4;
        forwardStartTime = millis();
      }
    }
    return;
  }

  // ===== 좌 pivot =====
  if(turnState == 2){
    pivot_left_run();
    if(millis() - pivotStartTime > blind_time_L){
      if(C == LOW){
        centerConfirmCount++;
      } else {
        centerConfirmCount = 0;
      }
      if(centerConfirmCount >= centerConfirmThreshold){
        centerConfirmCount = 0;
        turnState = 4;
        forwardStartTime = millis();
      }
    }
    return;
  }

  // ===== 라인 재탐색 =====
  if(turnState == 3){
    if(SL == LOW){
      pivot_left_run();
    } else {
      pivot_right_run();
    }
    if(C == LOW){
      centerConfirmCount++;
    } else {
      centerConfirmCount = 0;
    }
    if(centerConfirmCount >= centerConfirmThreshold){
      centerConfirmCount = 0;
      turnState = 4;
      forwardStartTime = millis();
    }
    return;
  }

  // ===== 라인 완전 이탈 =====
  if(L==HIGH && C==HIGH && R==HIGH){
    turnState = 3;
    centerConfirmCount = 0;
    rightKickDone = false;
    leftKickDone  = false;
    return;
  }

  // ===== 우회전 =====
  if(R==LOW && L==HIGH){
    turnState = 1;
    pivotStartTime = millis();
    centerConfirmCount = 0;
    rightKickDone = false;
    return;
  }

  // ===== 좌회전 =====
  if(L==LOW && R==HIGH){
    turnState = 2;
    pivotStartTime = millis();
    centerConfirmCount = 0;
    leftKickDone = false;
    return;
  }

  // ===== 직진 =====
  drive_pwm(base_Speed, base_Speed);
  SL=L; SC=C; SR=R;
  delay(5);
}

// =========================================
void pivot_right_run(){
  if(!rightKickDone){
    rightKickDone = true;
    rightKickStart = millis();
  }
  int spd = (millis() - rightKickStart < kickstart_time) ? kickstart_pwm : turn_Speed_R;

  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  analogWrite(RightMotor_E_pin, spd);

  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(LeftMotor_E_pin, spd);
}

void pivot_left_run(){
  if(!leftKickDone){
    leftKickDone = true;
    leftKickStart = millis();
  }
  int spd = (millis() - leftKickStart < kickstart_time) ? kickstart_pwm : turn_Speed_L;

  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  analogWrite(RightMotor_E_pin, spd);

  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  analogWrite(LeftMotor_E_pin, spd);
}

void drive_pwm(int rightPWM, int leftPWM){
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  analogWrite(RightMotor_E_pin, rightPWM);

  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(LeftMotor_E_pin, leftPWM);
}