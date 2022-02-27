def herão(a,b,c):
    p = (a+b+c)//2
    area = (p * (p - a) * (p - b) * (p - c)) ** 0.5
    print(area)

a, b, c = map(int, input().split())
herão(a,b,c)