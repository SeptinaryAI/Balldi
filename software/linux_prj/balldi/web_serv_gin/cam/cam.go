/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
package cam

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lcamv4l2api -lpthread -ldl -lstdc++
#include "cam_v4l2.h"
#include "mnnyoloapi.h"
*/
import "C"
import (
	"errors"
	"sync"
	"unsafe"
)

const (
	CAM_STAT_GIVEUP	= 10
	CAM_STAT_SUCC	= 0

	RS_STAT_SUCC	= 0
	RS_STAT_GIVEUP	= 10

	RST_MAX_LEN		= 20
)

type Reason_rst struct {
	X1		int
	Y1		int
	X2		int
	Y2		int
	Label	int
	Id		int
	Score	float32
}

var camapi_lock sync.Mutex

func Init_camapi() (int, error) {
	camapi_lock.Lock()

	ret := C.init_v4l2_cam()
	if int(ret) != 0 {
		return int(ret), errors.New(string("init_video"))
	}

	camapi_lock.Unlock()
	return 0, nil
}

func Close_camapi() (int, error) {
	C.close_v4l2_cam()
	return 0, nil
}

func Set_use_yolo(val int) () {
	C.set_use_reasoning(C.int(val))
}

func Get_bytes() ([]byte, int, error) {
	var p *C.uchar
	var l C.int

	ret_ := C.get_frame(&p, &l)
	ret := int(ret_)

	if ret != 0 {
		if ret == RS_STAT_GIVEUP {
			//give up
			return nil, ret, nil
		}
		return nil, ret, errors.New(string("get_frame"))
	}

	return C.GoBytes(unsafe.Pointer(p), l), 0, nil
}

func Get_reason() ([]byte, int, []Reason_rst, error) {
	var p_ *C.uchar
	var l_ C.int
	var rsts_ C.t_rslts
	var l_rst_ C.int

	ret_ := C.get_frame_reason(&p_, &l_, &rsts_, &l_rst_)
	ret := int(ret_)

	//failed
	if ret != 0 {
		if ret == RS_STAT_GIVEUP {
			//give up
			return nil, ret, nil, nil
		}
		return nil, ret, nil, errors.New(string("get_reason"))
	}

	//no reasoning
	l_rst := int(l_rst_)
	if l_rst == 0 {

		C.get_frame_reason_finish()
		return nil, ret, nil, nil
	}

	var rsts_sw = make([]Reason_rst, l_rst)

	for i:=0; i<l_rst; i++ {
		rsts_sw[i].X1 		= int(	  	C.cgo_get_x1(	&rsts_, C.uint(i))	);
		rsts_sw[i].Y1 		= int(	  	C.cgo_get_y1(	&rsts_, C.uint(i))	);
		rsts_sw[i].X2 		= int(	  	C.cgo_get_x2(	&rsts_, C.uint(i))	);
		rsts_sw[i].Y2 		= int(	  	C.cgo_get_y2(	&rsts_, C.uint(i))	);
		rsts_sw[i].Label 	= int(		C.cgo_get_label(&rsts_, C.uint(i))	);
		rsts_sw[i].Id 		= int(	  	C.cgo_get_id(	&rsts_, C.uint(i))	);
		rsts_sw[i].Score 	= float32(	C.cgo_get_score(&rsts_, C.uint(i))	);
	}

	ret_bytes := C.GoBytes(unsafe.Pointer(p_), l_)

	C.get_frame_reason_finish()

	return ret_bytes, 0, rsts_sw, nil
}

