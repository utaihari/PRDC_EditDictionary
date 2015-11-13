#coding:utf-8
import os.path
import shutil

IMAGE_DIR = "caltech101"
SUBSET_DIR = "caltech10"

# 対象カテゴリ
TARGET_CATEGORY = ["accordion", "bonsai", "cougar_face", "dalmatian", "dollar_bill",
                   "euphonium", "hedgehog", "grand_piano", "Motorbikes", "yin_yang", ]

# ファイル番号が50以下の画像のみ対象
TOP = 50

# 出力ディレクトリがなければ作る
if not os.path.exists(SUBSET_DIR):
    os.mkdir(SUBSET_DIR)

for file in os.listdir(IMAGE_DIR):
    try:
        cat = file.split("-")[0]            # ファイルのカテゴリ名を取得
        num = int(file.split("-")[1][0:4])  # ファイル番号を取得
    except:
        continue
    
    # 対象カテゴリで数字がTOP以下のファイルのみコピー
    if cat in TARGET_CATEGORY and num <= TOP:  # 対象カテゴリの場合
        source_image = "%s/%s" % (IMAGE_DIR, file)
        dest_image = "%s/%s" % (SUBSET_DIR, file)
        shutil.copyfile(source_image, dest_image)
