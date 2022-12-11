
class LsDumpData
  attr_accessor :data
end
class LSCSV
  
  def self.to_hash(file_name)
    file = open(Langscore::TRANSLATE_FOLDER + "/" + file_name)
    return {} if file == nil

    rows = parse_col(parse_row(file))
    varidate(file_name, rows)

    #To Hash
    row_index = (1...@@header.size).select do |i|
      Langscore::SUPPORT_LANGUAGE.include?(@@header[i])
    end

    result = {}
    rows[1...rows.size].each do |r|
      origin = r[0]
      trans  = r[1...@@header.size]

      transhash = {}
      row_index.each do |i|
        transhash[@@header[i]] = r[i]
      end

      result[origin] = transhash
    end
    result
  end
  
  def self.to_array_without_origin(file_name)
    hash = to_hash(file_name)
    return hash.values
  end

  def self.varidate(file_name, rows)
    raise "Invalid CSV Data" if @@header.nil?
    size = rows[0].size
    mismatch_cells = rows.select{ |r| r.size != size }
    if mismatch_cells.empty? == false
      p "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
      p "File : #{file_name}, Header size : #{size}, Languages : #{rows[0]}"
      raise "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
    end
  end

  def self.open(name)
    result = nil
    begin
      file_name = name+".rvdata2"
      #p "try load_data(#{file_name})"
      trans_file = load_data(file_name)
      if trans_file.class == LsDumpData
        #p "OK"
        # result = trans_file.data.split(/(?<=[\n])\s*/)
        result = trans_file.data
      end
    rescue => e
      begin
        #p "failed load_data(#{file_name}) : #{e}"

        file_name = name+".csv"
        trans_file = File.open(file_name, "rb:utf-8:utf-8")
        #p "OK"
        result = trans_file.read
      rescue
        p "Warning : Not Found Transcript File #{file_name}"
      end
    end

    if !result.nil? && 0<result.length
      splited = result.split("\n", 2)
      #2行無ければ無効
      return nil if splited.size < 2

      @@header = splited[0].split(',')
    end

    return result
  end

  def self.parse_row(csv_text)
    return nil if csv_text.length == 0
    return csv_text
  end

  def self.parse_col(rows)
    return if rows == nil
    result = []
    cols = []
    find_quote = false

    add_col = lambda do |col|
      #セルに分けた時点で先頭・末尾の"が不要になるため、削除する
      if col.start_with?("\"") && col.end_with?("\"")
        col = col.slice!(1..col.length-2)
      end

      #"を表すための""はCSVでのみ必須のため、読み込み時点で""は"と解釈。
      col.gsub!("\"\"", "\"") 

      #各文章は\nで終わるのがVXAceの仕様っぽいので、
      #末尾に\nが無ければ追加。ただし、ヘッダー行に当たる1行目は除く。
      if 1 < result.size
        if col.end_with?("\n") == false
          col += "\n"
        end
      end
      cols.push(col)
    end

    col = ""
    rows.each_char do |c|

      if c=="\""
        find_quote = !find_quote
      end

      if find_quote
        #""内なら無条件で追加
        col += c
        next
      end

      #以下は""で括られていない場合に通る
      if c==","
        find_quote = false
        add_col.call(col)
        col = ""
      elsif c=="\n"
        find_quote = false
        add_col.call(col)
        col = ""

        #念のため、行中のセルがヘッダーと一致しない場合に空セルで埋める。
        #最終列が空の場合で該当する。
        #埋めないとvaridateで弾かれる。
        if cols.size < @@header.size          
          cols.fill("", cols.size, @@header.size-cols.size)
        end
        result.push(cols)
        cols = []
      else
        col += c
      end

    end

    #最終行の行末がEOFの場合colに内容が残りっぱなしになるので、ここで確認する。
    if col.empty? == false
      add_col.call(col)
    end

    if cols.empty? == false
      #最終行の行末がnlではなくEOFの場合に、この条件に引っかかる
      if cols.size < @@header.size          
        cols.fill("", cols.size, @@header.size-cols.size)
      end
      result.push(cols)
    end

    result
  end
end
