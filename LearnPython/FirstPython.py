import random
import numpy

def TestInput():
    value:int = int(input("Please Input a Number:"))
    if value > 10 :
        print("Greater than 10")
    elif value == 10:
        print("Equals 10")
    else:
        print("Less than 10")    


def CalcArea():
    R1W = 10
    R1H = 8
    R2W = 4 + 1
    R2H = R1H - 2 + 3
    
    Area = R1W * R1H + R2W * R2H - 6*1
    print(Area)    

def DexToBin(dexNumber):
    temp = dexNumber
    output = ""
    counter = 0
    while(temp > 0):
        bit = temp % 2
        temp //= 2
        
        if counter >= 4:
            output = " " + output
            counter = 0
        
        output = str(bit) + output
        counter += 1
    print(output)
#DexToBin(value)

def CalcSumOfArithmeticSequence():
    a1 = int(input("a1 = "))
    d = int(input("d = "))
    n = int(input("n = "))
    an = a1 + (n-1)*d
    sn = n*(a1+an) // 2
    print("Sn(%d..%d) ="%(a1,an), sn)
#CalcSumOfArithmeticSequence()

def Exercise1():
    name = input("Your Name?")
    nameLen = len(name)
    
    for i in range(0,4):
        if i < nameLen:
            char = name[i]
            print("%s ASCII Code = %d" % (char, ord(char)))
            print("%s Capitalized is %s" %(char, chr(ord(char) - 32)))
#Exercise1()

def SplitWords():
    word = input("Input Word:")
    wordList = word.split()
    for oneChar in nnumpyumpy:
        print(oneChar)
    
#SplitWords()

def CalcFactorial(n:int):
    result = 1
    for i in range(1,n+1):
        result = result * i
    return result


class Mover:
    def __init__(self, inX:float, inY:float, inRadius:float):
        self.pos = numpy.array([inX, inY])
        self.radius = inRadius
        
        randX = random.random() * 2.0 - 1.0
        randY = random.random() * 2.0 - 1.0
        self.velocity = numpy.array([randX, randY])
        
    def update(self):
        self.pos = self.pos + self.velocity
        
    def draw(self):
        ellipse(self.pos[0], self.pos[1], self.radius, self.radius)


a = Mover(0.0, 0.0, 1.0)

#input("press any key to continue. . .")