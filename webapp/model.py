from webapp.WebApp import db

class DEVICES(db.Model):
    __tablename__='devices'
    device_id = db.Column(db.String(255),primary_key=True)
    device_name = db.Column(db.String(255))
    device_type = db.Column(db.String(255))
    device_mac = db.Column(db.String(255))
    device_ip = db.Column(db.String(255))
    onoff = db.Column(db.BIGINT)
    config = db.Column(db.String(255))
    position = db.Column(db.BIGINT)
    XYZT = db.Column(db.String(255))
    comments = db.Column(db.String(255))
    created_at = db.Column(db.DateTime)
    last_active_at = db.Column(db.DateTime)

class POSITION(db.Model):
    __tablename__ = 'position'
    id = db.Column(db.BIGINT,primary_key=True)
    NAME = db.Column(db.String(255))

class DEVICE_TYPE(db.Model):
    __tablename__ = 'device_type'
    type_code = db.Column(db.String(255),primary_key=True)
    type_name = db.Column(db.String(255))

class DEVICE_0100(db.Model):
    __tablename__='device_0100'
    id = db.Column(db.BIGINT,primary_key=True)
    device_id = db.Column(db.String(255))
    humidity = db.Column(db.DECIMAL(10, 2))
    temperature_f = db.Column(db.DECIMAL(10, 2))
    temperature_c = db.Column(db.DECIMAL(10, 2))
    heat_index = db.Column(db.DECIMAL(10, 2))
    air_quality = db.Column(db.String(255))
    created_at = db.Column(db.DateTime)

class DEVICE_0200(db.Model):
    __tablename__='device_0200'
    id = db.Column(db.BIGINT,primary_key=True)
    device_id = db.Column(db.String(255))
    alarm = db.Column(db.String(255))
    created_at = db.Column(db.DateTime)


class DEVICE_0300(db.Model):
    __tablename__='device_0300'
    id = db.Column(db.BIGINT,primary_key=True)
    device_id = db.Column(db.String(255))
    socket = db.Column(db.BIGINT)
    name = db.Column(db.String(255))
    status = db.Column(db.String(255))
    comments = db.Column(db.String(255))
    created_at = db.Column(db.DateTime)