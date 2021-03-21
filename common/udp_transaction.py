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
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        period = eval(device.config).get('period')
        data_resp = device.device_id + "|" + device.device_ip + "|" + device.device_type + "|" + "ACK" + "|" + str(device.onoff) + "|" + period + "|\n"

        # 将ack包送往_1553b   进程共享字典m.dict()有点奇怪，不能直接append....
        list_send = _1553b['UDP_SEND']
        list_send.append([addr, data_resp])
        _1553b['UDP_SEND'] = list_send

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
        DEVICES.query.filter(DEVICES.device_id == data[0]).update({"device_ip": data[1], "last_active_at": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")})
        db.session.commit()
    else:
        print("[0100]未知动作:%s" % data[3])


#处理消息[设备0200]
#----"deviceuuid|localip|devicetype|doit|onoff|\n"----
def transaction_0200(data, addr, _1553b):
    if data[3] == 'SYN': #设备上线报文处理
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        if device is None:  #如果设备不存在，即自动注册
           device_new = DEVICES(device_id=data[0], device_type=data[2], device_ip=data[1], onoff=0, created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

           db.session.add(device_new)
           db.session.commit()

        #回送ACK包
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        data_resp = device.device_id + "|" + device.device_ip + "|" + device.device_type + "|" + "ACK" + "|" + str(device.onoff) + "|\n"

        # 将ack包送往_1553b   进程共享字典m.dict()有点奇怪，不能直接append....
        list_send = _1553b['UDP_SEND']
        list_send.append([addr, data_resp])
        _1553b['UDP_SEND'] = list_send

    elif data[3] == 'ALARM':  #收到侵入告警
        #告警事件入库
        DEVICE_0200_new = DEVICE_0200(device_id=data[0], alarm=data[5], created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

        db.session.add(DEVICE_0200_new)
        db.session.commit()
    elif data[3] == 'ACTIVE':  # 接收到心跳包文
        # 刷新最新活动时间
        DEVICES.query.filter(DEVICES.device_id == data[0]).update(
            {"device_ip": data[1], "last_active_at": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")})
        db.session.commit()
    else:
        print("[0200]未知动作:%s" % data[3])


#处理消息[设备0300]
#----"deviceuuid|localip|devicetype|doit|onoff|status|\n"----
def transaction_0300(data, addr, _1553b):
    if data[3] == 'SYN': #设备上线报文处理
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        if device is None:  #如果设备不存在，即自动注册
           device_new = DEVICES(device_id=data[0], device_type=data[2], device_ip=data[1], onoff=0, created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           device_0300_new_1 = DEVICE_0300(device_id=data[0], socket=1, status='1', created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           device_0300_new_2 = DEVICE_0300(device_id=data[0], socket=2, status='1', created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           device_0300_new_3 = DEVICE_0300(device_id=data[0], socket=3, status='1', created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           device_0300_new_4 = DEVICE_0300(device_id=data[0], socket=4, status='1', created_at=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
           db.session.add(device_new)
           db.session.add(device_0300_new_1)
           db.session.add(device_0300_new_2)
           db.session.add(device_0300_new_3)
           db.session.add(device_0300_new_4)
           db.session.commit()

        #回送ACK包
        device = DEVICES.query.filter_by(device_id=data[0]).first()
        data_resp = device.device_id + "|" + device.device_ip + "|" + device.device_type + "|" + "ACK" + "|" + str(device.onoff) + "|\n"

        # 将ack包送往_1553b   进程共享字典m.dict()有点奇怪，不能直接append....
        list_send = _1553b['UDP_SEND']
        list_send.append([addr, data_resp])
        _1553b['UDP_SEND'] = list_send
    elif data[3] == 'ACTIVE':  # 接收到心跳包文
        socket_1_status = (eval(data[5]))[0]
        socket_2_status = (eval(data[5]))[1]
        socket_3_status = (eval(data[5]))[2]
        socket_4_status = (eval(data[5]))[3]
        # 更新插座状态
        DEVICE_0300.query.filter(DEVICE_0300.device_id == data[0],DEVICE_0300.socket == 1).update({"status": socket_1_status})
        DEVICE_0300.query.filter(DEVICE_0300.device_id == data[0],DEVICE_0300.socket == 2).update({"status": socket_2_status})
        DEVICE_0300.query.filter(DEVICE_0300.device_id == data[0],DEVICE_0300.socket == 3).update({"status": socket_3_status})
        DEVICE_0300.query.filter(DEVICE_0300.device_id == data[0],DEVICE_0300.socket == 4).update({"status": socket_4_status})

        # 刷新最新活动时间
        DEVICES.query.filter(DEVICES.device_id == data[0]).update(
            {"device_ip": data[1], "last_active_at": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")})
        db.session.commit()
    else:
        print("[0300]未知动作:%s" % data[3])