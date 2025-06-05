#include<Wire.h>
#define TCA_ADDRESS 0x70
#define IR_ADDRESS 0x39
#define HALL_ADDRESS 0x10

# API 버전
#define API_REVISION "gp2ap130s_api_v300"

# 명령어
#define OP_SHUTDOWN 0x00
#define OP_RUN 0xA0

# 센서 상태
#define SENSOR_OFF 0
#define SENSOR_ON 1

# 임계값 - C 코드 기준으로 수정
#define PS_HTH 155  // high threshold
#define PS_LTH 125  // low threshold

// 인터럽트 타입
#define INT_PROX 0
#define INT_PULSE 1
#define INT_LEVEL 2
#define INT_EVERY_TIME 3

// IR 센서 데이터를 위한 구조체 정의
struct gp2ap_data {
    uint8_t slave_addr;      // I2C 슬레이브 주소
    uint8_t sensor_enabled;  // 센서 활성화 상태
    uint8_t device_id;       // 디바이스 ID
    uint16_t ps_count;       // 원시 ADC 값
    uint16_t ps_count_ave;   // 평균 ADC 값
    uint16_t offset_count;   // 오프셋 값
    uint8_t interrupt_type;  // 인터럽트 타입
    uint8_t ps_state;        // 근접 상태
    uint16_t ps_low_th;      // 하위 임계값
    uint16_t ps_high_th;     // 상위 임계값
};

// 전역 변수
struct gp2ap_data ir_sensor;

void setup() {
    Wire.begin();
    Serial.begin(115200);
}

void loop() {
    Tca_Select(0);
    
    // IR 센서 초기화
    init_params(&ir_sensor);
    ir_sensor.slave_addr = IR_ADDRESS;
    ir_sensor.interrupt_type = INT_PROX;
    start_measurement(&ir_sensor);
    
    // 설정 레지스터 값 설정 - C 코드 기준으로 업데이트
    uint8_t wdata;
    wdata = 0x08; gp2ap_i2c_write(0x84, &wdata, &ir_sensor); delay(10); // PWTIME = 0x84, 0x08 (16ms)
    wdata = 0x93; gp2ap_i2c_write(0x85, &wdata, &ir_sensor); delay(10); // Ppulse = 0x85, 0x93 (3ea)
    wdata = 0xB0; gp2ap_i2c_write(0x86, &wdata, &ir_sensor); delay(10); // Pgain 원래값으로 복원 (0x50 -> 0xB0)
    wdata = 0x7D; gp2ap_i2c_write(0x88, &wdata, &ir_sensor); delay(10); // PS_LTH Low byte (125)
    wdata = 0x00; gp2ap_i2c_write(0x89, &wdata, &ir_sensor); delay(10); // PS_LTH High byte (125)
    wdata = 0x9B; gp2ap_i2c_write(0x8A, &wdata, &ir_sensor); delay(10); // PS_HTH Low byte (155)
    wdata = 0x00; gp2ap_i2c_write(0x8B, &wdata, &ir_sensor); delay(10); // PS_HTH High byte (155)
    wdata = 0x10; gp2ap_i2c_write(0x8F, &wdata, &ir_sensor); delay(10); // CLK[2:0]=001, internal clock adjustment (0x70 -> 0x10)
    wdata = 0x61; gp2ap_i2c_write(0xC1, &wdata, &ir_sensor); delay(10); // 오프셋 원래값으로 복원 (0x41 -> 0x61)
    wdata = 0x30; gp2ap_i2c_write(0xC5, &wdata, &ir_sensor); delay(10); // Gain_F 원래값으로 복원 (0x10 -> 0x30)
    wdata = 0x24; gp2ap_i2c_write(0xD0, &wdata, &ir_sensor); delay(10); // IIR_SW[1:0]=10, IIR[2:0]=100, Average 16 times
    wdata = 0x3C; gp2ap_i2c_write(0xD1, &wdata, &ir_sensor); delay(10); // IIR_THD = 0xD1, 0x3C (60)
    wdata = 0x09; gp2ap_i2c_write(0xF0, &wdata, &ir_sensor); delay(10); // PLDrive 원래값으로 복원 (0x05 -> 0x09)
    wdata = 0x44; gp2ap_i2c_write(0xF2, &wdata, &ir_sensor); delay(10); // TC1[2:0]=100, TC0[2:0]=100, Temperature Characteristics Correction
    
    // 센서 데이터 읽기
    read_device_id(&ir_sensor);
    read_raw_data(&ir_sensor);
    read_ps_ave_data(&ir_sensor);
    read_prox_data(&ir_sensor);
    
    // 결과 출력
    Serial.print("IR: ");
    Serial.print(ir_sensor.ps_count);
    Serial.print(",");
    Serial.print("ID: ");
    Serial.print(ir_sensor.device_id, HEX);
    Serial.print(",");
    Serial.println("Hall: -1");
    
    delay(100);
}

// I2C 읽기 함수
void gp2ap_i2c_read(uint8_t reg, uint8_t *data, uint8_t len, struct gp2ap_data *p_gp) {
    Wire.beginTransmission(p_gp->slave_addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    
    Wire.requestFrom(p_gp->slave_addr, len, false);
    for(uint8_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }
    Wire.endTransmission(true);
}

// I2C 쓰기 함수
void gp2ap_i2c_write(uint8_t reg, uint8_t *data, struct gp2ap_data *p_gp) {
    Wire.beginTransmission(p_gp->slave_addr);
    Wire.write(reg);
    Wire.write(*data);
    Wire.endTransmission();
}

// 파라미터 초기화
void init_params(struct gp2ap_data* p_gp) {
    p_gp->ps_count = 0;
    p_gp->ps_count_ave = 0;
    p_gp->ps_low_th = PS_LTH;
    p_gp->ps_high_th = PS_HTH;
}

// 소프트웨어 리셋
void software_reset(struct gp2ap_data* p_gp) {
    uint8_t wdata = 0x01;
    gp2ap_i2c_write(0x82, &wdata, p_gp);
}

// 디바이스 초기화 - C 코드 기준으로 업데이트
void init_device(struct gp2ap_data* p_gp) {
    uint8_t wdata;
    
    // 기본 설정
    wdata = 0x08; gp2ap_i2c_write(0x84, &wdata, p_gp); delay(2); // PWTIME (16ms)
    wdata = 0x93; gp2ap_i2c_write(0x85, &wdata, p_gp); delay(2); // Ppulse (3ea)
    wdata = 0xB0; gp2ap_i2c_write(0x86, &wdata, p_gp); delay(2); // Pgain
    
    // 임계값 설정 - C 코드 방식으로 업데이트
    wdata = 0x7D; gp2ap_i2c_write(0x88, &wdata, p_gp); delay(2); // PS_LTH Low byte (125)
    wdata = 0x00; gp2ap_i2c_write(0x89, &wdata, p_gp); delay(2); // PS_LTH High byte
    wdata = 0x9B; gp2ap_i2c_write(0x8A, &wdata, p_gp); delay(2); // PS_HTH Low byte (155)
    wdata = 0x00; gp2ap_i2c_write(0x8B, &wdata, p_gp); delay(2); // PS_HTH High byte
    
    wdata = 0x10; gp2ap_i2c_write(0x8F, &wdata, p_gp); delay(2); // CLK 설정 변경 (0x70 -> 0x10)
    wdata = 0x61; gp2ap_i2c_write(0xC1, &wdata, p_gp); delay(2); // 오프셋 설정
    wdata = 0x30; gp2ap_i2c_write(0xC5, &wdata, p_gp); delay(2); // Gain_F
    wdata = 0x24; gp2ap_i2c_write(0xD0, &wdata, p_gp); delay(2); // IIR 설정
    wdata = 0x3C; gp2ap_i2c_write(0xD1, &wdata, p_gp); delay(2); // IIR_THD
    wdata = 0x09; gp2ap_i2c_write(0xF0, &wdata, p_gp); delay(2); // PLDrive
    wdata = 0x44; gp2ap_i2c_write(0xF2, &wdata, p_gp); delay(2); // Temperature Characteristics
    
    set_interrupt(p_gp);
}

// 측정 시작
void start_measurement(struct gp2ap_data* p_gp) {
    software_reset(p_gp);
    init_params(p_gp);
    init_device(p_gp);
    enable_sensor(p_gp, SENSOR_ON);
}

// 센서 활성화/비활성화
void enable_sensor(struct gp2ap_data* p_gp, uint8_t onoff) {
    uint8_t wdata;
    
    if (onoff) {
        p_gp->sensor_enabled = 1;
        wdata = OP_RUN;
    } else {
        p_gp->sensor_enabled = 0;
        wdata = OP_SHUTDOWN;
    }
    gp2ap_i2c_write(0x80, &wdata, p_gp);
}

// 인터럽트 설정
void set_interrupt(struct gp2ap_data* p_gp) {
    uint8_t wdata82h, wdata83h, wdata87h;
    
    switch (p_gp->interrupt_type) {
        case INT_PROX:
            wdata82h = 0x00;
            wdata83h = 0x20;
            wdata87h = 0x18;
            break;
        case INT_PULSE:
            wdata82h = 0x04;
            wdata83h = 0x10;
            wdata87h = 0x18;
            break;
        case INT_LEVEL:
            wdata82h = 0x00;
            wdata83h = 0x10;
            wdata87h = 0x18;
            break;
        case INT_EVERY_TIME:
            wdata82h = 0x04;
            wdata83h = 0x10;
            wdata87h = 0x80;
            break;
    }
    gp2ap_i2c_write(0x82, &wdata82h, p_gp);
    gp2ap_i2c_write(0x83, &wdata83h, p_gp);
    gp2ap_i2c_write(0x87, &wdata87h, p_gp);
}

// 근접 데이터 읽기
void read_prox_data(struct gp2ap_data* p_gp) {
    uint8_t rdata;
    gp2ap_i2c_read(0x81, &rdata, 1, p_gp);
    p_gp->ps_state = (rdata & 0x08) ? 1 : 0;
}

// 원시 데이터 읽기
void read_raw_data(struct gp2ap_data* p_gp) {
    uint8_t rdata[2];
    gp2ap_i2c_read(0x90, rdata, 2, p_gp);
    p_gp->ps_count = ((uint16_t)(rdata[1] & 0x7F) << 8) | rdata[0];
}

// 평균 데이터 읽기
void read_ps_ave_data(struct gp2ap_data* p_gp) {
    uint8_t rdata[2];
    gp2ap_i2c_read(0x92, rdata, 2, p_gp);
    p_gp->ps_count_ave = ((uint16_t)(rdata[1] & 0x7F) << 8) | rdata[0];
}

// 디바이스 ID 읽기 - C 코드에서는 0xA0 사용하지만 0x4B가 올바른 레지스터
void read_device_id(struct gp2ap_data* p_gp) {
    uint8_t rdata;
    gp2ap_i2c_read(0x4B, &rdata, 1, p_gp);  // Sharp GP2AP130S 디바이스 ID 레지스터
    p_gp->device_id = rdata;
}



// 멀티플렉서 채널 선택
void Tca_Select(uint8_t i) {
    if (i > 7) return;
    Wire.beginTransmission(TCA_ADDRESS);
    Wire.write(1 << i);
    Wire.endTransmission();
}
