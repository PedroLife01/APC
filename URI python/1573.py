def arestaCubo(A,B,C):
    return int((A*B*C)**(1/3))

A,B,C = map(int, input().split())
print(arestaCubo(A,B,C))
while A!=B!=C:
    A, B, C = map(int, input().split())
    print(arestaCubo(A,B,C))
