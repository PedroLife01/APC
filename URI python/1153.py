def fatorial(n):
    if n <=1:
        return 1
    else:
        return n*(fatorial(n-1))

x = int(input())
print(fatorial(x))
