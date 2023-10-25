from flask import Flask

from threading import Thread



app = Flask('')



@app.route('/')
def home():
    return "독서실 봇이 실행중 입니다."



def run():

  app.run(host='0.0.0.0',port=8080)



def keep_alive():

    t = Thread(target=run)

    t.start()