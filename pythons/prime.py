import math

def isPrime(n):
	if (n&1 == 0):
		return False
	x = 3
	e = int(math.sqrt(n))
	e |= 1
	while x < e:
		if (n%x == 0):
			return False
		x += 2
	return True

N = 2**32 - 1
while not(isPrime(N)):
	N+=1

print(N)