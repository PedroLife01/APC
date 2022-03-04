def collatz(s, n, cont):
    if cont < n:
        if s % 2 != 0:
            clltz = int(3*s - 1)
            return collatz(clltz, n, cont+1)
        else:
            clltz = int(s/2)
            return collatz(clltz, n, cont+1)
    else:
        return s