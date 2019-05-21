from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify
)

import serialReader
import data_reader

bp = Blueprint('visualize', __name__)

@bp.route('/Visualize')
def load_download():

    return render_template("Visualize.html")

@bp.route('/Plot')
def load_plot():

    return render_template("Plot.html")