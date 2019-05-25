import os


from flask import Flask

globalData = {'serialOpen': False, 'terminalHistory': "", 'currentLogFile': "", 'currentCSV': "123"}

SerialPort = None

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    # app.config.from_mapping(
    #     SECRET_KEY='dev'
    # )

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    from . import dashboard
    app.register_blueprint(dashboard.bp)

    from . import configure
    app.register_blueprint(configure.bp)

    from . import download
    app.register_blueprint(download.bp)

    from . import visualize
    app.register_blueprint(visualize.bp)

    from . import common
    app.register_blueprint(common.bp)


    return app


def getSerial():
    return SerialPort


