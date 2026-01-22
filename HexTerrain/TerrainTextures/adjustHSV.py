from PIL import Image

# 打开图像
image = Image.open('Plateau9.png').convert('HSV')

# 获取图像数据
h, s, v = image.split()

# 调整色调
h = Image.eval(h, lambda x: (x + 347) % 360)
s = Image.eval(s, lambda x: 0.65*x)
#v = Image.eval(v, lambda x: 1.1*x)

# 合并图像
image = Image.merge('HSV', (h, s, v)).convert('RGB')

# 保存图像
image.save('Plateau3.png')