def funcao(a, b):
    if a > b:
        return a
    if a < b:
        a = a + b
    else:
        return b

    return 2 * a

x = funcao(2, 3)
y = funcao(5, 1)
z = funcao(13, 13)

print(x,y,z)