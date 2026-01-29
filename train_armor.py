import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import datasets, transforms
import torch.onnx
import os

# ----------------配置----------------
DATA_DIR = './build/data'  # 指向你刚才分类好的 data 文件夹路径
MODEL_PATH = 'armor_model.onnx'
BATCH_SIZE = 16
EPOCHS = 30  # 训练轮数
LR = 0.001

# ----------------模型定义----------------
# 一个超轻量级的 CNN，专门处理 20x28 或 20x40 的小图
class ArmorNumNet(nn.Module):
    def __init__(self, num_classes):
        super(ArmorNumNet, self).__init__()
        # 输入: 1通道(灰度), 20宽, 40高
        self.features = nn.Sequential(
            nn.Conv2d(1, 16, kernel_size=3, padding=1), # -> 16x20x40
            nn.BatchNorm2d(16),
            nn.ReLU(),
            nn.MaxPool2d(2),                            # -> 16x10x20
            
            nn.Conv2d(16, 32, kernel_size=3, padding=1),# -> 32x10x20
            nn.BatchNorm2d(32),
            nn.ReLU(),
            nn.MaxPool2d(2),                            # -> 32x5x10
        )
        self.classifier = nn.Sequential(
            nn.Linear(32 * 5 * 10, 128),
            nn.ReLU(),
            nn.Dropout(0.5),
            nn.Linear(128, num_classes)
        )

    def forward(self, x):
        x = self.features(x)
        x = x.view(x.size(0), -1) # 展平
        x = self.classifier(x)
        return x

# ----------------主流程----------------
def main():
    # 1. 数据预处理
    # C++ 读取是用 imread(GRAYSCALE)，值在0-255。
    # PyTorch 的 ToTensor 会把 0-255 映射到 0.0-1.0。
    transform = transforms.Compose([
        transforms.Grayscale(num_output_channels=1),
        transforms.Resize((40, 20)), # 高40，宽20
        transforms.ToTensor(),       # 归一化到 [0, 1]
    ])

    # 2. 加载数据集
    if not os.path.exists(DATA_DIR):
        print(f"错误：找不到数据集文件夹 {DATA_DIR}")
        return

    dataset = datasets.ImageFolder(root=DATA_DIR, transform=transform)
    print(f"类别对应关系: {dataset.class_to_idx}") 
    # 注意：ImageFolder 会按字母顺序排序文件夹，例如 {'1': 0, '2': 1, 'negative': 2 ...}
    # 记住这个对应关系，C++里要用到！

    train_loader = torch.utils.data.DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True)

    # 3. 初始化模型
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = ArmorNumNet(num_classes=len(dataset.classes)).to(device)
    optimizer = optim.Adam(model.parameters(), lr=LR)
    criterion = nn.CrossEntropyLoss()

    # 4. 训练
    print("开始训练...")
    for epoch in range(EPOCHS):
        model.train()
        total_loss = 0
        correct = 0
        total = 0
        for images, labels in train_loader:
            images, labels = images.to(device), labels.to(device)
            
            optimizer.zero_grad()
            outputs = model(images)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()
            
            total_loss += loss.item()
            _, predicted = torch.max(outputs.data, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum().item()
            
        print(f"Epoch {epoch+1}/{EPOCHS}, Loss: {total_loss:.4f}, Acc: {100 * correct / total:.2f}%")

    # 5. 导出 ONNX
    print(f"正在导出 ONNX 模型到 {MODEL_PATH} ...")
    model.eval()
    # 创建一个虚拟输入：Batch=1, Channel=1, Height=40, Width=20
    dummy_input = torch.randn(1, 1, 40, 20).to(device)
    
    torch.onnx.export(model, dummy_input, MODEL_PATH,
                      input_names=['input'], output_names=['output'],
                      dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}})
    
    print("完成！请将生成的 .onnx 文件复制到 C++ 项目的 models 目录下。")

if __name__ == '__main__':
    main()