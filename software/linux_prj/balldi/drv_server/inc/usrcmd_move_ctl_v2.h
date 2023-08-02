/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef USRCMD_MOVE_CTL_V2_H
#define USRCMD_MOVE_CTL_V2_H

#define MAX_TURN_LR_VAL             1180
#define MIN_TURN_LR_VAL             -1180

#define MAX_WHEEL_MOTO_RATE         80
#define MIN_WHEEL_MOTO_RATE         -80

#define MAX_WHEEL_MOTO_RATE_ADJ     40
#define MIN_WHEEL_MOTO_RATE_ADJ     -40
#define DLY_WHEEL_MOTO_RATE_ADJ     5

#define RATE_LOW_PASS_STEP          8
#define WHEEL_RATE_FIX_INTEGER      35      //because I use brush DC Motor.... I should test my car to adjust this macro

#define MAX_FLY_MOTO_RATE           100
#define MIN_FLY_MOTO_RATE           -100

#define EMERGENCY_STOP_THR          45.0    //pitch threshold

#define PERIOD_MOVE_CONTROL         75000   //us
#define MAX_STOP_STEP               3      //max steps after stoping sending control cmd

#define D_ABS(i)           ((i) < 0 ? -(double)(i) : (double)(i))

#define IS_FLOAT_ZERO(f)   ( ( (f) < 0.000001 && (f) > -0.000001 ) ? 1 : 0 )

enum e_move_status {
    MVSTAT_STOP,
    MVSTAT_MOVING,
};

int usr_ctl_go_straight_v2(int rate);
int usr_ctl_turn_lr_v2(int turn);
int usr_ctl_set_move_lr_enable_v2(int b);
int usr_ctl_set_move_fly_enable_v2(int b);

#endif
