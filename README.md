# arduino-line-tracer

IR 센서 3개 기반 라인트레이서 RC카 — 2026 동아리 경진대회 최우수상(2위)

> 대회(2026.04.02) 이후 코드를 정리해 업로드했습니다.

## 개요

- 색상 반전 트랙(라인 위에서 센서 값이 `LOW`)을 달리는 4륜 RC카
- IR 센서 3개: 왼쪽 `A5`, 가운데 `A4`, 오른쪽 `A3`
- 2026 동아리 경진대회 최우수상(2위)

## 제어 방식

센서 3개(L/C/R)의 조합으로 상태를 정하는 **상태머신**입니다.
초기에는 PID 제어를 시도했지만 튜닝이 어려워 포기하고 상태머신으로 전환했습니다.

| `turnState` | 동작 |
|---|---|
| 0 | 직진 (`base_Speed`) |
| 1 | 우 제자리 회전 |
| 2 | 좌 제자리 회전 |
| 3 | 라인 완전 이탈 시 마지막 방향으로 재탐색 |
| 4 | 회전 후 직진 안정화 |

판단 우선순위 (`loop()` 위에서부터):

1. 교차로 강제 직진 중이면 센서를 무시하고 직진
2. 세 센서 모두 라인(`LOW`) → 교차로 감지 카운트
3. 상태 4 / 1 / 2 / 3 진행
4. 세 센서 모두 `HIGH` → 재탐색(상태 3)
5. 오른쪽만 라인 → 우회전, 왼쪽만 라인 → 좌회전
6. 그 외 → 직진

## 회전

양쪽 바퀴를 서로 반대 방향으로 돌리는 **제자리 회전(pivot)** 입니다.
회전 속도는 **좌 200 / 우 220**으로 비대칭 튜닝했습니다.

## 교차로 · 360도 구간

세 센서가 모두 라인으로 읽히면 `cross_detect_speed`(80)로 감속하면서 횟수를 셉니다.
**4번 연속**(`crossConfirmThreshold`) 감지되면 교차로로 판단하고, **370ms**(`cross_straight_time`) 동안 센서를 무시한 채 `base_Speed`로 직진합니다.

## 오감지 방지

- **블라인드 타임 300ms** — 회전 시작 후 300ms 동안은 회전 종료 판정을 하지 않음
- **중앙 센서 연속 12회 확인** — 가운데 센서가 12번 연속 라인을 읽어야 회전 종료
- **회전 후 60ms 직진 안정화** — 회전이 끝나면 60ms 동안 직진(상태 4)
- **킥스타트** — 회전 시작 직후 40ms 동안 255 PWM으로 구동해 정지 마찰을 이김

## 튜닝 파라미터

| 변수 | 값 | 설명 |
|---|---|---|
| `base_Speed` | 130 | 직진 PWM |
| `turn_Speed_L` | 200 | 좌회전 PWM |
| `turn_Speed_R` | 220 | 우회전 PWM |
| `blind_time_L` | 300 | 좌회전 블라인드 타임 (ms) |
| `blind_time_R` | 300 | 우회전 블라인드 타임 (ms) |
| `forward_boost_time` | 60 | 회전 후 직진 안정화 시간 (ms) |
| `cross_detect_speed` | 80 | 교차로 감지 구간 감속 PWM |
| `kickstart_pwm` | 255 | 킥스타트 PWM |
| `kickstart_time` | 40 | 킥스타트 시간 (ms) |
| `centerConfirmThreshold` | 12 | 회전 종료에 필요한 중앙 센서 연속 감지 횟수 |
| `crossConfirmThreshold` | 4 | 교차로 판정에 필요한 연속 감지 횟수 |
| `cross_straight_time` | 370 | 교차로 강제 직진 시간 (ms) |

## 핀 배치

| 핀 | 용도 |
|---|---|
| 5 | 오른쪽 모터 PWM (`RightMotor_E_pin`) |
| 6 | 왼쪽 모터 PWM (`LeftMotor_E_pin`) |
| 8, 9 | 오른쪽 모터 방향 |
| 10, 11 | 왼쪽 모터 방향 |
| A5 / A4 / A3 | 왼쪽 / 가운데 / 오른쪽 IR 센서 |

## 버전 비교

**v1(3/31): delay 기반 3점 회전 → 최종(4/2): millis 기반 상태머신으로 개편.**
v1([`versions/v1-three-point-turn/linetracer1.ino`](versions/v1-three-point-turn/linetracer1.ino))은 바깥쪽 센서가 라인을 읽으면 `delay()`로 300ms 후진한 뒤 300ms 제자리 회전하는 3점 회전 방식이었고, 회전 중에는 센서를 읽지 못했습니다.
최종 버전은 `delay()` 대신 `millis()`로 시간을 재면서 매 루프마다 센서를 읽는 상태머신으로 바꿔, 회전 중에도 중앙 센서로 종료 시점을 판단하고 교차로 처리·블라인드 타임·킥스타트를 추가했습니다.

## 파일 구성

```
arduino-line-tracer/
├── arduino-line-tracer.ino              # 최종 버전 (4/2)
├── versions/
│   └── v1-three-point-turn/
│       └── linetracer1.ino              # 초기 버전 (3/31)
└── README.md
```

Arduino IDE는 폴더명과 스케치명이 같아야 열리므로, 레포를 `arduino-line-tracer` 폴더로 클론한 뒤 `arduino-line-tracer.ino`를 열면 됩니다.
