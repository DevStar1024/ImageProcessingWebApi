from PIL import Image
import os



#align :   "snake" , "raster"
#direction :  "horizontal" , "vertical"
#sortOrder : true,false
#overlapX, overlapY : number for overlapping

FILE_NAME_TEMPLATE = "tile_image_series"


def mergeImageWithOverlap(imageDir,rows,cols,align,direction,sortOrder,overlapX,overlapY, output_path,ext):
    
    print("The directory name we have to merge is ...")
    print(imageDir)
    arrFileName = []
    for root, dir, files in os.walk(imageDir):
        for filename in files:
            if FILE_NAME_TEMPLATE in filename and ext in filename:
                arrFileName.append(os.path.join(imageDir, filename))

    arrFileName = sorted(arrFileName)
    if sortOrder == False:
        arrFileName = arrFileName[::-1]
    
    arrProcessFileNames = []
    for i in range(rows):
        temp = arrFileName[cols*i: (cols) * (i+1)]
        if i % 2 == 1  and align == "snake":
            temp = temp[::-1]
        arrProcessFileNames.append(temp)

    print(arrProcessFileNames)

    horizontalImages = []

    for rowImages in arrProcessFileNames:
        tempImage = Image.open(rowImages[0])
        tempImage = tempImage.resize((int (tempImage.size[0] / 4), int(tempImage.size[1] / 4) ))
        for imgPath in rowImages:
            if rowImages.index(imgPath) == 0:
                continue
            
            img = Image.open(imgPath)
            img = img.resize((int(img.size[0] / 4),int( img.size[1] / 4) ))
            tempImage = mergeHorizontal(tempImage, img, overlapX)

        horizontalImages.append(tempImage)

    final = horizontalImages[0]
    for img in horizontalImages:
        if(horizontalImages.index(img) == 0): continue
        final = mergeVertical(final, img, overlapY)
    
    final.save(output_path)
    return 


def mergeHorizontal(im1, im2, overlap):
    w = im1.size[0] + im2.size[0] - overlap
    h = max(im1.size[1], im2.size[1])
    im = Image.new("RGB", (w, h))

    im.paste(im1)
    im.paste(im2, (im1.size[0] - overlap, 0))

    return im

def mergeVertical(im1, im2, overlap):
    w = max(im1.size[0], im2.size[0])
    h = im1.size[1] + im2.size[1] - overlap
    im = Image.new("RGB", (w, h))

    im.paste(im1)
    im.paste(im2, (0, im1.size[1] - overlap))

    return im





# dir = "./test"
# rows = 4
# cols = 4
# align = "snake"
# direction = "horizontal"
# sortOrder = True
# overlapX = 250
# overlapY = 250
# output_path = "./final_result.jpg"
# ext = ".tif"





# merged_image =  mergeImageWithOverlap(dir,rows,cols,align,direction,sortOrder,overlapX,overlapY, output_path, ext)


