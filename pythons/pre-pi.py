import sys
sys.set_int_max_str_digits(99999)

A=2
B=1
C=2
N=1
M=3
while N < 5000:
  A=A*M+C
  B*=M
  N+=1
  C*=N
  M+=2

def gcd(x, y):
  if x == 0:
    return 1
  while y != 0:
    t = x % y
    x = y
    y = t
  return x

print("* π = ", A/B)
r = gcd(gcd(A, B), C)
A = A // r
B = B // r
C = C // r
print("* a = ", int(A))
print("* b = ", int(B))
print("* c = ", int(C))
print("* n = ", N)
print("* m = ", M)