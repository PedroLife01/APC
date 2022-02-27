lap, ipa, pag, dis = map(int, input().split())

valor = (lap * 1500) + (ipa * 1000)

if ((lap + ipa) >= 3):
    valor = valor - 500

if (pag == 0):
    valor = valor * 0.97
else:
    valor = valor * 0.90

if (dis <= 50):
    valor = valor + 100
else:
    valor = valor + 200

print(f'{valor:.2f}')