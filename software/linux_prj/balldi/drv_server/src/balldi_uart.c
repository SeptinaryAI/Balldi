/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <termios.h>
#include <fcntl.h>
#include "balldi_uart.h"

static int fd_balldi;

int get_fd_balldi(void)
{
    return fd_balldi;
}

void init_uart_fd(int fd)
{
    struct termios tio;
    bzero(&tio, sizeof(struct termios));

    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;

    tio.c_iflag &= ~(IXON|IXOFF|IXANY);
    tio.c_iflag &= ~(INLCR|ICRNL|IGNCR);

    tio.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);

    tio.c_oflag &= ~OPOST;
    tio.c_oflag &= ~(ONLCR|OCRNL);
    tio.c_oflag &= ~(ONLRET);

    tio.c_cc[VTIME] = 1;
    tio.c_cc[VMIN] = 0;

    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);

    ioctl(fd, TCFLSH, 2);

    if(0 != tcsetattr(fd, TCSANOW, &tio))
    {
        printf("serial init set error.\n");
    }
}

void init_uart(void)
{
    fd_balldi = open(FD_BALLDI, O_RDWR | O_NOCTTY | O_NONBLOCK);
    //fd_balldi = open(FD_BALLDI, O_RDWR | O_NOCTTY);

    if(-1 == fd_balldi)
    {
        printf("uart init failed .\n");
		return ;
    }

    if(fcntl(fd_balldi, F_SETFL, 0) < 0)
    {
        printf("can't open balldi serial port!\n");
		return ;
    }

    init_uart_fd(fd_balldi);
}

void close_uart_fd(void)
{
    if(-1 == fd_balldi)
        return;

    close(fd_balldi);
}

