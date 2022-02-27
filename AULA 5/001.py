def area(x,y,z):
    if z == 'retangulo' :
       area = x*y
       print(f'O retangulo tem {area} de area')
    elif z == 'triangulo' :
        area = (x*y)//2
        print(f'O triangulo tem {area} de area')
    else:
        area = (x*y)//2
        print(f'O losango tem {area} de area')

area(10,2,'losango')

