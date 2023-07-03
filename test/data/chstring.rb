#Test Script

=begin
CommentArea
=end

TextList = {
  "あHoniい" => "𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"
}

"HoniHoni" #"HoniHoni"

class String
  def String(text)
    if TextList.has_key?(text)
      p "trans #{text}"
      return TextList[text]
    end
    text
  end

  def initialize_clone
    p "call initialize_clone"
    super
  end

  def initialize_copy
    p "call initialize_clone"
    super
  end

  def to_str
    p "call to_str"
    self
  end
end

s = "あいうえお"


# pp "Hoge".private_methods

p TextList["A"]
p "B"
p "A"

"翻訳テキスト".lstrans("chstring:12:2")
"翻訳テキスト2".lstrans'chstring:13:2'

'\C[16]プレイ時間\X[104]\C[0]\T[%4$3d\'%3$02d]'