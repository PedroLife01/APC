n1,n2,n3,n4 = map(float, input().split())
media1 = ((n1*2)+(n2*3)+(n3*4)+(n4*1))/10
print(f'Media: {media1:.1f}')
if media1 >= 7.0:
    print('Aluno aprovado.')
elif media1 >= 5.0 and media1 <= 6.9:
    print('Aluno em exame.')
    n5 = float(input())
    print(f'Nota do exame: {n5}')
    mediaFinal = (media1 + n5) / 2
    if mediaFinal >= 5.0:
        print('Aluno aprovado.')
        print(f'Nota Final: {mediaFinal}')
    else:
        print('Aluno reprovado.')
        print(f'Nota Final: {mediaFinal}')
else:
        print('Aluno reprovado.')