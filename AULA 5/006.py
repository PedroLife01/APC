def corrida(a,b):
    if a*3 > b*2:
        print('Jorge ultrapassou e nao sera alcancado!')
    else:
        print(f'J: {a*3} G: {b*2}')
        corrida(a*3,b*2)
