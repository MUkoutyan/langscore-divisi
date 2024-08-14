#==========LSCSV==========
class LsDumpData
  attr_accessor :data
end
class LSCSV

  $lscsv_resource_locker = Mutex::new
  def self.to_hash(file_name)
    $lscsv_resource_locker.lock
    file = open(Langscore::TRANSLATE_FOLDER + "/" + file_name)
    $lscsv_resource_locker.unlock

    return from_content(file, file_name)
  end

  def self.from_content(content, file_name = '')

    return {} if content == nil

    # VXAceでは\r\nと\nが混在するため、改行コードは統一しない。

    header = fetch_header(content)
    rows = parse_col(header, parse_row(content))
    varidate(file_name, header, rows)

    row_index = [*1..header.size].select! do |i|
      Langscore::SUPPORT_LANGUAGE.include?(header[i])
    end

    #To Hash
    result = {}
    rows[1...rows.size].each do |r|
      # origin = r[0].chomp("\n")
      origin = r[0]
      transhash = {}
      row_index.each do |i|
        transhash[header[i]] = r[i]
      end
      result[origin] = transhash
    end
    result
  end
  
  def self.to_array_without_origin(file_name)
    hash = to_hash(file_name)
    return hash.values
  end

  def self.varidate(file_name, header, rows)
    raise "Invalid CSV Data" if header.nil?
    size = rows[0].size
    mismatch_cells = rows.select{ |r| r.size != size }
    if mismatch_cells.empty? == false
      ls_output_log "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
      ls_output_log "File : #{file_name}, Header size : #{size}, Languages : #{rows[0]}"
      raise "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
    end
  end

  def self.open(name)
    result = nil
    begin
      file_name = name+".rvdata2"
      trans_file = load_data(file_name)
      if trans_file.class == LsDumpData
        ls_output_log "load_data #{file_name}"
        result = trans_file.data
      end
    rescue => e
      begin

        file_name = name+".csv"
        trans_file = File.open(file_name, "rb:utf-8:utf-8")
        ls_output_log "open #{file_name}"
        result = trans_file.read
      rescue
        ls_output_log "Warning : Not Found Transcript File #{file_name}"
      end
    end

    return result
  end

  def self.fetch_header(csv_text)
    if !csv_text.nil? && 0<csv_text.length
      splited = csv_text.split("\n", 2)
      #2行無ければ無効
      return nil if splited.size < 2

      header = splited[0].split(',')
      header.map!{|lang| lang.chomp }
      return header
    end

    return nil
  end

  def self.parse_row(csv_text)
    return nil if csv_text.length == 0
    return csv_text
  end

  def self.parse_col(header, rows)
    return if rows == nil
    result = []
    cols = []
    bracketed_dq = false

    add_col = lambda do |col|
      col.chomp!  #末尾に改行があれば削除
      cols.push(col)
    end

    read_and_poeek_next_char = lambda do |i|
      return "" if rows.length <= (i+1)
      return rows[i+1];
    end

    col = ""
    str_length = rows.length
    str_index = 0
    while str_index < str_length

      c = rows[str_index]

      if c == "\""
        next_char = read_and_poeek_next_char.call(str_index)
        break if next_char == ""

        if bracketed_dq == false && col.length == 0
          bracketed_dq = true;
          str_index += 1
          next;
        elsif next_char == "\""
          str_index += 1;
          col += next_char;
        end

        if bracketed_dq && (next_char == "," || next_char == "\r" || next_char == "\n")
          bracketed_dq = false;
        end

        str_index += 1
        next
      end

      if bracketed_dq
        #""内なら無条件で追加
        col += c
        str_index += 1
        next
      end

      #以下は""で括られていない場合に通る
      if c==","
        bracketed_dq = false
        add_col.call(col)
        col = ""
      elsif c=="\n"
        bracketed_dq = false
        col += c #一旦改行を追加。add_col内のchompが適用できるようにする。
        add_col.call(col)
        col = ""

        #念のため、行中のセルがヘッダーと一致しない場合に空セルで埋める。
        #最終列が空の場合で該当する。
        #埋めないとvaridateで弾かれる。
        if cols.size < header.size          
          cols.fill("", cols.size, header.size-cols.size)
        end
        result.push(cols)
        cols = []
      else
        col += c
      end

      str_index += 1

    end

    #最終行の行末がEOFの場合colに内容が残りっぱなしになるので、ここで確認する。
    if col.empty? == false
      add_col.call(col)
    end

    if cols.empty? == false
      #最終行の行末がnlではなくEOFの場合に、この条件に引っかかる
      if cols.size < header.size          
        cols.fill("", cols.size, header.size-cols.size)
      end
      result.push(cols)
    end

    result
  end
end
#==========LSCSV==========