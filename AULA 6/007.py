def media(n, cont, soma):
    if cont < n:
        cont += 1
        i = int(input())
        soma += i
        return media(n, cont, soma)
    else:
        if n == 0:
            return 0
        else:
            return soma/n