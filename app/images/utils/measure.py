from mainApi.app.images.utils.basic import *
import math
import numpy as np



def get_angle_angle_values(xpoints, ypoints, size):
    temp = []
    for i in range(size):
        if(i == 0) :
            p1 = Point(xpoints[size-1], ypoints[size-1])
            p2 = Point(xpoints[i], ypoints[i])
            p3 = Point(xpoints[i+1],ypoints[i+1])
        elif (i == size-1):
            p3 = Point(xpoints[0], ypoints[0])
            p2 = Point(xpoints[i], ypoints[i])
            p1 = Point(xpoints[i-1],ypoints[i-1])
        else :
            p2 = Point(xpoints[i], ypoints[i])
            p1 = Point(xpoints[i-1],ypoints[i-1])
            p3 = Point(xpoints[i+1],ypoints[i+1])
        temp.append(angle_Angle_Between_Three_Points(p1,p2,p3))
    return temp



def get_angle_radius_values(xpoints, ypoints, size):
    temp = []

    for i in range(size):
        if(i == 0) :
            p1 = Point(xpoints[size-1], ypoints[size-1])
            p2 = Point(xpoints[i], ypoints[i])
            p3 = Point(xpoints[i+1],ypoints[i+1])
        elif (i == size-1):
            p3 = Point(xpoints[0], ypoints[0])
            p2 = Point(xpoints[i], ypoints[i])
            p1 = Point(xpoints[i-1],ypoints[i-1])
        else :
            p2 = Point(xpoints[i], ypoints[i])
            p1 = Point(xpoints[i-1],ypoints[i-1])
            p3 = Point(xpoints[i+1],ypoints[i+1])
        temp.append(angle_Radius(p1,p2,p3))
    return temp


def get_bestfit_mean_values(long_lengths, short_lenghts, size):
    temp = []
    for i in range(size):
        temp.append((long_lengths[i] + short_lenghts[i]) / 2)
    return temp


def get_bestfit_rmse_values(long_lengths, short_lenghts, size):
    temp = []
    for i in range(size):
        temp.append(math.sqrt(long_lengths[i] * short_lenghts[i]) )
    return temp


def get_line_angle_values(values_x1, values_y1, values_x2, values_y2, total_size):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        angle = line_Angle(Line(stPoint, enPoint))
        temp.append(angle)
    return temp


def  get_line_arc_angle(values_center_x,values_center_y,values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        centerPoint = Point(values_center_x[i], values_center_y[i])
        angle = angle_Angle_Between_Three_Points(stPoint, centerPoint, enPoint)
        temp.append(angle)
    return temp


def  get_line_arc_radius(values_center_x,values_center_y,values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        centerPoint = Point(values_center_x[i], values_center_y[i])
        radius = angle_Radius(stPoint, centerPoint, enPoint)
        temp.append(radius)
    return temp

def get_line_chord_length(total_size):
    temp = []
    for i in range(total_size):
        temp.append(0)
    return temp


def get_line_endX(values_x1, values_y1, values_x2, values_y2, total_size ):
    return values_x2


def get_line_endY(values_x1, values_y1, values_x2, values_y2, total_size ):
    return values_y2

def get_line_startX(values_x1, values_y1, values_x2, values_y2, total_size ):
    return values_x1

def get_line_startY(values_x1, values_y1, values_x2, values_y2, total_size ):
    return values_y1

def get_line_endXY(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value_xy = line_EndXY(Line(stPoint, enPoint))
      
        temp.append(value_xy)
    return temp


def get_line_startXY(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value_xy = line_StartXY(Line(stPoint, enPoint))
        temp.append(value_xy)
    return temp

def get_line_length(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value = line_Length(Line(stPoint, enPoint))
        temp.append(value)
    return temp

def get_line_positionX(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value = line_PositionX(Line(stPoint, enPoint))
        temp.append(value)
    return temp

def get_line_positionXY(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value = line_PositionXY(Line(stPoint, enPoint))
        temp.append(value)
    return temp

def get_line_positionY(values_x1, values_y1, values_x2, values_y2, total_size ):
    temp = []
    for i in range(total_size):
        stPoint = Point(values_x1[i], values_y1[i])
        enPoint = Point(values_x2[i], values_y2[i])
        value = line_PositionY(Line(stPoint, enPoint))
        temp.append(value)
    return temp


def get_object_class(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp

def get_object_classname(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp

def get_object_parent(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp



def get_region_centerXY(values_x, values_y,total_size):
    temp = []
    for i in range(total_size):
        value = (values_x[i] + values_y[i]) / 2
        temp.append(value)
    return temp


def get_region_intensity(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp


def get_region_percent(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp


def get_region_perimeter(total_size):
    temp = []
    for i in range(total_size):
        temp.append(1)
    return temp





def processBasicMeasureData(path, data):
    data = data.iloc[: , :-1]

    values_class_name = data['no']
    values_pixels_in_area = data['0:pixels']
    values_x_center_of_gravity = data['1:point-x']
    values_y_center_of_gravity = data['2:point-y']
    values_min_x_in_area = data['3:0x']
    values_max_x_in_area = data['4:x1']
    values_min_y_in_area = data['5:y0']
    values_max_y_in_area = data['6:y1']
    values_area_width = data['7:width']
    values_area_height = data['8:height']
    values_inverse_flattening_rate = data['9:inv-oblate']
    values_short_side_length = data['10:S-length']
    values_long_side_length = data['11:L-length1']
    values_long_side_direction = data['22:deg1']
    values_short_side_direction = data['21:deg0']

    values_average_pixel_in_area = data['23:average']

    

    total_size = values_class_name.shape[0]

    #get the angle values
    values_angle_angle = get_angle_angle_values(values_x_center_of_gravity, values_y_center_of_gravity, total_size)
    data['30:angle-angle'] = values_angle_angle
    
    data['31:angle-centerX'] = values_x_center_of_gravity
    data['32:angle-centerY'] = values_y_center_of_gravity
    

    #get the angle radius
    values_angle_radius = get_angle_radius_values(values_x_center_of_gravity, values_y_center_of_gravity, total_size)
    data['33:angle-radius'] = values_angle_radius
    
    data['34:bestfit-maxdist'] = values_long_side_length
    data['35:bestfit-mindist'] = values_short_side_length


    #get the mean length 
    data['36:bestfit-meandist'] = get_bestfit_mean_values(values_long_side_length, values_short_side_length, total_size)

    #get the rmse lenght
    data['37:bestfit-rmsedist'] = get_bestfit_rmse_values(values_long_side_length, values_short_side_length, total_size)



    #get the line angle with the horizontal line, This line is defined by min point and maxpoint
    values_line_angle = get_line_angle_values(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['38:line-angle'] = values_line_angle


    #line arc angle.
    #This will be the angle between the start, end point of the line and center point of the area

    values_line_arc_angle = get_line_arc_angle(values_x_center_of_gravity,values_y_center_of_gravity,values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size  )
    data['39:line-arc-angle'] = values_line_angle


    data['40:line-arc-centerX'] = values_x_center_of_gravity
    data['41:line-arc-centerY'] = values_y_center_of_gravity

    data['42:line-arc-radius'] = get_line_arc_radius(values_x_center_of_gravity,values_y_center_of_gravity,values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size )

    #This function is not implmented yet
    data['43:line-chord-length'] = get_line_chord_length(total_size)


    data['44:line-endX'] = get_line_endX(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size )
    data['45:line-endY'] = get_line_endY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['46:line-endXY'] = get_line_endXY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)

    data['47:line-length'] = get_line_length(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)

    data['48:line-positionX'] = get_line_positionX(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['49:line-positionXY'] = get_line_positionXY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['50:line-positionY'] = get_line_positionY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)

    data['51:line-startX'] = get_line_startX(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size )
    data['52:line-startXY'] = get_line_startXY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['53:line-startY'] = get_line_startY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
   

    data['54:object-class'] = get_object_class(total_size)
    data['55:object-classname'] = get_object_classname(total_size)
    data['56:object-parent'] = get_object_parent(total_size)


    data['57:point-intensity'] = values_average_pixel_in_area

    data['58:point-locationX'] = get_line_positionX(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['59:point-locationXY'] = get_line_positionXY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)
    data['60:point-locationY'] = get_line_positionY(values_min_x_in_area, values_min_y_in_area, values_max_x_in_area, values_max_y_in_area, total_size)


    data['61:region-area'] = values_pixels_in_area
    data['62:axis-major'] = values_long_side_direction
    data['63:axis-minor'] = values_short_side_direction

    data['64:region-centerX'] = values_x_center_of_gravity
    data['65:region-centerXY'] = get_region_centerXY(values_x_center_of_gravity,values_y_center_of_gravity,total_size)
    data['66:region-centerY'] = values_y_center_of_gravity

    data['67:region-direction'] = values_long_side_direction

    data['68:region-intensity'] = get_region_intensity(total_size)
    data['69:region-area-percent'] = get_region_percent(total_size)
    data['70:region-perimeter'] = get_region_perimeter(total_size)

    return data


    