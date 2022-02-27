def print_rectangle(x,y,z):
    print(x)
    print(x * 'x')
    espaco_em_branco = (x - 2) * ' '
    print(f'x{espaco_em_branco}x')
    print(f'x{espaco_em_branco}x')
    print(x * 'x')
    print(y)
    print(y * 'x')
    espaco_em_branco = (y - 2) * ' '
    print(f'x{espaco_em_branco}x')
    print(f'x{espaco_em_branco}x')
    print(y * 'x')
    print(z)
    print(z * 'x')
    espaco_em_branco = (z - 2) * ' '
    print(f'x{espaco_em_branco}x')
    print(f'x{espaco_em_branco}x')
    print(z * 'x')

n1,n2,n3 = map(int, input().split())
print_rectangle(n1,n2,n3)