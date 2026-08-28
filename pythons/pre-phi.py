import sys
sys.set_int_max_str_digits(99999)

LIM=2**33-1
A=13
B=8
C=1
N=1
M=8
while A < LIM:
  N-=2
  C*=N
  M+=8
  A=A*M+C
  B*=M

def gcd(x, y):
  if x == 0:
    return 1
  while y != 0:
    t = x % y
    x = y
    y = t
  return x

r = gcd(gcd(A, B), abs(C))
A = A // r
B = B // r
C = C // r
print(" * r = ", r)
print(" * phi = ", (A/B))
print(" * a = ", int(A))
print(" * b = ", int(B))
print(" * c = ", int(C))
print(" * n = ", N)
print(" * m = ", M)

# print(f"GCD of {a}, {b}, and {c} \nis: {result}")