
class Pair:
    def __init__(self, n, d):
        self.nom = n
        self.denom = d
    def tot(self, P):
        xt = self.nom * P.nom - self.denom * P.denom
        yt = self.nom * P.denom + self.denom * P.nom
        self.nom, self.denom = yt, xt
    def amo(self):
        return self.nom / self.denom
    def __repr__(self):
        return f" {self.nom}/{self.denom} "

print("Start ...")
for i in range(3, 99999999999, 2):
    A = Pair(1,i)
    S = Pair(1,i)
    c = 1
    while S.nom < S.denom:
        S.tot(A)
        c += 1
    if S.nom > S.denom:
        print(f" Pair {S.amo():08.4e} x {c}", end="\r", flush=True)
    else:
        print(f" Pair {S.amo():08.4e} x {c}")
        break

