def valid_triangle(a,b,c):
    if a>=b+c or b>=c+a or c>=a+b:
        print(f'invalido')
    else:
        if a==b==c:
            print('equilatero')
        elif a==b or a==c or b==c:
            print('isosceles')
        else:
            print('escaleno')
