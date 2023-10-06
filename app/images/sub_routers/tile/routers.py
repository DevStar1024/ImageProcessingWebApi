import asyncio
import concurrent
import os
import pydantic
from fastapi.responses import JSONResponse, FileResponse
from fastapi import (
    Request,
    Body,
    APIRouter,
    Depends,
    status,
    UploadFile,
    File,
    Form,
    HTTPException,
)
from motor.motor_asyncio import AsyncIOMotorDatabase
from typing import Any, FrozenSet, List
import jsons
import string
from tokenize import String
from PIL import Image
import json
from bson import ObjectId, json_util
import uuid
import aiofiles
import subprocess
import re



from mainApi.app.auth.auth import get_current_user
from mainApi.app.db.mongodb import get_database
from mainApi.app.images.sub_routers.tile.models import (
    AlignNaiveRequest,
    TileModelDB,
    FileModelDB,
    AlignedTiledModel,
    NamePattenModel,
    MergeImgModel,
    ExperimentModel,
    MetadataModel,
    UserCustomModel,
)
from mainApi.app.images.utils.asyncio import shell
from mainApi.app.images.utils.tiling import (
    add_image_tiles,
    delete_tiles_in,
    get_all_tiles,
)
from mainApi.app.images.utils.experiment import (
    add_experiment,
    add_experiment_with_folders,
    add_experiment_with_files,
    get_experiment_data,
    add_experiment_with_folder,
    convert_npy_to_jpg,
    get_model,
)
import mainApi.app.images.utils.deconvolution as Deconv
import mainApi.app.images.utils.super_resolution.functions as SuperResolution
from mainApi.app.images.utils.folder import get_user_cache_path, clear_path
from mainApi.app.auth.models.user import UserModelDB, PyObjectId
from mainApi.config import STATIC_PATH, CURRENT_STATIC
import tifftools
from mainApi.app.images.utils.convert import get_metadata
import shutil

import bioformats as bf
# from bioformats import logback
from mainApi.app.images.utils.contrastlimits import calculateImageStats
from mainApi.app.images.utils.focus_stack import focus_stack

import cv2
import numpy as np
import base64
import glob

from mainApi.app.images.sub_routers.tile.utils import mergeImageWithOverlap

TILING_RESULT_IMAGE_FILE_NAME = 'result.ome.tiff'


CHANNELS_PATTERN = ['S',"B","G","R","C","Y","M","Overlay"]

router = APIRouter(
    prefix="/tile",
    tags=["tile"],
)


def extract_numbers(string):
    if isinstance(string, (str, bytes)):
        numbers = re.findall(r'\d+', string)
        res = ""
        for item in numbers:
            res += item
        if len(res) == 0:
            return '0'
        return res    
    return '0'

# Upload Image file
@router.post(
    "/upload_tiles",
    response_description="Upload Image Tiles",
    status_code=status.HTTP_201_CREATED,
    response_model=List[TileModelDB],
)
async def upload_tiles(
    files: List[UploadFile] = File(...),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    current_user_path = os.path.join(
        STATIC_PATH, str(PyObjectId(current_user.id)), "images"
    )
    if not os.path.exists(current_user_path):
        os.makedirs(current_user_path)
    elif clear_previous:
        for f in os.listdir(current_user_path):
            os.remove(os.path.join(current_user_path, f))
        await db["tile-image-cache"].delete_many(
            {"user_id": PyObjectId(current_user.id)}
        )
    result = await add_image_tiles(
        path=current_user_path,
        files=files,
        current_user=current_user,
        db=db,
    )
    return JSONResponse(result)


@router.get(
    "/get_tiles",
    response_description="Get all image tiles",
    status_code=status.HTTP_200_OK,
)
async def get_tiles(
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    tiles = await get_all_tiles(current_user, db)

    return JSONResponse(tiles)



def getDirName(dir,series,row, col):
    dir_name =  str(series).replace(" ","") + "_" + str(row) + "_" + str(col)
    new_path = f"{CURRENT_STATIC}/{dir}/{dir_name}"
    new_rel_path = new_path.rsplit("/static/", 1)[1]
    new_abs_path = os.path.join(STATIC_PATH, new_rel_path)

    return new_abs_path


@router.post(
    "/delete_tiles",
    response_description="Delete Tiles",
    status_code=status.HTTP_200_OK,
)
async def delete_tiles(
    request: Request,
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> Any:
    body_bytes = await request.body()
    params = json.loads(body_bytes)

    fileLists = params["filelists"]
    listsToDeleteFiles = []
    for file in fileLists:
        filename = file.split('/')[-1].split(".")[0]
        listsToDeleteFiles.append(filename)
    
    experiment_name = fileLists[0].split("/")[0]
 
    oldExpData = await db["experiment"].find_one({"experiment_name": experiment_name, "user_id": current_user.id})
    

    merged_data = []
    folder_mapping = {}

    # Create a mapping of folder names to their corresponding experiment data

    for folder_data in oldExpData["experiment_data"]:
            folder = folder_data["folder"]
            print("Files are ")
            print(folder_data["files"])
            if folder not in folder_mapping:
                folder_mapping[folder] = []
                for deletedItem in listsToDeleteFiles:
                    for file in folder_data["files"]:
                        if deletedItem in file:
                            folder_data["files"].remove(file)
                folder_mapping[folder].append(folder_data["files"])

    # Merge experiment data for matching folder names
    for folder, file_lists in folder_mapping.items():
        merged_files = []
        for files in file_lists:
            merged_files += files
        # save tiling flag when merging folder images too
        merged_data.append({"folder": folder, "files": list(set(merged_files)), "tiling": None})


    await db["experiment"].update_one(
        {"_id": oldExpData["_id"]},
        {"$set": {"experiment_data": merged_data}},
    )


    
    return JSONResponse({"result" : "success"})





# @router.post(
#     "/build_pyramid",
#     response_description="Build Pyramid",
#     status_code=status.HTTP_200_OK,
# )
# async def build_pyramid(
#     request: Request,
#     user: UserModelDB = Depends(get_current_user),
#     db: AsyncIOMotorDatabase = Depends(get_database),
# ) -> List[FileModelDB]:
#     body_bytes = await request.body()
#     ashlar_params = json.loads(body_bytes)


def getChannelKey(values, ch_character):
    if(ch_character == ""):
        return "Overlay"
    for key,value in values.items():
        if value == ch_character:
             return key
                
    return "Overlay"




@router.post(
    "/update_tiles_meta_info",
    response_description="Update Tiles Metainfo",
    status_code=status.HTTP_200_OK,
)
async def update_tiles_meta_info(
    request: Request,
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> Any:
    body_bytes = await request.body()




    data = json.loads(body_bytes)


    channelValues = {}
    metainfo = data["tiles_meta_info"][0]
    for key in CHANNELS_PATTERN:
        channelValues[key] = metainfo[key]
    


    for meta_info in data["tiles_meta_info"]:
        if(meta_info["row"] == "") :
            meta_info["row"] = 0
        if(meta_info["col"] == "") :
            meta_info["col"] = 0
        

    #Create the Folders for the correspoding row and cols and channel

    for meta_info in data["tiles_meta_info"]:
        dir = meta_info["path"].rsplit("/", 1)[0]
        new_abs_dir_path = getDirName( dir, meta_info["strSeries"],meta_info["row"],meta_info["col"])

        if not os.path.exists(new_abs_dir_path):
            os.makedirs(new_abs_dir_path)
        
        for ch in CHANNELS_PATTERN:
            new_abs_channel_dir_path = os.path.join(new_abs_dir_path, ch)
            if not os.path.exists(new_abs_channel_dir_path):
                os.makedirs(new_abs_channel_dir_path)


    check_arr = []
    for meta_info in data["tiles_meta_info"]:
        name = str(meta_info["row"]) + str(meta_info["col"]) + str(meta_info["field"])

        series = meta_info["field"]
        series = extract_numbers(series)
        row = meta_info["row"]
        col = meta_info["col"]
        ashlar_path = ""
        
        


        merge_arr = []
       

        
        
            # cv2.imwrite(save_file_path,res_image)
            # try :
            #     img = Image.open(ashlar_path)
            #     img.thumbnail([100, 100])
            #     img.save(input_pre + '.timg', 'png')
            # except :
            #     pass

       


        if "LiveDead" not in meta_info["strSeries"]:
            dir = meta_info["path"].rsplit("/", 1)[0]
            new_abs_dir_path = getDirName(dir, meta_info["strSeries"],meta_info["row"],meta_info["col"])
            series = meta_info["field"]
            series = extract_numbers(series)
            ext = meta_info["filename"].rsplit(".", 1)[1]
            ch = getChannelKey(channelValues, meta_info["channel"])


            old_rel_path = meta_info["path"].rsplit("/static/", 1)[1]
            old_abs_path = os.path.join(STATIC_PATH, old_rel_path)
            



                
            file_name = f"tile_image_series{str(series).rjust(5, '0')}.{ext}"
            save_file_path = os.path.join(new_abs_dir_path,ch, file_name)

            old_thumb_path = os.path.join(STATIC_PATH, old_rel_path.replace(ext, "timg"))
            new_thumb_name = file_name.replace(ext, "timg")
            save_thumb_path = os.path.join(new_abs_dir_path,ch, new_thumb_name)

            shutil.copy(old_abs_path, save_file_path)
            shutil.copy(old_thumb_path, save_thumb_path)

            ashlar_path = save_file_path


            await db["tile-image-cache"].update_one(
            {"_id": ObjectId(meta_info["_id"])},
            {
                "$set": {
                    "series": int(series),
                    "field" : meta_info["field"],
                    "strSeries" : meta_info["strSeries"],
                    "row" : meta_info["row"],
                    "col" : extract_numbers(meta_info["col"]),
                    "time" : extract_numbers(meta_info["time"]),
                    "z" : extract_numbers(meta_info["z"]),
                    "channel" : getChannelKey(channelValues, meta_info["channel"]),
                    "ashlar_path": ashlar_path,
                    'objective' : extract_numbers(meta_info["objective"])
                }
            },
         )


            continue

        if not name in  check_arr:
            merge_arr = []
            check_arr.append(name)
            filenames_arr = []
            for meta in data["tiles_meta_info"]:
                if meta["row"] == row and meta["col"] == col and meta["field"] == meta_info["field"]:
                    old_rel_path = meta["path"].rsplit("/static/", 1)[1]
                    old_abs_path = os.path.join(STATIC_PATH, old_rel_path)
                    filenames_arr.append(old_abs_path)
                    #Merge the images by field

                    image = cv2.imread(old_abs_path,-1)
                    if image.dtype == 'uint16' :
                        ratio = np.amax(image) / 256 
                        image = (image / ratio).astype('uint8')
                    merge_arr.append(image)

            dir = meta_info["path"].rsplit("/", 1)[0]
            new_abs_dir_path = getDirName( dir, meta_info["strSeries"],meta_info["row"],meta_info["col"])
            series = meta_info["field"].split("f")[1]
            ext = meta_info["filename"].rsplit(".", 1)[1]
            file_name = f"tile_image_series{str(series).rjust(5, '0')}.{ext}"
            


            if(len(merge_arr) == 2):
                merge_arr.append(merge_arr[0])
            
            if(len(merge_arr) == 1 and len(merge_arr[0].shape) < 3):
                merge_arr.append(merge_arr[0])
                merge_arr.append(merge_arr[0])
            
            t = [merge_arr[0], merge_arr[1],merge_arr[2]]
            res_image = cv2.merge(t)
            

            save_file_path = os.path.join(new_abs_dir_path,'Overlay',file_name)

            if os.path.exists(save_file_path):
                os.remove(save_file_path)


            cv2.imwrite(save_file_path,res_image)
            ashlar_path = save_file_path

            input_pre = os.path.splitext(ashlar_path)[0]



            try :
                img = Image.open(ashlar_path)
                img.thumbnail([100, 100])
                if os.path.exists(input_pre + '.timg'):
                    os.remove(input_pre + '.timg')
                img.save(input_pre + '.timg', 'png')
            except :
                pass

            


            await db["tile-image-cache"].update_one(
            {"_id": ObjectId(meta_info["_id"])},
            {
                "$set": {
                    "series": int(series),
                    "field" : meta_info["field"],
                    "strSeries" : meta_info["strSeries"],
                    "row" : meta_info["row"],
                    "col" : extract_numbers(meta_info["col"]),
                    "time" : extract_numbers(meta_info["time"]),
                    "z" : extract_numbers(meta_info["z"]),
                    "channel" : getChannelKey(channelValues, meta_info["channel"]),
                    "ashlar_path": ashlar_path,
                    'objective' : extract_numbers(meta_info["objective"])
                }
            },
        )




        #dir = meta_info["path"].rsplit("/", 1)[0]
        #ext = meta_info["filename"].rsplit(".", 1)[1]

        # new_path = f"{CURRENT_STATIC}/{dir}/tile_image_series{str(meta_info['series']).rjust(5, '0')}.{ext}"
        # old_rel_path = meta_info["path"].rsplit("/static/", 1)[1]
        # new_rel_path = new_path.rsplit("/static/", 1)[1]
        
       
        # old_abs_path = os.path.join(STATIC_PATH, old_rel_path)
        # new_abs_path = os.path.join(STATIC_PATH, new_rel_path)

        # shutil.copy(old_abs_path, new_abs_path)

        

@router.post(
    "/create_tiles",
    response_description="Create Tiles",
    status_code=status.HTTP_200_OK,
)
async def create_tiles(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    body_bytes = await request.body()
    data = json.loads(body_bytes)
    tiles = []
    for tile_path in data["paths"]:
        tiles.append({
            "user_id": user.id,
            "filename": tile_path.rsplit('/', 1)[1],
            "path": f"{CURRENT_STATIC}/{user.id}/{tile_path}"
        })

    await db["tile-image-cache"].delete_many(
        {"user_id": user.id}
    )

    # insert new tile images
    insert_res = await db["tile-image-cache"].insert_many(tiles)
    inserted_ids = [str(id) for id in insert_res.inserted_ids]

    return JSONResponse(inserted_ids)



async def normalizeImage(rel_dir):
    input_filename = "ashlar_output.ome.tiff"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "normalize_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)
    output_rel_path = os.path.join('/static/', rel_dir, output_filename)


    #Read the image
    img = bf.load_image(input_path)
    img = np.array(img)
    metadata = bf.get_omexml_metadata(input_path)


     # Split the image into color channels
    channels = cv2.split(img)

    # Calculate the shading correction factor for each channel
    merged_img = []
    for channel in channels:
        cv2.normalize(channel, channel, 0, 65535, cv2.NORM_MINMAX)
        merged_img.append(channel)


    # Merge the corrected channels into a single image
    merged = cv2.merge(merged_img)

    dir = os.path.join(STATIC_PATH, rel_dir)
    temp_ome_path = os.path.join(dir, "result.ome.tiff")


    if os.path.exists(temp_ome_path):
            os.remove(temp_ome_path)

    pixel_type = bf.omexml.PT_UINT8
    bf.write_image(temp_ome_path,merged, pixel_type)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_ome_path}' '{output_path}'"
    await shell(bfconv_cmd)


async def correctionImage(rel_dir):
    input_filename = "ashlar_output.ome.tiff"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "correction_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

    output_rel_path = os.path.join('/static/', rel_dir, output_filename)

    #Read the image

    
    img = bf.load_image(input_path)
    img = np.array(img)

    # Split the image into color channels
    channels = cv2.split(img)

    # Calculate the shading correction factor for each channel
    shading = []
    for channel in channels:
        blurred = cv2.GaussianBlur(channel, (0, 0), sigmaX=50, sigmaY=50)
        shading_channel = channel.astype(np.float32) / blurred.astype(np.float32)
        cv2.normalize(shading_channel, shading_channel, 0, 65535, cv2.NORM_MINMAX)
        shading.append(shading_channel)

    # Apply the shading correction factor to each channel
    corrected = []
    for i, channel in enumerate(channels):
        corrected_channel = np.multiply(channel.astype(np.float32), shading[i])
        corrected.append(corrected_channel)

    # Merge the corrected channels into a single image
    corrected = cv2.merge(corrected)


    dir = os.path.join(STATIC_PATH, rel_dir)
    temp_ome_path = os.path.join(dir, "result.ome.tiff")


    if os.path.exists(temp_ome_path):
            os.remove(temp_ome_path)

    pixel_type = bf.omexml.PT_UINT8
    bf.write_image(temp_ome_path,corrected, pixel_type)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_ome_path}' '{output_path}'"
    await shell(bfconv_cmd)


async def gammaImage(rel_dir):

    for i in range(8,13):
        gamma = i / 10.0
        input_filename = "ashlar_output.ome.tiff"
        input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

        output_filename = "gamma" + str(i) +  "_output.ome.tiff"
        output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

        output_rel_path = os.path.join('/static/', rel_dir, output_filename)

        #Read the image
        image = bf.load_image(input_path)
        image = np.array(image) * 255
    
        image = image.astype('uint8')

        table = np.array([((i / 255.0) ** gamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
        # apply gamma correction using the lookup table
        final_image =  cv2.LUT(image, table)


        dir = os.path.join(STATIC_PATH, rel_dir)
        temp_ome_path = os.path.join(dir, "result.ome.tiff")

    
        if os.path.exists(temp_ome_path):
                os.remove(temp_ome_path)

        pixel_type = bf.omexml.PT_UINT8
        bf.write_image(temp_ome_path,final_image, pixel_type)

        bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_ome_path}' '{output_path}'"
        await shell(bfconv_cmd)





async def snapToEdge(rel_dir):
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)

    # Load the images
    file_list = glob.glob(tiles_dir + "/*.timg")

    images = []
    
    for file in file_list:
        image = cv2.imread(file)
        images.append(image)

    
    # Define the number of images and the size of the output image
    num_images = len(images)
    output_size = (num_images * images[0].shape[1], images[0].shape[0])

    # Create an empty output image
    output = np.zeros((output_size[1], output_size[0], 3), dtype=np.uint8)

    # Loop through the images and align them
    for i in range(num_images):
        # Detect the edges of the current image
        edges1 = cv2.Canny(images[i], 100, 200)
        
        if i == 0:
            # For the first image, just copy it to the output
            output[0:images[i].shape[0], 0:images[i].shape[1], :] = images[i]
        else:
            # Detect the edges of the previous image
            edges2 = cv2.Canny(images[i-1], 100, 200)
            
            # Find the matching edges and align the images
            result = cv2.matchTemplate(edges1, edges2, cv2.TM_CCOEFF_NORMED)
            _, _, _, max_loc = cv2.minMaxLoc(result)
            h, w = images[i].shape[:2]
            aligned_img = images[i][:, max_loc[0]:max_loc[0]+w, :]
            
            # Copy the aligned image to the output
            output[0:images[i].shape[0], i*w:(i+1)*w, :] = aligned_img
    
    output = cv2.GaussianBlur(output, (3, 3), 0)

    temp_name = "temp_output.jpg"
    temp_output = os.path.join(STATIC_PATH, rel_dir, temp_name)

    temp_output_path = os.path.join('/static/', rel_dir, temp_name)

    # Save the output image
    cv2.imwrite(temp_output, output)

    output_filename = "snap_to_edge.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_output}' '{output_path}'"
    await shell(bfconv_cmd)




async def merge_Image(dir,rows,cols,align,direction,sortOrder,ext):
    filenames = os.listdir(dir)
    
    imageFileNames = []

    for filename in  filenames:
        if "tile_image_series" in filename:
            if str("." + ext) in filename:
                imageFileNames.append(filename)


    imageFileNames = sorted(imageFileNames, key = lambda x : x.lower())

    fullsize = len(imageFileNames)

    if sortOrder == False :
        imageFileNames.reverse()

    chunks = []

    
    if (direction == "horizontal") :
        for i in range(0, fullsize, cols) :
            chunks.append(imageFileNames[i:i+cols])
        # Reverse every second sub-array for snake layout
        if (align == "snake") :
            for i in range(1, len(chunks), 2):
                    chunks[i] = chunks[i][::-1]
        


    #if the direction is vertical
    if direction == "horizontal":
        chunks = [imageFileNames[i:i+cols] for i in range(0, fullsize, cols)]
        if align == "snake":
            for i in range(1, len(chunks), 2):
                chunks[i] = chunks[i][::-1]
    elif direction == "vertical":
        chunks = [[] for i in range(rows)]
        for i in range(rows):
            for j in range(cols):
                chunks[i].append(imageFileNames[j * rows + i])
        if align == "snake":
            temp = chunks
            chunks = [[] for i in range(rows)]
            for i in range(rows):
                for j in range(cols):
                    if j % 2 == 1:
                        chunks[i].append(temp[rows - i - 1][j])
                    else:
                        chunks[i].append(temp[i][j])


    finalArr = []
    # Iterate over each row
    for row in chunks:
        tempArr = []
        # Iterate over each column in the row
        for item in row:
            t =  os.path.join(dir, str(item))
            img = cv2.imread(t)
            tempArr.append(img)
        finalArr.append(tempArr)


    row_images = []

    for i in range(rows):
        merge_Image = cv2.hconcat(finalArr[i])
        row_images.append(merge_Image)

    result = cv2.vconcat(row_images)


    temp_path = os.path.join(dir, "result.png")
    cv2.imwrite(temp_path,result)

    output_filename = "ashlar_output.ome.tiff"
    output_filePath = os.path.join(dir, output_filename)
    
    temp_ome_path = os.path.join(dir, "result.ome.tiff")

    
    if os.path.exists(temp_ome_path):
            os.remove(temp_ome_path)

    pixel_type = bf.omexml.PT_UINT8
    bf.write_image(temp_ome_path, result, pixel_type)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_ome_path}' '{output_filePath}'"
    await shell(bfconv_cmd)


def doShadingCorrection(input_path, output_path):
    
    img  = cv2.imread(input_path)


    # get dimensions of image
    dimensions = img.shape
    
    # height, width, number of channels in image
    height = img.shape[0]
    width = img.shape[1]

    if width > 5000 or height > 5000 :
        image = cv2.resize(img,(int(width / 4), int(height / 4)))
    elif width < 1000 or height < 1000 :
        image = cv2.resize(img,(int(width  * 2), int(height * 2)))
    else :
        image = cv2.resize(img,(int(width  / 2), int(height / 2)))
    #gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(image, (3, 3), 0)
    illumination_map = image / blur
    illumination_map = cv2.normalize(illumination_map, None, 0, 255, cv2.NORM_MINMAX, dtype=cv2.CV_8UC1)
    corrected_image = cv2.divide(image, illumination_map, scale=255)
    
    cv2.imwrite(output_path, corrected_image)


def doShadingCorrectionWithNewMethod(input_path, output_path):

    img  = cv2.imread(input_path)
    

    # get dimensions of image
    dimensions = img.shape
    
    # height, width, number of channels in image
    height = img.shape[0]
    width = img.shape[1]

    if width > 5000 or height > 5000 :
        image = cv2.resize(img,(int(width / 4), int(height / 4)))
    elif width < 1000 or height < 1000 :
        image = cv2.resize(img,(int(width  * 2), int(height * 2)))
    else :
        image = cv2.resize(img,(int(width  / 2), int(height / 2)))
    #gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(image, (5, 5), 0)
    illumination_map = image / blur
    illumination_map = cv2.normalize(illumination_map, None, 0, 255, cv2.NORM_MINMAX, dtype=cv2.CV_8UC1)
    corrected_image = cv2.divide(image, illumination_map, scale=255)
    
    cv2.imwrite(output_path, corrected_image)



def adjust_gamma(input_path, gamma, output_path):
    img  = cv2.imread(input_path)
    
    # get dimensions of image
    dimensions = img.shape
    
    # height, width, number of channels in image
    height = img.shape[0]
    width = img.shape[1]

    if width > 5000 or height > 5000 :
        image = cv2.resize(img,(int(width / 4), int(height / 4)))
    elif width < 1000 or height < 1000 :
        image = cv2.resize(img,(int(width  * 2), int(height * 2)))
    else :
        image = cv2.resize(img,(int(width  / 2), int(height / 2)))
	# build a lookup table mapping the pixel values [0, 255] to
	# their adjusted gamma values
    gamma = gamma / 10.0
    image = image.astype('uint8')
    table = np.array([((i / 255.0) ** gamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
    # apply gamma correction using the lookup table
    result =  cv2.LUT(image, table)
    cv2.imwrite(output_path, result)



def whiteBalanceImage(input_path, output_path):
    img  = cv2.imread(input_path)
    
    # get dimensions of image
    dimensions = img.shape

    # height, width, number of channels in image
    height = img.shape[0]
    width = img.shape[1]
    if width > 5000 or height > 5000 :
        image = cv2.resize(img,(int(width / 4), int(height / 4)))
    elif width < 1000 or height < 1000 :
        image = cv2.resize(img,(int(width  * 2), int(height * 2)))
    else :
        image = cv2.resize(img,(int(width  / 2), int(height / 2)))

    #gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    wimage = ~image

    cv2.imwrite(output_path, wimage)




    return




def blackBalanceImage(input_path, output_path, threadValue):
    img  = cv2.imread(input_path)
    
    # get dimensions of image
    dimensions = img.shape

    # height, width, number of channels in image
    height = img.shape[0]
    width = img.shape[1]
    if width > 5000 or height > 5000 :
        image = cv2.resize(img,(int(width / 4), int(height / 4)))
    elif width < 1000 or height < 1000 :
        image = cv2.resize(img,(int(width  * 2), int(height * 2)))
    else :
        image = cv2.resize(img,(int(width  / 2), int(height / 2)))

    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    image[gray < threadValue] = (0,0,0)

    cv2.imwrite(output_path, image)

    return

@router.post(
    "/build_pyramid",
    response_description="Build Pyramid",
    status_code=status.HTTP_200_OK,
)
async def build_pyramid(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    body_bytes = await request.body()
    ashlar_params = json.loads(body_bytes)

    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )
    rel_path = tile["path"].rsplit('/static/', 1)[1]
    #rel_dir = rel_path.rsplit("/", 1)[0]

    #print(rel_dir)
    rel_dir = ashlar_params["dirname"] 
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)

    ext = tile["filename"].rsplit(".", 1)[1]
    output_temp_file_name = "ashlar_output.jpg"
    output_thumb_temp_file_name = "ashlar_output.timg"
    output_filename = "ashlar_output.ome.tiff"

    output_temp_path = os.path.join(STATIC_PATH, rel_dir, output_temp_file_name)
    output_thumb_file_path = os.path.join(STATIC_PATH, rel_dir, output_thumb_temp_file_name)
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)
    output_rel_path = os.path.join(CURRENT_STATIC, rel_dir, output_filename)

    print("Merge Params are : ")
    print(ashlar_params)
    mergeImageWithOverlap(tiles_dir,ashlar_params["height"],ashlar_params["width"],ashlar_params["layout"],ashlar_params["direction"],ashlar_params["sortOrder"],ashlar_params["overlapX"],ashlar_params["overlapY"], output_temp_path, ext)
    
    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_temp_path}' '{output_path}'"
    await shell(bfconv_cmd)
    

    img = Image.open(output_temp_path)
    img.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    img.save(output_thumb_file_path, 'png')



    shading_temp_name = "shading_output.jpg"
    shading_output_name = "shading_output.ome.tiff"
    shading_output_thumbname = "shading_output.timg"
    #For Shading Correction image


    shading_temp_path = os.path.join(STATIC_PATH, rel_dir, shading_temp_name)
    shading_thumb_file_path = os.path.join(STATIC_PATH, rel_dir, shading_output_thumbname)
    shading_path = os.path.join(STATIC_PATH, rel_dir, shading_output_name)

    doShadingCorrection(output_temp_path, shading_temp_path)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{shading_temp_path}' '{shading_path}'"
    await shell(bfconv_cmd)
    
    shading_img = Image.open(shading_temp_path)
    shading_img.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    shading_img.save(shading_thumb_file_path, 'png')


    shading_temp1_name = "shading_output1.jpg"
    shading_output1_name = "shading_output1.ome.tiff"
    shading_output1_thumbname = "shading_output1.timg"
    #For Shading Correction image


    shading_temp1_path = os.path.join(STATIC_PATH, rel_dir, shading_temp1_name)
    shading_thumb1_file_path = os.path.join(STATIC_PATH, rel_dir, shading_output1_thumbname)
    shading1_path = os.path.join(STATIC_PATH, rel_dir, shading_output1_name)

    doShadingCorrection(output_temp_path, shading_temp1_path)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{shading_temp1_path}' '{shading1_path}'"
    await shell(bfconv_cmd)
    
    shading_img1 = Image.open(shading_temp1_path)
    shading_img1.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    shading_img1.save(shading_thumb1_file_path, 'png')





    #for white and black balance

    black_temp_name = "black_output.jpg"
    black_output_name = "black_output.ome.tiff"
    black_output_thumbname = "black_output.timg"


    black_temp_path = os.path.join(STATIC_PATH, rel_dir, black_temp_name)
    black_thumb_file_path = os.path.join(STATIC_PATH, rel_dir, black_output_thumbname)
    black_path = os.path.join(STATIC_PATH, rel_dir, black_output_name)

    blackBalanceImage(output_temp_path, black_temp_path, 100)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{black_temp_path}' '{black_path}'"
    await shell(bfconv_cmd)
    
    black_img = Image.open(black_temp_path)
    black_img.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    black_img.save(black_thumb_file_path, 'png')



    # for white


    white_temp_name = "white_output.jpg"
    white_output_name = "white_output.ome.tiff"
    white_output_thumbname = "white_output.timg"


    white_temp_path = os.path.join(STATIC_PATH, rel_dir, white_temp_name)
    white_thumb_file_path = os.path.join(STATIC_PATH, rel_dir, white_output_thumbname)
    white_path = os.path.join(STATIC_PATH, rel_dir, white_output_name)

    whiteBalanceImage(output_temp_path, white_temp_path)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{white_temp_path}' '{white_path}'"
    await shell(bfconv_cmd)
    
    white_img = Image.open(white_temp_path)
    white_img.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    white_img.save(white_thumb_file_path, 'png')



    



    # For gamma




    for gamma in range(7,13):
        gamma_output_filename = "gamma" + str(gamma) +  "_output.jpg"
        gamma_thumb_filename = "gamma" + str(gamma) +  "_output.timg"
        gamma_result_filename = "gamma" + str(gamma) +  "_output.ome.tiff"
        gamma_output_path = os.path.join(STATIC_PATH, rel_dir, gamma_output_filename)
        gamma_result_path = os.path.join(STATIC_PATH, rel_dir, gamma_result_filename)
        gamma_result_thumb_path = os.path.join(STATIC_PATH, rel_dir, gamma_thumb_filename)

        adjust_gamma(output_temp_path,gamma,  gamma_output_path)


        bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{gamma_output_path}' '{gamma_result_path}'"
        await shell(bfconv_cmd)
        
        shading_img1 = Image.open(gamma_output_path)
        shading_img1.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
        shading_img1.save(gamma_result_thumb_path, 'png')


    
    return JSONResponse(output_rel_path)

########################################
# Result Tiled Image normalize
########################################


@router.post(
    "/result_tile_normalize",
    response_description="Result Tile Normalize",
    status_code=status.HTTP_200_OK,
)
async def result_tile_normalize(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
   
    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )
    rel_path = tile["path"].rsplit('/static/', 1)[1]
    rel_dir = params["dirname"] 
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)
 
    input_filename = "ashlar_output.ome.tiff"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "normalize_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)
    output_rel_path = os.path.join('/static/', rel_dir, output_filename)

    await normalizeImage(rel_dir)
    
    return JSONResponse(output_rel_path)




########################################
# Result Tiled Image correction
########################################


@router.post(
    "/result_tile_correction",
    response_description="Result Tile Correction",
    status_code=status.HTTP_200_OK,
)
async def result_tile_correct(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:

  
    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )
    rel_path = tile["path"].rsplit('/static/', 1)[1]
    rel_dir = params["dirname"] 
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)
 
    input_filename = "ashlar_output.ome.tiff"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "correction_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

    output_rel_path = os.path.join('/static/', rel_dir, output_filename)

    #Read the image

    
    img = bf.load_image(input_path)
    img = np.array(img)

    # Split the image into color channels
    channels = cv2.split(img)

    # Calculate the shading correction factor for each channel
    shading = []
    for channel in channels:
        blurred = cv2.GaussianBlur(channel, (0, 0), sigmaX=50, sigmaY=50)
        shading_channel = channel.astype(np.float32) / blurred.astype(np.float32)
        cv2.normalize(shading_channel, shading_channel, 0, 65535, cv2.NORM_MINMAX)
        shading.append(shading_channel)

    # Apply the shading correction factor to each channel
    corrected = []
    for i, channel in enumerate(channels):
        corrected_channel = np.multiply(channel.astype(np.float32), shading[i])
        corrected.append(corrected_channel)

    # Merge the corrected channels into a single image
    corrected = cv2.merge(corrected)

    pixel_type = bf.omexml.PT_UINT16
    if os.path.exists(output_path):
         os.remove(output_path)
    bf.write_image(output_path, corrected, pixel_type)

    result_path = os.path.join(STATIC_PATH, rel_dir, TILING_RESULT_IMAGE_FILE_NAME)

    # bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_path}' '{output_path}'"
    # await shell(bfconv_cmd)

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_path}' '{result_path}'"
    await shell(bfconv_cmd)
    


    return JSONResponse(output_rel_path)







#################################################
#  Result Tiled Image Gamma Operation
##################################################


@router.post(
    "/result_tile_gamma",
    response_description="Result Tile Gamma Operation",
    status_code=status.HTTP_200_OK,
)
async def result_tile_normalize(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:

    body_bytes = await request.body()
    params = json.loads(body_bytes)
    
    
    gamma = params['gamma']

    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )
    rel_path = tile["path"].rsplit('/static/', 1)[1]
    rel_dir = rel_path.rsplit("/", 1)[0]
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)
 
    input_filename = "ashlar_output.jpg"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "gamma" + str(gamma) +  "_output.jpg"
    output_thumb_filename = "gamma" + str(gamma) +  "_output.timg"
    result_filename = "gamma" + str(gamma) +  "_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)
    result_path = os.path.join(STATIC_PATH, rel_dir, result_filename)
    result_thumb_path = os.path.join(STATIC_PATH, rel_dir, output_thumb_filename)

    output_rel_path = os.path.join('/static/', rel_dir, result_filename)

    #Read the image
    image = bf.load_image(input_path)
    image = np.array(image) * 255
   
    image = image.astype('uint8')

    table = np.array([((i / 255.0) ** gamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
	# apply gamma correction using the lookup table
    final_image =  cv2.LUT(image, table)

    # Write the image

    if os.path.exists(output_path):
         os.remove(output_path)


    
    if os.path.exists(result_path):
         os.remove(result_path)
    

    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_path}' '{result_path}'"
    await shell(bfconv_cmd)



    gammaImage = Image.open(shading_temp1_path)
    gammaImage.thumbnail([100 * int(ashlar_params["width"]), 100 * int(ashlar_params["height"])])
    gammaImage.save(result_thumb_path, 'png')

    
    return JSONResponse(output_rel_path)





#################################################
#  Result Tiled Image BestFit Operation
##################################################


@router.post(
    "/result_tile_bestfit",
    response_description="Result Tile BestFit Operation",
    status_code=status.HTTP_200_OK,
)
async def result_tile_bestfit(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:

    
    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )

    rel_path = tile["path"].rsplit('/static/', 1)[1]
    rel_dir = rel_path.rsplit("/", 1)[0]
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)
 
    input_filename = "ashlar_output.ome.tiff"
    input_path = os.path.join(STATIC_PATH, rel_dir, input_filename)

    output_filename = "temp_output.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

    output_rel_path = os.path.join('/static/', rel_dir, output_filename)

    #Read the image
    image = bf.load_image(input_path)
    image = np.array(image)

    # Increase gamma 
    #image_data = image.get_image_data()
    

    # Write the image

    if os.path.exists(output_path):
         os.remove(output_path)

    bf.write_image(output_path, pixels=image, pixel_type='uint16')

    result_path = os.path.join(STATIC_PATH, rel_dir, TILING_RESULT_IMAGE_FILE_NAME)
    
    if os.path.exists(result_path):
         os.remove(result_path)
    shutil.copy(output_path, result_path)

    
    return JSONResponse(output_rel_path)



#################################################
#  Result Tiled Image Snap To Edge Functions
##################################################


@router.post(
    "/result_tile_snap_to_edge",
    response_description="Result Tile Snap To Edge Operation",
    status_code=status.HTTP_200_OK,
)
async def result_tile_snap_to_edge(
    request: Request,
    user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:

    
    tile = await db["tile-image-cache"].find_one(
        {"user_id": user.id}
    )
    rel_path = tile["path"].rsplit('/static/', 1)[1]
    rel_dir = rel_path.rsplit("/", 1)[0]
    tiles_dir = os.path.join(STATIC_PATH, rel_dir)


    # Load the images
    file_list = glob.glob(tiles_dir + "/*.timg")



    images = []
    
    for file in file_list:
        image = cv2.imread(file)
        images.append(image)

    
    # Define the number of images and the size of the output image
    num_images = len(images)
    output_size = (num_images * images[0].shape[1], images[0].shape[0])

    # Create an empty output image
    output = np.zeros((output_size[1], output_size[0], 3), dtype=np.uint8)

    # Loop through the images and align them
    for i in range(num_images):
        # Detect the edges of the current image
        edges1 = cv2.Canny(images[i], 100, 200)
        
        if i == 0:
            # For the first image, just copy it to the output
            output[0:images[i].shape[0], 0:images[i].shape[1], :] = images[i]
        else:
            # Detect the edges of the previous image
            edges2 = cv2.Canny(images[i-1], 100, 200)
            
            # Find the matching edges and align the images
            result = cv2.matchTemplate(edges1, edges2, cv2.TM_CCOEFF_NORMED)
            _, _, _, max_loc = cv2.minMaxLoc(result)
            h, w = images[i].shape[:2]
            aligned_img = images[i][:, max_loc[0]:max_loc[0]+w, :]
            
            # Copy the aligned image to the output
            output[0:images[i].shape[0], i*w:(i+1)*w, :] = aligned_img
    
    output = cv2.GaussianBlur(output, (3, 3), 0)



    temp_name = "temp_output.jpg"
    temp_output = os.path.join(STATIC_PATH, rel_dir, temp_name)

    temp_output_path = os.path.join('/static/', rel_dir, temp_name)

    # Save the output image
    cv2.imwrite(temp_output, output)



    output_filename = "snap_to_edge.ome.tiff"
    output_path = os.path.join(STATIC_PATH, rel_dir, output_filename)

    output_rel_path = os.path.join('/static/', rel_dir, output_filename)


    bfconv_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{temp_output}' '{output_path}'"
    await shell(bfconv_cmd)

    return JSONResponse(output_rel_path)






#############################################################################
# Register Experiment
#############################################################################
@router.post(
    "/register_experiment",
    response_description="Register Experiment",
    status_code=status.HTTP_201_CREATED,
    response_model=List[ExperimentModel],
)
async def register_experiment(
    request: Request,
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    print(request)
    data = await request.form()

    files = data.get("images").split(",")
    experiment_name = data.get("experiment_name")
    result = await add_experiment(
        experiment_name,
        files,
        clear_previous=clear_previous,
        current_user=current_user,
        db=db,
    )
    return JSONResponse({"success": result})


#############################################################################
# Get Experiment data by name
#############################################################################
@router.get(
    "/get_experiment_data/{experiment_name}",
    response_description="Get Experiment Data",
    response_model=List[ExperimentModel],
)
async def get_image(
    experiment_name: str,
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    # tiles = await db['experiment'].find({'experiment_name': "experiment_1"})
    # all_tiles = [doc async for doc in db['experiment'].find()]
    # print(all_tiles)

    tiles = [
        doc
        async for doc in db["experiment"].find(
            {"experiment_name": experiment_name, "user_id": current_user.id}
        )
    ]
    # print(tiles)
    if len(tiles) == 0:
        return JSONResponse(
            {"success": False, "error": "Cannot find the experiment data"}
        )

    experiment = tiles[0]
    files = experiment["fileNames"]

    metadatas = []
    for file in files:
        metadata = get_metadata(file)
        print("get_experiment_data:", file, metadata)
        metadatas.append(metadata)

    return JSONResponse({"success": True, "data": files, "metadata": metadatas})


#############################################################################
# Get Experiment names
#############################################################################
@router.get(
    "/get_experiment_names",
    response_description="Get Experiment names",
    response_model=List[str],
)
async def get_image(
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[str]:
    experiment_names = [
        doc["experiment_name"]
        async for doc in db["experiment"].find({"user_id": current_user.id})
    ]
    print(experiment_names)
    return JSONResponse({"success": True, "data": experiment_names})


#############################################################################
# Get Experiment datas
#############################################################################
@router.get(
    "/get_experiments_datas",
    response_description="Get Experiments",
    response_model=List[ExperimentModel],
)
async def get_experiments(
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    userId = str(PyObjectId(current_user.id))
    exp_datas = [
        doc
        async for doc in db["experiment"].find(
            {"user_id": userId}, {"_id": 0, "update_time": 0}
        )
    ]

    return JSONResponse({"success": True, "data": exp_datas})


#############################################################################
# Get Meta datas
#############################################################################
@router.get(
    "/get_meta_datas",
    response_description="Get Metadatas",
    response_model=List[MetadataModel],
)
async def get_metadatas(
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[MetadataModel]:
    userId = str(PyObjectId(current_user.id))
    meta_datas = [doc async for doc in db["metadata"].find({}, {"_id": 0})]
    return JSONResponse({"success": True, "data": meta_datas})


#############################################################################
# Get Image By its full path
#############################################################################
@router.post(
    "/get_image_by_path",
    response_description="Get Image By its full path",
    response_model=List[TileModelDB],
)
async def merge_image(
    merge_req_body: str = Body(embed=True),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[TileModelDB]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    imagePath = merge_req_body

    return FileResponse(imagePath, media_type="image/tiff")


#############################################################################
# New Upload Image file


@router.post(
    "/upload_images/{folder_name}",
    response_description="Upload Files",
    status_code=status.HTTP_201_CREATED,
    response_model=List[FileModelDB],
)
async def upload_images(
    folder_name: str,
    files: List[UploadFile] = File(...),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # Make user directory
    if not os.path.exists(current_user_path):
        os.makedirs(current_user_path)

    # Check if the folder exists, if not make a new one
    path = os.path.join(current_user_path, folder_name)
    print(path)
    if os.path.isdir(path):
        result = {}
        result["error"] = "Folder is already existing"
        return JSONResponse(result)
    else:
        os.mkdir(path)
        res = await db["tile-image-cache"].delete_many(
            {"user_id": PyObjectId(current_user.id)}
        )
        result = await add_image_tiles(
            path=path,
            files=files,
            clear_previous=clear_previous,
            current_user=current_user,
            db=db,
        )
        result["path"] = os.path.join(
            CURRENT_STATIC, str(PyObjectId(current_user.id)) + "/" + folder_name
        )

    return JSONResponse(result)


#############################################################################
# New Upload Experiment with Folder


@router.post(
    "/set_experiment",
    response_description="Register Experiment",
    status_code=status.HTTP_201_CREATED,
    response_model=List[ExperimentModel],
)
async def register_experiment_with_folder(
    request: Request,
    files: List[UploadFile] = File(...),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    data = await request.form()
    # files = data.get('images')
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    new_experiment_path = os.path.join(current_user_path, data.get("experiment_name"))
    new_folder_path = os.path.join(new_experiment_path, data.get("folderName"))

    print("This is user path", current_user_path)
    print("This is experiment path", new_experiment_path)
    print("This is folder path", new_folder_path)
    # Make user directory
    if not os.path.exists(current_user_path):
        os.makedirs(current_user_path)

    # # Check if the folder exists, if not make a new one
    # path = os.path.join(current_user_path, folder_name)
    # print(path)
    if os.path.isdir(new_experiment_path):
        result = {}
        result["exp_error"] = "Experiment name is already exist"
        return JSONResponse(result)
    else:
        os.mkdir(new_experiment_path)

    if os.path.isdir(new_folder_path):
        result = {}
        result["folder_error"] = "Folder name is already exist"
        return JSONResponse(result)
    else:
        os.mkdir(new_folder_path)
        # res = await db['tile-image-cache'].delete_many({"user_id": PyObjectId(current_user.id)})
        result = await add_experiment_with_folder(
            folderPath=new_experiment_path,
            experiment_name=data.get("experiment_name"),
            folderName=data.get("folderName"),
            files=files,
            clear_previous=clear_previous,
            current_user=current_user,
            db=db,
        )
    #     result["path"] = os.path.join(CURRENT_STATIC, str(PyObjectId(current_user.id)) + "/" + folder_name)

    return JSONResponse(result)


@router.post(
    "/set_experiment_with_files",
    response_description="Register Experiment with Files",
    status_code=status.HTTP_201_CREATED,
    response_model=List[ExperimentModel],
)
async def register_experiment_with_folder(
    request: Request,
    files: List[UploadFile] = File(...),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    data = await request.form()
    # files = data.get('images')
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    new_experiment_path = os.path.join(current_user_path, data.get("experiment_name"))

    print("This is user path", current_user_path)
    print("This is experiment path", new_experiment_path)
    # Make user directory
    if not os.path.exists(current_user_path):
        os.makedirs(current_user_path)

    if os.path.isdir(new_experiment_path):
        result = {}
        result["error"] = "Experiment name is already exist"
        return JSONResponse(result)
    else:
        os.mkdir(new_experiment_path)
        result = await add_experiment_with_files(
            folderPath=new_experiment_path,
            experiment_name=data.get("experiment_name"),
            files=files,
            clear_previous=clear_previous,
            current_user=current_user,
            db=db,
        )
    #     result["path"] = os.path.join(CURRENT_STATIC, str(PyObjectId(current_user.id)) + "/" + folder_name)

    return JSONResponse(result)


#############################################################################
## Set Experiment with folders
@router.post(
    "/set_experiment_with_folders",
    response_description="Register with Folder",
    status_code=status.HTTP_201_CREATED,
    response_model=List[ExperimentModel],
)
async def register_experiment_with_folders(
    request: Request,
    files: List[UploadFile] = File(...),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[ExperimentModel]:
    data = await request.form()
    experiment_name = data.get("experiment_name")
    paths = data.get("path")
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    new_experiment_path = os.path.join(current_user_path, experiment_name)

    if not os.path.exists(current_user_path):
        os.makedirs(current_user_path)

    if not os.path.isdir(new_experiment_path):
        os.mkdir(new_experiment_path)

    result = await add_experiment_with_folders(
        folderPath=new_experiment_path,
        experiment_name=experiment_name,
        files=files,
        paths=paths,
        current_user=current_user,
        db=db,
        tiling=data.get("tiling"),
    )

    return JSONResponse(result)


# Return one Image file
@router.get(
    "/get_image/{folder}/{image}",
    response_description="Get Image Tiles",
    response_model=List[TileModelDB],
)
async def get_image(
    image: str,
    folder: str,
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[TileModelDB]:
    print("image get", folder, image)
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    metadata_for_single_img = get_metadata(
        os.path.join(current_user_path + "/" + folder, image)
    )
    print("This metadata for single data--------", metadata_for_single_img)
    return FileResponse(
        os.path.join(current_user_path + "/" + folder, image), media_type="image/tiff"
    )


# Return Image tree
@router.get(
    "/get_image_tree",
    response_description="Get Image Tiles",
    response_model=List[FileModelDB],
)
async def get_image(
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[FileModelDB]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))

    if os.path.isdir(current_user_path) == False:
        os.mkdir(current_user_path)
        return JSONResponse({"error": "You have no image data, please upload"})

    sub_dirs = os.listdir(current_user_path)
    if len(sub_dirs) == 0:
        return JSONResponse({"error": "You have no image data, please upload"})

    output = [
        dI
        for dI in os.listdir(current_user_path)
        if os.path.isdir(os.path.join(current_user_path, dI))
    ]
    response = []

    for folderName in output:
        current_folder = os.path.join(current_user_path, folderName)
        files = [
            {"value": os.path.join(current_folder, f), "label": f}
            for f in os.listdir(current_folder)
            if os.path.isfile(os.path.join(current_folder, f))
        ]
        response.append(
            {"value": current_folder, "label": folderName, "children": files}
        )

    return JSONResponse({"data": response})


# Return merge Image files
@router.post(
    "/get_merged_image",
    response_description="Get Image Tiles",
    response_model=List[TileModelDB],
)
async def merge_image(
    merge_req_body: str = Body(embed=True),
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[TileModelDB]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    reqbody = merge_req_body.split("&")

    images = reqbody[0].split(",")
    newImageName = reqbody[1]

    tff_lst = [os.path.join(current_user_path + "/", image) for image in images]
    print("requested image list:\n", tff_lst)
    if len(tff_lst) > 0 and os.path.isfile(tff_lst[0]):
        if os.path.isfile(os.path.join(current_user_path + "/", newImageName)):
            return FileResponse(
                os.path.join(current_user_path + "/", newImageName),
                media_type="image/tiff",
            )
        tff = tifftools.read_tiff(tff_lst[0])
        for other in tff_lst[1:]:
            if os.path.isfile(other):
                othertff = tifftools.read_tiff(other)
                tff["ifds"].extend(othertff["ifds"])
        tifftools.write_tiff(tff, os.path.join(current_user_path + "/", newImageName))
        return FileResponse(
            os.path.join(current_user_path + "/", newImageName), media_type="image/tiff"
        )
    else:
        return JSONResponse({"error": "Requested images are not exist!"})
    # return JSONResponse({"aa": tff_lst[0], "bb": newImageName})
    # return JSONResponse({"aa": merge_req_body.fileNames, 'bb': merge_req_body.newImageName})


# Alignment tilings
# @router.get(
#     "/list",
#     response_description="Upload Image Tiles",
#     response_model=List[TileModelDB],
#     status_code=status.HTTP_200_OK,
# )
# async def get_tile_list(
#     current_user: UserModelDB = Depends(get_current_user),
#     db: AsyncIOMotorDatabase = Depends(get_database),
# ) -> List[TileModelDB]:
#     print(current_user, "tiles -----------")
#     tiles = []


#     id = 1
    
#     async for doc in  db["tile-image-cache"].find({"user_id": current_user.id}):
#         tiles.append({ 
#             "id" : doc["_id"],
#             "absolute_path" :  doc["path"],
#              "file_name": doc["filename"],
#              "content_type": "image/jpeg",  # MIME type
#              "width_px":  320,
#              "height_px" : 180
#              })

#         id = id + 1


    
#     return pydantic.parse_obj_as(List[TileModelDB], tiles)


# @router.get(
#     "/align_tiles_naive",
#     response_description="Align Tiles",
#     response_model=List[AlignedTiledModel],
#     status_code=status.HTTP_200_OK,
# )
# async def _align_tiles_naive(
#     request: AlignNaiveRequest, tiles: List[TileModelDB] = Depends(get_tile_list)
# ) -> any:
#     """
#     performs a naive aligning of the tiles simply based on the given rows and method.
#     does not perform any advanced stitching or pixel checking

#     Called using concurrent.futures to make it async
#     """
#     print(tiles, " : align_tiles_naive : ----------------------------")
#     loop = asyncio.get_event_loop()
#     with concurrent.futures.ProcessPoolExecutor() as pool:
#         # await result
#         aligned_tiles = await loop.run_in_executor(
#             pool, align_tiles_naive, request, tiles
#         )
#         return aligned_tiles


# @router.get("/align_tiles_ashlar",
#             response_description="Align Tiles",
#             # response_model=List[AlignedTiledModel],
#             status_code=status.HTTP_200_OK)
# async def _align_tiles_ashlar(tiles: List[TileModelDB] = Depends(get_tile_list)) -> any:
#     """
#         performs a naive aligning of the tiles simply based on the given rows and method.
#         does not perform any advanced stitching or pixel checking

#         Called using concurrent.futures to make it async
#     """

#     loop = asyncio.get_event_loop()
#     with concurrent.futures.ProcessPoolExecutor() as pool:
#         # await result
#         aligned_tiles = await loop.run_in_executor(pool, align_ashlar, tiles, "img_r{row:03}_c{col:03}.tif")
#         return aligned_tiles


# Update Name and File - Name&&File Functions
@router.post(
    "/update",
    response_description="Update Image Tiles With Name",
    status_code=status.HTTP_200_OK,
)
async def update(
    tiles: List[NamePattenModel],
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
):
    # make sure we are not trying to alter any tiles we do not own
    # we check this first and if they are trying to update any un owned docs we dont update any
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    current_files = []
    current_row = 0
    current_col = 1
    for tile in tiles:
        additional_tile = {
            "file_name": tile.filename,
            "series": tile.series,
            "row_index": tile.row,
            "column_index": tile.col,
            "channel": tile.channel,
            "field": tile.field,
            "z_position": tile.z_position,
            "time_point": tile.time_point,
        }
        await db["tile-image-cache"].update_one(
            {"file_name": tile.filename}, {"$set": additional_tile}
        )


# View Controls
@router.post(
    "/deconvol2D",
    response_description="Convolution about 2D image",
    status_code=status.HTTP_201_CREATED,
    response_model=List[TileModelDB],
)
async def upload_image_name(
    files_name: str = Form(""),
    effectiveness: int = Form(1),
    isroi: bool = Form(False),
    roiPoints: object = Form(...),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[TileModelDB]:
    files_name = files_name.split("/")[-1]
    dictRoiPts = jsons.loads(roiPoints)
    abs_path = Deconv.SupervisedColorDeconvolution(
        files_name, effectiveness, isroi, dictRoiPts
    )
    abs_path = abs_path.split("/")[-1]
    path = []
    path.append(abs_path)
    result = {"Flag_3d": False, "N_images": 1, "path_images": path}
    return JSONResponse(result)


@router.post(
    "/deconvol3D",
    response_description="Deconvolution about 3D image",
    status_code=status.HTTP_201_CREATED,
    response_model=List[TileModelDB],
)
async def deconvol3D(
    gamma: float = Form(1.0),
    file_name: str = "",
    effectiveness: int = Form(1),
    isroi: bool = Form(False),
    roiPoints: object = Form(...),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[TileModelDB]:
    dictRoiPts = jsons.loads(roiPoints)
    file_path = Deconv.RechardDeconvolution3d(
        file_name, effectiveness, isroi, dictRoiPts, gamma
    )
    cal = await add_image_tiles(
        path=file_path,
        files=File(...),
        clear_previous=Form(False),
        current_user=current_user,
        db=db,
    )
    result = {"Flag_3d": cal[0], "N_images": cal[1], "path_images": cal[2]}
    return JSONResponse(result)


@router.get(
    "/super-resolution/{experiment}/{filename}/{scale}",
    response_description="image super resolution",
    status_code=status.HTTP_201_CREATED,
    response_model=List[TileModelDB],
)
async def GetSuperResolution(
    experiment: str,
    filename: str,
    scale: int,
    user: UserModelDB = Depends(get_current_user),
) -> List[TileModelDB]:
    filepath = os.path.join(STATIC_PATH, str(user.id), experiment, filename, scale)
    out_filepath = SuperResolution.EDSuperResolution(filepath)
    rel_path = out_filepath.rsplit(str(STATIC_PATH), 1)[1]

    return JSONResponse({"result": rel_path})


@router.post(
    "/delete", response_description="Update Image Tiles", status_code=status.HTTP_200_OK
)
async def delete_tiles(
    tiles: List[TileModelDB],
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
):
    # make sure we are not trying to delete any tiles we do not own
    # we check this first and if they are trying to delete any un owned docs we dont update any
    for tile in tiles:
        if tile.user_id != current_user.id:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Cannot update tile that does not belong to user",
                headers={"WWW-Authenticate": "Bearer"},
            )
    results = []
    for tile in tiles:
        result = await db["tile-image-cache"].delete_one({"_id": tile.id})
        results.append(result)
    return results


@router.get(
    "/export_stitched_image",
    response_description="Export stitched Image",
    response_model=List[AlignedTiledModel],
    status_code=status.HTTP_200_OK,
)
# async def export_stitched_image(tiles: List[AlignedTiledModel]) -> List[TileModel]:
async def export_stitched_image() -> List[TileModelDB]:
    """This is meant to called after the images are aligned, so it takes a list of AlignedTiledModel in the body"""
    pass
    # loop = asyncio.get_event_loop()
    # with concurrent.futures.ProcessPoolExecutor() as pool:
    #     result = await loop.run_in_executor(pool, cpu_bound)  # wait result
    #     print(result)


#############################################################################
# Get Image Raw Data
#############################################################################
@router.get(
    "/get_channel_states/{concatedName}", response_description="Get Image Raw Data"
)
async def get_image_raw_data(
    concatedName: str,
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
):
    # experiment_names = [doc['experiment_name'] async for doc in db['experiment'].find()]
    # if len(experiment_names) <= 0:
    #     return JSONResponse({"success": False, "error": "Cannot find the experiment name"})
    # experiment_name = experiment_names[0]

    pos = concatedName.find("&")
    experiment_name = concatedName[(pos + 1 - len(concatedName)) :]
    imageName = concatedName[0:pos]
    print("get_image_raw_data: ", concatedName, experiment_name, imageName)

    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    image_path = os.path.join(current_user_path + "/", experiment_name + "/", imageName)
    print("get_image_raw_data: ", image_path)
    if not os.path.isfile(image_path):
        return JSONResponse({"success": False, "error": "Cannot find the image"})

    # logback.basic_config()
    # image, scale = bioformats.load_image(image_path, rescale=False, wants_max_intensity=True)

    # raw_data = []
    # for r in range(0, image.shape[0]):
    #     for c in range(0, image.shape[1]):
    #         raw_data.append(image[r][c])

    # return StreamingResponse(io.BytesIO(image.tobytes()), media_type="image/raw")

    domain, contrastLimits = calculateImageStats(image_path)
    return JSONResponse(
        {
            "success": True,
            "domain": [int(domain[0]), int(domain[1])],
            "contrastLimits": [int(contrastLimits[0]), int(contrastLimits[1])],
        }
    )


# Get focus stacked images
@router.post(
    "/focus-stack",
    response_description="Get focus stacked images",
    status_code=status.HTTP_200_OK,
    response_model=List[TileModelDB],
)
async def get_focus_stacked(
    imageFiles: List[UploadFile] = File(...),
) -> List[TileModelDB]:
    tmp_uuid = str(uuid.uuid4())
    tmp_path = os.path.join(STATIC_PATH, "tmp", tmp_uuid)
    os.makedirs(tmp_path)

    input_path = os.path.join(tmp_path, "input")
    os.makedirs(input_path)

    output_path = os.path.join(tmp_path, "output")
    os.makedirs(output_path)

    for imageFile in imageFiles:
        imagePath = os.path.join(input_path, imageFile.filename)
        async with aiofiles.open(imagePath, "wb") as f:
            imageData = await imageFile.read()
            await f.write(imageData)

    image_files = sorted(os.listdir(input_path))
    for img in image_files:
        if img.split(".")[-1].lower() not in ["jpg", "jpeg", "png"]:
            image_files.remove(img)

    focusimages = []
    for img in image_files:
        print("Reading in file {}".format(img))
        focusimages.append(
            cv2.imread("{input}/{file}".format(input=input_path, file=img))
        )

    output_file_path = os.path.join(output_path, "merged.png")
    merged = focus_stack(focusimages)
    cv2.imwrite(output_file_path, merged)

    return JSONResponse({"result": "static/tmp/{}/output/merged.png".format(tmp_uuid)})

@router.post("/dl_basic_segment",
             response_description="DL Basic Segment",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def dlBasicSegment(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")

    # parameter control
    custom_method = 'tissuenet'
    cell_diam = 30
    chan_segment = 0
    chan_2 = 0
    f_threshold = 0.4
    c_threshold = 0.0
    s_threshold = 0.0
    #Get file's full abs path
    temp_url = file_url.split('download/?path=')
    print('file_url', file_url)

    exp_path = '/app/mainApi/app/static/' + temp_url[1]
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    file_name_array = file_full_name.split(".")
    file_name = ""
    file_name_length = len(file_name_array)
    for i in range(file_name_length - 1):
        if(i == 0):
            file_name = file_name + file_name_array[i]
        if(i>0):
            file_name = file_name + '.' + file_name_array[i]
    print('file_name', file_name)
    # Run cellpose and test cell segmnent
    command_string = "python -m cellpose --image_path \"{file_full_path}\" --pretrained_model {custom_method} --chan {chan_segment} --chan2 {chan_2} --diameter {cell_diam} --stitch_threshold {s_threshold} --flow_threshold {f_threshold} --cellprob_threshold {c_threshold} --fast_mode  --save_png  --save_flows --save_outlines --save_ncolor --verbose".format(file_full_path=exp_path, custom_method=custom_method, chan_segment=chan_segment, chan_2=chan_2, cell_diam=cell_diam, s_threshold=s_threshold, f_threshold=f_threshold, c_threshold=c_threshold)
    print("my_command", command_string)
    os.system(command_string)
    model = {'outline': 1}
    result = await convert_npy_to_jpg(file_full_path=make_new_folder,clear_previous=clear_previous , model_info = model,file_name=file_name, current_user=current_user)
    delete_junk_data(file_url, make_new_folder)
    return JSONResponse({
        "success": result["img_path"],
        "zip_path": result["zip_path"],
        "csv_path": result["csv_path"],
        "count_path": result["count_path"]
    })

@router.post("/dl_test_segment",
             response_description="DL Basic Segment",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def dlTestSegment(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")

    # parameter control
    custom_method = 'tissuenet'
    cell_diam = data.get("cell_diam")
    chan_segment = data.get("chan_segment")
    chan_2 = data.get("chan_2")
    f_threshold = data.get("f_threshold")
    c_threshold = data.get("c_threshold")
    s_threshold = data.get("s_threshold")
    outline = data.get("outline")

    #Get file's full abs path
    temp_url = file_url.split('download/?path=')
    print('file_url', file_url)

    exp_path = '/app/mainApi/app/static/' + temp_url[1]
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    file_name_array = file_full_name.split(".")
    file_name = ""
    file_name_length = len(file_name_array)
    for i in range(file_name_length - 1):
        if(i == 0):
            file_name = file_name + file_name_array[i]
        if(i>0):
            file_name = file_name + '.' + file_name_array[i]
    print('file_name', file_name)
    # Run cellpose and test cell segmnent
    command_string = "python -m cellpose --image_path \"{file_full_path}\" --pretrained_model {custom_method} --chan {chan_segment} --chan2 {chan_2} --diameter {cell_diam} --stitch_threshold {s_threshold} --flow_threshold {f_threshold} --cellprob_threshold {c_threshold} --fast_mode  --save_png  --save_flows --save_outlines --save_ncolor --verbose".format(file_full_path=exp_path, custom_method=custom_method, chan_segment=chan_segment, chan_2=chan_2, cell_diam=cell_diam, s_threshold=s_threshold, f_threshold=f_threshold, c_threshold=c_threshold)
    print("my_command", command_string)
    os.system(command_string)
    model = {'outline': int(outline)}

    print(make_new_folder)
    print(file_name)
    
    result = await convert_npy_to_jpg(file_full_path=make_new_folder,clear_previous=clear_previous , model_info = model,file_name=file_name, current_user=current_user)
    
    delete_junk_data(file_url, make_new_folder)

    print("*" * 30)
    print(result)


    return JSONResponse({
        "success": result["img_path"],
        "zip_path": result["zip_path"],
        "csv_path": result["csv_path"],
        "count_path": result["count_path"]
    })


@router.post("/test_segment",
             response_description="Test Segment",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def test_segment(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")
    model_name = data.get("model_name")
    model = await get_model(model_name=model_name,clear_previous=clear_previous , current_user=current_user, db=db)
    print('my_model', model[0]['custom_name'])
    # parameter control
    custom_method = model[0]['custom_method']
    viewValue = model[0]['viewValue']
    outline = model[0]['outline']
    cell_diam = model[0]['cell_diam']
    chan_segment = model[0]['chan_segment']
    chan_2 = model[0]['chan_2']
    f_threshold = model[0]['f_threshold']
    c_threshold = model[0]['c_threshold']
    s_threshold = model[0]['s_threshold']
    #Get file's full abs path
    temp_url = file_url.split('download/?path=')
    print('file_url', file_url)

    exp_path = '/app/mainApi/app/static/' + temp_url[1]
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    file_name_array = file_full_name.split(".")
    file_name = ""
    file_name_length = len(file_name_array)
    for i in range(file_name_length - 1):
        if(i == 0):
            file_name = file_name + file_name_array[i]
        if(i>0):
            file_name = file_name + '.' + file_name_array[i]
    print('file_name', file_name)
    # Run cellpose and test cell segmnent
    command_string = "python -m cellpose --image_path \"{file_full_path}\" --pretrained_model {custom_method} --chan {chan_segment} --chan2 {chan_2} --diameter {cell_diam} --stitch_threshold {s_threshold} --flow_threshold {f_threshold} --cellprob_threshold {c_threshold} --fast_mode  --save_png  --save_flows --save_outlines --save_ncolor --verbose".format(file_full_path=exp_path, custom_method=custom_method, chan_segment=chan_segment, chan_2=chan_2, cell_diam=cell_diam, s_threshold=s_threshold, f_threshold=f_threshold, c_threshold=c_threshold)
    print("my_command", command_string)
    os.system(command_string)
    result = await convert_npy_to_jpg(file_full_path=make_new_folder,clear_previous=clear_previous , model_info = model[0],file_name=file_name, current_user=current_user)
    delete_junk_data(file_url, make_new_folder)
    return JSONResponse({
        "success": result["img_path"],
        "zip_path": result["zip_path"],
        "csv_path": result["csv_path"],
        "count_path": result["count_path"]
    })

@router.post("/get_mask_path",
             response_description="Get Mask Path",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def get_mask_path(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")
    final_temp = file_url
    #Get file's full abs path]
    file_url = file_url.replace('download/?path=', '')
    temp_url = file_url.split('/')
    temp_length = len(temp_url)
    file_url = temp_url[temp_length-2] + '/' + temp_url[temp_length-1]
    print('file_url', file_url)
    exp_path = os.path.join(current_user_path, file_url)
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    print('file_name', file_full_name)
    print('file_folder', make_new_folder)
    origin_file = ""
    if "_conv_outlines.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_outlines.ome.tiff')
        file_full_name = file_temp[0]
        origin_file = file_temp[0]
    if "_conv_masks.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_masks.ome.tiff')
        file_full_name = file_temp[0]
        origin_file = file_temp[0]
    if "_conv_flows.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_flows.ome.tiff')
        file_full_name = file_temp[0]
        origin_file = file_temp[0]
    if ".ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome.tiff')
        file_full_name = file_temp[0]
        origin_file = file_temp[0]
    print('origin_file', origin_file)
    host_url = final_temp.split("image/download/?path=")[0]
    mask_full_path = host_url + "static/" + str(PyObjectId(current_user.id)) + '/' + file_url.split('/')[0] + '/' + origin_file + ".ome_mask.png"
    print("mask_path", mask_full_path)
    return JSONResponse({"success": mask_full_path})

def delete_junk_data(file_name, 
            dir_name,
            current_user: UserModelDB = Depends(get_current_user),
            db: AsyncIOMotorDatabase = Depends(get_database)
)-> List[UserCustomModel]:
    origin_name = file_name.split('.ome.tiff')[0]
    origin_name = origin_name.split('/')[1]
    #delete segmentation files 
    seg_output = origin_name + '.ome_cp_output.png'
    print('seg_output', dir_name + seg_output)
    if(os.path.isfile(dir_name + seg_output)) :
        os.unlink(dir_name + seg_output)
    seg_dp = dir_name + origin_name + '.ome_dP.tif'
    if(os.path.isfile(seg_dp)) :
        os.unlink(seg_dp)
    seg_flow = dir_name + origin_name + '.ome_flows.tif'
    if(os.path.isfile(seg_flow)) :
        os.unlink(seg_flow)
    seg_input_mask = dir_name + origin_name + '.ome_mask.jpg'
    if(os.path.isfile(seg_input_mask)) :
        os.unlink(seg_input_mask)
    seg_res_mask = dir_name + origin_name + '.ome_conv_masks.jpg'
    if(os.path.isfile(seg_res_mask)) :
        os.unlink(seg_res_mask)
    seg_outline = dir_name + origin_name + '.ome_outlines.png'
    if(os.path.isfile(seg_outline)) :
        os.unlink(seg_outline)
    #delete training files
    dir_name = dir_name + 'train/'
    train_img = dir_name + origin_name + '_img.tiff'
    if(os.path.isfile(train_img)) :
        os.unlink(train_img)
    train_mask_flow = dir_name + origin_name + '_img_flows.tif'
    if(os.path.isfile(train_mask_flow)) :
        os.unlink(train_mask_flow)
    train_mask = dir_name + origin_name + '_masks.tiff'
    if(os.path.isfile(train_mask)) :
        os.unlink(train_mask)

@router.post(
    "/save_model",
    response_description="Save Model",
    status_code=status.HTTP_201_CREATED,
    response_model=List[UserCustomModel],
)
async def save_model(
    request: Request,
    clear_previous: bool = Form(False),
    current_user: UserModelDB = Depends(get_current_user),
    db: AsyncIOMotorDatabase = Depends(get_database),
) -> List[UserCustomModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    data = await request.form()
    custom_name = data.get("custom_name")
    usercustom = UserCustomModel(
        user_id=PyObjectId(current_user.id),
        custom_method=data.get("custom_method"),
        custom_name=data.get("custom_name"),
        custom_icon=data.get("custom_icon"),
        viewValue=data.get("viewValue"),
        outline=data.get("outline"),
        cell_diam=data.get("cell_diam"),
        chan_segment=data.get("chan_segment"),
        chan_2=data.get("chan_2"),
        f_threshold=data.get("f_threshold"),
        c_threshold=data.get("c_threshold"),
        s_threshold=data.get("s_threshold"),
    )
    print("model-name", usercustom)
    # print('model_info', dir(usercustom))
    models = [
        doc
        async for doc in db["usercustom"].find(
            {"custom_name": custom_name, "user_id": current_user.id}
        )
    ]
    if len(models) > 0:
        return JSONResponse({"error": "NO"})
    else:
        await db["usercustom"].insert_one(usercustom.dict(exclude={"id"}))
    return JSONResponse({"success": "OK"})

@router.post("/get_models",
             response_description="Get Model",
             status_code=status.HTTP_201_CREATED,
             response_model=List[UserCustomModel])
async def get_models(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[UserCustomModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    data = await request.form()
    model = 'all'
    models = []
    if model == 'all' :
        models = [doc async for doc in
                db['usercustom'].find({'user_id': current_user.id}, {'_id': 0, 'update_time': 0})]
    else :
        models = [doc async for doc in
             db['usercustom'].find({'custom_name': model, 'user_id': current_user.id}, {'_id': 0, 'update_time': 0})]
    for mo in models :
        mo['user_id'] = ''
    print('models', models)
    if len(models) == 0:
        return JSONResponse({"error": "NO"})
    return JSONResponse({"success": True, "data": models})

@router.post("/get_outlines",
             response_description="Get outlines",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def get_outlines(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")
    #Get file's full abs path
    mask_temp = file_url
    file_url = file_url.replace('download/?path=', '')
    temp_url = file_url.split('/')
    temp_length = len(temp_url)
    file_url = temp_url[temp_length-2] + '/' + temp_url[temp_length-1]
    print('file_url', file_url)
    exp_path = os.path.join(current_user_path, file_url)
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    file_name_array = file_full_name.split(".")
    file_name = ""
    file_name_length = len(file_name_array)
    for i in range(file_name_length - 1):
        if(i == 0):
            file_name = file_name + file_name_array[i]
        if(i>0):
            file_name = file_name + '.' + file_name_array[i]
    outlines = []
    valid_file_name = file_name
    mask_url= ""
    if file_name.find('_conv_masks') == -1 :
        valid_file_name = file_name
    else :
        valid_file_name = file_name.split('_conv_masks')[0]
    if os.path.isfile(make_new_folder + valid_file_name + '_cp_outlines.txt') == False :
        return JSONResponse({"success": 'NO'})
    else :
        #check if colored mask image exist
        print('o_mask', make_new_folder + valid_file_name + "_mask.jpg")
        if os.path.isfile(make_new_folder + valid_file_name + "_mask.jpg") == True :
            mask_url = mask_temp
            mask_temp = mask_temp.split('/')
            temp_length = len(mask_temp)
            mask_url = mask_url.replace(mask_temp[temp_length-1], valid_file_name + "_mask.jpg")
            print('mask_url', mask_url)
        else :
            mask_url = "NO"
        with open(make_new_folder + valid_file_name + '_cp_outlines.txt') as file:
            for item in file:
                outlines.append(item)
    return JSONResponse({"success": outlines, "mask_data": mask_url})

@router.post("/train_model",
             response_description="Train Model",
             status_code=status.HTTP_201_CREATED,
             response_model=List[ExperimentModel])
async def train_model(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")
    init_model = data.get("init_model")
    model_name = data.get("model_name")
    segment = data.get("segment")
    chan2 = data.get("chan2")
    weight_decay = data.get("weight_decay")
    learning_rate = data.get("learning_rate")
    n_epochs = data.get("n_epochs")
    #Get file's full abs path
    file_url = file_url.replace('download/?path=', '')
    temp_url = file_url.split('/')
    temp_length = len(temp_url)
    file_url = temp_url[temp_length-2] + '/' + temp_url[temp_length-1]
    print('file_url', file_url)
    exp_path = os.path.join(current_user_path, file_url)
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    print('file_name', file_full_name)
    print('file_folder', make_new_folder)
    origin_file = ""
    if "_conv_outlines.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_outlines.ome.tiff')
        origin_file = file_temp[0]
    if "_conv_masks.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_masks.ome.tiff')
        origin_file = file_temp[0]
    if "_conv_flows.ome.tiff" in file_full_name:
        file_temp = file_full_name.split('.ome_conv_flows.ome.tiff')
        origin_file = file_temp[0]
    print('origin_file', origin_file)
    original_img = origin_file + ".ome.tiff"
    mask_img = Image.open(make_new_folder + original_img)
    make_new_folder = make_new_folder + 'train/'
    original_mask = origin_file + "_mask.ome.png"
    inputPath = make_new_folder + original_mask
    if os.path.isdir(make_new_folder):
        make_new_folder = make_new_folder
    else:
        os.mkdir(make_new_folder)
    mask_img.save(make_new_folder + origin_file + "_img.tiff")
    outputPath = make_new_folder + origin_file + "_masks.tiff"
    out_file = origin_file + "_mask.ome.tiff"
    cmd_str = "sh /app/mainApi/bftools/bfconvert -separate -overwrite '" + inputPath + "' '" + outputPath + "'"
    print('=====>', out_file, outputPath, cmd_str)
    subprocess.run(cmd_str, shell=True)
    # Train user custom model
    command_string = "python -m cellpose --train --dir {make_new_folder} --pretrained_model {init_model} --chan {segment} --chan2 {chan_2} --img_filter _img --mask_filter _masks --learning_rate {learning_rate} --weight_decay {weight_decay} --n_epochs {n_epochs}  --fast_mode --verbose".format(make_new_folder=make_new_folder, init_model=init_model, segment=segment, chan_2=chan2, learning_rate=learning_rate, weight_decay=weight_decay, n_epochs=n_epochs)
    print("my_command", command_string)
    os.system(command_string)
    result = 'OK'
    return JSONResponse({"success": result})

@router.post("/upload_mask",
            response_description="Upload Chaged mask",
            status_code=status.HTTP_201_CREATED,
            response_model=List[ExperimentModel])
async def upload_mask(request: Request,
                         clear_previous: bool = Form(False),
                         current_user: UserModelDB = Depends(get_current_user),
                         db: AsyncIOMotorDatabase = Depends(get_database)) -> List[ExperimentModel]:
    current_user_path = os.path.join(STATIC_PATH, str(PyObjectId(current_user.id)))
    # print(request)
    data = await request.form()
    file_url = data.get("file_url")
    init_model = data.get("init_model")
    mask_info = data.get("mask_info")
    #Get file's full abs path
    file_url = file_url.replace('download/?path=', '')
    temp_url = file_url.split('/')
    temp_length = len(temp_url)
    file_url = temp_url[temp_length-2] + '/' + temp_url[temp_length-1]
    print('file_url', file_url)
    exp_path = os.path.join(current_user_path, file_url)
    exp_path = os.path.abspath(exp_path)
    directory = exp_path.split('/')
    directory_length = len(directory)
    make_new_folder = ""
    for i in range(directory_length - 2):
        make_new_folder = make_new_folder + '/' + directory[i+1]
    make_new_folder = make_new_folder + "/"
    #Get file's name except type
    file_full_name = directory[directory_length-1]
    print('file_name', file_full_name)
    print('file_folder', make_new_folder)
    origin_file = ""
    file_temp = file_full_name.split('.ome_conv_masks.ome.tiff')
    origin_file = file_temp[0]
    print('origin_file', origin_file)
    make_new_folder = make_new_folder + 'train/'
    if os.path.isdir(make_new_folder):
        make_new_folder = make_new_folder
    else:
        os.mkdir(make_new_folder)
    inputPath = make_new_folder + origin_file + "_mask.ome.png"
    print('input', inputPath)
    mask_info = mask_info.replace('data:image/png;base64,', '')
    mask_info = mask_info.replace(' ', '+')
    img_data = base64.b64decode(mask_info)
    with open(inputPath, "wb") as binary_file:
        # Write bytes to file
        binary_file.write(img_data)
    result = 'OK'
    return JSONResponse({"success": result})





