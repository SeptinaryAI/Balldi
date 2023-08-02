//global use
var ws_url = "ws://" + window.location.host + "/wsdrv";
var ws;

var ws_conn_times = 0;
var ws_cmd_get_dic = {};

var ws_callback =
{
    'ws_log':console.log
}

function reg_ws_ws_log(func) {
    ws_callback['ws_log'] = func;
}

function reg_cmd_get_dic(dict) {
    ws_cmd_get_dic = dict;
}

function ws_log(text) {
    ws_callback['ws_log'](text);
}

function ws_reconn(url) {
    setTimeout(function() {
        ws_init();
    }, 1000);
}

function ws_init(){
    ws = new WebSocket(ws_url);

    ws.onopen = function() {
        if(0 == ws_conn_times)
            ws_log("websocket connected !");
        else
            ws_log("websocket reconnected ! times: " + ws_conn_times);
        ws_conn_times++;
    }

    ws.onmessage = function(event) {
        let ws_get = event.data;
        let slice = ws_get.split(";");

        if(slice.length < 2)
            return ;

        if(undefined == ws_cmd_get_dic[slice[0]])
            ws_cmd_get_dic[slice[0]] = 0;

        let val = parseInt(slice[1]);

        if(isNaN(val))
            return ;

        ws_cmd_get_dic[slice[0]] = val;
    }

    ws.onerror = function() {
        ws_log("ws error. reconnecting ....");
        ws_reconn(url);
    }

    ws.onclose = function() {
        ws_log("ws closed. reconnecting ....");
        ws_reconn(url);
    }
}

function ws_send(cmd, val) {
    if((null != ws) && (ws.OPEN == ws.readyState))
    {
        ws.send(cmd + ";" + val);
    }
    else
    {
        ws_reconn();
    }
}

//class
class Ws_drv {
    constructor(url, write_log, cmd_get_dict) {
        this.url = url;
        this.conn_times = 0;
        this.cmd_get_dic = cmd_get_dict;
        this.write_log = write_log;
    }

    init() {
        let instance = this;
        this.ws = new WebSocket(this.url);
        this.ws.onopen = function() {
            if(0 == instance.conn_times)
                instance.write_log("websocket connected !");
            else
                instance.write_log("websocket reconnected ! times: " + instance.conn_times);
            instance.conn_times++;
        }

        this.ws.onmessage = function(event) {
            let ws_get = event.data;
            let slice = ws_get.split(";");

            if(slice.length < 2)
                return ;

            if(undefined == instance.cmd_get_dic[slice[0]])
                instance.cmd_get_dic[slice[0]] = 0;

            let val = parseInt(slice[1]);

            if(isNaN(val))
                return ;

            instance.cmd_get_dic[slice[0]] = val;
        }

        this.ws.onerror = function() {
            instance.ws.close();
            instance.write_log("ws error.");
        }

        this.ws.onclose = function() {
            instance.reconn(instance.url);
            instance.write_log("ws closed. reconnecting ....");
            instance.ws = null;
        }

        setTimeout(function() {
            if((null != this.ws) && (instance.ws.OPEN != instance.ws.readyState))
            {
                instance.init();
            }
        }, 2000);
    }

    send_cmd(cmd, val) {
        if((null != this.ws) && (this.ws.OPEN == this.ws.readyState))
        {
            this.ws.send(cmd + ";" + val);
        }
    }

    write_log(str) {
        console.log(str);
    }

    reconn() {
        let instance = this;
        setTimeout(function() {
            instance.init();
        }, 1000);
    }
}

class Ws_cam {
    constructor(url, write_log) {
        this.url = url;
        this.conn_times = 0;
        this.ws = new WebSocket(this.url);
        this.write_log = write_log;
    }

    init() {
        let instance = this;
        this.ws.onopen = function() {
            if(0 == instance.conn_times)
                instance.write_log("websocket connected !");
            else
                instance.write_log("websocket reconnected ! times: " + instance.conn_times);
            instance.conn_times++;
        }

        this.ws.onmessage = function(event) {
            let ws_get = event.data;
            //do something with dat
            console.log(ws_get);
        }

        this.ws.onerror = function() {
            instance.write_log("ws error. reconnecting ....");
        }

        this.ws.onclose = function() {
            instance.reconn(instance.url);
            instance.write_log("ws closed. reconnecting ....");
        }
    }

    send_cmd(cmd, val) {
        if((null != this.ws) && (this.ws.OPEN == this.ws.readyState))
        {
            if(cmd == "ping")
                this.ws.send("ping");
            else if(cmd == "get_frame")
                this.ws.send("get_frame");
            else
                this.write_log("error cmd");
        }
        else
        {
            this.reconn();
        }
    }

    write_log(str) {
        console.log(str);
    }

    reconn() {
        setTimeout(function() {
            this.init();
        }, 1000);
    }
}

export { reg_ws_ws_log, reg_cmd_get_dic, ws_init, ws_send, Ws_drv, Ws_cam}
