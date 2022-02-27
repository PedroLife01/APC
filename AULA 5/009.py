def drone(x):
    a = str(x)
    if a == "":
       print('Trajeto Completo!')
    else:
        if a[-1::] == "1":
            print('Norte')
            TirarUltimoDigito = a[:-1]
            drone(TirarUltimoDigito)
        elif a[-1::] == "2":
            print('Sul')
            TirarUltimoDigito = a[:-1]
            drone(TirarUltimoDigito)
        elif a[-1::] == "3":
            print('Leste')
            TirarUltimoDigito = a[:-1]
            drone(TirarUltimoDigito)
        elif a[-1::] == "4":
            print('Oeste')
            TirarUltimoDigito = a[:-1]
            drone(TirarUltimoDigito)

# [-1::] PEGA O ULTIMO DIGITO, SE QUISER PEGAR OUTRO SO TROCAR O '1'
# [:-1] TIRA O ULTIMO DIGITO, SE QUISER TIRAR OUTRO SO TROCAR O '1'
