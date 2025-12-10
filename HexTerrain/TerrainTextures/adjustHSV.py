from PIL import Image

# 打开图像
image = Image.open('Moor6.png').convert('HSV')

# 获取图像数据
h, s, v = image.split()

# 调整色调
h = Image.eval(h, lambda x: (x + 120) % 360)
s = Image.eval(s, lambda x: 0.25*x)
#v = Image.eval(v, lambda x: 1.1*x)

# 合并图像
image = Image.merge('HSV', (h, s, v)).convert('RGB')

# 保存图像
image.save('Moor3.png')