p1 = input() .split()
x1 = float(p1[0])
y1 = float(p1[1])
p2 = input() .split()
x2 = float(p2[0])
y2 = float(p2[1])
deltax = ((x2-x1)**2)
deltay = ((y2-y1)**2)
raiz = ((deltax+deltay)**0.5)
print('{:.4f}'.format(raiz))