import discord
from datetime import datetime
# import os
# TOKEN = os.environ['TOKEN']
# CHAT_CHANNEL_ID = os.environ['CHAT_CHANNEL_ID']

import DiscordToken
TOKEN = DiscordToken.TOKEN
CHAT_CHANNEL_ID = DiscordToken.CHAT_CHANNEL_ID

class MyClient(discord.Client):
    async def on_ready(self):
        print(f"{self.user.name}이 준비되었습니다.")

    async def on_message(self, message):
        if message.author == self.user:
            return
        else:
            if message.content == '현황':
                channel = self.get_channel(int(CHAT_CHANNEL_ID))
                await channel.send(Working_Members)
                for Wmember in Working_Members:
                    print(Wmember)
            if message.content == '종료':
                with open('text_file.txt', 'w', encoding='utf-8') as file:
                    for Wmember in Working_Members:
                        for key, value in Wmember.items():
                            file.write(f"{key}: {value}\n")
                        file.write('\n')
                file = discord.File("text_file.txt")
                channel = self.get_channel(int(CHAT_CHANNEL_ID))
                await channel.send(file=file)
                print(f"{self.user.name}이 종료됩니다.")
                await self.close()

    async def on_voice_state_update(self, member, before, after):
        if before.channel is None and after.channel is not None:
            # 사용자가 통화방에 입장한 경우
            Working_Members.append({'NAME': member.name, 'ENTER': datetime.now(), 'EXIT': 0, 'GAP': 0})
            print(f"입장: {Working_Members[-1]}")
        elif before.channel is not None and after.channel is None:
            # 사용자가 통화방에서 퇴장한 경우
            for Wmember in Working_Members:
                if Wmember['NAME'] == member.name and Wmember['EXIT'] == 0:
                    Wmember['EXIT'] = datetime.now()
                    Wmember['GAP'] = Wmember['EXIT'] - Wmember['ENTER']
                    print(f"퇴장: {Wmember}")


Working_Members = []


intents = discord.Intents.default()
intents.message_content = True
client = MyClient(intents=intents)
client.run(TOKEN)