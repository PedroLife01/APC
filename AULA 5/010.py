'''
Essa é uma forma de obter vários dados separados por espaços
em uma mesma linha. Por hora trate apenas como uma curiosidade :)
'''
n1,n2,n3 = map(int, input().split())

if n1 >= 5 and n2 >= 5 and n3 >= 5:
    print('APROVAR')
    # Cálculo da nota 1
    if n1 >= 5 and n1 < 7:
        print('Prova 1: MM')
    elif n1 >= 7 and n1 < 9:
        print('Prova 1: MS')
    else:
        print('Prova 1: SS')
    # Cálculo da nota 2
    if n2 >= 5 and n2 < 7:
        print('Prova 2: MM')
    elif n2 >= 7 and n2 < 9:
        print('Prova 2: MS')
    else:
        print('Prova 2: SS')
    # Cálculo da nota 3
    if n3 >= 5 and n3 < 7:
        print('Prova 3: MM')
    elif n3 >= 7 and n3 < 9:
        print('Prova 3: MS')
    else:
        print('Prova 3: SS')

else:
    print('REPROVAR')