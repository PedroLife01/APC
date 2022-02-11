N = float(input())


n100 = N//100
N = N%100

n50 = N//50
N = N%50

n20 = N//20
N = N%20

n10 = N//10
N = N%10

n5 = N//5
N = N%5

n2 = N//2
N = N%2

n1 = N//1
N = N%1

N = 100*N

n05 = N//50
N = N%50

n025 = N//25
N = N%25

n010 = N//10
N = N%10

n005 = N//5
N = N%5

n001 = N//1

print('NOTAS:')
print(f'{int(n100)} nota(s) de R$ 100.00')
print(f'{int(n50)} nota(s) de R$ 50.00')
print(f'{int(n20)} nota(s) de R$ 20.00')
print(f'{int(n10)} nota(s) de R$ 10.00')
print(f'{int(n5)} nota(s) de R$ 5.00')
print(f'{int(n2)} nota(s) de R$ 2.00')
print('MOEDAS:')
print(f'{int(n1)} moeda(s) de R$ 1.00')
print(f'{int(n05)} moeda(s) de R$ 0.50')
print(f'{int(n025)} moeda(s) de R$ 0.25')
print(f'{int(n010)} moeda(s) de R$ 0.10')
print(f'{int(n005)} moeda(s) de R$ 0.05')
print(f'{int(n001)} moeda(s) de R$ 0.01')