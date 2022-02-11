h, m, s = map(int, input().split(':'))
h = h*3600
m = m*60
s = s+h+m
print(f"La se foram {s} segundos que nao voltam mais...")