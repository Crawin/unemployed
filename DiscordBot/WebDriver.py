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
                print(Wmember['GAP'])
                temp_datetime = datetime.strptime(Wmember['GAP'], format)
                members[1][members[0].index(Wmember['NAME'])] += timedelta(hours=temp_datetime.hour,minutes=temp_datetime.minute,seconds=temp_datetime.second,microseconds=temp_datetime.microsecond)
        else:
            if Wmember['GAP'] != '0' and Wmember['GAP'] != 0:
                members[0].append(Wmember['NAME'])
                members[1].append(datetime.strptime(Wmember['GAP'], format))
    log += '<br><details><summary>'
    # for pre_key, pre_val in Previous_Logs.items():
    #     log += f'{pre_key}</summary><ul>'
    # for pre_log in pre_val:
    #     log += f'{pre_log}<br>'
    log += '</ul></details><br></div>'
    log += '<div style=float:right>'
    for i in range(0,len(members[0])):
        log += f'{members[0][i]}: {members[1][i].strftime(format)} / {Days_Times} | {((((members[1][i].hour * 60)+members[1][i].minute)*60+members[1][i].second)*1000+members[1][i].microsecond)/(Days_Times*60*60*10):.2f}%<br>'
    log += '</div>'
    return log

def run():
    global start_time
    start_time = datetime.utcnow() + timedelta(hours=9)
    Update_Days()
    app.run(host='0.0.0.0',port=8080)

def keep_alive():
    t = Thread(target=run)
    t.start()

def update_Working_Members(dic):
    global Working_Members
    Working_Members = dic.copy()
    Update_Days()

def Update_Days():
    global Days_Times, Previous_Logs
    now = datetime.utcnow() + timedelta(hours=9)
    if now.day == 1 or Days_Times == 0:   # 한달이 시작되면 그 달의 평일수 * 9를 계산
        days = GetDayOfMonth(now.year,now.month)            # 해당 달에 일수를 days에 저장
        if Days_Times == 0:
            now = datetime.utcnow() + timedelta(hours=9) - timedelta(days= now.day-1)
        while (now.weekday() < 5):                          # 반복문이 끝나면 토요일 혹은 일요일
            now += timedelta(days=1)
        if now.day == 1 and now.weekday() == 6:                    # 1일부터 일요일일때
            sat_days = (days - (now.day + 6)) // 7 + 1
            sun_days = (days - now.day) // 7 + 1
            Days_Times = (days - sat_days - sun_days)*9
        else:
            sat_days = (days - now.day) // 7 + 1
            sun_days = (days - (now.day + 1)) // 7 + 1
            Days_Times = (days - sat_days - sun_days)*9

    # Previous_Logs[datetime.strftime(now.date(),"%Y.%m")] = Working_Members.copy()

def IsLeapYear(year):
     if year % 4 != 0:
          return False

     if year % 400 == 0:
          return True

     if year % 100 != 0:
          return True

     return False

def GetDayOfMonth(year, month):

     # 먼저 입력받은 월이 2월인지 아닌지 확인함.
     if month == 2:
          # IsLeapYear함수를 호출하여 윤년인지 아닌지 판별함. 윤년이면 결과값이 True이므로 29를 리턴. 그렇지 않으면 28을 리턴함.
          if IsLeapYear(year):
               return 29
          else:
               return 28

     # 4,6,9,11월일 땐 무조건 30일이므로 30리턴. 해당 월 수가 더 적으므로 4,6,9,11월인지 먼저 조건문을 사용해 판단.
     if month == 4 or month == 6 or month == 9 or month == 11:
          return 30

     # 위의 조건문에 해당되지 않는 1,3,5,7,8,10,12월은 31을 리턴.
     return 31



# def update_Working_Members():
#     global Working_Members
#     # Working_Members = dic.copy()
#     read_text_file()
#     Update_Days()
# f = None
# def read_text_file():
#     global f
#     try:
#         f = open("text_file.txt", 'r', encoding='UTF-8')
#     except:
#         print('text_file.txt.가 존재하지 않습니다.')
#     if f:
#         temp = {}
#         print('File exist')
#         lines = f.readlines()
#         for line in lines:
#             if 'NAME: ' in line:
#                 line = line.replace('NAME: ', '').replace('\n', '')
#                 temp['NAME'] = line
#             if 'ENTER: ' in line:
#                 line = line.replace('ENTER: ', '').replace('\n', '')
#                 temp['ENTER'] = line
#             if 'EXIT: ' in line:
#                 line = line.replace('EXIT: ', '').replace('\n', '')
#                 temp['EXIT'] = line
#             if 'GAP: ' in line:
#                 line = line.replace('GAP: ', '').replace('\n', '')
#                 temp['GAP'] = line
#             if line == '\n':
#                 Working_Members.append(temp.copy())
#         print(Working_Members)
# update_Working_Members()
# keep_alive()