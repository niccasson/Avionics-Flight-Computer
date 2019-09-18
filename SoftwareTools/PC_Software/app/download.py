import subprocess
import time

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify,json
)

import serialReader
import data_reader
import generate_plotly
from app.dashboard import  terminal_in

bp = Blueprint('download', __name__)
import app

@bp.route('/Download')
def load_download():

    return render_template('Download.html')

@bp.route('/DownloadData', methods=(['GET','POST']))
def startDownload():

    S = app.SerialPort

    data = request.get_data().decode('utf-8')

    jsonData = json.loads(str(data))

    csvName = jsonData["csvName"]
    print(csvName)

    logName = jsonData["logName"]
    print(logName)

    S.close_log()
    # f = open(app.globalData['currentLogFile'], 'r')
    # print(f)
    # lines = f.readlines()
    # print(lines)
    # for line in lines:
    #     print(line)
    S.flush()
    time.sleep(1)
    S.write("return\r")
    time.sleep(1)
    S.open_log(logName)
    time.sleep(1)
    S.write("read\r")

    return jsonify("downloading data.")

@bp.route('/StopDownloadData', methods=(['GET']))
def stopDownload():

    terminal_in()
    S = app.SerialPort
    S.close_log()
    time.sleep(1)
    S.open_log('log.log')

    return_code = subprocess.call("data_parser.exe")
    generate_plotly.plot_data()

    return jsonify("Done downloading data.")