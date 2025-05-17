/////////////////////////////////////////////////////////////////////////
//Copyright (C) 2022 SHARP All Rights Reserved.
//This program is for modeling purposes only, and is not guaranteed.
//This program is API of the GP2AP130S sensor.
/////////////////////////////////////////////////////////////////////////

#include "gp2ap130s_api.h"

/**
 * @brief Send dummy code to determine the I2C address.
 *        See 1.2.3 in the Appendix for details.
 * 
 */
void send_dummy_code()
{
    Send8bitData();
}

/**
   @brief initialize parameters

   @param[in,out] p_gp structure of sensor
*/
void init_params(struct gp2ap_data* p_gp)
{
  p_gp->ps_count               = 0;
  p_gp->ps_count_ave           = 0;
  p_gp->ps_low_th              = PS_LTH;
  p_gp->ps_high_th             = PS_HTH;
}

/**
   @brief Software reset.

   @details The sensor's register value is initialized.

   @param[in,out] p_gp structure of sensor
*/
void software_reset(struct gp2ap_data* p_gp)
{
  uint8_t wdata;

  wdata = 0x01;
  gp2ap_i2c_write(0x82, &wdata, p_gp);
}

/**
   @brief Initialization function.

   @param[in,out] p_gp structure of sensor
*/
void init_device(struct gp2ap_data* p_gp)
{
  uint8_t wdata;

  send_dummy_code(); //Send a dummy code to determine the i2c address

  wdata = 0x08; gp2ap_i2c_write(0x84, &wdata, p_gp);
  wdata = 0x93; gp2ap_i2c_write(0x85, &wdata, p_gp);
  wdata = 0xB0; gp2ap_i2c_write(0x86, &wdata, p_gp);
  wdata = (p_gp->ps_low_th & 0x00FF);         gp2ap_i2c_write(0x88, &wdata, p_gp);
  wdata = ((p_gp->ps_low_th & 0xFF00) >> 8);  gp2ap_i2c_write(0x89, &wdata, p_gp);
  wdata = (p_gp->ps_high_th & 0x00FF);        gp2ap_i2c_write(0x8A, &wdata, p_gp);
  wdata = ((p_gp->ps_high_th & 0xFF00) >> 8); gp2ap_i2c_write(0x8B, &wdata, p_gp);
  wdata = 0x00; gp2ap_i2c_write(0x8E, &wdata, p_gp);
  wdata = 0x70; gp2ap_i2c_write(0x8F, &wdata, p_gp);
  wdata = 0x10; gp2ap_i2c_write(0xB1, &wdata, p_gp);

  wdata = 0x61; gp2ap_i2c_write(0xC1, &wdata, p_gp);
  wdata = 0x00; gp2ap_i2c_write(0xC2, &wdata, p_gp);
  wdata = 0x30; gp2ap_i2c_write(0xC5, &wdata, p_gp);

  wdata = 0x24; gp2ap_i2c_write(0xD0, &wdata, p_gp);
  wdata = 0x3C; gp2ap_i2c_write(0xD1, &wdata, p_gp);

  wdata = 0x09; gp2ap_i2c_write(0xF0, &wdata, p_gp);
  wdata = 0x44; gp2ap_i2c_write(0xF2, &wdata, p_gp);

  set_interrupt(p_gp);
}

/**
   @brief Start measurement. initialize and sensor on

   @param[in,out] p_gp structure of sensor
*/
void start_measurement(struct gp2ap_data* p_gp)
{
  software_reset(p_gp); /* software reset */
  init_params(p_gp); /* initialize parameters */
  init_device(p_gp); /* initialize register value */
  enable_sensor(p_gp, SENSOR_ON); /* sensor on */
}

/**
 * @brief start_measurement. (Already initialized)
 * 
 * @param[in,out] p_gp structure of sensor 
 */
void restart_measurement(struct gp2ap_data *p_gp)
{
  init_params(p_gp); /* initialize parameters */
  //SettingDynamicCalib(p_gp); /* setting for dynamic calib */
  enable_sensor(p_gp, SENSOR_ON); /* sensor on */
}

/**
   @brief Stop measurement

   @param[in,out] p_gp structure of sensor
*/
void stop_measurement(struct gp2ap_data* p_gp)
{
  enable_sensor(p_gp, SENSOR_OFF);
}

/**
 * @brief Trun on or off sensor 
 * 
 * @param[in.out] p_gp p_gp structure of sensor
 * @param onoff on or off
 */
void enable_sensor(struct gp2ap_data *p_gp, uint8_t onoff)
{
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

/**
 * @brief Setting for interrupt 
 * 
 * @param p_gp[in,out] data structrue of sensor
 */
void set_interrupt(struct gp2ap_data *p_gp)
{
  uint8_t wdata82h;
  uint8_t wdata83h;
  uint8_t wdata87h;

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

/**
  @brief  Clear ps flag 0x01h:FLAG bit

  @param[in,out] p_gp structure of sensor
*/
void clear_ps_flag(struct gp2ap_data* p_gp)
{
  uint8_t wdata;

  wdata = 0xFB; gp2ap_i2c_write(0x81, &wdata, p_gp);
}

/**
   @brief Get ps state data compared to raw data

   @param[in,out] p_gp structure of sensor
*/
void read_prox_data(struct gp2ap_data* p_gp)
{
  uint8_t rdata;
  uint8_t distance;

  /* RPOX(0x81) 0:far, 1:near */
  gp2ap_i2c_read(0x81, &rdata, sizeof(rdata), p_gp);

  if ((rdata & 0x08) == 0x08) {
    distance = 1;
  } else {
    distance = 0;
  }
  p_gp->ps_state = distance;
}

/**
   @brief Get ps count data

   @param[in,out] p_gp structure of sensor
*/
void read_raw_data(struct gp2ap_data* p_gp)
{
  uint8_t rdata[2];
  gp2ap_i2c_read(0x90, rdata, sizeof(rdata), p_gp);
  p_gp->ps_count = ((uint16_t)(rdata[1] & 0x7F) << 8) | rdata[0];
}

/**
   @brief Get the ps count data average

   @param[in,out] p_gp structure of sensor
*/
void read_ps_ave_data(struct gp2ap_data* p_gp)
{
  uint8_t rdata[2];
  gp2ap_i2c_read(0x92, rdata, sizeof(rdata), p_gp);
  p_gp->ps_count_ave = ((uint16_t)(rdata[1] & 0x7F) << 8) | rdata[0];
}

void read_offset_data(struct gp2ap_data* p_gp)
{
  uint8_t rdata[2];
  gp2ap_i2c_read(0x96, rdata, sizeof(rdata), p_gp);
  p_gp->offset_count = ((uint16_t)(rdata[1] & 0x7F) << 8) | rdata[0];
}

/** 
 * Get device id.
 * @param[in,out] p_gp structure of sensor
 * device id: 0xE0 
 */
uint8_t get_device_id(struct gp2ap_data* p_gp)
{
    uint8_t rdata;

    gp2ap_i2c_read(0xA0, &rdata, sizeof(rdata), p_gp);

    p_gp->device_id = rdata;

    return rdata;
}

