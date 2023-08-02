/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
package drv

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -ldrvapi
#include "drv_api.h"
*/
import "C"
import (
	"errors"
	"sync"
)

const (
	RET_SUCCEED_REPLY           = 10
	CMD_SET_SERVO_TARGET_TICK   = 0xA1
	CMD_SET_SERVO2_TARGET_TICK  = 0xA2
	CMD_SET_EN_LR               = 0xA8
	CMD_SET_SPEED_LR            = 0xAB
	CMD_SET_PROG_RESIS          = 0xC2
)

var drvapi_lock sync.Mutex

func Init_drvapi() (int){
	drvapi_lock.Lock()
	ret := int(C.init_api())
	drvapi_lock.Unlock()
    return ret
}

func Close_drvapi() {
	C.close_api()
}

func Send_cmd(cmd, val_send int) (int, error) {
	val_ret_c := C.int(0)

	ret_c := C.send_cmd(C.int(cmd), C.int(val_send), &val_ret_c)
	ret := int(ret_c)

	if ret != 0 && ret != RET_SUCCEED_REPLY {
		return int(val_ret_c), errors.New(string(ret))
	}

	return int(val_ret_c), nil
}

/*
func main() {
	val_ret := C.int(0)

	C.init_api()

	ret := C.send_cmd(CMD_GET_BAT_VOLTAGE, 0, &val_ret)
	fmt.Printf("ret = %d val_ret = %d\n", ret, val_ret)

	C.close_api()
}
*/
