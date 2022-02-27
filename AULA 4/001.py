import math
DIAMETRO = int(input())
A, L, P = map(int,input().split())
RAIO = DIAMETRO/2
VolumeBola = (4/3)*math.pi*(RAIO**3)
VolumeCaixa = A*L*P
if VolumeCaixa>=VolumeBola and A>3 and L>3 and P>3:
    print('S')
else:
    print('N')