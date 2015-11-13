#coding:utf-8
import codecs
import os
import shutil

TARGET = "Images"
OUTDIR = "Dataset"

for category in os.listdir(TARGET):
    for file in os.listdir("%s/%s" % (TARGET, category)):
        image_file = "%s/%s/%s" % (TARGET, category, file)    # 101_ObjectCategories/airplanes/image_0001.jpg
	id = int(file[-6:-4])
        rename_file = "%s/%s-%02d.tif" % (OUTDIR,category, id)   # caltech101/airplanes_image_0001.jpg
        print "%s -> %s" % (image_file, rename_file)
        shutil.copyfile(image_file, rename_file)
