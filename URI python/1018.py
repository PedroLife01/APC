NT = int(input())
N = NT
#100
n100 = N//100
N = N%100
#50
n50 = N//50
N = N%50
#20
n20 = N//20
N = N%20
#10
n10 = N//10
N = N%10
#5
n5 = N//5
N = N%5
#2
n2 = N//2
N = N%2
#1
n1 = N

print(NT)
print(f'{n100} nota(s) de R$ 100,00')
print(f'{n50} nota(s) de R$ 50,00')
print(f'{n20} nota(s) de R$ 20,00')
print(f'{n10} nota(s) de R$ 10,00')
print(f'{n5} nota(s) de R$ 5,00')
print(f'{n2} nota(s) de R$ 2,00')
print(f'{n1} nota(s) de R$ 1,00')
