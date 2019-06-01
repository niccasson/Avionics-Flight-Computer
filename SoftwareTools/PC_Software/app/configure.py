from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify
)

import serialReader
import data_reader

bp = Blueprint('configure', __name__)

@bp.route('/Configure')
def load_configure():

    return render_template('Configure.html')