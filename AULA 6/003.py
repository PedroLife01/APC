def confere(m1,a1,m2,a2):
    if a1<a2:
        return False
    elif a1>a2:
        return True
    else:
        if m1<=m2:
            return False
        else:
            return True

