import xlwings as xw


def result(base, delta):
    """Принимает 2 массива: с данными х,у, другой - диапазн у, и перобразует 
    неупорядоченный массив x,y в упорядоченный 

    Returns:
        Упорядоченный массив
    """
    out = [[base[i][0]] for i in range(len(base))]
    base[0].append(0)
    for i2 in range(1, len(base)):
        h = 1
        for i in delta:
            if base[0][h + 1] == 0: break
            if i > base[0][h + 1]: h += 1
            if base[0][h] <= i <= base[0][h + 1]:
                '''
                y = y0+ y'* x
                '''
                dx = i - base[0][h]
                dfdx = (base[i2][h + 1] - base[i2][h]) / (base[0][h + 1] -
                                                          base[0][h])
                f0 = base[i2][h]
                a = dfdx * dx + f0
                if i2 == 1: out[0].append(i)
                out[i2].append(a)
    return out


# app = xw.App(visible=True)
wb = xw.Book.caller()
sh = wb.sheets.active()

# values cells of Temp. rows. All, Fluid,Solid

temp_row = [34, 66]
frame = [[], []]

# Creating temp. diapason
# ? n or n+1

start_temp = int(sh.range('B1').value)
end_temp = int(sh.range('C1').value)
number_temp = sh.range('E1').value
step = int((end_temp - start_temp) // number_temp)
diap = [i for i in range(start_temp, end_temp, step)]

# Finding of maximum of tables

for temp_i in temp_row:

    max_row = temp_i + 1
    while sh.cells(max_row, 'A').value != 0.0:
        max_row += 1

    max_column = 2
    while sh.range((temp_i, max_column)).value != 0.0:
        max_column += 1
    '''
    Writing
    '''

    frame = sh.range((temp_i, 1), (max_row - 1, max_column - 1)).value
    res = result(frame, diap)
    sh.range(66 + temp_i, 1).value = res
wb.save()
