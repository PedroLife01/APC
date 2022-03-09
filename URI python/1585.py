def areaLosangolo(d1, d2):
    return (d1 * d2) / 2


n = int(input())

while n > 0:
    x, y = map(int, input().split())
    ans = int(areaLosangolo(x, y))
    print(f'{ans} cm2')
    n = n - 1
