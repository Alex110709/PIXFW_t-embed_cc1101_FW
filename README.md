# LilyGO T-Embed CC1101 JavaScript App Firmware

[![Build Status](https://github.com/your-repo/cc1101-fw/workflows/Build%20T-Embed%20CC1101%20Firmware/badge.svg)](https://github.com/your-repo/cc1101-fw/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ESP32-S3 기반 LilyGO T-Embed CC1101을 위한 JavaScript 앱 지원 펌웨어입니다. Flipper Zero와 유사한 JavaScript 기반 앱 개발 환경을 제공하면서 CC1101의 모든 Sub-GHz 통신 기능을 지원합니다.

## 🚀 주요 특징

### 📡 Sub-GHz RF 통신
- CC1101 트랜시버 완전 지원 (315/433/868/915 MHz)
- ASK/OOK, GFSK, MSK 변조 방식
- 실시간 RF 스캐닝 및 신호 분석
- 신호 송수신 및 프로토콜 디코딩
- RF Jammer 및 Spectrum Analyzer 기능 (Bruce 펌웨어 호환)

### 🛠️ JavaScript 앱 개발 환경
- mJS 경량 JavaScript 엔진
- Flipper Zero 스타일 앱 아키텍처
- 샌드박스 환경과 권한 관리
- 실시간 앱 로딩 및 실행

### 🖥️ 사용자 인터페이스
- LVGL 기반 그래픽 UI (1.9" LCD 170x320)
- 로터리 엔코더 및 버튼 입력 지원
- Flipper Zero 스타일 직관적인 메뉴 시스템
- 상태 표시줄 및 알림 시스템

### 🌐 네트워킹 지원
- Wi-Fi 연결 및 AP 모드
- 웹 기반 IDE 및 앱 관리
- OTA 업데이트 지원
- RESTful API 인터페이스

## 📋 시스템 요구사항

### 하드웨어
- LilyGO T-Embed ESP32-S3 with CC1101 module
- 1.9" LCD Display (170x320)
- 로터리 엔코더 및 버튼
- USB-C 포트

### 소프트웨어
- ESP-IDF v4.4 이상
- Python 3.7 이상
- Git

## 🔧 빌드 및 설치

### 1. 개발 환경 설정

```bash
# ESP-IDF 설치 (Ubuntu/Debian)
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# ESP-IDF 다운로드
cd ~
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v4.4.2
./install.sh esp32s3

# 환경 변수 설정
source ~/esp-idf/export.sh
```

### 2. 프로젝트 빌드

```bash
# 프로젝트 클론
git clone https://github.com/your-repo/cc1101-fw.git
cd cc1101-fw

# 빌드 시스템 사용
./build.sh configure  # 프로젝트 설정
./build.sh build      # 펌웨어 빌드

# 또는 Makefile 사용
make configure
make build
```

### 3. 펌웨어 플래싱

```bash
# 자동 포트 감지
./build.sh flash

# 수동 포트 지정
./build.sh flash /dev/ttyUSB0

# 플래시 후 모니터링
./build.sh flash-monitor
```

## 📱 JavaScript API 사용법

### RF 통신 API

```javascript
// RF 모듈 설정
rf.setFrequency(433920000); // 433.92 MHz
rf.setModulation("GFSK");

// 신호 수신
rf.startReceive();
rf.setReceiveCallback((signal) => {
    console.log("Received:", signal.data);
    console.log("RSSI:", signal.rssi);
});

// 신호 송신
const data = [0x12, 0x34, 0x56, 0x78];
rf.transmit(data);

// RF Jammer
rf.startJammer(433920000);
rf.stopJammer();

// Spectrum Analyzer
rf.startSpectrumAnalyzer(433000000, 434000000, 100000);
const rssi = rf.getRssiAtFrequency(433920000);
rf.stopSpectrumAnalyzer();
```

### UI 개발 API

```javascript
// 화면 생성
const screen = ui.createScreen();

// 버튼 추가
const button = ui.createButton(screen, "Send Signal", 10, 50, 100, 40);
button.onClick(() => {
    rf.transmit([0xAA, 0xBB, 0xCC]);
    notify.show("Info", "Signal sent!", 2000);
});

// 화면 활성화
ui.setActiveScreen(screen);
```

### 저장소 API

```javascript
// 파일 작업
storage.writeText("/apps/data.txt", "Hello World");
const content = storage.readText("/apps/data.txt");

// 설정 저장
storage.setConfig("frequency", "433920000");
const freq = storage.getConfig("frequency", "433920000");
```

## 📱 코어 앱 (Flipper Zero 스타일)

### RF Scanner
주파수 대역에서 RF 신호를 스캔하고 분석합니다.
- 방향키로 주파수 선택
- OK 버튼으로 스캔 시작/중지

### RF Jammer
지정된 주파수에서 RF 간섭을 생성합니다.
- 방향키로 주파수 선택
- 위/아래 버튼으로 점머 모드 선택
- OK 버튼으로 점머 시작/중지

### Spectrum Analyzer
주파수 스펙트럼을 분석하고 시각화합니다.
- 방향키로 주파수 범위 선택
- 위/아래 버튼으로 단계 크기 선택
- OK 버튼으로 분석 시작/중지

### Signal Generator
사용자 정의 RF 신호를 생성합니다.
- 방향키로 주파수 선택
- 위/아래 버튼으로 변조 방식 선택
- OK 버튼으로 신호 전송

## 🏗️ 프로젝트 구조

```
cc1101-fw/
├── main/                    # 메인 애플리케이션
│   ├── main.c              # 진입점
│   └── system/             # 시스템 관리
├── components/              # ESP-IDF 컴포넌트
│   ├── cc1101/             # CC1101 RF 드라이버
│   ├── mjs_engine/         # JavaScript 엔진
│   ├── lvgl_port/          # LVGL UI 포트
│   ├── js_api/             # JavaScript API
│   ├── app_manager/        # 앱 관리자
│   ├── storage_service/    # 스토리지 서비스
│   └── network_service/    # 네트워킹 서비스
├── apps/                   # JavaScript 앱들
│   ├── core/              # 코어 앱 (메뉴, 설정 등)
│   └── examples/          # 예제 앱들
├── tools/                  # 개발 도구
│   ├── sdk/               # JavaScript SDK
│   └── web_ide/           # 웹 IDE
└── docs/                   # 문서
```

## 📚 주요 컴포넌트

### CC1101 RF 드라이버
- SPI 통신 인터페이스
- 주파수 및 변조 설정
- 송수신 버퍼 관리
- 프리셋 설정 지원
- Jammer 및 Spectrum Analyzer 기능

### JavaScript 엔진 (mJS)
- 경량 JavaScript 런타임
- 네이티브 API 바인딩
- 모듈 시스템 지원
- 에러 핸들링

### UI 시스템 (LVGL)
- 그래픽 위젯 라이브러리
- 터치 및 엔코더 입력
- 테마 및 스타일링
- 애니메이션 지원

### 앱 관리자
- JavaScript 앱 설치/제거
- 샌드박스 환경
- 권한 관리
- 리소스 제한

## 🔒 보안 및 권한

### 앱 샌드박스
각 JavaScript 앱은 격리된 환경에서 실행됩니다:

- **메모리 제한**: 앱당 최대 64KB