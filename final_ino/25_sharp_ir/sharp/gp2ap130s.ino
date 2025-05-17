/**
  @file gp2ap130s.c
  @brief the GP2AP130S sensor
  Copyright (C) 2022 SHARP All Rights Reserved.
  This program is for modeling purposes only, and is not guaranteed.
  This program is the control of the PROX sensor.
*/

#include <arduino.h>
#include <Wire.h>


#ifdef __cplusplus
extern "C" {
#include "./api/gp2ap130s_api.h"
#include "./api/gp2ap130s_api.c"
}
#endif

#define ARDUINO_UNO
#define DEBUG

#define SOFT_REVISION      "gp2ap130s_v300"
#define I2C_SLAVE_ADDRESS  0x39 
 
/* Measurement time */
uint32_t time_started = 0;
uint32_t time_measurement = 0;
uint8_t interrupt_handler = 0; 
volatile int pinInterrupt = 2; /* interrupt proximity indicator */

struct gp2ap_data st_gp;

void Interrupt() {
  interrupt_handler = 1;
}

void setup() {

  // i2c
  Wire.begin();

  // Serial communication
  Serial.begin(115200);

  // attachInterrupt(digitalPinToInterrupt(pinInterrupt), Interrupt, FALLING);

  /* Initialize */
  st_gp.slave_addr = I2C_SLAVE_ADDRESS;
  st_gp.sensor_enabled = 0;

  st_gp.interrupt_type = INT_PROX;
  //st_gp.interrupt_type = INT_PULSE;
  //st_gp.interrupt_type = INT_LEVEL;
  //st_gp.interrupt_type = INT_EVERY_TIME;

  init_params(&st_gp);
  init_device(&st_gp);

  enable_sensor(&st_gp, SENSOR_ON);

  help_menu();
}

/**
   @brief This function displays the measurement results on the terminal screen
   @param[in,out] p_gp structure of sensor
*/

void output_data(struct gp2ap_data* p_gp)
{
  time_measurement = millis();
  Serial.print(time_measurement - time_started);
  Serial.print(", ");
  Serial.println(p_gp->ps_count_ave);
}

void loop() 
{
  kb_operation(&st_gp); // serial communication

  if (interrupt_handler) {
    Serial.println("INT"); // INT_PROX: it is output only when detected.

    if (st_gp.interrupt_type == INT_LEVEL) {
      clear_ps_flag(&st_gp);
    }

    interrupt_handler = 0;
  }

  if (st_gp.sensor_enabled) { // to check ps count

    delay(200); // ms

    read_ps_ave_data(&st_gp);

    output_data(&st_gp); // output the results to terminal
  }

}
