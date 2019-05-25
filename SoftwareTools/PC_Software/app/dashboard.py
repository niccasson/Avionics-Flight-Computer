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

@bp.route('/disconnect', methods=(['GET']))
def disconnect():

    S = app.SerialPort
    serialReader.mutex.release()


    return jsonify("Disconnected")

@bp.route('/connect', methods=(['GET','POST']))
def connect():
    print("CONNECTING")
    data = request.get_data().decode('utf-8')

    jsonData = json.loads(str(data))

    baud = jsonData["baudrate"]
    com = jsonData["COM"]
    print("com: {}  baud: {}".format(com, baud))

    try:
        app.SerialPort = data_reader.SerialFunctions(baud, com, 'log.log')
        print("starting thread")
        serialReader.run(app.SerialPort)

    except:
        print("error opening serial port:")
        print(sys.exc_info()[0])
        return jsonify({'res': 0})

    print("RESPONDING")
    return jsonify({'res': 1})

@bp.route('/terminal_out', methods=(['GET','POST']))
def terminal_out():

    print("Sending data to flight comp")
    data = request.get_data().decode('utf-8')

    cmd = json.loads(str(data))['text']
    print("Sending: {}".format(cmd))
    S = app.SerialPort
    S.write(cmd)

    return jsonify(cmd)


@bp.route('/terminal_in', methods=(['GET','POST']))
def terminal_in():

    print("terminal in: {}".format(id(serialReader.buffer)))

    text = serialReader.buffer[0]
    serialReader.buffer[0] = ""

    print("Received data: {}".format(text))
    print(serialReader.buffer[0])
    return jsonify(text)
