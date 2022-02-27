def raiz(x):
    if x <= a:
        return 1
    else:
        a+=1
        return a+(a/(a+(1/(a+1)/raiz(a))))
n = int(input())
a=1
print(raiz(n))