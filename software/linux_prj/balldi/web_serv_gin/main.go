/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
package main

import (
	"net/http"
	"web_serv_gin/ws"
	"web_serv_gin/cmd"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.LoadHTMLGlob("templates/*")

	r.Static("/js", "./js")
	r.Static("/css", "./css")
	r.StaticFile("/balldi.stl", "./res/balldi.stl")

	r.GET("/ping", func(c *gin.Context) {
		c.JSON(200, gin.H{
			"message": "pong",
		})
	})

	r.GET("/wsdrv", ws.Web_socket)			//机电驱动
	r.GET("/wscam", ws.Web_socket_cam)		//摄像头
	r.GET("/wsrsn", ws.Web_socket_reason)	//推理结论

    r.POST("/command/", cmd.Command)

	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "index.html", gin.H{
			//
		})
	})
	r.GET("/control", func(c *gin.Context) {
		c.HTML(http.StatusOK, "control.html", gin.H{
			//
		})
	})
	r.GET("/pos", func(c *gin.Context) {
		c.HTML(http.StatusOK, "pos.html", gin.H{
			//
		})
	})
	r.GET("/func", func(c *gin.Context) {
		c.HTML(http.StatusOK, "func.html", gin.H{
			//
		})
	})
	r.GET("/cam", func(c *gin.Context) {
		c.HTML(http.StatusOK, "cam.html", gin.H{
			//
		})
	})
	r.GET("/led", func(c *gin.Context) {
		c.HTML(http.StatusOK, "led.html", gin.H{
			//
		})
	})
	r.GET("/bat", func(c *gin.Context) {
		c.HTML(http.StatusOK, "bat.html", gin.H{
			//
		})
	})
	r.GET("/cmd", func(c *gin.Context) {
		c.HTML(http.StatusOK, "cmd.html", gin.H{
			//
		})
	})

	r.Run() // listen and serve on 0.0.0.0:8080
}
