def equacao1(p, q):
    r1 = p+q
    r2 = p-q
    equacao2(r1*r2, q)

def equacao2(r, s):
    print(r**2 - s**2)

num1 = int(input())
num2 = int(input())

equacao2(num1,num2)