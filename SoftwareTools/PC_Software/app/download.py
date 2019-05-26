from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify
)

import serialReader
import data_reader

bp = Blueprint('download', __name__)

@bp.route('/Download')
def load_download():

    return render_template('Download.html')

@bp.route('DownloadData', methods=(['GET','POST']))
def startDownload():


