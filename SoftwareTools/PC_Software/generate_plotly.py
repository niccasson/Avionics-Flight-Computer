#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CREATED BY BYARUHANGA ASIIMWE (Sami)
-modified by Joseph Howarth 2019-05-18
    -> Changed excel file reader to csv reader.
"""

import plotly as ply
#import plotly.graph_objs as go
import plotly.offline as ply

# import xlrd
# book = xlrd.open_workbook('flightComputer.csv')
# sheet = book.sheet_by_index(0) #data is on sheet 1

import csv
import numpy as np
from urllib3.packages.rfc3986.parseresult import authority_from


def plot_data():
    DATA_FILE_PATH = 'flightComputer.csv'

    DATA_LABELS = {'time':0,'acceleration_x':1,'acceleration_y':2,'acceleration_z':3,'rotation_x':4,'rotation_y':5,'rotation_z':6,'pressure':7,'temperature':8}

    ###LIST FOR TRACES
    ##NOTE: CHANGE THE COLUMNS ACCORDING TO THE XLS DATA FROM FLIGHT COMP
    time = []               #column 1 i.e 0 WILL BE THE X FOR ALL VALUES
    acceleration = []
    acceleration_x = []       #column 6 in xls, 5 in py
    acceleration_y = []       #column 6 in xls, 5 in py
    acceleration_z = []       #column 6 in xls, 5 in py

    rotation_x = []
    rotation_y = []
    rotation_z = []

    air_pressure = []       #col 51 in xls, 50 in py
    temperature = []        #col 50 in xls, 49 in py

    velocity = []           #col 5 in xls, 4 in py
    altitude = []           #col 2 in xls, 1 in py
    inertia = []            #col 23 in xls, 22 in py
    thrust = []             #col 29 in xls, 28 in py

    data = []

    with open(DATA_FILE_PATH, newline='') as csvfile:
        data_reader = csv.reader(csvfile, delimiter=',', quotechar='|')


        for n,row in enumerate(data_reader):

            if n == 0:
                print(len(row))
                for i in range(len(row)):
                    data.append([])

            for i in range(len(row)):

                data[i].append(int(row[i]))

    data = np.array(data)
    data = data.astype('float64')
    print(data[DATA_LABELS['pressure']])
    #Data conditioning
    for key in DATA_LABELS.keys():
        print(key)

        if 'acceleration' in key:
            print(key)
            data[DATA_LABELS[key]] = data[DATA_LABELS[key]]/(2**15)*12
            print(data[DATA_LABELS[key]][1])

        if 'temperature' in key:
            print(key)
            data[DATA_LABELS[key]] = data[DATA_LABELS[key]]/100
            print(data[DATA_LABELS[key]][1])

        if 'pressure' in key:
            print(key)
            data[DATA_LABELS[key]] = data[DATA_LABELS[key]]/100000
            print(data[DATA_LABELS[key]][1])

        if 'time' in key:
            print(key)
            data[DATA_LABELS[key]] = data[DATA_LABELS[key]]/1000
            print(data[DATA_LABELS[key]][1])


    ##Accleration trace
    trace1 = {
            'x':data[DATA_LABELS['time']], #put the x values here, they are same for all i.e time
            'y':data[DATA_LABELS['acceleration_x']], #put the y values i want, i.e this is acceleration column
            'mode':'lines', #want a line curve
            'name':'ACCELERATION X (m/s^2)', #name of the trace i want i.e name of y
            'type': 'scatter',#want to create scatter plots of these values
            'xaxis':'x',
            'yaxis':'y'
            }


    trace2 = {
            'x':data[DATA_LABELS['time']],
            'y':data[DATA_LABELS['acceleration_y']],
            'mode':'lines',
            'name':'ACCELERATION Y (m/s)',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }


    trace3 = {
            'x':data[DATA_LABELS['time']],
            'y':data[DATA_LABELS['acceleration_z']],
            'mode':'lines',
            'name':'ACCELERATION Z',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }

    trace4 = {
            'x':data[DATA_LABELS['time']],
            'y':inertia,
            'mode':'lines',
            'name':'VELOCITY',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }

    trace5 = {
            'x':data[DATA_LABELS['time']],
            'y':thrust,
            'mode':'lines',
            'name':'THRUST (N)',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }

    trace6 = {
            'x':data[DATA_LABELS['time']],
            'y':data[DATA_LABELS['pressure']],
            'mode':'lines',
            'name':'AIR PRESSURE (kPa)',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }

    trace7 = {
            'x':data[DATA_LABELS['time']],
            'y':data[DATA_LABELS['temperature']],
            'mode':'lines',
            'name':'TEMPERATURE (C)',
            'type': 'scatter',
            'xaxis':'x',
            'yaxis':'y'
            }


    #do same for all the components i want
    data = [trace1,trace2,trace3,trace4,trace5,trace6,trace7] #put the name of all the traces
    #
    layout = {
      "autosize": True,
      "dragmode": "pan",
      "showlegend": True,
      "title": {
    #    "x": 0.48,
        "font": {"size": 20},
        "text": "FLIGHT DATA ANALYSIS (UMSATS ROCKETRY)"
      },
      "xaxis": {
        "autorange": True,
        "range": [0, 180.91],
        "rangeslider": {
          "autorange": True,
          "range": [0, 180.91],
          "visible": False
        },
        "title": {"text": "Time (s)"},
        "type": "linear"
      },
      "yaxis": {
        "autorange": True,
        "range": [-173.70122222222219, 3205.563222222222],
        "showspikes": False,
        "title": {"text": "Flight data"},
        "type": "linear"
      }
    }

    fig = dict(data = data, layout = layout)
    ply.plot(fig, filename= 'app/templates/Plot.html',auto_open=False)


