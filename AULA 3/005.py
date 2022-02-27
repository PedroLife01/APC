import math


def piscininha(y):
    print("Então vou entrar na piscininha. Aff, ela parece um retângulo...")
    print(y * '#')
    espaco_em_branco = (y - 2) * ' '
    print(f'#{espaco_em_branco}#')
    print(f'#{espaco_em_branco}#')
    print(f'#{espaco_em_branco}#')
    print(y * '#')


def fala_do_lado(x: object) -> object:
    print(f'E se {x} for o lado do quadrado?')
    areadoquadrado = x * x
    print(f'Olha a área do quadrado: {areadoquadrado}')
    print(f'E se {x} for o raio de um círculo?')
    areadocirculo = math.pi * (x ** 2)
    print(f'Olha a área do circulo: {areadocirculo}')
    piscininha(x)


fala_do_lado(10)
