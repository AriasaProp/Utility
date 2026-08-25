import sys
sys.set_int_max_str_digits(99999)

A=1
B=1
C=1
N=0
while N < 9:
  N+=1
  A=A*N+C
  B*=N

def gcd(x, y):
  if x == 0:
    return 1
  while y != 0:
    t = x % y
    x = y
    y = t
  return x

print("* e = ", A/B)
# r = gcd(gcd(A, B), C)
# print("* r = ", r)
# A = A // r
# B = B // r
# C = C // r
print("  * a = ", int(A))
print("  * b = ", int(B))
print("  * c = ", int(C))
print("  * n = ", N)