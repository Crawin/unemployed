import asyncio

import discord
from discord.ext import tasks
import token

from datetime import datetime

TOKEN = token.TOKEN
CHAT_CHANNEL_ID = token.CHAT_CHANNEL_ID
VOICE_CHANNEL_ID = token.VOICE_CHANNEL_ID

class MyClient(discord.Client):
    async def on_ready(self):
        channel = self.get_channel(CHAT_CHANNEL_ID)
        await channel.send('ON')
        self.check_voice_channel.start()

    async def on_message(self, message):
        if message.author == self.user:
            return
        else:
            if message.content == '현황':
                channel = self.get_channel(CHAT_CHANNEL_ID)
                await channel.send(Working_Members)
            pass

    @tasks.loop(seconds=1)
    async def check_voice_channel(self):
        global Working_Members
        channel = self.get_channel(VOICE_CHANNEL_ID)
        # 먼저 WM의 이름들과 CM의 이름들을 리스트로 만든 후 비교 한다음 없는 이름을 갖고와서 working에 없나 channel에 없나 확인 후 working에 없으면 입장, channel에 없으면 퇴장 시키자.


        # # 채널 이용자를 일하는 멤버 리스트에 속하나?
        # for Cmember in channel.members:
        #     if self.compare_working_members(Cmember.name):
        #         Working_Members.append({'NAME': Cmember.name, 'IN': datetime.now().strftime('%Y/%m/%d %H:%M:%S'), 'OUT': 0})

        #일하는 멤버 리스트엔 있지만 채널엔 없다?


        await asyncio.sleep(1)

    def compare_working_members(self, Cmember):
        global Working_Members
        # print(f'C: {Cmember}, W: {Working_Members}')
        for Wmember in Working_Members:
            if Cmember in Wmember['NAME']:
                return False
        return True

    def compare_channel_members(self, Cmembers):
        for Wmember in Working_Members:
            for Cmember in Cmembers:
                if Wmember['NAME'] in Cmember.name:
                    continue




Working_Members = []

intents = discord.Intents.default()
intents.message_content = True
client = MyClient(intents=intents)
client.run(TOKEN)