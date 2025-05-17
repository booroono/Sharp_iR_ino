/**
  @file gp2ap130s_api.h
  @brief  API of the GP2AP130S sensor
*/

#ifndef __GP2AP130S_API_H__
#define __GP2AP130S_API_H__

#include <stdint.h>

/* api software version */
#define API_REVISION        "gp2ap130s_api_v300"

/* command */
#define OP_SHUTDOWN           0x00
#define OP_RUN                0xA0

#define SENSOR_OFF            0
#define SENSOR_ON             1

#define PS_HTH                (uint16_t)155 /* high threshold */
#define PS_LTH                (uint16_t)125 /* low threshold */

/* interrupt type mode */
#define INT_PROX              0
#define INT_PULSE             1
#define INT_LEVEL             2
#define INT_EVERY_TIME        3

#define DUMMY_DATA            0

struct gp2ap_data
{
  uint8_t    slave_addr; /* i2c slave address */

  uint8_t    sensor_enabled; /* ps enable */

  uint8_t    device_id;

  uint16_t   ps_count; /* ps adc count */
  uint16_t   ps_count_ave; /* ps adc count */
  uint16_t   offset_count;

  uint8_t    interrupt_type; /* interrupt type mode */

  uint8_t    ps_state;

  uint16_t   ps_low_th;
  uint16_t   ps_high_th;
};


void gp2ap_i2c_read(uint8_t , uint8_t *, uint8_t , struct gp2ap_data *);
void gp2ap_i2c_write(uint8_t , uint8_t *, struct gp2ap_data *);

uint8_t Send8bitData();
void send_dummy_code();

void init_params(struct gp2ap_data *);
void software_reset(struct gp2ap_data *);
void init_device(struct gp2ap_data *);
void start_measurement(struct gp2ap_data *);
void restart_measurement(struct gp2ap_data *);
void stop_measurement(struct gp2ap_data *);
void enable_sensor(struct gp2ap_data *, uint8_t);
void set_interrupt(struct gp2ap_data *);
void clear_ps_flag(struct gp2ap_data *);
void read_prox_data(struct gp2ap_data *);
void read_raw_data(struct gp2ap_data *);
void read_ps_ave_data(struct gp2ap_data *);
void read_offset_data(struct gp2ap_data *);
uint8_t get_device_id(struct gp2ap_data *);

#endif //__GP2AP130S_API_H__
