
class LSCSV

  def self.to_hash(file_name)
    file = open(Langscore::TRANSLATE_FOLDER + "/" + file_name)
    return {} if file == nil

    rows = parse_col(parse_row(file))

    varidate(file_name, rows)

    #To Hash
    header = rows[0]
    row_index = (1...header.size).select do |i|
      Langscore::SUPPORT_LANGUAGE.include?(header[i])
    end

 
    #改行コードを全てRGSS側の\r\nに統一
    result = {}
    rows[1...rows.size].each do |r|
      origin = r[0]
      trans  = r[1...header.size]

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

  def self.varidate(file_name, rows)
    size = rows[0].size
    mismatch_cells = rows.select{ |r| r.size != size }
    if mismatch_cells.empty? == false
      p "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
      p "File : #{file_name}, Header size : #{size}, Languages : #{rows[0]}"
      raise "Error! : Missmatch Num Cells : #{mismatch_cells.first}" 
    end
  end

  def self.open(file_name)
    begin
      trans_file = load_data(file_name)
    rescue
      begin
        trans_file = File.open(file_name)
      rescue
        p "Warning : Not Found Transcript File #{file_name}"
        return nil
      end
    end
    trans_file
  end

  def self.parse_row(file)
    return if file == nil
    csv_text = file.readlines()
    lines = []
    line_buffer = ""
    csv_text.each do |l|
      dq_count = l.count('\"');

      if line_buffer != ""
        dq_count += 1
      end

      if (dq_count%2) == 0
        line_buffer += l
        lines.push(line_buffer)
        line_buffer = ""
      else
        line_buffer += l
      end
    end

    return lines
  end

  def self.parse_col(rows)
    return if rows == nil
    result = []
    cols = []
    find_quote = false

    add_col = lambda do |col|
      if col.start_with?("\"") && col.end_with?("\"")
        col = col.slice!(1..col.length-2)
      end
      col.gsub!("\n\n", "\r\n")
      cols.push(col)
    end

    rows.each do |r|
      col = ""
      r.each_char do |c|

        if c=="\""
          find_quote = !find_quote
        end

        if find_quote
          col += c
          next
        end

        if c==","
          find_quote = false
          add_col.call(col)
          col = ""
          next
        elsif c=="\n"
          find_quote = false
          break
        end

        col += c
      end

      add_col.call(col)

      result.push(cols)
      cols = []

    end
    if cols.empty? == false
      result.push(cols)
    end

    result
  end
end
