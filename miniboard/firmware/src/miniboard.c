/* OSU Robotics Club Rover 2016
 * Miniboard Firmware
 * 
 * miniboard.c - Main control loop.
 * Author(s): Nick Ames
 */
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <util/atomic.h>
#include "uart.h"
#include "comm.h"
#include "commgen.h"
#include "adc.h"
#include "tetrad.h"
#include "callsign.h"
#include "gps.h"
#include "compass.h"
#include "ax12.h"
#include "s-bus.h"
#include "videoswitch.h"
#include "imu.h"
#include "gpio.h"
#include "soil_sensor.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "time.h"

/* AX12 addresses */
#define PAN_AX12 1
#define TILT_AX12 2
#define PITCH_AX12 4
#define WRIST_AX12 5
#define SQUEEZE_AX12 6
#define CFLEX1_AX12 9
#define CFLEX2_AX12 8
#define CSEAL_AX12 7

/* Triggers for variable length data read/write commands. */
void camera_command_trigger(void){
	
}

void callsign_trigger(void){
	
}

void soil_sensor_send_trigger(void){
}

void soil_sensor_recv_trigger(void){
	
}

void soil_result_trigger(void){
	
}


/* ISR that fires if an interrupt is enabled without a corresponding handler. */
ISR(BADISR_vect){
	DDRB |= _BV(PB7);
	while(1){
		PORTB |= _BV(PB7);
		_delay_ms(100);
		PORTB &= ~_BV(PB7);
		_delay_ms(200);
		PORTB |= _BV(PB7);
		_delay_ms(300);
		PORTB &= ~_BV(PB7);
		_delay_ms(300);
	}
}

#define COMM_TIMEOUT 1 /* If this time (in seconds) elapses without a
                        * message from the computer, the connection is
                        * considered to be lost. In that case, S-BUS control
                        * may be enabled, or the rover may be paused.
                        * The maximum timeout is 4. */
                        
static bool CommTimedOut; /* If true, the connection to the computer has been lost. */

/* Start a countdown timer for COMM_TIMEOUT seconds.
 * Once it expires, CommTimedOut will be set to true */
void reset_timeout_timer(void){
	TCCR4B = 0;
	TCNT4 = 0;
	OCR4A = (uint16_t) 15625*COMM_TIMEOUT;
	TIMSK4 = _BV(OCIE4A);
	TCCR4B = _BV(CS42) | _BV(CS40); /* Set 1024 prescaler. */
	CommTimedOut = false;
}

ISR(TIMER4_COMPA_vect){
	CommTimedOut = true;
	TCCR4B = 0;
}

/* Setup all peripherals and subsystems. */
void init(void){
	comm_init();
	gps_init();
	comp_init();
	imu_init();
	sbus_init();
	soil_init();
	time_init();
	sei();
	reset_timeout_timer();
	ax12_init(AX12_BAUD);
	ax12_set_continuous_mode(PAN_AX12);
	ax12_set_continuous_mode(TILT_AX12);
	ax12_set_continuous_mode(PITCH_AX12);
	ax12_set_continuous_mode(WRIST_AX12);
	ax12_set_continuous_mode(SQUEEZE_AX12);
	ax12_set_continuous_mode(CFLEX1_AX12);
	ax12_set_continuous_mode(CFLEX2_AX12);
	ax12_set_continuous_mode(CSEAL_AX12);
}

/* Get a value, then atomically set the target variable. */
#define atomic_set(target, value) do {typeof(target) __valstore = value; ATOMIC_BLOCK(ATOMIC_RESTORESTATE){target = __valstore;} } while(0)

/* When inverting a int8_t, -128 would become 128, which is bad. This
 * function clamps an input to [127, 127]. */
uint8_t clamp127(int8_t value){
	if(value == -128){
		return -127;
	} else {
		return value;
	}
}

/* Direct control channel data mappings */
#define SA 0
#define SB 1
#define SC 2
#define SD 3
#define SWE 4
#define SF 5
#define SG 6
#define SH 7
#define XBOX_A 8
#define XBOX_B 9
#define XBOX_X 10
#define XBOX_Y 11
#define XBOX_LB 12
#define XBOX_RB 13
#define XBOX_LSC 14
#define XBOX_RSC 15
#define XBOX_SEL 16
#define XBOX_START 17
#define XBOX_HOME 18
#define XBOX_DPL 19
#define XBOX_DPR 20
#define XBOX_DPU 21
#define XBOX_DPD 22

/* Direct control channel data functions */
#define MODE_SWITCH SWE
#define TURN_SWITCH SG
#define PAUSE_SWITCH SF
#define DRIVE_LEFT Data->fr_joylv 
#define DRIVE_RIGHT Data->fr_joyrv
#define DRIVE_SPEED Data->fr_sidel
#define ARM_BASE Data->fr_joylh
#define ARM_BICEP Data->fr_joylv
#define ARM_FOREARM Data->fr_joyrv
#define ARM_PITCH Data->fr_joyrh
#define ARM_GRABBER Data->fr_sidel
#define ARM_EE Data->fr_sider
#define CAMERA_SELECT SH
#define PAN Data->xbox_joyrh
#define TILT Data->xbox_joyrv
#define ZOOM_IN Data->xbox_triggerr
#define ZOOM_OUT Data->xbox_triggerl
#define ARM_DRILL Data->fr_sidel
#define SAMPLE_CAM_FOCUS XBOX_A
#define SAMPLE_CAM_SHUTTER XBOX_B
#define CAMERA_INC XBOX_RB
#define CAMERA_DEC XBOX_LB
#define DRILL_FWD XBOX_DPU
#define DRILL_REV XBOX_DPD
#define SAMPLE_JOINT1 Data->xbox_joylv 
#define SAMPLE_JOINT2 Data->xbox_joylh
#define SAMPLE_SEAL_SWL XBOX_DPL
#define SAMPLE_SEAL_SWR XBOX_DPR


/* Camera indices */
#define CAM_FRONT 1
#define CAM_NAV 5
#define CAM_BACK 6
#define CAM_JOINT 2
#define CAM_EFECTOR 3
#define CAM_UNUSED 4

/* Wrapper for code reuse */
int8_t joy_ch(int8_t val){
	return val;
}
	
/* Return a joystick input switch value. */
bool switch_ch(uint8_t index){
	if(index < 8){
		return !!(Data->fr_buttons & _BV(index));
	} else if(index >=8 && index < 16){
		return !!(Data->xbox_buttons_low & _BV(index-8));
	} else if(index >=16 && index < 23){
		return !!(Data->xbox_buttons_high & _BV(index-16));
	} else {
		return 0;
	}
}
	
/* Set motor speeds based on joystick inputs, if
 * enable bit is set. */
void direct_control(void){
	static uint8_t prev_cam_inc;
	static uint8_t prev_cam_dec;
	static uint8_t prev_cam_select;
	static uint8_t prev_cam;
	uint8_t cam_inc, cam_dec, cam_select;
	bool quit;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		quit = !(Data->xbox_buttons_high & _BV(7));
		Data->xbox_buttons_high &= ~_BV(7);
	}
	if(quit)return;
	
	if(!switch_ch(PAUSE_SWITCH)){
		Data->pause_state = 1;
	} else {
		Data->pause_state = 0;
	}
	
	Data->pan_speed = -joy_ch(PAN);
	Data->tilt_speed = joy_ch(TILT);
	if(Data->selected_camera == CAM_NAV){
		if(joy_ch(ZOOM_IN) > 0){
			Data->nav_action = 2;
		} else if(joy_ch(ZOOM_OUT) > 0){
			Data->nav_action = 1;
		} else {
			Data->nav_action = 0;
		}
	} else {
		Data->nav_action = 0;
	}
	
	if(Data->selected_camera == CAM_EFECTOR){
		if(joy_ch(ZOOM_IN) > 0){
			Data->cam_action = 1;
		} else if(joy_ch(ZOOM_OUT) > 0){
			Data->cam_action = 2;
		} else if(switch_ch(SAMPLE_CAM_FOCUS)){
			Data->cam_action = 4;
		} else if(switch_ch(SAMPLE_CAM_SHUTTER)){
			Data->cam_action = 3;
		} else {
			Data->cam_action = 0;
		}
	} else {
		Data->cam_action = 0;
	}
	
	cam_select = switch_ch(CAMERA_SELECT);
	if(cam_select && !prev_cam_select){
		prev_cam = Data->selected_camera;
		Data->selected_camera++;
	}
	prev_cam_select = cam_select;
	cam_inc = switch_ch(CAMERA_INC);
	if(cam_inc && !prev_cam_inc){
		prev_cam = Data->selected_camera;
		Data->selected_camera++;
	}
	prev_cam_inc = cam_inc;
	cam_dec = switch_ch(CAMERA_DEC);
	if(cam_dec && !prev_cam_dec){
		prev_cam = Data->selected_camera;
		Data->selected_camera--;
	}
	prev_cam_dec = cam_dec;
	if(Data->selected_camera > 6){
		prev_cam = 6;
		Data->selected_camera = 1;
	}
	if(Data->selected_camera == 0){
		prev_cam = 1;
		Data->selected_camera = 6;
	}
	if(Data->selected_camera == CAM_UNUSED){
		if(prev_cam == (CAM_UNUSED - 1)){
			Data->selected_camera = CAM_UNUSED + 1;
		} else {
			Data->selected_camera = CAM_UNUSED - 1;
		}
	}
	
	if(switch_ch(DRILL_FWD)){
		Data->arm_motor_4 = 127;
	} else if(switch_ch(DRILL_REV)){
		Data->arm_motor_4 = -127;
	} else {
		Data->arm_motor_4 = 0;
	}
	
	Data->cflex1_speed = 4*joy_ch(SAMPLE_JOINT1);
	Data->cflex2_speed = 4*joy_ch(SAMPLE_JOINT2);
	
	if(switch_ch(SAMPLE_SEAL_SWR)){
		Data->cseal_speed = -400;
	} else if(switch_ch(SAMPLE_SEAL_SWL)){
		Data->cseal_speed = 400;
	} else {
		Data->cseal_speed = 0;
	}
	
	
	if(!switch_ch(MODE_SWITCH)){
		/* Mode 1 - Drive */
		int8_t left = joy_ch(DRIVE_LEFT);
		int8_t right = joy_ch(DRIVE_RIGHT);
		int16_t speed_factor = joy_ch(DRIVE_SPEED) + 128;
		speed_factor = 256 - speed_factor;
		speed_factor = 1 + (speed_factor/64);
		Data->l_f_drive = left/speed_factor;
		Data->l_m_drive = left/speed_factor;
		Data->l_b_drive = left/speed_factor;
		Data->r_f_drive = right/speed_factor;
		Data->r_m_drive = right/speed_factor;
		Data->r_b_drive = right/speed_factor;
		Data->swerve_state = switch_ch(TURN_SWITCH);
		Data->arm_mode = 0;
		if(switch_ch(TURN_SWITCH)){
			Data->swerve_state = 2;
		} else {
			Data->swerve_state = 1;
		}
	} else {
		/* Mode 2 - Arm */
		Data->arm_motor_1 = joy_ch(ARM_BASE);
		Data->arm_motor_2 = -joy_ch(ARM_BICEP);
		Data->arm_motor_3 = joy_ch(ARM_FOREARM);
		Data->arm_motor_5 = 0;
		Data->ee_speed = -8*joy_ch(ARM_EE);
		Data->arm_mode = 1;
		Data->grabber_rotation_speed = 8*joy_ch(ARM_PITCH);
		Data->grabber_speed = 8*joy_ch(ARM_GRABBER);
	}
}
	
void miniboard_main(void){
	init();
	
	bool super_pause;
	/* Miniboard main loop. */
	while(1){
		super_pause = !Data->pause_state || CommTimedOut;
		/* GPS */
		/* (handled in-module) */
		
		/* Direct Control */
		direct_control();
		
		/* Tetrad */
		uart_wait(AX12_UART);
		tetrad_init();
		if(super_pause){
			/* Paused */
			tetrad_set_speed(0, 0, 0);
			tetrad_set_speed(1, 0, 0);
			tetrad_set_speed(2, 0, 0);
			tetrad_set_speed(3, 0, 0);
			tetrad_set_speed(4, 0, 0);
			tetrad_set_speed(5, 0, 0);
		} else {
			/* Not Paused */
			tetrad_set_speed(0, -clamp127(Data->l_f_drive), Data->r_m_drive);
			tetrad_set_speed(1, -clamp127(Data->l_m_drive), Data->r_f_drive);
			tetrad_set_speed(2, -clamp127(Data->l_b_drive), Data->r_b_drive);
			int8_t swerve_speed;
			if(1 == Data->swerve_state){
				/* Staight */
				swerve_speed = -127;
			} else if(2 == Data->swerve_state){
				/* Turn */
				swerve_speed = 127;
			} else {
				/* No motion */
				swerve_speed = 0;
			}
			tetrad_set_speed(3, swerve_speed, -clamp127(Data->arm_motor_1));
			tetrad_set_speed(4, Data->arm_motor_2, -clamp127(Data->arm_motor_3));
			tetrad_set_speed(5, Data->arm_motor_4, Data->arm_motor_5);
		}
		
		/* Video Switch */
		videoswitch_select(Data->selected_camera);
		
		/* Battery voltage */
		atomic_set(Data->battery_voltage, battery_mV());
		
		/* Compass */
		compass_retrieve();
		
		/* AX12 */
		uart_wait(AX12_UART);
		ax12_init(AX12_BAUD);
		if(super_pause) {
			ax12_disable(AX12_ALL_BROADCAST_ID);
		} else {
			ax12_enable(AX12_ALL_BROADCAST_ID);
			ax12_set_goal_position(Data->ax12_addr, Data->ax12_angle);
			ax12_continuous_speed(PAN_AX12, Data->pan_speed*3);
			ax12_continuous_speed(TILT_AX12, Data->tilt_speed*3);
			ax12_continuous_speed(CFLEX1_AX12, Data->cflex1_speed);
			ax12_continuous_speed(CFLEX2_AX12, Data->cflex2_speed);
			ax12_continuous_speed(CSEAL_AX12, Data->cseal_speed);
			if(Data->arm_mode != 0){
				ax12_continuous_speed(PITCH_AX12, Data->ee_speed);
				ax12_continuous_speed(WRIST_AX12, Data->grabber_rotation_speed);
				ax12_continuous_speed(SQUEEZE_AX12, Data->grabber_speed);
				
			}
		}

		/* S-Bus */
		/* Handled in module. */
		
		/* Soil Sensor */
		if(Data->soil_measure == 1){
			soil_measure();
		}
		
		/* IMU */
 		imu_accel(&Data->accel_x, &Data->accel_y, &Data->accel_z);
		imu_gyro(&Data->gyro_x, &Data->gyro_y, &Data->gyro_z);
		
		/* GPIO */
		Data->gpio_state = gpio_get_state();
		
		/* Sample Camera Actions */
		sample_cam_button(Data->cam_action);
		
		/* Navigation camera actions */
		nav_cam_button(Data->nav_action);
		
		/* Blink LED. */
		DDRB |= _BV(PB7);
		PORTB ^= _BV(PB7);
	}
}

// void copy_txd3(void){
// 	/* A0 = PF0 = non-inverting */
// 	/* A1 = PF1 = inverting */
// 	/* TXD3 = PJ1 */
// 	PORTF |= _BV(PF0);
// 	PORTF &= ~_BV(PF1);
// 	DDRF |= _BV(PF1) | _BV(PF0);
// 	_delay_ms(1);
// 	while(uart_tx_in_progress(3)){
// 		if(PINJ & _BV(PJ1)){
// 			PORTF |= _BV(PF0);
// 			PORTF &= ~_BV(PF1);
// 		} else {
// 			PORTF &= ~_BV(PF0);
// 			PORTF |= _BV(PF1);
// 		}
// 	}
// 	for(int i=10000;i>0;i--){
// 		if(PINJ & _BV(PJ1)){
// 			PORTF |= _BV(PF0);
// 			PORTF &= ~_BV(PF1);
// 		} else {
// 			PORTF &= ~_BV(PF0);
// 			PORTF |= _BV(PF1);
// 		}
// 	}
// 	DDRF &= ~(_BV(PF1) | _BV(PF0));
// 	PORTF &= ~(_BV(PF1) | _BV(PF0));
// }
// 
// 
// void soil_sensor_test(void){
// 	uart_enable(0, 9600, 1, 0);
// 	uart_enable(3, 9600, 1, 0);
// 	sei();
// 	char str[256];
// 	snprintf(str, 256, "Hello\r\n");
// 	uart_tx(0, str, strlen(str));
// 	snprintf(str, 256, "///FV=?\r\n");
// 	uart_tx(3, str, strlen(str));
// 	copy_txd3();
// 	while(1){
// 		char c;
// 		if(uart_rx(3, &c, 1)){
// 			uart_tx(0, &c, 1);
// 		}
// 	}
// 	
// 	while(1);
// }
// 
void ax12_test(void){
	init();
	uint8_t target_addr = 4;
	while(1){
		ax12_init(1000000);
		ax12_enable(AX12_ALL_BROADCAST_ID);
		ax12_toggle_led(AX12_ALL_BROADCAST_ID, 1);
		ax12_set_id(AX12_ALL_BROADCAST_ID, target_addr);
		ax12_set_baud_rate(AX12_ALL_BROADCAST_ID, 9);
		_delay_ms(300);
		DDRB |= _BV(PB7);
 		PORTB ^= _BV(PB7);
	}
}

void ax12_reset_addr(void){
	uint8_t target_addr = 9;
	init();
	while(1){
		for(uint16_t i=0;i<255;i++){
			uint32_t speed = 2000000UL / (i + 1);
			for(uint8_t i=0;i<3;i++){
				ax12_init(speed);
				ax12_toggle_led(AX12_ALL_BROADCAST_ID, 1);
				ax12_set_id(AX12_ALL_BROADCAST_ID, target_addr);
				ax12_set_baud_rate(AX12_ALL_BROADCAST_ID, 9);
				uart_wait(AX12_UART);
				_delay_ms(4);
				DDRB |= _BV(PB7);
				PORTB ^= _BV(PB7);
			}
		}
	}
}

// void ax12_test2(){
// 	ax12_init();
// 	while(1){
// 		/* Put an AX12 into continuous rotation mode. */
// 		ax12_set_continuous_mode(5);
// 		ax12_continuous_speed(5, 0);
// 		_delay_ms(100);
// 		DDRB |= _BV(PB7);
// 		PORTB ^= _BV(PB7);
// 	}
// }

int main(void){
	/* For testing, remove the following call and insert your code below.
	 * You might need to copy stuff from init(). Don't commit your modified
	 * miniboard.c to the main branch! */
	miniboard_main();
	//soil_sensor_test();
	//ax12_test();
	//ax12_reset_addr();
	return(0);
}
