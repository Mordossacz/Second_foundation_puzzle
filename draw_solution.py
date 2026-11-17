from turtle import *
import re
from pathlib import Path

SIZE = 30
COLORS = ["black", "red", "blue", "green", "yellow", "orange"]

def rys_trojkat(trojkat, wartosc):
    p1, p2, p3 = trojkat

    idz_do(p1)
    if p1[1] > p2[1]:
        rt(60)

    fillcolor(COLORS[wartosc])
    begin_fill()
    for i in range(3):
        fd(SIZE)
        lt(120)
    end_fill()

    if p1[1] > p2[1]:
        lt(60)
    wroc_z(p1)

def rys_punkt(punkt, col):
    idz_do(punkt)

    fillcolor(col)
    begin_fill()
    pu()
    bk(5)
    rt(90)
    circle(5)
    lt(90)
    fd(5)
    pd()

    end_fill()

    wroc_z(punkt)

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

def klucz_naturalny(sciezka: Path):
    # Dzieli nazwę na fragmenty tekstowe i liczbowe (np. ['costam_', 10, '.txt'])
    return [int(fragment) if fragment.isdigit() else fragment 
            for fragment in re.split(r'(\d+)', sciezka.name)]

def parsuj_zbiory(linie_sekcji: list[str]) -> list[set]:
    """Pomocnicza funkcja zamieniająca linie ze zbiorami (np. '{1, 2}, {3, 4},') na listę obiektów set."""
    wynik = []
    for linia in linie_sekcji:
        oczyszczona = linia.rstrip(",").strip()
        if oczyszczona:
            zbiory = ast.literal_eval(f"[{oczyszczona}]")
            wynik.extend(zbiory)
    return wynik


def parsuj_plik(sciezka: Path):
    with sciezka.open("r", encoding="utf-8") as plik:
        linie = [linia.strip() for linia in plik if linia.strip()]

    # Szukamy indeksów obu separatorów
    try:
        idx_start = linie.index("startings")
        idx_stop = linie.index("endings")
    except ValueError as e:
        print(f"Błąd struktury w pliku {sciezka.name}: brak wymaganej sekcji ({e})")
        return None

    if idx_start >= idx_stop:
        print(f"Błąd w pliku {sciezka.name}: sekcja 'punkty_start' musi być przed 'punkty_stop'")
        return None

    # 1. Sekcja pierwsza: trójkąty i ich wartość
    trojkaty = []
    for linia in linie[:idx_start]:
        wspolrzedne_str, wartosc_str = linia.rsplit(",", 1)
        wierzcholki = ast.literal_eval(wspolrzedne_str.strip())  # list[tuple[int, int]]
        wartosc = int(wartosc_str.strip())                      # int
        trojkaty.append((wierzcholki, wartosc))

    # 2. Sekcja druga: punkty_start
    punkty_start = parsuj_zbiory(linie[idx_start + 1 : idx_stop])

    # 3. Sekcja trzecia: punkty_stop
    punkty_stop = parsuj_zbiory(linie[idx_stop + 1 :])

    return trojkaty, punkty_start, punkty_stop

import ast
folder = Path(".")

solve_id = input()

tracer(0)
pu()
bk(700)
lt(90)
fd(400)
rt(90)
pd()
ile_solve_w_kolumnie = 0
MAX_SOLVE_KOL = 6
SOLVE_SIZE = 150

for sciezka in sorted(folder.glob("solve_for_" + solve_id + "_*"), key=klucz_naturalny):
    # Upewniamy się, że to plik, a nie folder o takiej nazwie
    if sciezka.is_file():
        dane = []
        tracer(0)
        
        with sciezka.open("r", encoding="utf-8") as plik:
            print(f"=== Wczytuję: {sciezka.name} ===")
            dane = parsuj_plik(sciezka)

            if dane:
                trojkaty, punkty_start, punkty_stop = dane
                # print(f"Trójkąty ({len(trojkaty)}):", trojkaty)
                # print("Punkty start:", punkty_start)
                # print("Punkty stop:", punkty_stop)
                # print()

                for trojkat, wartosc in trojkaty:
                    rys_trojkat(trojkat, wartosc)


                    print(f"T: {trojkat} | {wartosc}")
                
                for p in punkty_start:
                    rys_punkt(p, "red")
                    print(f"start: {p}")

                for p in punkty_stop:
                    rys_punkt(p, "blue")
                    print(f"stop: {p}")

        update()
        pu()
        rt(90)
        fd(SOLVE_SIZE)
        lt(90)
        pd()
        ile_solve_w_kolumnie += 1
        if MAX_SOLVE_KOL <= ile_solve_w_kolumnie:
            pu()
            fd(SOLVE_SIZE)
            lt(90)
            fd(SOLVE_SIZE * MAX_SOLVE_KOL)
            rt(90)
            pd()
            ile_solve_w_kolumnie = 0
        # resetscreen()

input()