import os
import sys

cmgenPath = r"D:\Projects\Filament\FilamentBin\bin\cmgen.exe"
inputPath = sys.argv[1]

splitPath = os.path.split(inputPath)
splitName = os.path.splitext(splitPath[1])

print(inputPath, "...")
commandStr1 = f"{cmgenPath} -x {splitName[0]} --format=ktx --size=64 --extract-blur=0.1 {inputPath}"
commandStr2 = f"{cmgenPath} -x {splitName[0]} --format=ktx --size=256 --extract-blur=0.1 {inputPath}"

os.system(commandStr1)

outputDir = os.path.join(splitPath[0], splitName[0])
skyboxOldName = os.path.join(outputDir, f"{splitName[0]}_skybox.ktx")
skyboxNewName = skyboxOldName.replace("_skybox", "_skybox_tiny")
os.rename(skyboxOldName, skyboxNewName)

os.system(commandStr2)