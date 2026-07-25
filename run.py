A=1
B=2
C=1
N=3
M=4
while N < 8000:
  C*=N
  A*=M
  A+=C
  B*=M
  N+=2
  M+=4

def gcd(x, y):
  if x == 0:
    return 1
  while y != 0:
    t = x % y
    x = y
    y = t
  return x

# Contoh penggunaan
r = gcd(gcd(A, B), C)
A = A // r
B = B // r
C = C // r
print("* a = ", int(A))
print("* b = ", int(B))
print("* c = ", int(C))
print("* n = ", N)
print("* m = ", M)

# print(f"GCD of {a}, {b}, and {c} \nis: {result}")