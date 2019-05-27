from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify,json
)

import serialReader
import data_reader

bp = Blueprint('download', __name__)
import app

@bp.route('/Download')
def load_download():

    return render_template('Download.html')

@bp.route('/DownloadData', methods=(['GET','POST']))
def startDownload():

    data = request.get_data().decode('utf-8')

    jsonData = json.loads(str(data))

    csvName = jsonData["csvName"]
    print(csvName)

    app.SerialPort.close_file()
    f = open(app.globalData['currentLogFile'], 'r')
    print(f)
    lines = f.readlines()
    print(lines)
    for line in lines:
        print(line)


    return jsonify("downloading data.")