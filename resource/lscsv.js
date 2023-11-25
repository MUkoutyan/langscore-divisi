//==========LSCSV==========

class LSCSV
{

  constructor(){
  }

  to_map(file, file_name = '') 
  {
    // var file = open(Langscore.TRANSLATE_FOLDER + "/" + file_name)
    // if (file === null) { return {} }

    //CRLFをLFに統一
    var replaced = file.replace(/\r\n/g, '\n');
    var header = this.fetch_header(replaced);

    var rows = this.parse_col(header, this.parse_row(replaced));
    this. varidate(file_name, header, rows);

    var row_index = Langscore.Support_Language.map(lang => header.indexOf(lang)).filter(i => 0<=i);

    //To Hash
    var result = new Map();
    //※ヘッダーと列数が一致しない行は除外
    rows.slice(1, rows.size).filter(row => header.length === row.length).forEach(function (r) 
    {
      var origin = r[0];
      var transhash = new Map();
      row_index.forEach(i => transhash[header[i]] = r[i]);
      result[origin] = transhash;
    })
    return result;
  }

  to_array_without_origin(file_name) {
    hash = this.to_map(file_name);
    return hash.values;
  }

  varidate(file_name, header, rows) {
    if (header === null) { throw "Invalid CSV Data" }

    var size = rows[0].length;
    var mismatch_cells = rows.filter(r => r.size !== size);
    if (!mismatch_cells) {
      console.log("Error! : Missmatch Num Cells : " + mismatch_cells.first);
      console.log("File : ", file_name, ", Header size : ", size, ", Languages : ", rows[0]);
      throw "Error! : Missmatch Num Cells : " + mismatch_cells.first;
    }
  }

  fetch_header(csv_text) {
    if (csv_text !== null && 0 < csv_text.length) {
      var splited = csv_text.split('\n');
      //2行無ければ無効
      if (splited.size < 2) { return null }

      var header = splited[0].split(',');
      return header.map(lang => lang.trim() );
    }

    return null;
  }

  parse_row(csv_text) {
    if (!csv_text) { return null; }
    return csv_text;
  }

  parse_col(header, rows) {
    if (rows === null) { return; }
    var result = [];
    var cols = [];
    var bracketed_dq = false;

    const add_col = function (col) {
      col = col.replace(/(\r\n|\n|\r)$/, "");  //末尾に改行があれば削除
      cols.push(col);
    }

    const read_and_poeek_next_char = function(i)
    {
      if(rows.length <= (i+1)){ return ""; }
      return rows[i+1];
    };

    var col = "";
    for (var i = 0; i < rows.length; ++i) 
    {
      var c = rows[i];
      if (c === "\"") 
      {
        var next_char = read_and_poeek_next_char(i);
        if(next_char === ""){ break; }

        if(bracketed_dq === false && col.length === 0){
          bracketed_dq = true;
          continue;
        }
        else if(next_char === "\""){
          i += 1;
          col += next_char;
        }

        if(bracketed_dq && (next_char === "," || next_char === "\r" || next_char === "\n"))
        {
          bracketed_dq = false;
        }

        continue;
      }

      if (bracketed_dq) {
        //""内なら無条件で追加
        col += c;
        continue;
      }

      //以下は""で括られていない場合に通る
      if (c === ",") {
        bracketed_dq = false;
        add_col(col);
        col = "";
      }
      else if (c === "\n") {
        bracketed_dq = false;
        col += c //一旦改行を追加。add_col内のchompが適用できるようにする。
        add_col(col);
        col = "";

        //念のため、行中のセルがヘッダーと一致しない場合に空セルで埋める。
        //最終列が空の場合で該当する。
        //埋めないとvaridateで弾かれる。
        if (cols.size < header.size) {
          cols.fill("", cols.size, header.size - cols.size);
        }
        result.push(cols);
        cols = [];
      }
      else {
        col += c;
      }

    }

    //最終行の行末がEOFの場合colに内容が残りっぱなしになるので、ここで確認する。
    if (col.length !== 0) {
      add_col(col);
    }

    if (cols) {
      //最終行の行末がnlではなくEOFの場合に、この条件に引っかかる
      if (cols.size < header.size) {
        cols.fill("", cols.size, header.size - cols.size);
      }
      result.push(cols);
    }

    return result;
  }
}
const _lscsv = new LSCSV();
//==========LSCSV==========