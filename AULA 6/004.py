def simples(n):
    if n == 0:
        return 0
    else:
        print(n)
        return simples(n-1)