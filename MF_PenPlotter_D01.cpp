/*
  MF_PenPlotter_D01.cpp
  
  for Arduino 1.8.2
  
  Kyosuke Ishikawa
  Email    : kyosuke@da2.so-net.ne.jp
  Facebook : https://www.facebook.com/kyosuke.ishikawa.mycomfab
  YouTube  : https://www.youtube.com/user/mycomfab
  
  The MIT License
  http://www.opensource.org/licenses/mit-license
  
  Version 0.1 (2017.05.13)
*/

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include "MF_PenPlotter_D01.h"

/*---------------------------------------------------------------------------*/
/* definitioniables                                                          */
/*---------------------------------------------------------------------------*/

#define SERVO_NUM		2
#define SERVO_TICK_NUM	50

#define ADC_NUM_MAX     1024
#define ADC_NUM_SCALE_X 550
#define ADC_NUM_SCALE_Y 410

/*---------------------------------------------------------------------------*/
/* global variables                                                          */
/*---------------------------------------------------------------------------*/

int mf_plot_pin_pos[SERVO_NUM] = {0, 1};
int pin_rc_srv = 9;

int servo_on_flag[SERVO_NUM];
int servo_tick_count[SERVO_NUM];
int servo_motor_on[SERVO_NUM];
int servo_pos_target[SERVO_NUM];
int servo_pos_current[SERVO_NUM];

Servo rc_srv;

/*---------------------------------------------------------------------------*/
/* local function prototypes                                                 */
/*---------------------------------------------------------------------------*/

int plot_x_conv_mm_to_ad(int mm);
int plot_y_conv_mm_to_ad(int mm);

void servo_init(void);
void servo_set_pos(int ch, int pos);
void servo_process(void);

/*---------------------------------------------------------------------------*/
/* mf_plot_init                                                              */
/*---------------------------------------------------------------------------*/

void mf_plot_init(void)
{
  Wire.begin();
  rc_srv.attach(pin_rc_srv);
  mf_plot_pen_up();
  servo_init();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_move_rapid                                                        */
/*---------------------------------------------------------------------------*/

void mf_plot_move_rapid(int x, int y)
{
  char	end_flag_x, end_flag_y;
  char	ch;
  int	servo_pos_x, servo_pos_y;
  int	servo_pos_target_x, servo_pos_target_y;
  
  servo_pos_target_x = plot_x_conv_mm_to_ad(x);
  servo_pos_target_y = plot_y_conv_mm_to_ad(y);
  end_flag_x = 0;
  end_flag_y = 0;
  
  while( end_flag_x == 0 || end_flag_y == 0 )	{
    delay(10);
    if ( end_flag_x == 0 )	{
      servo_pos_x = mf_plot_servo_adc_get(0);
      if ( servo_pos_target_x > servo_pos_x )	{
        mf_plot_motor_cont(0, MF_PLOT_MOT_BACK);
      } else if ( servo_pos_target_x < servo_pos_x )	{
        mf_plot_motor_cont(0, MF_PLOT_MOT_FORWARD);
      } else	{
        mf_plot_motor_cont(0, MF_PLOT_MOT_STOP);
        end_flag_x = 1;
      }
    }
    if ( end_flag_y == 0 )	{
      servo_pos_y = mf_plot_servo_adc_get(1);
      if ( servo_pos_target_y > servo_pos_y )	{
        mf_plot_motor_cont(1, MF_PLOT_MOT_BACK);
      } else if ( servo_pos_target_y < servo_pos_y )	{
        mf_plot_motor_cont(1, MF_PLOT_MOT_FORWARD);
      } else	{
        mf_plot_motor_cont(1, MF_PLOT_MOT_STOP);
        end_flag_y = 1;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* mf_plot_move                                                              */
/*---------------------------------------------------------------------------*/

void mf_plot_move(int x, int y)
{
  int	cur_x;
  int	cur_y;
  int	dis_x;
  int	dis_y;
  char	dir_x;
  char	dir_y;
  int	d;
  char	end_flag;
  char	ch;

  x = plot_x_conv_mm_to_ad(x);
  y = plot_y_conv_mm_to_ad(y);
  cur_x = mf_plot_servo_adc_get(0);
  cur_y = mf_plot_servo_adc_get(1);
  
  if ( cur_x < x )	{
    dis_x = x - cur_x;
    dir_x = 2;
  } else	{
    dis_x = cur_x - x;
    dir_x = -2;
  }
  if ( cur_y < y )	{
    dis_y = y - cur_y;
    dir_y = 2;
  } else	{
    dis_y = cur_y - y;
    dir_y = -2;
  }
  if ( cur_x == x && cur_y == y )	{
    return;
  }
  
  if ( dis_x > dis_y )	{
    d = 0;
    end_flag = 0;
    while( end_flag == 0 )	{
      if ( cur_x <= x + 1 && cur_x >= x - 1 )	{
        end_flag = 1;
      } else	{
        cur_x = cur_x + dir_x;
        servo_set_pos(0, cur_x);
        d = d + dis_y;
        if ( d >= dis_x )	{
          cur_y = cur_y + dir_y;
          servo_set_pos(1, cur_y);
          d = d - dis_x;
        }
        servo_process();
      }
    }
    servo_set_pos(0, x + dir_x * 2);
    servo_process();
  } else	{
    d = 0;
    end_flag = 0;
    while( end_flag == 0 )	{
      if ( cur_y <= y + 1 && cur_y >= y - 1 )	{
        end_flag = 1;
      } else	{
        cur_y = cur_y + dir_y;
        servo_set_pos(1, cur_y);
        d = d + dis_x;
        if ( d >= dis_y )	{
          cur_x = cur_x + dir_x;
          servo_set_pos(0, cur_x);
          d = d - dis_y;
        }
        servo_process();
      }
    }
    servo_set_pos(1, y + dir_y * 2);
    servo_process();
  }
}

/*---------------------------------------------------------------------------*/
/* mf_plot_pen_up                                                            */
/*---------------------------------------------------------------------------*/

void mf_plot_pen_up(void)
{
  rc_srv.write(map(0x0BC, 0, 1023, 0, 180));
  delay(300);
}

/*---------------------------------------------------------------------------*/
/* mf_plot_pen_down                                                          */
/*---------------------------------------------------------------------------*/

void mf_plot_pen_down(void)
{
  rc_srv.write(map(0x160, 0, 1023, 0, 180));
  delay(300);
}

/*---------------------------------------------------------------------------*/
/* mf_plot_pen_updown                                                        */
/*---------------------------------------------------------------------------*/

void mf_plot_pen_updown(void)
{
  rc_srv.write(map(0xF0, 0, 1023, 0, 180));
  delay(200);
  mf_plot_pen_down();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_motor_cont                                                        */
/*---------------------------------------------------------------------------*/

void mf_plot_motor_cont(int ch, int dir)  {
  Wire.beginTransmission(MF_PLOT_ADR_MOT_DRV + ch);
  Wire.write(0x00);
  Wire.write(0xFC + dir);
  Wire.endTransmission();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_motor_forward                                                     */
/*---------------------------------------------------------------------------*/

void mf_plot_motor_forward(int ch)  {
  Wire.beginTransmission(MF_PLOT_ADR_MOT_DRV + ch);
  Wire.write(0x00);
  Wire.write(0xFC+0x02);
  Wire.endTransmission();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_motor_back                                                        */
/*---------------------------------------------------------------------------*/

void mf_plot_motor_back(int ch)  {
  Wire.beginTransmission(MF_PLOT_ADR_MOT_DRV + ch);
  Wire.write(0x00);
  Wire.write(0xFC+0x01);
  Wire.endTransmission();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_motor_stop                                                        */
/*---------------------------------------------------------------------------*/

void mf_plot_motor_stop(int ch)  {
  Wire.beginTransmission(MF_PLOT_ADR_MOT_DRV + ch);
  Wire.write(0x00);
  Wire.write(0x03);
  Wire.endTransmission();
}

/*---------------------------------------------------------------------------*/
/* mf_plot_servo_adc_get                                                     */
/*---------------------------------------------------------------------------*/

int mf_plot_servo_adc_get(int ch)
{
  if ( ch < SERVO_NUM )	{
    return analogRead(mf_plot_pin_pos[ch]);
  } else	{
    return 0;
  }
}

/*---------------------------------------------------------------------------*/
/* servo_init                                                                */
/*---------------------------------------------------------------------------*/

void servo_init(void)
{
  char	i;
  
  for ( i=0 ; i<SERVO_NUM ; i++ )	{
    servo_on_flag[i]     = 0;
    servo_tick_count[i]  = 0;
    servo_motor_on[i]    = 0;
    servo_pos_current[i] = mf_plot_servo_adc_get(i);
    servo_pos_target[i]  = servo_pos_current[i];
  }
}

/*---------------------------------------------------------------------------*/
/* servo_set_pos                                                             */
/*---------------------------------------------------------------------------*/

void servo_set_pos(int ch, int pos)
{
  if ( ch < SERVO_NUM )	{
    servo_pos_target[ch] = pos;
    servo_on_flag[ch] = 1;
    servo_tick_count[ch] = 0;
    servo_motor_on[ch] = 1;
  }
}

/*---------------------------------------------------------------------------*/
/* servo_process                                                             */
/*---------------------------------------------------------------------------*/

void servo_process(void)
{
  char	ch;
  char	d;
  
  while ( servo_on_flag[0] == 1 || servo_on_flag[1] == 1 )	{
    delay(1);
    for ( ch=0 ; ch<SERVO_NUM ; ch++ )	{
      if ( servo_on_flag[ch] == 1 )	{
        if ( servo_motor_on[ch] == 1 )	{
          servo_pos_current[ch] = mf_plot_servo_adc_get(ch);
          if ( servo_pos_current[ch] >= servo_pos_target[ch]-1 && servo_pos_current[ch] <= servo_pos_target[ch]+1 )	{
            mf_plot_motor_cont(ch, MF_PLOT_MOT_STOP);
            servo_motor_on[ch] = 0;
          } else	{
            if ( servo_pos_current[ch] < servo_pos_target[ch] )	{
              mf_plot_motor_cont(ch, MF_PLOT_MOT_BACK);
            } else if ( servo_pos_current[ch] > servo_pos_target[ch] )	{
              mf_plot_motor_cont(ch, MF_PLOT_MOT_FORWARD);
            }
          }
        }
        servo_tick_count[ch]++;
        if ( servo_tick_count[ch] >= SERVO_TICK_NUM )	{
          servo_on_flag[ch] = 0;
          if ( servo_motor_on[ch] == 1 )	{
            mf_plot_motor_cont(ch, MF_PLOT_MOT_STOP);
            servo_motor_on[ch] = 0;
          }
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* plot_x_conv_mm_to_ad                                                      */
/*---------------------------------------------------------------------------*/

int plot_x_conv_mm_to_ad(int mm)
{
  int	ad;
  
  if ( mm < 0 )	{
    mm = 0;
  }
  ad = (long)(ADC_NUM_SCALE_X - mm) * ADC_NUM_MAX / ADC_NUM_SCALE_X;
  if ( ad >= ADC_NUM_MAX )	{
    ad = ADC_NUM_MAX - 1;
  }
  return ad;
}

/*---------------------------------------------------------------------------*/
/* plot_y_conv_mm_to_ad                                                      */
/*---------------------------------------------------------------------------*/

int plot_y_conv_mm_to_ad(int mm)
{
  int	ad;
  
  if ( mm < 0 )	{
    mm = 0;
  }
  ad = (long)mm * ADC_NUM_MAX / ADC_NUM_SCALE_Y;
  if ( ad >= ADC_NUM_MAX )	{
    ad = ADC_NUM_MAX - 1;
  }
  return ad;
}

