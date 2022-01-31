primeira = input() .split()
p1 = int(primeira[0])
n1 = int(primeira[1])
v1 = float(primeira[2])
valor1 = n1*v1
segunda = input() .split()
p2 = int(segunda[0])
n2 = int(segunda[1])
v2 = float(segunda[2])
valor2 = n2*v2
TOTAL = valor1+valor2
print('VALOR A PAGAR: R$ {:.2f}'.format(TOTAL))