import fitz
import pandas as pd
from collections import defaultdict
import re

MAIN_HEADERS = {'act', 'heat', 'dose', 'inh', 'clear'}
FOCUS_HEADERS = {'Act', 'Heat'}

def find_page(doc, element):
    for page_number, page in enumerate(doc):
        text = page.get_text('text').strip()
        if text.startswith(element):
            return page_number
        
def extract_word_data(words):
    return [
        {'x0': w[0], 'y0': w[1], 'x1': w[2], 'y1': w[3], 'text': w[4].strip()}
        for w in words
        ]
        
def group_into_rows(text_with_coords):
    rows_dict = defaultdict(list)
    for word in text_with_coords:
        y_key = round(word['y0'] / 2) * 2
        rows_dict[y_key].append(word)

    row_ys = sorted(rows_dict.keys())
    return rows_dict, row_ys

def populate_rows(rows_dict, row_ys):
    rows = []
    rows_coords = []
    for y in row_ys:
        row_words = sorted(rows_dict[y], key=lambda it: it['x0'])
        rows.append([rw['text'] for rw in row_words])
        rows_coords.append([
            (rw['x0'], rw['x1'], rw['text']) for rw in row_words
            ])
        
    return rows, rows_coords

def identify_all_headers(rows_coords, row_ys):
    header_indices = []
    for i, coords in enumerate(rows_coords):
        for (x0, x1, txt) in coords:
            if txt.lower() in MAIN_HEADERS:
                header_indices.append((i, txt.lower(), x0, row_ys[i]))

    return header_indices
            
def group_headers_by_row(header_indices):
    headers_by_row = defaultdict(list)
    for idx, txt, x0, y in header_indices:
        headers_by_row[idx].append((txt, x0, y))

    return headers_by_row

def determine_end_row(rows, sorted_row_indices, row_idx):
    next_row_idx = None
    for next_idx in sorted_row_indices:
        if next_idx > row_idx:
            next_row_idx = next_idx
            break
    
    return next_row_idx if next_row_idx is not None else len(rows)

def table_segments(headers_by_row, rows, page, margin=10):
    table_segments = []
    sorted_row_indices = sorted(headers_by_row.keys())
    
    for row_idx in sorted_row_indices:
        headers_in_row = sorted(headers_by_row[row_idx], key=lambda h: h[1])
        end_idx = determine_end_row(rows, sorted_row_indices, row_idx)
        
        # Create a table "segment" for each table with a header in the row
        for h_pos, (keyword, hx, hy) in enumerate(headers_in_row):
            # Determine horizontal bounds for this table
            left_x = hx
            if h_pos + 1 < len(headers_in_row):
                right_x = headers_in_row[h_pos + 1][1] - margin
            else:
                right_x = page.rect.width
            
            table_segments.append({
                'keyword': keyword.capitalize(),
                'row start': row_idx,
                'row end': end_idx,
                'x start': left_x,
                'x end': right_x
            })

    return table_segments

def filter_by_horizontal_bounds(seg, rows_coords, margin=10):
    seg_rows_coords = []
    for row_coords in rows_coords[seg['row start'] : seg['row end']]:
        filtered_row = []
        for (x0, x1, txt) in row_coords:
            # Include word if its center falls within bounds
            center = (x0 + x1) / 2
            if (seg['x start'] - margin) <= center <= (seg['x end'] + margin):
                filtered_row.append((x0, x1, txt))
        if filtered_row:
            seg_rows_coords.append(filtered_row)

    return seg_rows_coords

def find_header_rows(seg_rows_coords, check_rows=3):
    header_end_idx=1
    time_re = re.compile(
        r'^\d+(\.\d+)?(\^?\d+)?(s|m|d|y|yr|yrs?)$',
        re.IGNORECASE
        )
    
    seg_rows_text = [[txt for (_, _, txt) in row] for row in seg_rows_coords]
    for i in range(min(check_rows, len(seg_rows_text))):
        time_matches = sum(
            1 for x in seg_rows_text[i] if time_re.match(x.strip())
            )
        if time_matches >= 2:
            header_end_idx = i + 1

    return header_end_idx

def find_x_positions(seg_rows_coords, margin=10):
    x_positions = []
    for row in seg_rows_coords[:min(margin, len(seg_rows_coords))]:
        for (x0, x1, txt) in row:
            x_positions.append(x0)

    return sorted(set(x_positions))

def find_column_starts(x_positions, col_gap_threshold=20):
    col_starts = []
    if x_positions:
        col_starts = [x_positions[0]]
        for x in x_positions[1:]:
            if x - col_starts[-1] > col_gap_threshold:
                col_starts.append(x)

    return col_starts

def create_column_boundaries(col_starts, x_start, x_end, margin=5):
    col_boundaries = []
    for i, start in enumerate(col_starts):
        if i < len(col_starts) - 1:
            end = col_starts[i + 1]
        else:
            end = x_end
        col_boundaries.append((start - margin, end))
    
    if not col_boundaries:
        col_boundaries = [(x_start, x_end)]

    return col_boundaries

def extract_headers(col_boundaries, header_end_idx, seg_rows_coords):
    headers = []
    for col_idx in range(len(col_boundaries)):
        header_parts = []
        for h_idx in range(header_end_idx):
            row_coords = seg_rows_coords[h_idx]
            for (x0, x1, txt) in row_coords:
                cl, cr = col_boundaries[col_idx]
                center = (x0 + x1) / 2
                if cl <= center < cr:
                    header_parts.append(txt.strip())

        header_text = (
            ' '.join(header_parts) if header_parts else f'col_{col_idx}'
            )
        headers.append(header_text)

    return headers

def rows_to_table(col_boundaries, header_end_idx, seg_rows_coords):
    table_rows = []
    for row_coords in seg_rows_coords[header_end_idx:]:
        if not row_coords:
            continue
            
        cells = [''] * len(col_boundaries)
        for (x0, x1, txt) in row_coords:
            center = (x0 + x1) / 2
            # Find which column this word belongs to
            best_col = None
            for ci, (cl, cr) in enumerate(col_boundaries):
                if cl <= center < cr:
                    best_col = ci
                    break
            
            if best_col is not None:
                if cells[best_col]:
                    cells[best_col] += ' ' + txt.strip()
                else:
                    cells[best_col] = txt.strip()
        
        # Only add row if it has some content
        if any(c.strip() for c in cells):
            table_rows.append(cells)

    return table_rows

def build_dataframe(table_rows, headers):
    if table_rows:
        df = pd.DataFrame(table_rows, columns=headers)
        mask = df.apply(
            lambda r: r.astype(str).str.strip().eq('').all(), axis=1
            )
        df = df.loc[~mask].reset_index(drop=True)
    else:
        df = pd.DataFrame(columns=headers)

    return df

def refine_df(df, keyword):
    df.rename(columns={'col_6' : '10000 y'}, inplace=True)
    numerator, denominator = list(df[keyword][:2])[::-1]
    units = f'{numerator}/{denominator.split('−')[0]}'
    df.drop(0, axis=0, inplace=True)
    df.drop(len(df), axis=0, inplace=True)
    df.loc[1, keyword] = f'Total ({units})'
    df.replace('', 0.0, inplace=True)
    df = df.astype({col : float for col in df.columns[1:]})

    return df, units

def extract_tables(pdf_path, element):

    # Read in Fispact-II Manual, find and read page for selected element
    doc = fitz.open(pdf_path)
    page_number = find_page(doc, element)
    page = doc[page_number]
    words = page.get_text('words')
    
    # Identify and read cooling data from page
    text_with_coords = extract_word_data(words)
    rows_dict, row_ys = group_into_rows(text_with_coords)
    rows, rows_coords = populate_rows(rows_dict, row_ys)
    header_indices = identify_all_headers(rows_coords, row_ys)
    if not header_indices:
        return []

    headers_by_row = group_headers_by_row(header_indices)

    # Iterate through each table to collect and store its data
    results = []
    for seg in table_segments(headers_by_row, rows, page):
        seg_rows_coords = filter_by_horizontal_bounds(seg, rows_coords)
        if not seg_rows_coords:
            continue

        header_end_idx = find_header_rows(seg_rows_coords)
        x_positions = find_x_positions(seg_rows_coords) 
        col_starts = find_column_starts(x_positions)        
        col_boundaries = create_column_boundaries(
            col_starts, seg['x start'], seg['x end']
        )
        headers = extract_headers(
            col_boundaries, header_end_idx, seg_rows_coords
        )
        table = rows_to_table(
            col_boundaries, header_end_idx, seg_rows_coords
        )

        if seg['keyword'] in FOCUS_HEADERS:
            # Write out tabular data to a Pandas DataFrame
            df = build_dataframe(table, headers)
            df, units = refine_df(df, seg['keyword'])

            results.append({
                'Keyword': seg['keyword'],
                'Units'  : units,
                'Data'   : df
            })

    return results

def extract_fispact_totals(df):
    return df.iloc[0,1:].astype(float).to_numpy()