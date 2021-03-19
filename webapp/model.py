from webapp.WebApp import db

class DAY_DATAS(db.Model):
    __tablename__='DAY_DATAS'
    ID = db.Column(db.BIGINT,primary_key=True)
    CODE = db.Column(db.String(8)) #代码
    NAME = db.Column(db.String(255)) #股票名称
    TOPEN = db.Column(db.DECIMAL) #开盘价
    HIGH = db.Column(db.DECIMAL) #最高
    LOW = db.Column(db.DECIMAL) #最低
    TCLOSE = db.Column(db.DECIMAL) # 收盘价
    LCLOSE = db.Column(db.DECIMAL) #前日收盘价
    CHG = db.Column(db.DECIMAL)  #涨跌额
    PCHG = db.Column(db.DECIMAL)  #涨跌幅
    VOTURNOVER = db.Column(db.BIGINT)  #成交量(手)
    VATURNOVER = db.Column(db.DECIMAL)  #成交金额
    TURNOVER = db.Column(db.DECIMAL)  #换手率
    TCAP = db.Column(db.DECIMAL)  #总市值
    MCAP = db.Column(db.DECIMAL)  #流通市值
    DAY = db.Column(db.Date)  #交易日
    MA5 = db.Column(db.DECIMAL)  #MA5
    MA10 = db.Column(db.DECIMAL)  #MA10
    MA20 = db.Column(db.DECIMAL)  #MA20
    CHANGEHAND = db.Column(db.DECIMAL)  #换手率
    CREATE_TIME = db.Column(db.DateTime) #采集时间

class LISTS(db.Model):
    __tablename__ = 'LISTS'
    CODE = db.Column(db.String(8),primary_key=True)
    NAME = db.Column(db.String(50))

class TMP_FAILED(db.Model):
    __tablename__ = 'TMP_FAILED'
    F_CODE = db.Column(db.String(50),primary_key=True)
    F_NAME = db.Column(db.String(50))

class RELATION(db.Model):
    __tablename__ = '__RELATION'
    CODE = db.Column(db.String(50),primary_key=True)
    FLAG = db.Column(db.String(50))
    REMARK = db.Column(db.String(50))
    FAVORITE_TIME = db.Column(db.DateTime) #收藏时间