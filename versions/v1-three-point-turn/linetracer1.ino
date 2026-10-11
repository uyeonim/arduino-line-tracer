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

// ===== 속도 설정 =====
int straight_Speed = 115;
int turn_Speed     = 108;

// 3점 회전 타이밍 (ms) - 조절 가능
int reverse_time = 300;   // 후진 시간
int forward_time = 300;   // 전진 시간

int SL = 0, SC = 0, SR = 0;
bool turning = false;     // 현재 3점 회전 중인지

void setup() {
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(LeftMotor_E_pin,  OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin,  OUTPUT);
  pinMode(LeftMotor_4_pin,  OUTPUT);
  Serial.begin(9600);
  Serial.println("Welcome!");
}

void loop() {
  int L = digitalRead(L_Line);
  int C = digitalRead(C_Line);
  int R = digitalRead(R_Line);

  Serial.print("L:"); Serial.print(L);
  Serial.print(" C:"); Serial.print(C);
  Serial.print(" R:"); Serial.println(R);

  // 선 이탈시 마지막 방향으로 탐색
  if (L == HIGH && C == HIGH && R == HIGH) {
    if (SL == LOW) {
      three_point_left();
      Serial.println("탐색 좌");
    } else {
      three_point_right();
      Serial.println("탐색 우");
    }
    return;
  }

  // ===== 주행 판단 =====
  if (L == HIGH && C == LOW && R == HIGH) {
    go_straight();
    Serial.println("직진");

  } else if (R == LOW) {
    three_point_right();
    Serial.println("우회전");

  } else if (L == LOW) {
    three_point_left();
    Serial.println("좌회전");

  } else if (L == LOW && R == LOW) {
    motor_stop();
    Serial.println("정지");
  }

  SL = L; SC = C; SR = R;
  delay(15);
}

// ===== 3점 회전 함수 =====

void three_point_right() {
  // 1단계: 후진
  go_backward();
  delay(reverse_time);

  // 2단계: 오른쪽으로 전진 회전
  pivot_right();
  delay(forward_time);
}

void three_point_left() {
  // 1단계: 후진
  go_backward();
  delay(reverse_time);

  // 2단계: 왼쪽으로 전진 회전
  pivot_left();
  delay(forward_time);
}

// ===== 기본 모터 제어 함수 =====

void go_straight() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  analogWrite(RightMotor_E_pin, straight_Speed);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(LeftMotor_E_pin, straight_Speed);
}

void go_backward() {
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  analogWrite(RightMotor_E_pin, turn_Speed);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  analogWrite(LeftMotor_E_pin, turn_Speed);
}

void pivot_right() {
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  analogWrite(RightMotor_E_pin, turn_Speed);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(LeftMotor_E_pin, turn_Speed);
}

void pivot_left() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  analogWrite(RightMotor_E_pin, turn_Speed);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  analogWrite(LeftMotor_E_pin, turn_Speed);
}

void motor_stop() {
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin,  0);
}