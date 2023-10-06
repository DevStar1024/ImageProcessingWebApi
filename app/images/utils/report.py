
import numpy as np
import pandas as pd
from constant import MeasureHeader
import bokeh
from bokeh.plotting import show
import matplotlib.pyplot as plt
import flowkit as fk


def getDataFrameWithHeaders(csv_path, newHeaders):
    data = pd.read_csv(csv_path)
    data = data.drop(data.columns[0], axis=1)
    pre_columns = data.columns.tolist()
    tot_cnt = len(pre_columns)
    headers = {}

    for i in range(tot_cnt):
        old_header = pre_columns[i]
        new_header = newHeaders[i]
        headers[old_header] = new_header
    data = data.rename(columns=headers)
    return data


def getSampleForFlowKit(data):
    return sample
    sample = fk.Sample(data, sample_id='Data')



def drawContour(data,sample, x_string, y_string, save_png_path):
    x_data = data[x_string]
    y_data = data[y_string]

    delta_x = np.max(x_data) - np.min(x_data)
    delta_y = np.max(y_data) - np.min(y_data)

    min_y = np.min(y_data) - delta_y / 2
    max_y = np.max(y_data) + delta_y / 2
    min_x = np.min(x_data) - delta_x / 2
    max_x = np.max(x_data) + delta_x / 2

    f = sample.plot_contour(x_string,y_string, source='raw',plot_events=True, y_min= min_y, y_max=max_y, x_min=min_x, x_max=max_x,)
    plt.show()
    plt.savefig(save_png_path)



