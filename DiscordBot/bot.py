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
        global f
        try:
            f = open("text_file.txt", 'r', encoding='UTF-8')
        except:
            print('text_file.txt.가 존재하지 않습니다.')
        if f:
            temp = {}
            print('File exist')
            lines = f.readlines()
            for line in lines:
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
                    Working_Members.append(temp.copy())
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
            if message.content == '현황':
                await channel.send(Working_Members)
                try:
                    file = discord.File("text_file.txt")
                    await channel.send(file=file)
                except:
                    await channel.send('현황 파일이 존재하지 않습니다.')
                for Wmember in Working_Members:
                    print(Wmember)
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
                    Wmember['GAP'] = Wmember['EXIT'] - Wmember['ENTER']
                    print(f"퇴장: {Wmember}")
            #퇴장하면 싹 갈아엎기
            with open('text_file.txt', 'w', encoding='utf-8') as file:
                for Wmember in Working_Members:
                    for key, value in Wmember.items():
                        file.write(f"{key}: {value}\n")
                    file.write('\n')

    @tasks.loop(seconds=1)  # 1초마다 업데이트
    async def update_Working_Members(self):
        update_Working_Members(Working_Members)

Working_Members = []

keep_alive()

intents = discord.Intents.default()
intents.message_content = True
client = MyClient(intents=intents)
client.run(TOKEN)