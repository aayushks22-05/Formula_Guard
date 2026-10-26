import zipfile
import xml.etree.ElementTree as ET
import csv
import sys
import re

def col_letters_to_index(col_str):
    exp = 0
    col = 0
    for char in reversed(col_str.upper()):
        col += (ord(char) - ord('A') + 1) * (26 ** exp)
        exp += 1
    return col - 1

def index_to_col_letters(col_idx):
    res = []
    c = col_idx
    while c >= 0:
        res.append(chr(ord('A') + (c % 26)))
        c = (c // 26) - 1
    return "".join(reversed(res))

def parse_cell_ref(cell_ref):
    m = re.match(r"([A-Za-z]+)([0-9]+)", cell_ref)
    if m:
        return col_letters_to_index(m.group(1)), int(m.group(2))
    return None, None

def shift_formula(base_formula, d_col, d_row):
    def replace_ref(m):
        col_str = m.group(1)
        row_str = m.group(2)
        is_col_abs = col_str.startswith('$')
        is_row_abs = row_str.startswith('$')
        c_clean = col_str.replace('$', '')
        r_clean = int(row_str.replace('$', ''))
        c_idx = col_letters_to_index(c_clean)
        if not is_col_abs:
            c_idx += d_col
        if not is_row_abs:
            r_clean += d_row
        new_c = ("$" if is_col_abs else "") + index_to_col_letters(c_idx)
        new_r = ("$" if is_row_abs else "") + str(r_clean)
        return new_c + new_r

    pattern = r"(?<![A-Za-z0-9_])(\$?[A-Za-z]+)(\$?[0-9]+)(?!\()"
    return re.sub(pattern, replace_ref, base_formula)

def export_xlsx_formulas_to_csv(xlsx_path, csv_path):
    with zipfile.ZipFile(xlsx_path, 'r') as z:
        shared_strings = []
        if 'xl/sharedStrings.xml' in z.namelist():
            tree = ET.fromstring(z.read('xl/sharedStrings.xml'))
            ns = {'ns': 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}
            for si in tree.findall('.//ns:si', ns):
                t = si.find('ns:t', ns)
                shared_strings.append(t.text if t is not None and t.text else '')

        sheet_name = 'xl/worksheets/sheet1.xml'
        tree = ET.fromstring(z.read(sheet_name))
        ns = {'ns': 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}

        grid = {}
        max_r, max_c = 0, 0
        shared_masters = {}

        for c in tree.findall('.//ns:c', ns):
            r_str = c.attrib.get('r')
            if not r_str: continue
            col, row = parse_cell_ref(r_str)
            if col is None or row is None: continue

            if row > max_r: max_r = row
            if col > max_c: max_c = col

            f = c.find('ns:f', ns)
            v = c.find('ns:v', ns)

            if f is not None:
                t_attr = f.attrib.get('t')
                si = f.attrib.get('si')
                if f.text:
                    formula_text = f.text.strip()
                    grid[(row, col)] = "=" + formula_text
                    if t_attr == 'shared' and si:
                        shared_masters[si] = (formula_text, row, col)
                elif t_attr == 'shared' and si:
                    grid[(row, col)] = ('SHARED', si, row, col)
            elif v is not None and v.text:
                t_attr = c.attrib.get('t')
                val = v.text
                if t_attr == 's' and val.isdigit():
                    idx = int(val)
                    if idx < len(shared_strings):
                        val = shared_strings[idx]
                grid[(row, col)] = val
            else:
                grid[(row, col)] = ""

        for (r, col), val in list(grid.items()):
            if isinstance(val, tuple) and val[0] == 'SHARED':
                si = val[1]
                row = val[2]
                if si in shared_masters:
                    base_formula, base_row, base_col = shared_masters[si]
                    d_row = row - base_row
                    d_col = col - base_col
                    expanded_formula = "=" + shift_formula(base_formula, d_col, d_row)
                    grid[(row, col)] = expanded_formula
                else:
                    grid[(row, col)] = ""

    with open(csv_path, 'w', newline='', encoding='utf-8') as f_out:
        writer = csv.writer(f_out)
        for r in range(1, max_r + 1):
            row_vals = []
            for col in range(max_c + 1):
                row_vals.append(grid.get((r, col), ""))
            writer.writerow(row_vals)

if __name__ == '__main__':
    if len(sys.argv) > 2:
        export_xlsx_formulas_to_csv(sys.argv[1], sys.argv[2])
