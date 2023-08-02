/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef CAM_V4L2_API_H
#define CAM_V4L2_API_H

#ifdef __cplusplus
extern "C" {
#endif


//If youe camera don't support < 30Hz, you can set it to get fps < 30Hz
#define FRAME_JUMP_STEP         3  //3 frame will push 1 frame to stream, 30Hz  ->  10Hz

#define CAM_STAT_SUCC           0
#define CAM_STAT_GIVEUP         10

#define USR_BUF_SIZE    32

#define DEV_VIDEO "/dev/video0"
#define RET_ERR -1

typedef int (*fp_use_frame_raw)(unsigned char *addr, int len);

typedef struct
{
    void *start;
    int len;
} t_usrbuf;

int init_video(void);
int close_video(void);

int init_fmt(void);

int init_stream(void);
int close_stream(void);

int init_mmap(void);
int close_mmap();

//usr api
int init_v4l2_cam(void);
int close_v4l2_cam(void);

//usr api
void set_use_reasoning(int b);
int get_frame(unsigned char **pp, int *len);

#ifdef __cplusplus
}
#endif

#endif
