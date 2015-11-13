#coding:utf-8
import os
import shutil

IMAGE_DIR = "Dataset"
DIC_DIR = "dictionary"
LRN_DIR = "test"
TES_DIR = "training"
CANDI_DIR = "cand_dict"

# 対象カテゴリ
TARGET_CATEGORY = [
#                   "agricultural", 
#                   "airplane",
#                   "baseballdiamond",
#                   "beach",
		   "buildings",
#                  "chaparral",
                   "denseresidential",
                   "forest",
#                   "freeway",
#                   "golfcourse",
#                  "harbor",
                   "intersection",
#                   "mediumresidential",
#                   "mobilehomepark",
#                  "overpass",
#                  "parkinglot",
                   "river",
#                  "runway",
#                  "sparseresidential",
#                  "storagetanks",
#                  "tenniscourt",
                ]

# ファイル番号が50以下の画像のみ対象
#TOP = 50

#辞書データ
if not os.path.exists(DIC_DIR):
    os.mkdir(DIC_DIR)

for file in os.listdir(IMAGE_DIR):
    try:
        cat = file.split("-")[0]            # ファイルのカテゴリ名を取得
        num = int(file.split("-")[1][-6:-4])  # ファイル番号を取得
    except:
        continue

    # 対象カテゴリで数字がTOP以下のファイルのみコピー
    if cat in TARGET_CATEGORY and num >= 1 and num < 11:  # 対象カテゴリの場合
        source_image = "%s/%s" % (IMAGE_DIR, file)
        dest_image = "%s/%s" % (DIC_DIR, file)
        shutil.copyfile(source_image, dest_image)

#テストデータ
if not os.path.exists(LRN_DIR):
    os.mkdir(LRN_DIR)

for file in os.listdir(IMAGE_DIR):
    try:
        cat = file.split("-")[0]            # ファイルのカテゴリ名を取得
        num = int(file.split("-")[1][-6:-4])  # ファイル番号を取得
    except:
        continue

    # 対象カテゴリで数字がTOP以下のファイルのみコピー
    if cat in TARGET_CATEGORY and num >= 0 and num < 101:  # 対象カテゴリの場合
        source_image = "%s/%s" % (IMAGE_DIR, file)
        dest_image = "%s/%s" % (LRN_DIR, file)
        shutil.copyfile(source_image, dest_image)

#辞書候補データ
if not os.path.exists(CANDI_DIR):
    os.mkdir(CANDI_DIR)

for file in os.listdir(IMAGE_DIR):
    try:
        cat = file.split("-")[0]            # ファイルのカテゴリ名を取得
        num = int(file.split("-")[1][-6:-4])  # ファイル番号を取得
    except:
        continue

    # 対象カテゴリで数字がTOP以下のファイルのみコピー
    if cat in TARGET_CATEGORY and num >= 0 and num < 101:  # 対象カテゴリの場合
        source_image = "%s/%s" % (IMAGE_DIR, file)
        dest_image = "%s/%s" % (CANDI_DIR, file)
        shutil.copyfile(source_image, dest_image)


#サンプルデータ
if not os.path.exists(TES_DIR):
    os.mkdir(TES_DIR)

for file in os.listdir(IMAGE_DIR):
    try:
        cat = file.split("-")[0]            # ファイルのカテゴリ名を取得
        num = int(file.split("-")[1][-6:-4])  # ファイル番号を取得
    except:
        continue

    # 対象カテゴリで数字がTOP以下のファイルのみコピー
    if cat in TARGET_CATEGORY and num >= 0 and num < 100:  # 対象カテゴリの場合
        source_image = "%s/%s" % (IMAGE_DIR, file)
        dest_image = "%s/%s" % (TES_DIR, file)
        shutil.copyfile(source_image, dest_image)
