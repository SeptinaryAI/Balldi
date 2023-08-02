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
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"

	"web_serv_gin/drv"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func Web_socket(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	defer ws.Close()
	defer drv.Send_cmd(drv.CMD_SET_SERVO_TARGET_TICK, 28)	//pitch return
	defer drv.Send_cmd(drv.CMD_SET_SPEED_LR, 0)	//speed clean
	defer drv.Close_drvapi()

	if 0 != drv.Init_drvapi() {
        return
    }

	for {
		mt, message, err := ws.ReadMessage()
		if err != nil {
			fmt.Printf("ws read error! err %s\n", err.Error())
			break
		}
		slice := strings.Split(string(message), ";")
		if len(slice) < 2 {
			ws.WriteMessage(mt, []byte("err;fmt"))
			continue
		}
		//fmt.Printf("cmd:%s val_send:%s\n", slice[0], slice[1])

		cmd, err := strconv.Atoi(slice[0])
		if err != nil {
			ws.WriteMessage(mt, []byte("err;fmt"))
			continue
		}
		val_send, err := strconv.Atoi(slice[1])
		if err != nil {
			ws.WriteMessage(mt, []byte("err;fmt"))
			continue
		}

		val_ret, err := drv.Send_cmd(cmd, val_send)
		if err != nil {
			fmt.Printf("error: send_cmd ret 0x%x\n", err.Error())
			ws.WriteMessage(mt, []byte("err;send"))
			continue
		}
		//fmt.Printf("val_ret:%d\n", val_ret)

		err = ws.WriteMessage(mt, []byte(strconv.Itoa(cmd)+";"+strconv.Itoa(val_ret)))
		if err != nil {
			fmt.Printf("ws write error! %s\n", err.Error())
			break
		}
	}
}
