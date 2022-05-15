def SimpleShader(u, v):
    R = u * 255
    G = v * 255
    B = 0
    return (R,G,B)

def LoadConfigs():
    global filename, sizeX, sizeY
    
    configFile = open("config.ini", "r")
    configLines = configFile.readlines()
    configFile.close()
    
    for line in configLines:
        if "=" not in line:
            continue
        words = line.split("=")
        key = words[0].strip()
        value = words[1].strip()
        
        if key == "filename":
            filename = value
        elif key == "sizeX":
            sizeX = int(value)
        elif key == "sizeY":
            sizeY = int(value)
    print("Config Loaded!")
    print("filename=%s\nsizeX=%d\nsizeY=%d" %(filename, sizeX, sizeY))
    

def GenPicture():
    newPic = open(filename + ".ppm", "w")
    newPic.write("P3\n%d %d\n255\n" % (sizeX, sizeY))
    for y in range(0,sizeY):
        v = y / (sizeY-1)
        for x in range(0, sizeX):
            u = x / (sizeX-1)
            newPic.write("%d %d %d\n" % SimpleShader(u, v))
    newPic.close()    
##########################################################################################

def main():
    filename = "hello_world"
    sizeX = 300
    sizeY = 200
    
    LoadConfigs()
    GenPicture()    

if __name__ == "__main__":
    main()

