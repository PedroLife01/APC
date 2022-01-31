a,b,c = map(int, input().split())
maiorAB = (a+b+abs(a-b))//2
maiorAB_C = (maiorAB+c+abs(maiorAB-c))//2
print(f'{maiorAB_C} eh o maior')