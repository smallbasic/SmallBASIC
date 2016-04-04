'
' Base64 encoding and decoding
' see: https://tools.ietf.org/html/rfc4648
'

Unit Base64
Export encode

const alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
const padding="="

dim index(128)
for i = 0 to 63
  index(asc(mid(alphabet, i + 1, 1))) = i
next i
index(asc(padding)) = 0

func encode(message)
  local result=""
  local in_len = len(message)
  local offset = 1
  local group, group_len
  local b_a, b_b, b_c

  while offset <= in_len
    group_len = min(3, (in_len - offset + 1))
    select case group_len
    case 1
      ' final unit of encoded output will be two characters followed by two "=" padding characters.
      b_a = mid(message, offset, 1)
      result += mid(alphabet, (asc(b_a) rshift 2) + 1, 1)
      result += mid(alphabet, ((asc(b_a) band 0x3) lshift 4) + 1, 1)
      result += padding
      result += padding
    case 2
      ' final unit of encoded output will be three characters followed by one "=" padding character.
      b_a = mid(message, offset, 1)
      b_b = mid(message, offset + 1, 1)
      result += mid(alphabet, (asc(b_a) rshift 2) + 1, 1)
      result += mid(alphabet, (((asc(b_a) band 0x3) lshift 4) + (asc(b_b) rshift 4)) + 1, 1)
      result += mid(alphabet, ((asc(b_b) band 0xf) lshift 2) + 1, 1)
      result += padding
    case else
      b_a = mid(message, offset, 1)
      b_b = mid(message, offset + 1, 1)
      b_c = mid(message, offset + 2, 1)
      result += mid(alphabet, (asc(b_a) rshift 2) + 1, 1)
      result += mid(alphabet, (((asc(b_a) band 0x3) lshift 4) + (asc(b_b) rshift 4)) + 1, 1)
      result += mid(alphabet, (((asc(b_b) band 0xf) lshift 2) + (asc(b_c) rshift 6)) + 1, 1)
      result += mid(alphabet, (asc(b_c) band 0x3f) + 1, 1)
    end select
    offset += group_len
  wend
  encode = result
end

func decode(message)
  local result = ""
  local in_len = len(message)
  local i, b_a, b_b, b_c, b_d

  if (in_len % 4 != 0) then
     throw "Invalid base64 message length"
  endif

  for i = 1 to in_len step 4
    b_a = index(asc(mid(message, i, 1)))
    b_b = index(asc(mid(message, i + 1, 1)))
    b_c = index(asc(mid(message, i + 2, 1)))
    b_d = index(asc(mid(message, i + 3, 1)))
    result += chr(((b_a lshift 2) + (b_b rshift 4)))
    result += chr(((b_b band 0xf) lshift 4) + (b_c rshift 2))
    result += chr(((b_c band 0x3) lshift 6) + b_d)
  next offset
  decode = result
end

sub test
  if (len(alphabet) != 64) then
    throw "Invalid alphaet length"
  fi
  if (encode("foobar") != "Zm9vYmFy") then
    throw "base64 encoding error 1"
  endif
  if (encode("This is a longer string") != "VGhpcyBpcyBhIGxvbmdlciBzdHJpbmc=") then
    throw "base64 encoding error 2"
  endif
  if (encode("This result will end with two equals.") != "VGhpcyByZXN1bHQgd2lsbCBlbmQgd2l0aCB0d28gZXF1YWxzLg==") then
    throw "base64 encoding error 3"
  endif
  if (decode("Zm9vYmFy") != "foobar") then
    throw "base64 encoding error 4"
  endif
  if (decode("VGhpcyBpcyBhIGxvbmdlciBzdHJpbmc=") != "This is a longer string") then
    throw "base64 encoding error 5"
  endif
  if (decode("VGhpcyByZXN1bHQgd2lsbCBlbmQgd2l0aCB0d28gZXF1YWxzLg==") != "This result will end with two equals.") then
    throw "base64 encoding error 6"
  endif
end

test

