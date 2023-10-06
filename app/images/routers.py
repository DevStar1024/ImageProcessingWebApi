import os

from mainApi.app.images.utils.asyncio import shell
from fastapi import (
    APIRouter,
    HTTPException,
    Request,
    Response,
    Depends,
    Form,
    status
)
from fastapi.responses import JSONResponse, FileResponse
from mainApi.app.images.sub_routers.tile.routers import router as tile_router
from mainApi.config import STATIC_PATH
from mainApi.app.auth.auth import get_current_user
from mainApi.app.auth.models.user import UserModelDB, PyObjectId
import subprocess
import tempfile
from datetime import date
from typing import List
import json
import h5py as h5
from mainApi.app.images.h5.measure import update_h5py_file
import tifffile
import numpy as np
import bioformats
from PIL import Image
import javabridge as jv
import tifftools
import mainApi.app.images.utils.deconvolution as Deconv
import shutil
import zipfile
import pandas as pd
from mainApi.app.images.utils.measure import processBasicMeasureData
from mainApi.app.images.utils.mridge.mridge import processMRIDGEMethod
import cv2
import math

def zipdir(path, ziph):
    # ziph is zipfile handle
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file),
                       os.path.relpath(os.path.join(root, file),
                                       os.path.join(path, '..')))


router = APIRouter(prefix="/image", tags=[])

router.include_router(tile_router)

@router.get("/hdf5_download")
async def download_exp_image(
    request: Request,
    path: str
):
    full_path = f"mainApi/app/static/measure_out/{path}"
    if not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found")

    with open(full_path, "rb") as file:
        content = file.read()
        return Response(content)

@router.get("/zip_download")
async def download_zip(
    request: Request,
    path: str
):
    full_path = f"mainApi/app/static/{path}"
    if not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found")

    with open(full_path, "rb") as file:
        content = file.read()
        return Response(content)


@router.get("/download")
async def download_exp_image(
    request: Request,
    path: str
):
    full_path = f"{STATIC_PATH}/{path}"
    file_size = os.path.getsize(full_path)
    if not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found")
    range = request.headers["Range"]
    if range is None:
        return FileResponse(full_path, filename=path)
    ranges = range.replace("bytes=", "").split("-")
    range_start = int(ranges[0]) if ranges[0] else None
    range_end = int(ranges[1]) if ranges[1] else file_size - 1
    if range_start is None:
        return Response(content="Range header required", status_code=416)
    if range_start >= file_size:
        return Response(content="Range out of bounds", status_code=416)
    if range_end >= file_size:
        range_end = file_size - 1
    content_length = range_end - range_start + 1
    headers = {
        "Content-Range": f"bytes {range_start}-{range_end}/{file_size}",
        "Accept-Ranges": "bytes",
        "Content-Length": str(content_length),
    }
    with open(full_path, "rb") as file:
        file.seek(range_start)
        content = file.read(content_length)
        return Response(content, headers=headers, status_code=206)

@router.get("/download_csv")
async def download_exp_image(
    request: Request,
    path: str
):
    full_path = f"{STATIC_PATH}/{path}"
    print("download-csv-path:", full_path)
    file_size = os.path.getsize(full_path)
    if not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found")

    headers = {
        "Accept-Ranges": "bytes",
        "Content-Length": str(file_size),
    }
    with open(full_path, "rb") as file:
        content = file.read()
        return Response(content, headers=headers, status_code=206)

@router.post(
    "/before_process",
    response_description="Process image",
)
async def processImage(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    print("get-request-data:", data)
    imagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_url")
    folderName = date.today().strftime("%y%m%d%H%M%s")
    sharedImagePath = os.path.join("/app/shared_static", folderName)

    if not os.path.exists(sharedImagePath):
        os.makedirs(sharedImagePath)

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    newImagePath = os.path.join(sharedImagePath, fileName)

    cmd_str = "cp '{inputPath}' '{outputPath}'".format(
        inputPath=imagePath, outputPath=newImagePath
    )
    subprocess.call(cmd_str, shell=True)

    return JSONResponse({"success": "success", "image_path": newImagePath})

@router.post(
    "/ml_ict_process",
    response_description="ML IPS Process",
)
async def mlICTProcess(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_url")

    sensitivity = data.get("sensitivity")
    type = data.get("type")

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    originPath = imagePath.replace(fileName, '')
    tempPath = tempfile.mkdtemp()
    OUT_PUT_FOLDER = tempPath.split("/")[len(tempPath.split("/")) - 1]
    OUT_PUT_PATH = 'mainApi/app/static/ml_out/' + OUT_PUT_FOLDER

    fullPath = OUT_PUT_PATH + '/' + fileName
    csvPath = os.path.splitext(fullPath)[0] + '_300.csv'
    originCSVPath = os.path.splitext(imagePath)[0] + '_300.csv'
    zipPath = os.path.splitext(imagePath)[0] + '_measure_result.zip'

    if not os.path.exists(OUT_PUT_PATH):
        os.makedirs(OUT_PUT_PATH)

    cmd_str = "/app/mainApi/ml_lib/segB {inputPath} {outputPath}"
    if type == 'a':
        cmd_str += " /app/mainApi/ml_lib/typeB/src_paramA.txt"
    if type == 'b':
        cmd_str += " /app/mainApi/ml_lib/typeB/src_paramB.txt"
    if type == 'c':
        cmd_str += " /app/mainApi/ml_lib/typeB/src_paramC.txt"
    if type == 'd':
        cmd_str += " /app/mainApi/ml_lib/typeB/src_paramD.txt"

    cmd_str += " " + sensitivity
    cmd_str = cmd_str.format(inputPath=imagePath, outputPath=OUT_PUT_PATH + "/" + fileName)
    print('----->', cmd_str)
    subprocess.call(cmd_str, shell=True)


    data = pd.read_csv(csvPath)
    data = processBasicMeasureData(csvPath, data)
    data.to_csv(csvPath)
    
    shutil.copy(csvPath, originCSVPath)

    zipf = zipfile.ZipFile(zipPath, 'w', zipfile.ZIP_DEFLATED)
    zipdir(OUT_PUT_PATH + "/", zipf)
    zipf.close()

    zipPath = zipPath.replace('/app/mainApi/app/static/', '')

    return JSONResponse({"success": "success", "image_path": OUT_PUT_PATH + "/" + fileName, "zip_path": zipPath})



@router.post(
    "/ml_mfiber_process",
    response_description="ML MFIBER Process",
)
async def mlMFIBERProcess(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_url")

    sensitivity = data.get("sensitivity")
    method = data.get("method")

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    originPath = imagePath.replace(fileName, '')
    tempPath = tempfile.mkdtemp()
    OUT_PUT_FOLDER = tempPath.split("/")[len(tempPath.split("/")) - 1]
    OUT_PUT_PATH = 'mainApi/app/static/ml_out/' + OUT_PUT_FOLDER

    fullPath = OUT_PUT_PATH + '/' + fileName
    csvPath = os.path.splitext(fullPath)[0] + '_300.csv'
    originCSVPath = os.path.splitext(imagePath)[0] + '_300.csv'
    zipPath = os.path.splitext(imagePath)[0] + '_measure_result.zip'

    if not os.path.exists(OUT_PUT_PATH):
        os.makedirs(OUT_PUT_PATH)

    cmd_str = "/app/mainApi/ml_lib/segAB {inputPath} {outputPath}"
    if method == "mfiber1":
       cmd_str += " /app/mainApi/ml_lib/typeAB/mfiber/src_paramA1.txt"
    if method == "mfiber2":
        cmd_str += " /app/mainApi/ml_lib/typeAB/mfiber/src_paramA2.txt"
    

    cmd_str += " " + sensitivity
    cmd_str += " /app/mainApi/ml_lib/color_table.txt"
    cmd_str = cmd_str.format(inputPath=imagePath, outputPath=OUT_PUT_PATH + "/" + fileName)
    print('----->', cmd_str)
    subprocess.call(cmd_str, shell=True)


    data = pd.read_csv(csvPath)
    data = processBasicMeasureData(csvPath, data)
    data.to_csv(csvPath)
    
    shutil.copy(csvPath, originCSVPath)

    zipf = zipfile.ZipFile(zipPath, 'w', zipfile.ZIP_DEFLATED)
    zipdir(OUT_PUT_PATH + "/", zipf)
    zipf.close()

    zipPath = zipPath.replace('/app/mainApi/app/static/', '')

    return JSONResponse({"success": "success", "image_path": OUT_PUT_PATH + "/" + fileName, "zip_path": zipPath})



@router.post(
    "/ml_mridge_process",
    response_description="DL MRIDGE Process",
)
async def mlMRIDGEProcess(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_url")

    sensitivity = data.get("sensitivity")


    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    originPath = imagePath.replace(fileName, '')
    tempPath = tempfile.mkdtemp()
    OUT_PUT_FOLDER = tempPath.split("/")[len(tempPath.split("/")) - 1]
    OUT_PUT_PATH = 'mainApi/app/static/ml_out/' + OUT_PUT_FOLDER

    fullPath = OUT_PUT_PATH + '/' + fileName
    csvPath = os.path.splitext(fullPath)[0] + '_300.csv'
    originCSVPath = os.path.splitext(imagePath)[0] + '_300.csv'
    zipPath = os.path.splitext(imagePath)[0] + '_measure_result.zip'

    if not os.path.exists(OUT_PUT_PATH):
        os.makedirs(OUT_PUT_PATH)


    sample_csv_path = "./mainApi/ml_lib/sample.ome_300.csv"
    
    processMRIDGEMethod(fullPath, sensitivity)


    #print(fullPath)
    #print(csvPath)
    #print(zipPath)

    #cmd_str = "/app/mainApi/ml_lib/segAB {inputPath} {outputPath}"
   
    #cmd_str += " " + sensitivity
    # cmd_str += " /app/mainApi/ml_lib/color_table.txt"
    # cmd_str = cmd_str.format(inputPath=imagePath, outputPath=OUT_PUT_PATH + "/" + fileName)
    # print('----->', cmd_str)
    # subprocess.call(cmd_str, shell=True)


    # data = pd.read_csv(csvPath)
    # data = processBasicMeasureData(csvPath, data)
    # data.to_csv(csvPath)
    
    shutil.copy(sample_csv_path, originCSVPath)

    print(fullPath)

    # zipf = zipfile.ZipFile(zipPath, 'w', zipfile.ZIP_DEFLATED)
    # zipdir(OUT_PUT_PATH + "/", zipf)
    # zipf.close()

    zipPath = zipPath.replace('/app/mainApi/app/static/', '')

    return JSONResponse({"success": "success", "image_path": OUT_PUT_PATH + "/" + fileName, "zip_path": zipPath})






@router.post(
    "/ml_ict_process_test",
    response_description="ML IPS Process Test",
)
async def mlICTProcessTest(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_url")
    sensitivity = data.get("sensitivity")
    param = data.get("param")
    params = param.split(",")
    type = data.get("type")

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    tempPath = tempfile.mkdtemp()
    OUT_PUT_FOLDER = tempPath.split("/")[len(tempPath.split("/")) - 1]
    OUT_PUT_PATH = 'mainApi/app/static/ml_out/' + OUT_PUT_FOLDER
    paramPath = '/app/mainApi/app/static/ml_out/' + OUT_PUT_FOLDER + '/param.txt'

    if not os.path.exists(OUT_PUT_PATH):
        os.makedirs(OUT_PUT_PATH)

    path = ''

    if type == 'a':
        path = "/app/mainApi/ml_lib/typeB/src_paramA.txt"
    if type == 'b':
        path = "/app/mainApi/ml_lib/typeB/src_paramB.txt"
    if type == 'c':
        path = "/app/mainApi/ml_lib/typeB/src_paramC.txt"
    if type == 'd':
        path = "/app/mainApi/ml_lib/typeB/src_paramD.txt"

    with open(path, 'rb') as fsrc:
        with open(paramPath, 'wb') as fdst:
            fdst.write(fsrc.read())

    with open(paramPath, "a") as f:
        f.write("\n")
        for num in params:
            f.write(num + "\n")


    cmd_str = "/app/mainApi/ml_lib/segB {inputPath} {outputPath} " + paramPath

    cmd_str += " " + sensitivity
    cmd_str = cmd_str.format(inputPath=imagePath, outputPath=OUT_PUT_PATH + "/" + fileName)
    print('----->', cmd_str)
    subprocess.call(cmd_str, shell=True)
    return JSONResponse({"success": "success", "image_path": OUT_PUT_PATH + "/" + fileName})

@router.post(
    "/ml_ips_process",
    response_description="ML IPS Process",
)
async def mlIPSProcess(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    sensitivity = data.get("sensitivity")
    colors = data.get('colors')
    colorOption = data.get('colorOption')
    tilemergedFlag = data.get('tilingMergedImageFlag')
    colors = colors.split(",")
    imgPath = data.get("original_image_url")
    paths = imgPath.split(",")
    realPaths = []

    for path in paths:
        realPaths.append('/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + path)

    imagePath = ' '.join(realPaths)

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    tempPath = tempfile.mkdtemp()
    OUT_PUT_FOLDER = tempPath.split("/")[len(tempPath.split("/")) - 1]
    OUT_PUT_PATH = 'mainApi/app/static/ml_out/' + OUT_PUT_FOLDER
    #paramPath = '/app/mainApi/app/static/ml_out/' + OUT_PUT_FOLDER + '/color_table.txt'
    color_table_path = "app/mainApi/ml_lib/color_table.txt"

    if not os.path.exists(OUT_PUT_PATH):
        os.makedirs(OUT_PUT_PATH)

    #with open(paramPath, "a") as f:
    #    for color in colors:
    #        f.write(color + "\n")

    cmd_str = "/app/mainApi/ml_lib/segAB {inputPath} {outputPath} /app/mainApi/ml_lib/typeAB/src_param.txt " + sensitivity + " " + color_table_path + " > /app/mainApi/ml_lib/log.txt"
    

    resultInputPath = imagePath

    # if tilemergedFlag == "true":
    #     path = imagePath
    #     tempLists = path.split("/")
    #     fileName = tempLists[-1]

    #     t = tempLists[-2] + "/" + tempLists[-1]

    #     rel_dir = path.split(t)[0]

    #     s_channel_filepath = rel_dir + "S/" + fileName
    #     r_channel_filepath = rel_dir + "R/" + fileName
    #     g_channel_filepath = rel_dir + "G/" + fileName
    #     b_channel_filepath = rel_dir + "B/" + fileName
    #     merged_channel_filepath = rel_dir + "Overlay/" + fileName

        

    #     if colorOption == "S":
    #         resultInputPath = s_channel_filepath
    #     if colorOption == "R":
    #         resultInputPath = r_channel_filepath
    #     if colorOption == "G":
    #         resultInputPath = g_channel_filepath
    #     if colorOption == "B":
    #         resultInputPath = b_channel_filepath
    #     if colorOption == "B+R":
    #         resultInputPath = b_channel_filepath + " " + r_channel_filepath
    #     if colorOption == "B+G":
    #         resultInputPath = b_channel_filepath + " " + g_channel_filepath
    #     if colorOption == "B+G+R":
    #         resultInputPath = b_channel_filepath + " " + g_channel_filepath + " " + r_channel_filepath
    #     if colorOption == "S+G+R":
    #         resultInputPath = s_channel_filepath + " " + g_channel_filepath + " " + r_channel_filepath
    #     if colorOption == "S+B+R":
    #         resultInputPath = s_channel_filepath + " " + b_channel_filepath + " " + r_channel_filepath
    #     if colorOption == "S+B+G":
    #         resultInputPath = s_channel_filepath + " " + b_channel_filepath + " " + g_channel_filepath
    #     if colorOption == "S+B+G+R":
    #         resultInputPath = s_channel_filepath + " " + b_channel_filepath + " " + g_channel_filepath + r_channel_filepath
        
    # print(resultInputPath)

    cmd_str = cmd_str.format(inputPath=resultInputPath, outputPath=OUT_PUT_PATH + "/" + fileName)
    print('----->', cmd_str)
    subprocess.call(cmd_str, shell=True)
    return JSONResponse({"success": "success", "image_path": OUT_PUT_PATH + "/" + fileName})



def getMergeImageByCountResult(image1Path,  color_option, sensitivity, resultpath):
    try:    
        image = cv2.imread(image1Path)

        COLORS = [
        ( 0   , 0 , 255	),
        ( 0  , 0 , 128	),
        ( 255 ,  0, 255	),
        ( 128  , 0 ,128	),
        ( 0 ,255, 255	),
        ( 0 ,128 ,128	),
        ( 128 ,128 ,128	),
        ( 128 ,  0,   0	),
        ( 0 ,255,   0	),
        ( 0 ,128,   0	),
        ( 196 ,196, 196	),
        ( 128, 128,   0	),
        ( 255 ,255  , 0	),
        ( 255  , 0  , 0	),
        ]

        k = 1
        color_num = 0
        radius = 1
        if sensitivity == 'undefined':
            sensitivity = 50
        sensitivity = float(sensitivity) 

        if sensitivity < 35:
            radius = 1
        elif sensitivity < 70:
            radius = 2
        else:
            radius = 3

        if color_option == "S":
            color_num = 12
            k = 1
        if color_option == "R":
            color_num = 11
            k = 2
        if color_option == "G":
            color_num = 10
            k = 3
        if color_option == "B":
            color_num = 9
            k = 4
        if color_option == "B+R":
            color_num = 8
            k = 5
        if color_option == "B+G":
            color_num = 7
            k = 6

        if color_option == "B+G+R":
            color_num = 6
            k = 7
        if  color_option == "S+G+R":
            color_num = 5
            k = 8
        if color_option == "S+B+R" : 
            color_num = 3
            k = 9
        if color_option == "S+B+G":
            color_num = 2
            k = 10
        if color_option == "S+B+G+R":
            color_num = 0
            k = 11
                

        pixel_values = image.reshape((-1, 3))

        # convert to float
        pixel_values = np.float32(pixel_values)


        # define stopping criteria
        criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.2)

        _, labels, (centers) = cv2.kmeans(pixel_values, k, None, criteria, 10, cv2.KMEANS_RANDOM_CENTERS)


        # convert back to 8 bit values
        centers = np.uint8(centers)

        # flatten the labels array
        labels = labels.flatten()


        for c in range(k):
            norm = np.linalg.norm(centers[c])
            if(norm < 15): continue
            centers[c] = COLORS[c+color_num]

        # convert all pixels to the color of the centroids
        segmented_image = centers[labels.flatten()]

        # reshape back to the original image dimension
        segmented_image = segmented_image.reshape(image.shape)
        # show the image


        (height, width, channel) = segmented_image.shape


        step = radius * 3 + 1
        thickness = radius

        for x in range(1,width,step):
            for y in range(1, height,step):
                colors = segmented_image[y,x]
                color = (int(colors[0]), int(colors[1]), int(colors[2]))
                cv2.circle(image, (x,y), radius , color, thickness)


        cv2.imwrite(resultpath, image)
    
    except:
        return
        


@router.post(
    "/ml_convert_result",
    response_description="ML Convert Processed images to Ome.Tiff file",
)
async def mlConvertResult(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = data.get("image_path")
    colors = data.get("colors")
    sensitivity = data.get("sensitivity")

    print(colors)
    print(sensitivity)

    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    realName = os.path.splitext(fileName)[0]
    tempPath = tempfile.mkdtemp()
    print("ml-convert-result-filename:", realName)
    csvPath = os.path.splitext(imagePath)[0] + '_300.csv'
    outputFolder = '/app/mainApi/app/static' + tempPath
    
    
    originFilePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_path")
    originCSVPath = os.path.splitext(originFilePath)[0] + '_300.csv'

    print(originFilePath)

    ttpath = originFilePath.split(".")[0] + ".jpg"
    

    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)

    realPath = os.path.splitext(imagePath)[0] + 'a_2.jpg'
    countPath = os.path.splitext(imagePath)[0] + 'a_13.jpg'

    color_segmentedImage1path = os.path.splitext(imagePath)[0] + 'a_3.jpg'
    color_segmentedImage2path = os.path.splitext(imagePath)[0] + '_252.jpg'
    color_segmentedImage3path = os.path.splitext(imagePath)[0] + '_251.jpg'


    getMergeImageByCountResult(ttpath,  colors, sensitivity, color_segmentedImage1path)


    outputFolder = '/app/mainApi/app/static' + tempPath
    outputPath = outputFolder + '/' + realName + 'a_2.ome.tiff'
    outputPath1 = outputFolder + '/' + realName + 'a_2_temp.tiff'
    outputPath2 = outputFolder + '/' + realName + 'a_2_temp.ome.tiff'


    count_reult_path =  outputFolder + '/' + realName + 'a_13.ome.tiff'
    color_segment1_path = outputFolder + '/' + realName + 'a_3.ome.tiff'
    color_segment2_path = outputFolder + '/' + realName + '_252.ome.tiff'
    color_segment3_path = outputFolder + '/' + realName + '_251.ome.tiff'

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{countPath}' '{count_reult_path}'"
    await shell(bfconv_cmd)

    print("===>before convert by PILLOW")
    # Open the JPG file
    #input_file = '/path/to/input.jpg'
    image = Image.open(realPath)
    # Convert the image to OME-TIFF
    output_file = outputPath1
    image.save(outputPath1, format='TIFF', compression='tiff_lzw')



    cmd_str = "sh /app/mainApi/bftools/bfconvert -separate -overwrite '" + realPath + "' '" + outputPath1 + "'"
    print('=====>', cmd_str)
    subprocess.run(cmd_str, shell=True)

    cmd_str = "sh /app/mainApi/bftools/bfconvert -separate -overwrite '" + color_segmentedImage1path + "' '" + color_segment1_path + "'"
    print('=====>', cmd_str)
    subprocess.run(cmd_str, shell=True)

    # cmd_str = "sh /app/mainApi/bftools/bfconvert -separate -overwrite '" + color_segmentedImage2path + "' '" + color_segment2_path + "'"
    # print('=====>', cmd_str)
    # subprocess.run(cmd_str, shell=True)

    # cmd_str = "sh /app/mainApi/bftools/bfconvert -separate -overwrite '" + color_segmentedImage3path + "' '" + color_segment3_path + "'"
    # print('=====>', cmd_str)
    # subprocess.run(cmd_str, shell=True)


    print("===>after convert by PILLOW")





    data = pd.read_csv(csvPath)
    data = processBasicMeasureData(csvPath, data)
    data.to_csv(csvPath)

    shutil.copy(csvPath, originCSVPath)

    

    return JSONResponse({
        "success": "success",
        "image_path": tempPath + '/' + realName + 'a_2_temp.tiff',
        "count_path": tempPath + '/' + realName + 'a_13.ome.tiff',
        'color_segment1_result' : tempPath + '/' + realName + 'a_3.ome.tiff',
        'color_segment2_result' : tempPath + '/' + realName + '_252.ome.tiff',
        'color_segment3_result' : tempPath + '/' + realName + '_251.ome.tiff',
        "csv_path": csvPath
    })

@router.post(
    "/ml_convert_result_select",
    response_description="ML Convert Processed images to Ome.Tiff file",
)
async def mlConvertResultSelect(request: Request, current_user: UserModelDB = Depends(get_current_user)):
    data = await request.form()
    imagePath = data.get("image_path")
    originalImagePath = '/app/mainApi/app/static/' + str(PyObjectId(current_user.id)) + '/' + data.get("original_image_path")
    fileName = imagePath.split("/")[len(imagePath.split("/")) - 1]
    realName = os.path.splitext(fileName)[0]
    tempPath = tempfile.mkdtemp()
    print("ml-convert-result-filename:", realName)
    csvPath = os.path.splitext(imagePath)[0] + '_300.csv'
    outputFolder = '/app/mainApi/app/static' + tempPath


    originCSVPath = os.path.splitext(originalImagePath)[0] + '_300.csv'

    print(originCSVPath)



    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)

    realPath = os.path.splitext(imagePath)[0] + 'a_2.jpg'
    outputFolder = '/app/mainApi/app/static' + tempPath

    mergedPath = outputFolder + '/' + realName + '_merged.ome.tiff'

    # Load the OME-TIFF file
    ome_tiff = tifffile.imread(originalImagePath)

    # Get the number of channels
    num_channels = ome_tiff.shape[0]
    input_files = [outputFolder + '/' + 'output.tiff']

    print("===>origin channels:", num_channels)

    # Loop over each channel and save as a separate TIFF file
    for i in range(num_channels):
        # Get the image data for this channel
        channel_data = ome_tiff[i]

        # Save the channel data as a TIFF file
        tifffile.imsave(outputFolder + '/' + f'channel_{i}.tiff', channel_data)
        input_files.append(outputFolder + '/' + f'channel_{i}.tiff')

    # Load the JPEG file
    img = Image.open(realPath)
    # Convert the image to grayscale
    gray_img = img.convert('L')
    # Save the grayscale image as a TIFF file
    gray_img.save(outputFolder + '/' + 'output.tiff')

    my_string = ' '.join(input_files)
    cmd_str = f'python /app/mainApi/ml_lib/pyramid_assemble.py {my_string} {mergedPath} --pixel-size 1'
    print('=====>', cmd_str)
    subprocess.run(cmd_str, shell=True)

    # metadata = bioformats.get_omexml_metadata('/app/mainApi/app/static/6461894c49dbc4f3496599ba/1/my_test/at3_1m4_01.ome.tiff')
    # xml = bioformats.OMEXML(metadata)
    # print("ome-xml:", xml)

    data = pd.read_csv(csvPath)
    data = processBasicMeasureData(csvPath, data)
    data.to_csv(csvPath)

    shutil.copy(csvPath, originCSVPath)



    return JSONResponse({
        "success": "success",
        "image_path": tempPath + '/' + realName + '_merged.ome.tiff',
        "csv_path": csvPath
    })

@router.get("/test")
def read_root():
    print('sdfsdfsdf')
    with h5.File('example.h5', 'w') as f:
    # create a group
        group = f.create_group('mygroup')
        
        # create a dataset inside the group
        data = [1, 2, 3, 4, 5]
        group.create_dataset('mydata', data=data)

# read the data from the file
    with h5.File('example.h5', 'r') as f:
        # get the dataset
        dataset = f['mygroup/mydata']
        
        # print the dataset
        print(dataset[:])
    return {"Ping": "Pang"}




@router.post('/measure/update_measure_data')
async def update_measure_data(
    request: Request,
    keyList: List[str] = Form(...)
):
    print(request)
    data = await request.form()
    print('======> keyList', keyList)
    res = update_h5py_file(data, keyList)
    print(res)

    # for key in keyList:
    #     value = data.get(key)
    #     print(json.loads(value))
    #     print('=======>', key)
    return res




@router.post('/measure/processBasicMeasure',
    response_description="process Basic Measure",
    status_code=status.HTTP_200_OK,
    )
async def processBasicMeasure(
    request: Request,
):
    body_bytes = await request.body()
    params = json.loads(body_bytes)
    
    filepath = params["path"]

    pathlist = filepath.split("/image/download_csv?path=")
    filepath = '/app/mainApi/app/static' + pathlist[1]

    data = pd.read_csv(filepath)

    data = processBasicMeasureData(filepath, data)

    data.to_csv(filepath)
    
    print(data)
    print(filepath)

    return JSONResponse({"Result" : "Success"})



@router.post(
    "/deconv2D",
    response_description="Deconvolution 2D",
    status_code=status.HTTP_200_OK,
)
async def processDeconv2D(
    request: Request,
):
    body_bytes = await request.body()
    params = json.loads(body_bytes)

    filepath = params["filename"]
    effectiveness = params['effectiveness']
    isroi = params['isroi']
    dictRoiPts = params['dictRoiPts']

    #print(params)
    print("Start Processing for Deconvolution 2D")

    abs_path = await Deconv.FlowDecDeconvolution2D(
        filepath, effectiveness, isroi, dictRoiPts
    )



    return JSONResponse(abs_path) 




@router.post(
    "/deconv3D",
    response_description="Deconvolution 3D",
    status_code=status.HTTP_200_OK,
)
async def processDeconv3D(
    request: Request,
):
    body_bytes = await request.body()
    params = json.loads(body_bytes)

    filepath = params["filename"]
    effectiveness = params['effectiveness']
    isroi = params['isroi']
    dictRoiPts = params['dictRoiPts']

    #print(params)
    print("Start Processing for Deconvolution 3D")

    abs_path = await Deconv.FlowDecDeconvolution2D(
        filepath, effectiveness, isroi, dictRoiPts
    )

