import numpy as np
from skimage import exposure, io
from scipy import ndimage, signal
from flowdec import data as fd_data
from flowdec import psf as fd_psf
from flowdec import restoration as fd_restoration
from scipy.signal import fftconvolve
from skimage import color as sk_color
from skimage import data as sk_data
from skimage.util import crop
import os
import os.path as osp
from mainApi.config import STATIC_PATH
import time
from mainApi.app.images.utils.asyncio import shell

#import histomicstk as htk
# Deconvolution 3D
def RechardDeconvolution3d(file_name, effectiveness, isroi, dictRoiPts, gamma=0.2):
    fName, ext= '', ''
    for f in os.listdir(STATIC_PATH):
        if osp.splitext(f)[-1].lower() == '.tif' or osp.splitext(f)[-1].lower() == '.tiff':
            fName = f
            break
    if fName == '':
        fName = file_name
    ext = osp.splitext(fName)[-1].lower()
    data_path = osp.join(STATIC_PATH, fName)            
    originImg = io.imread(data_path)

    width = originImg.shape[1]
    height = originImg.shape[0]
    
    startX = round(dictRoiPts['startX'] * width / 100)
    startY = round(dictRoiPts['startY'] * height / 100)
    endX = round(dictRoiPts['endX'] * width / 100)
    endY = round(dictRoiPts['endY'] * height / 100)
    # crop image
    if isroi:
        actual = originImg[:, startY:endY, startX:endX]
    else:
        actual = originImg
    # Create a gaussian kernel that will be used to blur the original acquisition
    kernel = np.zeros_like(actual)
    for offset in [0, 1]:
        kernel[tuple((np.array(kernel.shape) - offset) // 2)] = 1
    kernel = ndimage.gaussian_filter(kernel, sigma=1.)

    # Run the deconvolution process and note that deconvolution initialization is best kept separate from 
    # execution since the "initialize" operation corresponds to creating a TensorFlow graph, which is a 
    # relatively expensive operation and should not be repeated across multiple executions
    timestamp = str(int(time.time()))

    algo = fd_restoration.RichardsonLucyDeconvolver(actual.ndim).initialize()
    res = algo.run(fd_data.Acquisition(data=actual, kernel=kernel), niter=effectiveness).data
    outFileName = str(fName).split(".")[0] + "_deconvol3d" + ext
    output_path = str(STATIC_PATH) + "/" + str(outFileName)
    
    if isroi:
        originImg[:, startY:endY, startX:endX] = res
        outImage = originImg
    else:
        outImage = res
    
    for f in os.listdir(STATIC_PATH):
        os.remove(os.path.join(STATIC_PATH, f))
    io.imsave(output_path, outImage)
    return output_path

# Deconvolution 2D
def RechardDeconvolution2d(file_name, effectiveness, isroi, dictRoiPts):

    ext = osp.splitext(file_name)[-1].lower()
    data_path = osp.join(STATIC_PATH, file_name) 
    originImg = io.imread(data_path)

    startX = round(dictRoiPts['startX'])
    startY = round(dictRoiPts['startY'])
    endX = round(dictRoiPts['endX'])
    endY = round(dictRoiPts['endY'])


    width = originImg.shape[1]
    height = originImg.shape[0]
    
    startX = round(dictRoiPts['startX'] * width / 100)
    startY = round(dictRoiPts['startY'] * height / 100)
    endX = round(dictRoiPts['endX'] * width / 100)
    endY = round(dictRoiPts['endY'] * height / 100)


    if isroi:
        img = originImg[startY:endY, startX:endX]
    else:
        img = originImg

    
    img = sk_color.rgb2gray(img) * 255
   

    timestamp = str(int(time.time()))

    outFileName = str(file_name).split(".")[0] + "_deconvol2d" + timestamp + ext    
    output_path = str(STATIC_PATH) + "/" + str(outFileName)

    
    psf = np.ones((3, 3)) / 9
           
    # Wrap image and PSF in "Acqusition" instance, which aids in doing comparisons and running
    # operations on all data associated with a data acquisition
    acquisition = fd_data.Acquisition(data=img, kernel=psf)

    # Run deconvolution using default arguments (will default to adding no padding to image
    # as its dimensions are already powers of 2)
    img_decon = fd_restoration.richardson_lucy(acquisition, niter=effectiveness) 
    img_decon = img_decon.astype(int)

    if isroi:        
        originImg[startY:endY, startX:endX] = sk_color.gray2rgb(img_decon)
        deconved_img = originImg
    else:
        deconved_img = img_decon 
    
    

    #for f in os.listdir(STATIC_PATH):
    #    os.remove(os.path.join(STATIC_PATH, f))
    io.imsave(output_path, deconved_img)

    return output_path

# Deconvolution 2D using Supervised Color Deconv
def SupervisedColorDeconvolution(file_name, effectiveness, isroi, dictRoiPts):

    ext = osp.splitext(file_name)[-1].lower()
    data_path = osp.join(STATIC_PATH, file_name)
    imInput = io.imread(data_path)[:, :, :3]

    startX = round(dictRoiPts['startX'])
    startY = round(dictRoiPts['startY'])
    endX = round(dictRoiPts['endX'])
    endY = round(dictRoiPts['endY'])
    
    if isroi:
        img = imInput[startY:endY, startX:endX]
    else:
        img = imInput
    stain_color_map = htk.preprocessing.color_deconvolution.stain_color_map
    stains = ['hematoxylin',  # nuclei stain
          'eosin',        # cytoplasm stain
          'null']         # set to null if input contains only two stains
    W = np.array([stain_color_map[st] for st in stains]).T

    imDeconvolved = htk.preprocessing.color_deconvolution.color_deconvolution(img, W).Stains[:, :, 1]

    outFileName = str(file_name).split(".")[0] + "_deconvol2d" + ext    
    output_path = str(STATIC_PATH) + "/" + str(outFileName)

    if isroi:        
        imInput[startY:endY, startX:endX] = sk_color.gray2rgb(imDeconvolved)
        deconved_img = imInput
    else:
        deconved_img = imDeconvolved 

    for f in os.listdir(STATIC_PATH):
        os.remove(os.path.join(STATIC_PATH, f))
    io.imsave(output_path, deconved_img)

    return output_path


def Deconvolution2DByChannel(img,effectiveness):

    img = img / 255.0
    # Create a gaussian kernel that will be used to blur the original acquisition
    kernel = np.zeros_like(img)
    for offset in [0, 1]:
        kernel[tuple((np.array(kernel.shape) - offset) // 2)] = 1
    kernel = ndimage.gaussian_filter(kernel, sigma=1.)


    psf = np.ones((1, 1)) / 1
    
    # Convolve the original image with our fake PSF
    #data = signal.fftconvolve(img, kernel, mode='same')

    # Run the deconvolution process and note that deconvolution initialization is best kept separate from 
    # execution since the "initialize" operation corresponds to creating a TensorFlow graph, which is a 
    # relatively expensive operation and should not be repeated across multiple executions
    algo = fd_restoration.RichardsonLucyDeconvolver(img.ndim).initialize()
    res = algo.run(fd_data.Acquisition(data=img, kernel=kernel), niter=effectiveness).data

    max_v = np.max(res)
    res = res * 255 / max_v
    res = res.astype(int)

    return res




async def FlowDecDeconvolution2D(file_name, effectiveness, isroi, dictRoiPts):

    ext = osp.splitext(file_name)[-1].lower()
    data_path = osp.join(STATIC_PATH, file_name) 
    originImg = io.imread(data_path)

    startX = round(dictRoiPts['startX'])
    startY = round(dictRoiPts['startY'])
    endX = round(dictRoiPts['endX'])
    endY = round(dictRoiPts['endY'])

    print("*" * 30)
    print("The Original Image is :")
    print(originImg)
    print("*" * 30)


    width = originImg.shape[1]
    height = originImg.shape[0]
    
    startX = round(dictRoiPts['startX'] * width / 100)
    startY = round(dictRoiPts['startY'] * height / 100)
    endX = round(dictRoiPts['endX'] * width / 100)
    endY = round(dictRoiPts['endY'] * height / 100)


    if isroi:
        img = originImg[startY:endY, startX:endX]
    else:
        img = originImg

    timestamp = str(int(time.time()))
    outFileName = str(file_name).split(".")[0] + "_deconvol2d" + timestamp + ext    
    output_path = str(STATIC_PATH) + "/" + str(outFileName)
    
    tempImageList = []
    result_image = originImg

    if(len(img.shape) >= 3):
        for i in range(img.shape[2]):
            channel_Img = img[:,:,i]
            deconv_channel_img = Deconvolution2DByChannel(channel_Img,effectiveness)
            tempImageList.append(deconv_channel_img)
            mergedDeconv2DImg = np.dstack(tempImageList)
            result_image = mergedDeconv2DImg
    
    else :
         result_image = Deconvolution2DByChannel(img,effectiveness)

    if isroi:        
        originImg[startY:endY, startX:endX] = result_image
        deconved_img = originImg
    else:
        deconved_img = img_decon 
    

    io.imsave(output_path, deconved_img)

    omeTiffFile = str(file_name).split(".")[0] + "_deconvol2d" + timestamp + ".ome.tiff"
    omePath = str(STATIC_PATH) + "/" + str(omeTiffFile)    
    bf_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_path}' '{omePath}'"
    await shell(bf_cmd)  


    return omePath










async def FlowDecDeconvolution3D(userid, filenames, effectiveness, isroi, dictRoiPts):

    converted_filenames = []
    for file_name in filenames:
        ext = osp.splitext(file_name)[-1].lower()

        data_path = osp.join(STATIC_PATH, userid, file_name) 
        originImg = io.imread(data_path)
        
        
        startX = round(dictRoiPts['startX'])
        startY = round(dictRoiPts['startY'])
        endX = round(dictRoiPts['endX'])
        endY = round(dictRoiPts['endY'])

        width = originImg.shape[1]
        height = originImg.shape[0]
        
        startX = round(dictRoiPts['startX'] * width / 100)
        startY = round(dictRoiPts['startY'] * height / 100)
        endX = round(dictRoiPts['endX'] * width / 100)
        endY = round(dictRoiPts['endY'] * height / 100)

        if isroi:
            img = originImg[startY:endY, startX:endX]
        else:
            img = originImg

        timestamp = str(int(time.time()))
        outFileName = str(file_name).split(".")[0] + "_deconvol3d_" + timestamp + ext    
        output_path = osp.join(STATIC_PATH, userid, outFileName) 
        
        tempImageList = []
        result_image = originImg

        if(len(img.shape) >= 3):
            for i in range(img.shape[2]):
                channel_Img = img[:,:,i]
                deconv_channel_img = Deconvolution2DByChannel(channel_Img,effectiveness)
                tempImageList.append(deconv_channel_img)
                mergedDeconv2DImg = np.dstack(tempImageList)
                result_image = mergedDeconv2DImg
        
        else :
            result_image = Deconvolution2DByChannel(img,effectiveness)



        if isroi:        
            originImg[startY:endY, startX:endX] = result_image
            deconved_img = originImg
        else:
            deconved_img = img_decon 
        

        io.imsave(output_path, deconved_img)

        omeTiffFile = str(file_name).split(".")[0] + "_deconvol3d_effectiveness_" + str(effectiveness) + "_" + (timestamp) + ".ome.tiff"
        omePath =  osp.join(STATIC_PATH, userid, omeTiffFile)  
        bf_cmd = f"sh /app/mainApi/bftools/bfconvert -separate -overwrite '{output_path}' '{omePath}'"
        await shell(bf_cmd)  

        converted_filenames.append(omeTiffFile)
    
    return converted_filenames
       
