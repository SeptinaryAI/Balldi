/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "dbg.h"
#include "msg_serv.h"
#include "usrcmd_tof.h"

int usr_rd_tof(uint16_t *val_out)
{
	if(NULL == get_shm_tof_ptr())
	{
		printf("tof share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
    uint32_t dist = *((uint32_t *)get_shm_tof_ptr());

	DBG_F("distance: %u \n", dist);

	if(dist >= TOF_MAX_DISTANCE)
	{
		//printf("out of range.\n");
        *val_out = 0xFFFF;
		return RET_OUT_RANGE;
	}

	*val_out = (uint16_t)(dist);

	return RET_SUCCEED;
}


