/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef USRCMD_MOVE_CTL_V3_H
#define USRCMD_MOVE_CTL_V3_H

#define MAX_TURN_LR_VAL             100
#define MIN_TURN_LR_VAL             -100

#define MAX_WHEEL_MOTO_RATE         90
#define MIN_WHEEL_MOTO_RATE         -80


#define PERIOD_MOVE_CONTROL         75000   //us


int usr_ctl_go_straight_v3(int rate);
int usr_ctl_turn_lr_v3(int turn);

#endif
