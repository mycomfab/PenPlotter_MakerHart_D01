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

#ifndef MF_PENPLOTTER_D01_h
#define MF_PENPLOTTER_D01_h

/*---------------------------------------------------------------------------*/
/* definitirons                                                              */
/*---------------------------------------------------------------------------*/

#define MF_PLOT_X_MAX      500
#define MF_PLOT_Y_MAX      400

#define MF_PLOT_ADR_MOT_DRV		0x60
#define MF_PLOT_MOT_FORWARD		0x02
#define MF_PLOT_MOT_BACK		0x01
#define MF_PLOT_MOT_STOP		0x03

/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/

void mf_plot_init(void);
void mf_plot_move(int x, int y);
void mf_plot_move_rapid(int x, int y);

void mf_plot_pen_up(void);
void mf_plot_pen_down(void);
void mf_plot_pen_updown(void);

int mf_plot_servo_adc_get(int ch);

void mf_plot_motor_cont(int ch, int dir);
void mf_plot_motor_forward(int ch);
void mf_plot_motor_back(int ch);
void mf_plot_motor_stop(int ch);

#endif
