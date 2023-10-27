from flask import Flask
from threading import Thread
from datetime import datetime, timedelta

app = Flask('')
start_time = None
Working_Members = []
@app.route('/')
def home():
    log = f'독서실 봇이 {(datetime.utcnow() + timedelta(hours=9)) - start_time}째 실행중입니다.'
    for Wmember in Working_Members:
        log = log + f'<p>{Wmember}</p>'
    return log

def run():
    global start_time
    start_time = datetime.utcnow() + timedelta(hours=9)
    app.run(host='0.0.0.0',port=8080)

def keep_alive():
    t = Thread(target=run)
    t.start()

def update_Working_Members(dic):
    global Working_Members
    Working_Members = dic.copy()