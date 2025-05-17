#include<Wire.h>
#define TCA_ADDRESS 0x70
#define IR_ADDRESS 0x39
#define HALL_ADDRESS 0x10

// IR 센서 데이터를 위한 구조체 정의
struct gp2ap_data {
    uint16_t raw_value;      // 원시 ADC 값
    uint16_t scaled_value;   // 스케일링된 값
};

int16_t data_z;

void setup() {
    Wire.begin();
    Serial.begin(115200);
}

void loop() {
  Tca_Select(0);
  
  // IR 센서 초기화
  IR_init_(IR_ADDRESS);
  
  // D0 레지스터에서 원시 값 읽기
  uint16_t raw_value = read_d0_value(IR_ADDRESS);
  
  // 결과 출력 - 기본 형식으로 (raw 데이터만)
  Serial.print("IR: ");
  Serial.print(raw_value);
  Serial.print(",");
  Serial.println("Hall: -1");
  
  delay(100); // 샘플링 간격 추가
}

void HALL_init_(unsigned char address) {
    Wire.beginTransmission(address);
    Wire.write(0x21);
    Wire.write(0x04);
    Wire.endTransmission();
}

void IR_init_(unsigned char address) {
    Wire.beginTransmission(address);
    Wire.write(0x82);
    Wire.write(0x04);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x84);
    Wire.write(0x08);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x85);
    Wire.write(0x93);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x88);
    Wire.write(0x68);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x89);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x8A);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x8B);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0x8F);
    Wire.write(0x70);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xB1);
    Wire.write(0x10);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xC1);
    Wire.write(0x61);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xC2);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xD0);
    Wire.write(0x24);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xD1);
    Wire.write(0x3C);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xF0);
    Wire.write(0x09);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(0xF2);
    Wire.write(0x44);
    Wire.endTransmission();

    // on
    Wire.beginTransmission(address);
    Wire.write(0x80);
    Wire.write(0xA0);
    Wire.endTransmission();
}

// D0 레지스터(90H, 91H)에서 원시 거리 값 읽기
uint16_t read_d0_value(unsigned char address) {
    uint8_t rdata[2];
    
    Wire.beginTransmission(address);
    Wire.write(0x90);
    Wire.endTransmission(false);
    
    Wire.requestFrom(address, 2, false);
    rdata[0] = Wire.read(); // 하위 바이트
    rdata[1] = Wire.read(); // 상위 바이트
    Wire.endTransmission(true);
    
    // 14비트 ADC 값 조합 (하위 바이트 + 상위 바이트의 하위 6비트)
    uint16_t raw_value = ((uint16_t)(rdata[1] & 0x3F) << 8) | rdata[0];
    
    return raw_value;
}

void Tca_Select(uint8_t i) {
    if (i > 7) return;
    Wire.beginTransmission(TCA_ADDRESS);
    Wire.write(1 << i);
    Wire.endTransmission();
}
