import xlwings as xw


def result(base, delta):
    """
    Создание массива значений. Приведение массива M(x,f), где f - совокупность функций от х, к заданному диапазону х, из другого массива.
    Используется метод линейной интерполяции

    base: (List)  Основная матрица значений

    delta: (List) Диапазон значений х, любой

    return: (List)  Измененная интерполяцией матрица данных

    """

    out = [[base[i][0]] for i in range(len(base))]
    base[0].append(0)
    for i2 in range(1, len(base)):
        h = 1
        for i in delta:
            if base[0][h + 1] == 0:
                break
            if i > base[0][h + 1]:
                h += 1
            if base[0][h] <= i <= base[0][h + 1]:

                # Упрощение функции
                # f = f0+ dfdx*dx

                dx = i - base[0][h]
                dfdx = (base[i2][h + 1] - base[i2][h]) / (base[0][h + 1] -
                                                          base[0][h])
                f0 = base[i2][h]
                a = dfdx * dx + f0
                if i2 == 1:
                    out[0].append(i)
                out[i2].append(a)
    return out


wb = xw.Book
sh = xw.sheets.active

# Значения номеров строк температурных интервалов, задающих матрицу значений

temp_rows = [34, 66]
frame = [[], []]

# Создание диапазона

start_temp = int(sh.range('B1').value)
end_temp = int(sh.range('C1').value)
number_temp = sh.range('E1').value
step = int((end_temp - start_temp) // number_temp)
diap = [i for i in range(start_temp, end_temp+1, step)]

# Нахождение длины
for temp_i in temp_rows:

    max_row = temp_i + 1
    while sh.cells(max_row, 'A').value != 0.0:
        max_row += 1

    max_column = 2
    while sh.range((temp_i, max_column)).value != 0.0:
        max_column += 1

    # Запись диапазона

    frame = sh.range((temp_i, 1), (max_row - 1, max_column - 1)).value
    res = result(frame, diap)
    sh.range(66 + temp_i, 1).value = res
wb.save
