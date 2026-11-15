from turtle import *

SIZE = 30
COLORS = ["black", "red", "blue", "green", "yellow", "orange"]
tracer(0)

def rys_trojkat(col):
    fillcolor(col)
    begin_fill()
    for i in range(3):
        fd(SIZE)
        lt(120)
    end_fill()

def idz_do(p):
    pu()
    fd(SIZE * p[0])
    lt(60)
    fd(SIZE * p[1])
    rt(60)
    pd()

def wroc_z(p):
    pu()
    fd(SIZE * p[0] * -1)
    lt(60)
    fd(SIZE * p[1] * -1)
    rt(60)
    pd()

import ast

dane = []

with open("test", "r", encoding="utf-8") as plik:
    for linia in plik:
        linia = linia.strip()
        if not linia:
            continue  # pominięcie pustych linii

        trojkat, wartosc = ast.literal_eval(linia)
        dane.append((trojkat, wartosc))

# Przykładowe odczytanie wczytanych danych:
for trojkat, wartosc in dane:
    p1, p2, p3 = trojkat

    idz_do(p1)
    if p1[1] > p2[1]:
        rt(60)

    rys_trojkat(COLORS[wartosc])
        
    if p1[1] > p2[1]:
        lt(60)
    wroc_z(p1)


    print(f"Punkty: {p1}, {p2}, {p3} | Wartość: {wartosc}")

update()