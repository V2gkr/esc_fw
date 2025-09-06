#ifndef __MOTOR_CONFIG_H 
#define __MOTOR_CONFIG_H

#define DC_VOLTAGE          12

#define FULL_CYCLE_FREQ     100
#define SIX_STEP_FREQ       (FULL_CYCLE_FREQ*6)

#define MOTOR_KV            1200
#define MOTOR_POLE_PAIR     7
#define MAX_RPM             (MOTOR_KV/DC_VOLTAGE)
#define MAX_RPM_ALLOWED     ((MAX_RPM*100)/80) /* max speed without load*/


#endif