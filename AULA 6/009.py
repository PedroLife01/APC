def mdc(a,b):
    if a < b:
        return mdc(b,a)
    if b == 0:
        return a
    else:
        r = a%b
        return mdc(b,r)


