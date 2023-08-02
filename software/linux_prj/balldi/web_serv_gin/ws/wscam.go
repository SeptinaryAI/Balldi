/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
package ws

import (
	"time"
	"fmt"
	"bytes"
	"strconv"
	"net/http"
	"web_serv_gin/cam"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

var upgrader_cam = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

var upgrader_reason = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func Web_socket_cam(c *gin.Context) {
	ws, err := upgrader_cam.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}

	defer ws.Close()
	defer cam.Close_camapi()

	cam.Init_camapi()

	for {
		var pool []byte
		var stat int

		pool, stat, err = cam.Get_bytes()

		if err != nil {
			//error
			fmt.Printf("get frame error! %s\n", err.Error())
			time.Sleep(10 * time.Millisecond)
			break
		}

		if stat == cam.CAM_STAT_GIVEUP {
			//give up
			time.Sleep(10 * time.Millisecond)
			continue
		}

		if len(pool) == 0 {
			//get empty frame
			time.Sleep(10 * time.Millisecond)
			continue
		}

		//give-up-frame skip
		//if pool == nil {
		//	continue
		//}

		//fmt.Printf("pool len: %d\n", len(pool))

		//for i := 0; i < 20; i++ {
		//	fmt.Printf("%x ", pool[i])
		//}
		//fmt.Printf("\n")

		err = ws.WriteMessage(websocket.BinaryMessage, pool)
		if err != nil {
			fmt.Printf("ws write error! %s\n", err.Error())
			break
		}
	}
}


func Web_socket_reason(c *gin.Context) {
	ws, err := upgrader_reason.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}

	defer ws.Close()
    defer cam.Set_use_yolo(0)   //stop yolo reasoning

    cam.Set_use_yolo(1)   //start yolo reasoning

	for {
		var pool []byte
		var rsts []cam.Reason_rst
		var stat int
		var rsts_str bytes.Buffer

		pool, stat, rsts, err = cam.Get_reason()

		if err != nil {
			//error
			fmt.Printf("get reason results error! %s\n", err.Error())
			time.Sleep(20 * time.Millisecond)
			continue
		}

		if stat == cam.RS_STAT_GIVEUP {
			//give up
			time.Sleep(20 * time.Millisecond)
			continue
		}

		if rsts == nil {
			//empty result
			continue
		}

		for i := 0; i < len(rsts); i++ {
			//fmt.Printf("x1:%d y1:%d x2:%d y2:%d label:%d id:%d score:%f\n",
			//	rsts[i].X1,
			//	rsts[i].Y1,
			//	rsts[i].X2,
			//	rsts[i].Y2,
			//	rsts[i].Label,
			//	rsts[i].Id,
			//	rsts[i].Score)
			fmt.Println(rsts)
			rsts_str.WriteString(strconv.Itoa(rsts[i].X1))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.Itoa(rsts[i].Y1))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.Itoa(rsts[i].X2))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.Itoa(rsts[i].Y2))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.Itoa(rsts[i].Label))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.Itoa(rsts[i].Id))
			rsts_str.WriteString(",")
			rsts_str.WriteString(strconv.FormatFloat(float64(rsts[i].Score), 'f', 4, 32))
			rsts_str.WriteString(";")
		}

		err = ws.WriteMessage(websocket.TextMessage, []byte(rsts_str.String()))
		if err != nil {
			fmt.Printf("ws write reason result data error! %s\n", err.Error())
			break
		}
		err = ws.WriteMessage(websocket.BinaryMessage, pool)
		if err != nil {
			fmt.Printf("ws write reason result image error! %s\n", err.Error())
			break
		}
	}
}
