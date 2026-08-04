import random
import math

def factorial(I):
    R=1
    while I > 1:
        R*=I
        I-=1
    return R

def factorial2(I):
    R=1
    p=0
    # print(I,"!=")
    while I:
        i = 3
        while i <= I:
            R *= i
            i += 2
        I >>= 1
        p += I
    R<<=p
    return R

for i in range(38):
    S = random.randint(0,89)

    T = math.factorial(S)
    A = factorial(S)
    B = factorial2(S)
    if A // B != 1:
        print("T:", T)
        print("A:", A)
        print("B:", B)
        print("LS:", A // B)
        break

print("Done!")
