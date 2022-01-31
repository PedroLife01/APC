valores = input() .split()
A = float(valores[0])
B = float(valores[1])
C = float(valores[2])
pi = 3.14159
triangulo = (A*C)/2
circulo = pi*(C**2)
trapezio = ((A+B)*C)/2
quadrado = B**2
retangulo = A*B
print('TRIANGULO: {:.3f}'.format(triangulo))
print('CIRCULO: {:.3f}'.format(circulo))
print('TRAPEZIO: {:.3f}'.format(trapezio))
print('QUADRADO: {:.3f}'.format(quadrado))
print('RETANGULO: {:.3f}'.format(retangulo))