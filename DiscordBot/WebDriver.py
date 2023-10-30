from flask import Flask
from threading import Thread
from datetime import datetime, timedelta

app = Flask('')
start_time = None
Previous_Logs = {}
Working_Members = []
Days_Times = 0
@app.route('/')
def home():
    members = [[],[]]      # members[0] 은 이름, members[1] 은 시간차이
    format = "%H:%M:%S.%f"
    log = f'독서실 봇이 {(datetime.utcnow() + timedelta(hours=9)) - start_time}째 실행중입니다.<br><br>'
    log += '<div style=float:left>'
    for Wmember in Working_Members:
        log = log + f'{Wmember}<br>'
        if Wmember['NAME'] in members[0]:
            if Wmember['GAP'] != '0' and Wmember['GAP'] != 0:
                temp_datetime = datetime.strptime(Wmember['GAP'], format)
                members[1][members[0].index(Wmember['NAME'])] += timedelta(hours=temp_datetime.hour,minutes=temp_datetime.minute,seconds=temp_datetime.second,microseconds=temp_datetime.microsecond)
        else:
            if Wmember['GAP'] != '0' and Wmember['GAP'] != 0:
                members[0].append(Wmember['NAME'])
                members[1].append(datetime.strptime(Wmember['GAP'], format))
    print(members)
    for pre_key, pre_val in Previous_Logs.items():
        log += f'<br><details><summary>{pre_key}</summary><ul>'
        for pre_log in pre_val:
            log += f'{pre_log}<br>'
        log += '</ul></details>'
    log += '</div>'
    log += '<div style=float:right>'
    format = "%d 일 %H:%M:%S.%f"
    for i in range(0,len(members[0])):
        log += f'{members[0][i]}: {members[1][i].strftime(format)} / {Days_Times} | {(((((((members[1][i].day - 1) * 24)+members[1][i].hour) * 60)+members[1][i].minute)*60+members[1][i].second)*1000000+members[1][i].microsecond)/(Days_Times*60*60*10000):.2f}%<br>'
    log += '</div>'
    return log

def run():
    global start_time
    start_time = datetime.utcnow() + timedelta(hours=9)
    app.run(host='0.0.0.0',port=8080)

def keep_alive():
    t = Thread(target=run)
    t.start()

def update_Working_Members(Wmem, PLog, DTimes):
    global Working_Members, Previous_Logs, Days_Times
    Working_Members = Wmem.copy()
    Previous_Logs = PLog.copy()
    Days_Times = DTimes
