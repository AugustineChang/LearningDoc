import os
import sys
import tos
import subprocess
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor, as_completed

#blender33Path = r"D:\Projects\Blender3.3\build_windows_x64_vc17_Release\bin\Release\blender.exe"
blender33Path = r"C:\Program Files\Blender Foundation\Blender 3.3\blender.exe"
rendererPath = "render.py"
configPath = "cam360_config.json"

def startOneTask(client, taskLine, taskId):
    global fileRoot
    commandStr = f"\"{blender33Path}\" -b -P {rendererPath} {configPath} {taskLine} {fileRoot}"    
    os.system(commandStr)
    #result = subprocess.run(commandStr, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # Print the output
    #print(result.stdout.decode())
    #print(result.stderr.decode())
    
    return (True, f"")
    

if __name__ == '__main__':
    
    tasksList = []
    fileRoot = r"D:\Projects\Jcd_Scripts\rendering\anise"
    #fileRoot = r"D:\Projects\Jcd_Scripts\rendering\dinosaur\dinosaur_002"
    os.walk(fileRoot)
    for root, dirs, files in os.walk(fileRoot):
        for file in files:
            lowerFile = file.lower()
            if lowerFile.endswith(".obj"):
                tasksList.append(os.path.join(root, file))
                break
        if len(tasksList) > 3:
            break
    
    failList = []
    skipList = []
    with tqdm(total=len(tasksList)) as pbar:
        with ThreadPoolExecutor(max_workers=2) as executor:
            futuresList = []
            counter = 0
            for taskLine in tasksList:
                taskLine = taskLine.strip('\n')
                fu = executor.submit(startOneTask, None, taskLine, counter)
                futuresList.append(fu)
                counter += 1
                
            for future in as_completed(futuresList):
                ret = future.result()
                if not ret[0]:
                    failList.append(f"{ret[1]}\n")
                pbar.update(1)
    
    numOfErrors = len(failList)
    if numOfErrors > 0:
        with open('osgjs_to_glb_errors.txt', 'w') as f:
            f.writelines(failList)
        print(f"NumOfErrors:{numOfErrors}, saved in osgjs_to_glb_errors.txt")
    else:
        print("complete successfully!")
    
    