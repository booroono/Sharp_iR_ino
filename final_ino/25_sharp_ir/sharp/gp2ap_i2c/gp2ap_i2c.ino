/**
  @file gp2ap_i2c.c
  @brief  i2c of the GP2AP sensor
*/

#ifdef __cplusplus
extern "C" {
#include "./api/gp2ap130s_api.h"
}
#endif

/**
   @brief i2c write
*/
void gp2ap_i2c_write(uint8_t word_address, uint8_t *wdata,
                     struct gp2ap_data *p_gp) {
  Wire.beginTransmission(p_gp->slave_addr);
  Wire.write(word_address);
  Wire.write(*wdata);
  Wire.endTransmission();
}

/**
   @brief i2c multi write
*/
void gp2ap_i2c_multi_write(uint8_t word_address, uint8_t *wdata,
                           uint8_t num, struct gp2ap_data *p_gp) {
  int i;
  Wire.beginTransmission(p_gp->slave_addr);
  Wire.write(word_address);
  for (i = 0; i < num; i++, wdata++) {
    Wire.write(*wdata);
    }
  Wire.endTransmission();
}

/**
   @brief gp2ap i2c read
*/
void gp2ap_i2c_read(uint8_t word_address, uint8_t *rdata,
                    uint8_t num, struct gp2ap_data *p_gp) {
  int i;
  Wire.beginTransmission(p_gp->slave_addr);
  Wire.write(word_address);
  Wire.endTransmission(false);

  Wire.requestFrom((int)p_gp->slave_addr, (int)num, false);
  if (num <= 32) {
    for (i = 0; i < num; i++, rdata++) {
      *rdata =  Wire.read();
    }
  }
  Wire.endTransmission(true);

  if (num > 32) {
    for (i = 0; i < 32; i++, rdata++) {
      *rdata =  Wire.read();
    }
    Wire.endTransmission(true);

    Wire.beginTransmission(p_gp->slave_addr);
    Wire.write(word_address + 32);
    Wire.endTransmission(false);

    Wire.requestFrom((int)p_gp->slave_addr, (int)num, false);
    for (i = 0; i < num - 32; i++, rdata++) {
      *rdata =  Wire.read();
    }
    Wire.endTransmission(true);
  }
}

/**
 * @brief Send only 8-bit data with i2c. 

 * 
 * @param data 8-bit data
 * @return uint8_t 0:success
                   1:data too long to fit in transmit buffer
                   2:received NACK on transmit of address
                   3:received NACK on transmit of data
                   4:other error
 */
uint8_t Send8bitData()
{
    Wire.beginTransmission(DUMMY_DATA);
    return Wire.endTransmission(); 
}
