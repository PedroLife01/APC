def ppa(a,b):
    # 1 = PEDRA
    # 2 = PAPEL
    # 3 = ATAQUE AEREO
    #TODOS DA PEDRA
    if a == 1 and b == 3:
        return 'Jogador 2 venceu'
    elif a == 1 and b == 2:
        return 'Jogador 1 venceu'
    elif a == 1 and b == 1:
        return 'Sem ganhador'
    if b == 1 and a == 3:
        return 'Jogador 1 venceu'
    elif b == 1 and a == 2:
        return 'Jogador 2 venceu'
    #TODOS DO PAPEL
    if b == 2 and a == 3:
        return 'Jogador 1 venceu'
    elif b == 2 and a == 2:
        return 'Ambos venceram'
    if a == 2 and b == 3:
        return 'Jogador 2 venceu'
    #TODOS ATAQUE AEREO
    if a == 3 and b == 3:
        return 'Aniquilacao mutua'


