def calculaDesconto(valorProduto):
    valor =  valorProduto*0.91
    return valor

valor = float(input('Informe o valor do produto: '))

valorFinal = calculaDesconto(valor)

print(f'O valor do produto com desconto de 9% é : {valorFinal}')
