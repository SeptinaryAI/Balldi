/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef TOF_APP_H
#define TOF_APP_H
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define RET_ERROR 1
#define RET_SUCC  0

#define TOF_SHARED_DATE_LEN     (sizeof(uint32_t))
#define TOF_LOOP_PERIOD_US      20000

int init_tof(void);
int close_tof(void);
int tof_get_distance(void);

#ifdef __cplusplus
}
#endif
