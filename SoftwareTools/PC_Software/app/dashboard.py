from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify
)

import serialReader
import data_reader

s = None
bp = Blueprint('dashboard', __name__)

@bp.route('/')
@bp.route('/index')
def interface():

    return render_template('index.html')

@bp.route('/connect', methods=(['GET']))
def connect():

    Ports = data_reader.list_COM_ports()
    print(Ports)

    return jsonify(Ports)


# @bp.route('/plot', methods=(['GET']))
# def plot():
#
#     template='Plot.html'
#
#     if len(request.args) != 0:
#
#         if request.args['action'] == 'connect':
#             template = 'dashboard/connect.html'
#
#         if request.args['action'] == 'dashboard':
#             template = 'dashboard.html'
#
#         if request.args['action'] == 'plot':
#             template = 'Plot.html'
#
#         if request.args['action'] == 'downloadData':
#             template = 'downloadProgress.html'
#             serialReader.run(4, "COM4", 115200, 'log.log')
#
#     return render_template(template)
