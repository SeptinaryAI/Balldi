#!/usr/bin/python3

from socketserver import BaseRequestHandler, TCPServer, ThreadingTCPServer
from threading import Thread

device_led_clock_stat = b'0'



class handler(BaseRequestHandler):

    def thread_serv(self, req):
        while True:
            global device_led_clock_stat
            msg = req.recv(128)
            #print('get: ' + msg.decode('utf-8'))

            if b'device_led_clock_get_stat' in msg:
                print('get: ' + msg.decode('utf-8'))
                snd = device_led_clock_stat+b':device_led_clock_stat'
                print('try send: ' + snd.decode('utf-8'))
            elif b'device_led_clock_set_stat' in msg:
                print('get: ' + msg.decode('utf-8'))
                l = msg.split(b':')
                if len(l) == 2:
                    device_led_clock_stat = l[1]
                snd = b'set device_led_clock_stat:'+device_led_clock_stat
                print('try send: ' + snd.decode('utf-8'))
            else:
                snd = b'unknown cmd'

            try:
                req.send(snd)
            except Exception as e:
                print('Error:', e)
                break
            finally:
                None

    def handle(self):
        snd = b''
        print('connection from ', self.client_address)
        self.thread_serv(self.request)
        #thread = Thread(target=self.thread_serv, args=(self.request,))
        #thread.start()


if __name__ == '__main__':
    serv = ThreadingTCPServer(('0.0.0.0', 8081), handler)
    print('serv on 8081')
    serv.serve_forever()
