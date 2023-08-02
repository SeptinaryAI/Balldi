/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef LED_SHM_H
#define LED_SHM_H

#define SHM_APP_FTOK_FILE "/tmp_app_shm"
#define SHM_APP_ID        3
#define SHM_LEN       1024


#define APP_SHM_ADDR_LED  128     //led app will use tmp_add_shm start from APP_SHM_ADDR_LED

int init_shm(void);
void close_shm(void);
size_t get_led_shm_addr(void);

#endif
