from urllib import request
import sys
from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify,json
)

import serialReader
import data_reader


import app
buffer = ""
bp = Blueprint('dashboard', __name__)

@bp.route('/')
@bp.route('/index')
def interface():

    return render_template('index.html')

@bp.route('/comports', methods=(['GET']))
def comports():

    ports = data_reader.list_COM_ports()
    print(ports)

    return jsonify(ports)

@bp.route('/connect', methods=(['GET','POST']))
def connect():

    data = request.get_data().decode('utf-8')

    jsonData = json.loads(str(data))

    baud = jsonData["baudrate"]
    com = jsonData["COM"]
    print("com: {}  baud: {}".format(com, baud))
    try:
        app.SerialPort = data_reader.SerialFunctions(baud, com, 'log.log')
        serialReader.run(app.SerialPort)

    except:
        print("error opening serial port:")
        print(sys.exc_info()[0])
        return jsonify({'res': 0})



    return jsonify({'res': 1})

@bp.route('/terminal_out', methods=(['GET','POST']))
def terminal_out():

    data = request.get_data().decode('utf-8')

    cmd = json.loads(str(data))['text']
    S = app.SerialPort
    S.write(cmd)


    return jsonify("\n"+cmd)


@bp.route('/terminal_in', methods=(['GET','POST']))
def terminal_in():

    text = serialReader.buffer[0]
    serialReader.buffer[0] = ""
    print(text)
    return jsonify(text)
