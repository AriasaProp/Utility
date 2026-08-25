import sys
sys.set_int_max_str_digits(99999)

LIM=2**31-1
A=83
B=48
C=1
N=1
M=12
while A < LIM:
  N+=2
  M+=6
  C*=N
  A=A*M+C
  B*=M
  N+=2
  M+=6
  C*=N
  A=A*M-C
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
print(" * √3 = ", (A/B))
print(" * a = ", int(A))
print(" * b = ", int(B))
print(" * c = ", int(C))
print(" * n = ", N)
print(" * m = ", M)

# print(f"GCD of {a}, {b}, and {c} \nis: {result}")