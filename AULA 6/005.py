import math
def distancia(x1,x2,y1,y2):
    d = math.sqrt((x2-x1)**2 + (y2-y1)**2)
    return d
bolaBrancaX,bolaBrancaY = map(int, input().split())
bola1X,bola1Y = map(int, input().split())
bola2X,bola2Y = map(int, input().split())
bola3X,bola3Y = map(int, input().split())

DistanciaBola1 = distancia(bola1X,bolaBrancaX,bola1Y,bolaBrancaY)
DistanciaBola2 = distancia(bola2X,bolaBrancaX,bola2Y,bolaBrancaY)
DistanciaBola3 = distancia(bola3X,bolaBrancaX,bola3Y,bolaBrancaY)

if DistanciaBola1 < DistanciaBola2 and DistanciaBola1 < DistanciaBola3:
    print('1')
elif DistanciaBola2 < DistanciaBola1 and DistanciaBola2 < DistanciaBola3:
    print('2')
else:
    print('3')


