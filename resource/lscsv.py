import re
from collections import defaultdict

class LSCSV:

    def __init__(self):
        pass

    def to_map(self, file, file_name=''):
        # CRLFをLFに統一
        replaced = file.replace('\r\n', '\n')
        header = self.fetch_header(replaced)

        rows = self.parse_col(header, self.parse_row(replaced))
        self.validate(file_name, header, rows)

        row_index = [header.index(lang) for lang in Langscore.Support_Language if lang in header]

        # To Dictionary
        result = {}
        # ヘッダーと列数が一致しない行は除外
        for r in rows[1:]:
            if len(header) == len(r):
                origin = r[0]
                transhash = {header[i]: r[i] for i in row_index}
                result[origin] = transhash

        return result

    def to_array_without_origin(self, file_name):
        hash_map = self.to_map(file_name)
        return list(hash_map.values())

    def validate(self, file_name, header, rows):
        if header is None:
            raise ValueError("Invalid CSV Data")

        size = len(rows[0])
        mismatch_cells = [r for r in rows if len(r) != size]
        if mismatch_cells:
            print(f"Error! : Mismatch Num Cells : {mismatch_cells[0]}")
            print(f"File : {file_name}, Header size : {size}, Languages : {rows[0]}")
            raise ValueError(f"Error! : Mismatch Num Cells : {mismatch_cells[0]}")

    def fetch_header(self, csv_text):
        if csv_text and len(csv_text) > 0:
            splited = csv_text.split('\n')
            # 2行無ければ無効
            if len(splited) < 2:
                return None

            header = splited[0].split(',')
            return [lang.strip() for lang in header]

        return None

    def parse_row(self, csv_text):
        if not csv_text:
            return None
        return csv_text

    def parse_col(self, header, rows):
        if rows is None:
            return None

        result = []
        cols = []
        bracketed_dq = False

        def add_col(col):
            col = re.sub(r'(\r\n|\n|\r)$', "", col)  # 末尾に改行があれば削除
            cols.append(col)

        def read_and_peek_next_char(i):
            if len(rows) <= (i + 1):
                return ""
            return rows[i + 1]

        col = ""
        i = 0
        while i < len(rows):
            c = rows[i]
            if c == "\"":
                next_char = read_and_peek_next_char(i)
                if next_char == "":
                    break

                if not bracketed_dq and len(col) == 0:
                    bracketed_dq = True
                    i += 1
                    continue
                elif next_char == "\"":
                    i += 1
                    col += next_char

                if bracketed_dq and next_char in {",", "\r", "\n"}:
                    bracketed_dq = False

                i += 1
                continue

            if bracketed_dq:
                # ""内なら無条件で追加
                col += c
                i += 1
                continue

            # 以下は""で括られていない場合に通る
            if c == ",":
                bracketed_dq = False
                add_col(col)
                col = ""
            elif c == "\n":
                bracketed_dq = False
                col += c  # 一旦改行を追加。add_col内のchompが適用できるようにする。
                add_col(col)
                col = ""

                # 念のため、行中のセルがヘッダーと一致しない場合に空セルで埋める。
                # 最終列が空の場合で該当する。
                # 埋めないとvalidateで弾かれる。
                if len(cols) < len(header):
                    cols.extend([""] * (len(header) - len(cols)))
                result.append(cols)
                cols = []
            else:
                col += c

            i += 1

        # 最終行の行末がEOFの場合colに内容が残りっぱなしになるので、ここで確認する。
        if len(col) != 0:
            add_col(col)

        if cols:
            # 最終行の行末がnlではなくEOFの場合に、この条件に引っかかる
            if len(cols) < len(header):
                cols.extend([""] * (len(header) - len(cols)))
            result.append(cols)

        return result

# サポートされる言語リストを定義
# テスト用のスクリプトなので、LSCSV側でLangscoreのダミーを実装。
class Langscore:
    Support_Language = ["ja", "en"]
