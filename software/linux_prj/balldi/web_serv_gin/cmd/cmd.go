/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
package cmd

import (
    "fmt"
    "os/exec"

	"github.com/gin-gonic/gin"
)


func Command(c *gin.Context) {
    command := c.PostForm("cmd")
    fmt.Printf("%s\n", command)
    cmd := exec.Command("python3", "/home/pi/balldi/simple_IoT/scripts/keyword_parse.py", command)
    out, _ := cmd.CombinedOutput()
    fmt.Printf("%s\n", out)
}
