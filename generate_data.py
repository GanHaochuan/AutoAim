import cv2
import numpy as np
import os
import random

# 配置
OUTPUT_DIR = "./build/data"
NUM_SAMPLES_PER_CLASS = 500  # 每个数字生成多少张
CLASSES = [1, 2, 3, 4, 5, 7] # RM 常用数字

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def generate_image(number):
    # 1. 创建黑底 (40x20)
    img = np.zeros((40, 20), dtype=np.uint8)
    
    # 2. 随机字体参数
    font = cv2.FONT_HERSHEY_SIMPLEX
    # 字体大小和粗细随机化
    scale = random.uniform(0.8, 1.1) 
    thickness = random.randint(1, 2)
    
    # 计算文字居中位置
    text = str(number)
    (text_w, text_h), baseline = cv2.getTextSize(text, font, scale, thickness)
    x = int((20 - text_w) / 2)
    y = int((40 + text_h) / 2) - 2 # 稍微向上调整
    
    # 3. 绘制文字 (白色)
    cv2.putText(img, text, (x, y), font, scale, (255), thickness)
    
    # 4. 数据增强 (核心：模拟真实环境)
    
    # [A] 随机透视变换 (模拟装甲板倾斜)
    pts1 = np.float32([[0,0], [20,0], [0,40], [20,40]])
    # 随机偏移四个角
    shift = 4
    pts2 = np.float32([
        [random.randint(0, shift), random.randint(0, shift)],
        [20 - random.randint(0, shift), random.randint(0, shift)],
        [random.randint(0, shift), 40 - random.randint(0, shift)],
        [20 - random.randint(0, shift), 40 - random.randint(0, shift)]
    ])
    M = cv2.getPerspectiveTransform(pts1, pts2)
    img = cv2.warpPerspective(img, M, (20, 40))
    
    # [B] 高斯模糊 (模拟运动模糊)
    if random.random() > 0.5:
        ksize = random.choice([3, 5])
        img = cv2.GaussianBlur(img, (ksize, ksize), 0)
        
    # [C] 随机噪声 (模拟噪点)
    noise = np.random.randint(0, 50, img.shape, dtype='uint8')
    img = cv2.add(img, noise)

    return img

def generate_negative_samples():
    """生成负样本：纯黑、纯白、随机线条"""
    img = np.zeros((40, 20), dtype=np.uint8)
    type = random.randint(0, 3)
    if type == 0: # 噪点图
        img = np.random.randint(0, 255, (40, 20), dtype=np.uint8)
    elif type == 1: # 模拟灯条的一半 (竖线)
        cv2.line(img, (random.randint(0,5), 0), (random.randint(0,5), 40), (255), 3)
    elif type == 2: # 纯黑加一点点噪点
        img = np.random.randint(0, 20, (40, 20), dtype=np.uint8)
    
    # 同样做模糊
    img = cv2.GaussianBlur(img, (3, 3), 0)
    return img

def main():
    print("正在生成合成数据...")
    
    # 生成数字样本
    for n in CLASSES:
        save_path = os.path.join(OUTPUT_DIR, str(n))
        ensure_dir(save_path)
        for i in range(NUM_SAMPLES_PER_CLASS):
            img = generate_image(n)
            cv2.imwrite(os.path.join(save_path, f"{i}.jpg"), img)
    
    # 生成负样本 (negative)
    save_path = os.path.join(OUTPUT_DIR, "negative")
    ensure_dir(save_path)
    for i in range(NUM_SAMPLES_PER_CLASS):
        img = generate_negative_samples()
        cv2.imwrite(os.path.join(save_path, f"{i}.jpg"), img)
        
    print(f"完成！数据已保存在 {OUTPUT_DIR}")
    print("现在你可以直接运行之前的 train_armor.py 来训练模型了。")

if __name__ == "__main__":
    main()