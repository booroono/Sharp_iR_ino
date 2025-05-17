#ifdef __cplusplus
extern "C" {
#include "./api/gp2ap130s_api.h"
}
#endif

#define KEYBOARD_0 0
#define KEYBOARD_1 1
#define KEYBOARD_2 2
#define KEYBOARD_3 3
#define KEYBOARD_4 4
#define KEYBOARD_5 5
#define KEYBOARD_6 6
#define KEYBOARD_7 7
#define KEYBOARD_8 8
#define KEYBOARD_9 9
#define KEYBOARD_a 10
#define KEYBOARD_b 11
#define KEYBOARD_c 12
#define KEYBOARD_d 13
#define KEYBOARD_e 14
#define KEYBOARD_f 15
#define KEYBOARD_i 18
#define KEYBOARD_j 19
#define KEYBOARD_k 20
#define KEYBOARD_l 21
#define KEYBOARD_m 22
#define KEYBOARD_n 23
#define KEYBOARD_o 24
#define KEYBOARD_p 25
#define KEYBOARD_v 31
#define KEYBOARD_x 33
#define KEYBOARD_h 17
#define KEYBOARD_r 27
#define KEYBOARD_s 28
#define KEYBOARD_t 29
#define KEYBOARD_z 35

/*
uint16_t m_HexToDec(unsigned char);
uint16_t m_GetData_hex();
uint16_t m_GetData_hex_woh();
uint16_t m_GetNum_1();
uint16_t m_GetNum_2();
uint16_t m_GetNum_3();
void i2c_write_command(struct gp2ap055s_data *, uint8_t *);
void i2c_read_command(struct gp2ap055s_data *);
void i2c_multi_read_command(struct gp2ap055s_data *);
void kb_operation(struct gp2ap055s_data *);
void output_gs_data(struct gp2ap055s_data *);
void help_menu();
void check_parameters(struct gp2ap055s_data *);
*/

/**
   HexToDec one char
*/
uint16_t m_HexToDec(unsigned char ch)
{
  uint16_t dec;

  if ((ch >= '0') && (ch <= '9')) {
    dec = ch - '0';
  } else if ((ch >= 'A') && (ch <= 'Z')) {
    dec = 10 + ch - 'A';
  } else if ((ch >= 'a') && (ch <= 'z')) {
    dec = 10 + ch - 'a';
  } else {
    dec = 0;
  }
  return dec;
}

/**
   GetDataHex one char
*/
uint16_t m_GetData_hex()
{
  uint16_t data = 0;
  unsigned char buf[3] = {0, 0, 0};

  buf[0] = Serial.read();
  buf[1] = Serial.read();
  buf[2] = Serial.read();
  if ((buf[2] == 'h') | (buf[2] == 'H')) {
    data = m_HexToDec(buf[0]) * 16 + m_HexToDec(buf[1]);
  }
  return data;
}

/**
   GetDataHex one char without h/H
*/
uint16_t m_GetData_hex_woh()
{
  uint16_t data = 0;
  unsigned char buf[2] = {0, 0};

  buf[0] = Serial.read();
  buf[1] = Serial.read();
  data = m_HexToDec(buf[0]) * 16 + m_HexToDec(buf[1]);
  return data;
}

/**
   GetNumber one char
*/
uint8_t m_GetNum_1()
{
  uint8_t buf1;
  while (Serial.available() != 1) {} //

  buf1 = Serial.read();
  buf1 = m_HexToDec(buf1);
  return buf1;
}

/**
   GetNumber two char
*/
uint16_t m_GetNum_2()
{
  uint16_t data = 0;
  unsigned char buf2[2];

  buf2[0] = Serial.read();
  buf2[1] = Serial.read();
  data = m_HexToDec(buf2[0]) * 10 + m_HexToDec(buf2[1]);
  return data;
}

/**
   GetNumber three char
*/
uint16_t m_GetNum_3()
{
  uint16_t data = 0;
  unsigned char buf3[3];

  buf3[0] = Serial.read();
  buf3[1] = Serial.read();
  buf3[2] = Serial.read();
  data = m_HexToDec(buf3[0]) * 100 + m_HexToDec(buf3[1]) * 10 + m_HexToDec(buf3[2]);
  return data;
}

void output2serialhex(int data)
{
  if (data <= 15) {
    Serial.print("0");
    Serial.print(data, HEX);
  } else {
    Serial.print(data, HEX);
  }
}

/**
* @brief key input
*/
int16_t get_data_manual_input()
{
  char input_data[10];
  int16_t data;

  gets(input_data);
  data = atol(input_data);

  return data;
}

/**
* @param[in] struct gp2ap_data pointer
*/
void i2c_write_command(struct gp2ap_data *p_gp)
{
  uint8_t rev_data;
  uint8_t word_address;
  uint8_t wdata;

  while (Serial.available() != 6) {
  }
  rev_data = Serial.read();
  word_address = m_HexToDec(rev_data) * 16;
  rev_data = Serial.read();
  word_address += m_HexToDec(rev_data);
  rev_data = Serial.read(); /* check h */

  rev_data = Serial.read();
  wdata = m_HexToDec(rev_data) * 16;
  rev_data = Serial.read();
  wdata += m_HexToDec(rev_data);
  rev_data = Serial.read(); /* check h */

  gp2ap_i2c_write(word_address, &wdata, p_gp);
}

/**
* @param[in] struct gp2ap_data pointer
*/
void i2c_read_command(struct gp2ap_data *p_gp, uint8_t *rdata)
{
  uint8_t rev_data;
  uint8_t word_address;

  while (Serial.available() != 3) {
  }
  rev_data = Serial.read();
  word_address = m_HexToDec(rev_data) * 16;
  rev_data = Serial.read();
  word_address += m_HexToDec(rev_data);
  rev_data = Serial.read(); /* check h */

  gp2ap_i2c_read(word_address, rdata, 0x1, p_gp);
}

/**
* @param[in] struct gp2ap_data pointer
*/
void i2c_multi_read_command(struct gp2ap_data *p_gp, uint8_t *rdata, uint8_t *length)
{
  uint8_t rev_data;
  uint8_t word_address;
  uint8_t read_byte2[64];
  uint8_t len, i;

  while (Serial.available() < 5) {
  }

  rev_data = Serial.read();
  word_address = m_HexToDec(rev_data) * 16;
  rev_data = Serial.read();
  word_address += m_HexToDec(rev_data);
  rev_data = Serial.read();

  rev_data = Serial.read();
  len = m_HexToDec(rev_data) * 10;
  rev_data = Serial.read();
  len += m_HexToDec(rev_data);
  gp2ap_i2c_read(word_address, read_byte2, len, p_gp);
  for (i = 0; i < len; i++) {
    rdata[i] = read_byte2[i];
  }
  *length = len;
}

/**
   keyboard input
   @param[in,out]
*/
void kb_operation(struct gp2ap_data *p_gp)
{
  uint8_t rev_data;
  uint8_t read_byte;
  uint8_t read_byte2[64];
  uint8_t i;
  uint8_t len;

  if (Serial.available() > 0) {
    rev_data = Serial.read();
    //rev_data = m_HexToDec(rev_data);
    switch (rev_data) {
      case 's':
        init_params(p_gp);
        enable_sensor(p_gp, SENSOR_ON);
        time_started = millis();
        break;
      case 'p':
        stop_measurement(p_gp);
        break;
      case KEYBOARD_h: /* "h" HELP */
        help_menu();
        break;
      case '0': /* i2c write */
        i2c_write_command(p_gp);
        break;
      case '1': /* i2c read */
        i2c_read_command(p_gp, &read_byte);
        output2serialhex(read_byte);
        break;
      case '7': /* i2c sequential read */
        i2c_multi_read_command(p_gp, read_byte2, &len);
        for (i = 0; i < len; i++) {
          output2serialhex(read_byte2[i]);
        }
        break;
      case 'f': /* output software version */
        Serial.println(SOFT_REVISION);
        break;
      default:
        Serial.println("Invalid number - Exit debug");
        break;
    }
  }
}

/**
* @brief help menu.
*/
void help_menu()
{
  Serial.println(SOFT_REVISION);
  Serial.println("HELP");
  Serial.println("s : Start measurement");
  Serial.println("p : Stop measurement");
  Serial.println("h : Help");
}
