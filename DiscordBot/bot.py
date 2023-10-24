import discord
from datetime import datetime
import logging
import os
# from WebDriver import keep_alive
# keep_alive()

# TOKEN = os.environ['TOKEN']
# CHAT_CHANNEL_ID = os.environ['CHAT_CHANNEL_ID']

import DiscordToken
TOKEN = DiscordToken.TOKEN
CHAT_CHANNEL_ID = DiscordToken.CHAT_CHANNEL_ID

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
        print(f"{self.user.name}이 {datetime.now()}에 준비되었습니다.")

    async def on_message(self, message):
        if message.author == self.user:
            return
        else:
            logger.info(f'({message.author}) 이 ({message.content}) 를 입력하였습니다.')
            if message.content == '현황':
                channel = self.get_channel(int(CHAT_CHANNEL_ID))
                await channel.send(Working_Members)
                file = discord.File("text_file.txt")
                channel = self.get_channel(int(CHAT_CHANNEL_ID))
                await channel.send(file=file)
                for Wmember in Working_Members:
                    print(Wmember)
            elif message.content == '종료':
                file = discord.File("text_file.txt")
                channel = self.get_channel(int(CHAT_CHANNEL_ID))
                await channel.send(file=file)
                print(f"{self.user.name}이 종료됩니다.")
                await self.close()
            else:
                await message.channel.send('없는 명령어 입니다.')

    async def on_voice_state_update(self, member, before, after):
        if before.channel is None and after.channel is not None:
            # 사용자가 통화방에 입장한 경우
            Working_Members.append({'NAME': member.name, 'ENTER': datetime.now(), 'EXIT': 0, 'GAP': 0})
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
                    Wmember['EXIT'] = datetime.now()
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


Working_Members = []

intents = discord.Intents.default()
intents.message_content = True
client = MyClient(intents=intents)
client.run(TOKEN)