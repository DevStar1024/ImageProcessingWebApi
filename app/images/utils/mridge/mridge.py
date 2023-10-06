
import cv2
import numpy as np
import os

MAX_PIXEL_VALUE = 255
MIN_PIXEL_VALUE = 0
COLOR_RED = (0, 0, 255)
COLOR_YELLOW = (0,255,255)

MIN_COUNT_AREA_POINT_NUMBER = 200

def processMRIDGEMethod(inputfilepath, sensitivity):

    # Get the directory path
    dir_path = os.path.dirname(inputfilepath)

    # Get the file path
    file_name = os.path.basename(inputfilepath)

    file_extension = file_name.split(".")[-1]

    a_13_output_filename = file_name.split("."+file_extension)[0] + "a_13.jpg"


    image = cv2.imread(inputfilepath, 0)
    height, width = image.shape

    max_value = MAX_PIXEL_VALUE
    threshold_value = sensitivity  * MAX_PIXEL_VALUE / 100
    _, binary_image = cv2.threshold(image, threshold_value, max_value, cv2.THRESH_BINARY)

    #cv2.imshow("original Image", image)
    #cv2.imshow('Binary Image', binary_image)

    kernel = np.ones((3, 3), np.uint8)
    erode_image = cv2.erode(binary_image, kernel, iterations=3)
    #cv2.imshow("erode", erode_image)
    dilated = cv2.dilate(erode_image, kernel, iterations=1)
    #cv2.imshow("dilated", dilated)
    _, labels, stats, _ = cv2.connectedComponentsWithStats(dilated, connectivity=8)
    min_area = MIN_COUNT_AREA_POINT_NUMBER  # Define the minimum area for connected components to be considered
    filtered_image = np.zeros_like(dilated)
    for label in range(1, len(stats)):
        if stats[label, cv2.CC_STAT_AREA] >= min_area:
            filtered_image[labels == label] = MAX_PIXEL_VALUE

    dilated_filter_image = cv2.dilate(filtered_image, kernel, iterations=1)
    delta_image = binary_image - dilated_filter_image


    white_cell_points = cv2.findNonZero(filtered_image)
    while_ridge_points = cv2.findNonZero(delta_image)


    #the result image for the cell and ridge detection
    image_cell = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    for point in while_ridge_points:
        x, y = point[0]
        cv2.circle(image_cell, (x,y), 1, COLOR_YELLOW, -1)

    for point in white_cell_points:
        x, y = point[0]
        cv2.circle(image_cell, (x,y), 1, COLOR_RED, -1)


    #image for the process watershed
    temp_image = binary_image - delta_image

    cv2.imwrite(os.path.join(dir_path, a_13_output_filename), image_cell)
