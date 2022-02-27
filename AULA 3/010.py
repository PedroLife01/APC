def calcula(dias,diasRemoto,flag):
    diasPresencial = dias-diasRemoto
    if flag==0:
        passagem = diasPresencial*11
        print(passagem)
    else:
       passagem = diasPresencial*17
       print(passagem)