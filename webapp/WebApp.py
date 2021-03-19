#!/usr/bin/python3
# coding=utf-8
from flask import render_template,request,jsonify, session, redirect, url_for
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from webapp.config import conf

app = Flask(__name__, static_folder='static', template_folder='templates')
app.config['SQLALCHEMY_DATABASE_URI'] = "mysql://%s:%s@%s/%s" % (conf.get('MYSQL', 'user'), conf.get('MYSQL', 'password'), conf.get('MYSQL', 'host'), conf.get('MYSQL', 'dbName'))
db = SQLAlchemy(app)

def working(_1553b):
    from webapp import model
    # [首页]
    @app.route('/', methods=['GET'])
    def index():
        return render_template('index.html')

    app.run(host=conf.get('WEB', 'host'), port=conf.get('WEB', 'port'), debug=False)
    print("webapp已就绪")
