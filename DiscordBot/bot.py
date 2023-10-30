import discord
from discord.ext import tasks
from datetime import datetime, timedelta
import logging
import os
from WebDriver import keep_alive, update_Working_Members

TOKEN = os.environ['TOKEN']
CHAT_CHANNEL_ID = os.environ['CHAT_CHANNEL_ID']

logger = logging.getLogger()
logger.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s - %(message)s')

stream_handler = logging.StreamHandler()
stream_handler.setFormatter(formatter)
logger.addHandler(stream_handler)

f = False

class MyClient(discord.Client):
    async def on_ready(self):
        global f, Working_Members, Previous_Logs
        try:
            f = open("text_file.txt", 'r', encoding='UTF-8')
        except:
            print('text_file.txt.가 존재하지 않습니다.')
        if f:
            temp = {}
            print('File exist')
            lines = f.readlines()
            key = None
            for line in lines:
                if line[0] == '<':          # 분기점
                    key = line.replace('<','').replace('>','').replace('\n','')
                else:
                    if 'NAME: ' in line:
                        line = line.replace('NAME: ', '').replace('\n', '')
                        temp['NAME'] = line
                    if 'ENTER: ' in line:
                        line = line.replace('ENTER: ', '').replace('\n', '')
                        temp['ENTER'] = line
                    if 'EXIT: ' in line:
                        line = line.replace('EXIT: ', '').replace('\n', '')
                        temp['EXIT'] = line
                    if 'GAP: ' in line:
                        line = line.replace('GAP: ', '').replace('\n', '')
                        temp['GAP'] = line
                    if line == '\n':
                        if key == 'current':
                            Working_Members.append(temp.copy())
                        else:
                            try:
                                Previous_Logs[key].append(temp.copy())
                            except:
                                Previous_Logs[key] = [temp.copy()]
            f.close()
        self.update_Working_Members.start()
        print(f"{self.user.name}이 {datetime.utcnow() + timedelta(hours=9)}에 준비되었습니다.")

    async def on_message(self, message):
        global Working_Members
        if message.author == self.user:
            return
        else:
            logger.info(f'({message.author}) 이 ({message.content}) 를 입력하였습니다.')
            channel = self.get_channel(int(CHAT_CHANNEL_ID))
            if message.content.startswith('현황'):
                if message.content == '현황':
                    # await channel.send(Working_Members)
                    try:
                        file = discord.File("text_file.txt")
                        await channel.send(file=file)
                    except:
                        await channel.send('현황 파일이 존재하지 않습니다.')
                    for Wmember in Working_Members:
                        print(Wmember)
                else:
                    current_name = message.content.replace('현황','').replace(' ','')
                    name_log = ''
                    for Wmember in Working_Members:
                        if Wmember['NAME'] == current_name:
                            for key, value in Wmember.items():
                                name_log = name_log + f'{key}: {value} \n'
                            name_log = name_log + '\n'
                    if name_log == '':
                        await channel.send('해당 ID의 현황이 존재하지 않습니다.')
                    else:
                        # await channel.send(name_log)
                        temp_file_name = current_name + '.txt'
                        with open(temp_file_name, 'w', encoding='utf-8') as file:
                            file.write(name_log)
                        file = discord.File(temp_file_name)
                        await channel.send(file=file)
                        os.remove(temp_file_name)

            elif message.content == '종료':
                try:
                    file = discord.File("text_file.txt")
                    await channel.send(file=file)
                    print(f"{self.user.name}이 종료됩니다.")
                except:
                    await channel.send('현황 파일이 존재하지 않습니다.')
                    print(f"{self.user.name}이 종료됩니다.")
                await self.close()
            elif message.content == '삭제':
                try:
                    file = discord.File("text_file.txt")
                    await channel.send(file=file)
                    os.remove('text_file.txt')
                except:
                    await channel.send('현황 파일이 존재하지 않습니다.')
                    print('현황 파일이 존재하지 않습니다.')
                Working_Members = []
            # else:
            #     await message.channel.send('없는 명령어 입니다.')

    async def on_voice_state_update(self, member, before, after):
        if before.channel is None and after.channel is not None:
            # 사용자가 통화방에 입장한 경우
            Working_Members.append({'NAME': member.name, 'ENTER': datetime.utcnow() + timedelta(hours=9), 'EXIT': 0, 'GAP': 0})
            # 입장하면 파일에 추가 작성
            with open('text_file.txt', 'a', encoding='utf-8') as file:
                for key, value in Working_Members[-1].items():
                    file.write(f"{key}: {value}\n")
                file.write('\n')
            print(f"입장: {Working_Members[-1]}")
        if before.channel is not None and after.channel is None:
            # 사용자가 통화방에서 퇴장한 경우
            for Wmember in Working_Members:
                if Wmember['NAME'] == member.name and (Wmember['EXIT'] == 0 or Wmember['EXIT'] == str(0)):
                    Wmember['EXIT'] = datetime.utcnow() + timedelta(hours=9)
                    if type(Wmember['ENTER']) == str:
                        format = "%Y-%m-%d %H:%M:%S.%f"
                        Wmember['ENTER'] = datetime.strptime(Wmember['ENTER'], format)
                    # Wmember['GAP'] = Wmember['EXIT'] - Wmember['ENTER']
                    Wmember['GAP'] = str(Wmember['EXIT'] - Wmember['ENTER'])
                    Wmember['ENTER'] = str(Wmember['ENTER'])
                    Wmember['EXIT'] = str(Wmember['EXIT'])
                    print(f"퇴장: {Wmember}")
            #퇴장하면 싹 갈아엎기
            with open('text_file.txt', 'w', encoding='utf-8') as file:
                for PLog_key, PLog_val in Previous_Logs.items():
                    file.write(f'<{PLog_key}>\n')
                    for PLog in PLog_val:
                        for key, value in PLog.items():
                            file.write(f"{key}: {value}\n")
                        file.write('\n')
                    file.write(f'</{PLog_key}>\n')
                file.write('<current>\n')
                for Wmember in Working_Members:
                    for key, value in Wmember.items():
                        file.write(f"{key}: {value}\n")
                    file.write('\n')

    @tasks.loop(seconds=1)  # 1초마다 업데이트
    async def update_Working_Members(self):
        self.Update_Days()
        update_Working_Members(Working_Members,Previous_Logs,Days_Times)
            
    def Update_Days(self):
        global Days_Times, Previous_Logs, BackUp_Flag, Working_Members
        now = datetime.utcnow() + timedelta(hours=9)
        if now.day == 1 or Days_Times == 0:   # 한달이 시작되면 그 달의 평일수 * 9를 계산
            days = self.GetDayOfMonth(now.year,now.month)            # 해당 달에 일수를 days에 저장
            if Days_Times == 0:
                now = datetime.utcnow() + timedelta(hours=9) - timedelta(days= now.day-1)
                BackUp_Flag = False
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
            if now.day == 1 and BackUp_Flag:
                BackUp_Flag = False
                Previous_Logs[datetime.strftime(now.date(),"%Y.%m")] = Working_Members.copy()
                Working_Members = []
        else:
            BackUp_Flag = True

    def IsLeapYear(self,year):
        if year % 4 != 0:
            return False
        
        if year % 400 == 0:
            return True
        
        if year % 100 != 0:
            return True
        
        return False
    
    def GetDayOfMonth(self,year, month):
        
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

Working_Members = []
Previous_Logs = {}
Days_Times = 0
BackUp_Flag = True

keep_alive()

intents = discord.Intents.default()
intents.message_content = True
client = MyClient(intents=intents)
client.run(TOKEN)