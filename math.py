

A=1
B=2
M=4
N=3
C=1
for i in range(0,7,1):
    C*=N
    A=A*M+C
    B*=M
    N+=2
    M+=4

print("A %d",A)
print("B %d",B)
print("C %d",C)
print("M %d",M)
print("N %d",N)
