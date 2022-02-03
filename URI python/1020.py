N = int(input())
#ANO
nAno = N//365
N = N%365
#MES
nMes = N//30
N = N%30
#DIA
nDia = N
print(f'{nAno} ano(s)')
print(f'{nMes} mes(es)')
print(f'{nDia} dia(s)')