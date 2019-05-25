from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for,jsonify,json
)



import app

bp = Blueprint('common', __name__)

@bp.route('/writeGlobal',methods=(['GET','POST']))
def saveToDisk():

    data = request.get_data().decode('utf-8')
    # print("!!!!!!!!!!!:{}".format(request.get_data()))
    jsonData = json.loads(str(data))
    print(jsonData)
    print([x for x in jsonData.keys()])
    for key in jsonData.keys():
        app.globalData[key] = jsonData[key]

    print(app.globalData)
    return jsonify("data saved.")

@bp.route('/readGlobal')
def readFromDisk():

    print(app.globalData)
    return json.dumps(app.globalData)

