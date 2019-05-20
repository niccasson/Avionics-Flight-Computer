from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

import serialReader

bp = Blueprint('ctrl', __name__)

@bp.route('/', methods=(['GET']))
def interface():

    return render_template('ctrl/interface.html')

@bp.route('/connect', methods=(['GET']))
def connect():

    return render_template('ctrl/connect.html')

@bp.route('/plot', methods=(['GET']))
def plot():
    template=''
    if request.args['action'] == 'connect':
        template = 'ctrl/connect.html'

    if request.args['action'] == 'dashboard':
        template = 'dashboard.html'

    if request.args['action'] == 'plot':
        template = 'Plot.html'

    if request.args['action'] == 'downloadData':
        template = 'downloadProgress.html'
        serialReader.run(4, "COM4", 115200, 'log.log')

    return render_template(template)
