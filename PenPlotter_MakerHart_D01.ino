/*
  PenPlotter_MakerHart_D01.ino
  
  for Arduino 1.8.2
  
  Kyosuke Ishikawa
  Email    : kyosuke@da2.so-net.ne.jp
  Facebook : https://www.facebook.com/kyosuke.ishikawa.mycomfab
  YouTube  : https://www.youtube.com/user/mycomfab
  
  The MIT License
  http://www.opensource.org/licenses/mit-license
  
  Version 0.1 (2017.05.13)
*/

#include <Math.h>
#include "MF_PenPlotter_D01.h"
#include "PlotData_WorldMap.h"

int pin_led = 13;
int pin_sw1 = 6;
int pin_sw2 = 7;
int pin_sw3 = 8;
int pin_ui_vr[2] = {2, 3};

int serial_control_pen_pos;
int serial_control_prev_x, serial_control_prev_y;

/*---------------------------------------------------------------------------*/
/* setup                                                                     */
/*---------------------------------------------------------------------------*/

void setup() {
  pinMode(pin_led, OUTPUT);
  pinMode(pin_sw1, INPUT_PULLUP);
  pinMode(pin_sw2, INPUT_PULLUP);
  pinMode(pin_sw3, INPUT_PULLUP);
  
  Serial.begin(9600);
  serial_control_pen_pos = 1;
  serial_control_prev_x = 0;
  serial_control_prev_y = 0;
  
  mf_plot_init();
  
  digitalWrite(pin_led, HIGH);
}

/*---------------------------------------------------------------------------*/
/* loop                                                                      */
/*---------------------------------------------------------------------------*/

void loop() {
  if ( digitalRead(pin_sw1) == 0 )	{
    main_manual_control();
  } else if ( digitalRead(pin_sw2) == 0 )	{
    main_plot_xmastree();
    mf_plot_move_rapid(MF_PLOT_X_MAX, 0);
  } else if ( digitalRead(pin_sw3) == 0 )	{
    main_plot_vector_datas((unsigned short *)plot_data_vector_wmap);
    mf_plot_move_rapid(MF_PLOT_X_MAX, 0);
  } else  {
    main_serial_control();
  }
}

/*---------------------------------------------------------------------------*/
/* main_plot_vector_datas                                                    */
/*---------------------------------------------------------------------------*/

void main_plot_vector_datas(unsigned short *plot_data_vector)
{
  unsigned short	ptr;
  unsigned char		pen_mode;
  unsigned char		end_flag;
  unsigned short	x1, y1;
  unsigned short	x2, y2;
  
  ptr = 0;
  pen_mode = 0;
  end_flag = 0;
  while ( end_flag == 0 )	{
    if ( pen_mode == 0 )	{
      x1 = pgm_read_word_near(plot_data_vector+ptr);
      ptr++;
      y1 = pgm_read_word_near(plot_data_vector+ptr);
      ptr++;
      x2 = x1;
      y2 = y1;
      mf_plot_move_rapid(x1, y1);
      mf_plot_pen_down();
      pen_mode = 1;
    } else	{
      if ( pgm_read_word_near(plot_data_vector+ptr) == 0xFFFF )	{
        ptr++;
        mf_plot_pen_up();
        end_flag = 1;
      } else if ( pgm_read_word_near(plot_data_vector+ptr) == 0x8000 )	{
        ptr++;
        mf_plot_pen_up();
        pen_mode = 0;
      } else	{
        x1 = pgm_read_word_near(plot_data_vector+ptr);
        ptr++;
        y1 = pgm_read_word_near(plot_data_vector+ptr);
        ptr++;
        if ( x1 == x2 || y1 == y2 )	{
          mf_plot_move_rapid(x1, y1);
        } else	{
          mf_plot_move(x1, y1);
        }
        x2 = x1;
        y2 = y1;
      }
    }
  }
  mf_plot_pen_up();
  mf_plot_move_rapid(MF_PLOT_X_MAX, 0);
}

/*---------------------------------------------------------------------------*/
/* main_plot_xmastree                                                        */
/*---------------------------------------------------------------------------*/

void main_plot_xmastree(void)
{
  int	star_size;
  int	x, y;
  int	x1, x2, y1, y2;
  int	n, nmax;
  
  star_size = 50;
  nmax = 20;
  main_plot_star(MF_PLOT_X_MAX-MF_PLOT_X_MAX/8, MF_PLOT_Y_MAX/2, star_size, 90);
  x1 = MF_PLOT_X_MAX-MF_PLOT_X_MAX/8-star_size/2;
  x2 = x>=MF_PLOT_X_MAX/8;
  mf_plot_pen_up();
  mf_plot_move_rapid(x1, MF_PLOT_Y_MAX/2);
  for ( n=0 ; n<nmax ; n++ )  {
    x = x1 - (x1-x2)*n/nmax;
    y1 = MF_PLOT_Y_MAX/2 - (MF_PLOT_Y_MAX*3/8)*n/nmax;
    y2 = MF_PLOT_Y_MAX/2 + (MF_PLOT_Y_MAX*3/8)*n/nmax;
    mf_plot_pen_up();
    for ( y=y1 ; y<=y2 ; y++ )  {
      if ( y==y1 )  {
        if ( y%30>15 )  {
          mf_plot_move_rapid(x - (y%30-15), y);
        } else  {
          mf_plot_move_rapid(x + (y%30-15), y);
        }
        mf_plot_pen_down();
      } else  {
        if ( y%30>15 )  {
          mf_plot_move(x - (y%30-15), y);
        } else  {
          mf_plot_move(x + (y%30-15), y);
        }
      }
    }
  }
  mf_plot_pen_up();
}

/*---------------------------------------------------------------------------*/
/* main_plot_star                                                            */
/*---------------------------------------------------------------------------*/

void main_plot_star(int x, int y, int l, int r)
{
  int	n;
  int	r1, r2;
  int	x1, y1, x2, y2;
  
  mf_plot_pen_up();
  for ( n=0 ; n<6 ; n++ )  {
    x1 = x + (int)(l * sin((n * 360 / 5 + r) * 2 * M_PI / 360));
    y1 = y - (int)(l * cos((n * 360 / 5 + r) * 2 * M_PI / 360));
    if ( n == 0 )  {
      mf_plot_move_rapid(x1, y1);
      mf_plot_pen_down();
    } else  {
      mf_plot_move(x1, y1);
      mf_plot_pen_updown();
    }
    if ( n<5)  {
      x2 = x - (int)(l/2 * sin(((n+3) * 360 / 5 + r) * 2 * M_PI / 360));
      y2 = y + (int)(l/2 * cos(((n+3) * 360 / 5 + r) * 2 * M_PI / 360));
      mf_plot_move(x2, y2);
      mf_plot_pen_updown();
    }
  }
  mf_plot_pen_up();
}

/*---------------------------------------------------------------------------*/
/* main_manual_control                                                       */
/*---------------------------------------------------------------------------*/

void main_manual_control(void)
{
  int	ch;
  int	val_pos, val_vr, val_dif;
  
  for ( ch=0 ; ch<2 ; ch++ )	{
    val_pos = mf_plot_servo_adc_get(ch);
    val_vr  = analogRead(pin_ui_vr[ch]);
    val_dif = (val_pos >> 2) - (val_vr >> 2);
    if ( val_dif > 0 )  {
      mf_plot_motor_cont(ch, MF_PLOT_MOT_FORWARD);
    } else if ( val_dif < 0 )  {
      mf_plot_motor_cont(ch, MF_PLOT_MOT_BACK);
    } else  {
      mf_plot_motor_cont(ch, MF_PLOT_MOT_STOP);
    }
  }
  delay(50);
  for ( ch=0 ; ch<2 ; ch++ )	{
    mf_plot_motor_cont(ch, MF_PLOT_MOT_STOP);
  }
}

/*---------------------------------------------------------------------------*/
/* main_serial_control                                                       */
/*---------------------------------------------------------------------------*/

void main_serial_control(void)
{
  int	dat, x, y;
  
  dat = Serial.read();
  if ( dat != -1 )  {
    if ( dat == 0xAA )  {
      Serial.write(0x55);
    } else if ( dat == 'E' )  {
      mf_plot_pen_up();
      serial_control_pen_pos = 1;
      mf_plot_move_rapid(MF_PLOT_X_MAX, 0);
      Serial.write(0x55);
    } else if ( dat == 'U' )  {
      mf_plot_pen_up();
      serial_control_pen_pos = 1;
      Serial.write(0x55);
    } else if ( dat == 'D' )  {
      mf_plot_pen_down();
      serial_control_pen_pos = 0;
      Serial.write(0x55);
    } else if ( dat == 'M' )  {
      do  {
        dat = Serial.read();
      } while ( dat == -1 );
      do  {
        x = Serial.read();
      } while ( x == -1 );
      x = x + dat * 0x100;
      do  {
        dat = Serial.read();
      } while ( y == -1 );
      do  {
        y = Serial.read();
      } while ( y == -1 );
      y = y + dat * 0x100;
      if ( x >= 0 && x <= MF_PLOT_X_MAX && y >= 0 && y <= MF_PLOT_Y_MAX )  {
        if ( serial_control_pen_pos == 1 )  {
          mf_plot_move_rapid(x, y);
        } else  {
          if ( serial_control_prev_x == x || serial_control_prev_y == y )  {
            mf_plot_move_rapid(x, y);
          } else  {
            mf_plot_move(x, y);
          }
        }
        serial_control_prev_x = x;
        serial_control_prev_y = y;
      }
      Serial.write(0x55);
    } else  {
      Serial.write(0x55);
    }
  }
}

