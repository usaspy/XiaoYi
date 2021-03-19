#!/usr/bin/python3
# coding=utf-8
from webapp.model import *
from webapp.WebApp import db
import socket
import datetime

#处理消息[设备0100]
#"deviceuuid|localip|devicetype|doit|onoff|period|\n";
def transaction_0100(data,addr,_1553b):
    if data[3] == 'SYN': #设备上线报文处理
        device = DEVICES.query.filter_by(device_id=data[0], device_type=data[2]).first()
        #print(device.config)
        if device is None:  #如果设备不存在，即自动注册
           device_new = DEVICES(device_id=data[0],device_type=data[2],device_ip=data[1],onoff=0,config="{'period':300}",created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           db.session.add(device_new)
           db.session.commit()

        #回送ACK包
        try:
            period = eval(device.config).get('period')
        except Exception:
            period = 300
        data_resp = device.device_id + "|" + device.device_ip + "|" + device.device_type + "|" + "ACK" + "|" + str(device.onoff) + "|" + str(period) + "|\n"

        sock_c = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock_c.sendto(data_resp.encode("utf-8"), addr)

    elif data[3] == 'ACTIVE':
        #db_session.query(User).filter(User.id == 179074001).update({"name":"XXX"})
        pass
    else:
        return

