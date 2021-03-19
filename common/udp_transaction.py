#!/usr/bin/python3
# coding=utf-8
from webapp.model import *
from webapp.WebApp import db
import socket
import datetime

#处理消息[设备0100]
#----"deviceuuid|localip|devicetype|doit|onoff|period|\n"----
def transaction_0100(data, addr, _1553b):
    if data[3] == 'SYN': #设备上线报文处理
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        if device is None:  #如果设备不存在，即自动注册
           config = {'period': data[5]}
           device_new = DEVICES(device_id=data[0], device_type=data[2], device_ip=data[1], onoff=0, config=str(config), created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

           db.session.add(device_new)
           db.session.commit()

        #回送ACK包
        period = eval(device.config).get('period')
        data_resp = device.device_id + "|" + device.device_ip + "|" + device.device_type + "|" + "ACK" + "|" + str(device.onoff) + "|" + period + "|\n"

        sock_c = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock_c.sendto(data_resp.encode("utf-8"), addr)

    elif data[3] == 'ACTIVE':  #接收到心跳包文或正常数据报文
        if data[4] == "1":
            humidity = (eval(data[6]))[0]
            temperature_c = (eval(data[6]))[1]
            temperature_f = (eval(data[6]))[2]
            heat_index = (eval(data[6]))[3]
            air_quality = eval(data[7])

            #环境数据入库
            DEVICE_0100_new = DEVICE_0100(device_id=data[0], humidity=humidity, temperature_f=temperature_f,
                                          temperature_c=temperature_c, heat_index=heat_index, air_quality=str(air_quality),
                                          created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

            db.session.add(DEVICE_0100_new)
            db.session.commit()
        #刷新最新活动时间
        DEVICES.query.filter(DEVICES.device_id == data[0]).update({"last_active_at": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")})
        db.session.commit()
    else:
        return

