'app-plug-in
'menu Calculator
'
' $Id: calc.bas,v 1.5 2005-05-11 23:30:14 zeeb90au Exp $
' calc.bas Copyright (c) Chris Warren-Smith April 2005
' Demonstration for html user interfaces
'
' Version 2.0
'
' This program is distributed under the terms of the GPL v2.0 or later
' Download the GNU Public License (GPL) from www.gnu.org
'
'
option predef grmode 390X290

out_x = 0
out_y = 0

func compute(byref operator, lval, rval)
  local result = 0
  if (operator = "+") then
    result = lval + rval
  elseif (operator = "-") then
    result = lval - rval
  elseif (operator = "/") then
    if (rval == 0) then
      rval = 1
    fi
    result = lval / rval
  elseif (operator = "*") then
    result = lval * rval
  elseif (lval > 0) then
    result = lval
  else 
    result = rval
  fi
  compute = result
end

' evaluate the expression string
func eval(byref expr)
  local strl,num,pt,result,operator,inner,endb,i,j,nbracket

  strl = len(expr)
  num = 0
  pt = 0
  result = 0
  operator = ""

  for i = 0 to strl
    if (i < strl)
      ch = mid(expr, i+1, 1)
    else
      ' end of last operand
      eval = compute(operator, result, num)
      exit func
    fi
    if ch = "." then
      pt = 1
    elseif isnumber(ch) then
      if pt > 0 then
        num = num + (ch/(10^pt))
        pt++
      else
        num = num*10+ch        
      fi
    else
      ' non number
      if ch = "(" then
        ' find the matching )
        if (len(operator) = 0) then
          ' short form notation eg 3(1+1)
          operator = "*"
          if (result = 0) then
            result = num
          else
            result = compute(operator, result, num)
          fi
        else
          ' complete the previous expression
          result = compute(operator, result, num)
        fi
        nbracket = 1
        endb = 0
        i++
        for j = i to strl-1
          ch = mid(expr, j+1, 1)
          if ch = "(" then
            nbracket++
          elseif ch = ")" then
            nbracket--
            if nbracket = 0 then
              endb = j
              exit for
            fi
          fi
        next j

        if (endb = 0) then
          eval = "ERR"
          exit func
        endif
        inner = mid(expr, i+1, endb-i)
        if len(operator) = 0 then
          result = eval(inner)
        else
          num = eval(inner)
          result = compute(operator, result, num)
        fi
        i = endb
        if (i+1 = strl) then
          ' nothing more to process
          eval = result 
          exit func
        fi
        ' scan for next operator
        operator = ""
      elseif ch = ")" then
        eval = result 
        exit func
      else
        ' operator character
        if (len(operator) = 0) then
          ' first operand
          result = num
        else
          ' resolve the current expression
          result = compute(operator, result, num)
        endif
        operator = ch
      fi
      ' scan for next number
      num = 0
      pt = 0
    fi
  next i
  eval = result
end

sub mk_bn(byref f, fgc, bgc, x, y, w, h, caption)
  local button
  button.x = x
  button.y = y
  button.width = w
  button.height = h
  button.label = caption
  button.color = fgc
  button.backgroundColor = bgc
  button.noFocus = true
  f.inputs << button
end

func createForm
  local w = 60
  local h = 60
  local y = 40
  local x = 10
  local fcb = 3
  local ncb = 4
  local ocb = 5

  local fcf = 1
  local ncf = 15
  local ocf = 7

  out_x = x + 3
  out_y = 5

  mk_bn(f, fcf, fcb, x, y, w, h, "MC")
  mk_bn(f, ncf, ncb, -1, y, w, h, "7")
  mk_bn(f, ncf, ncb, -1, y, w, h, "8")
  mk_bn(f, ncf, ncb, -1, y, w, h, "9")
  mk_bn(f, ocf, ocb, -1, y, w, h, "+")
  mk_bn(f, ocf, ocb, -1, y, w, h, "-")

  y += h
  mk_bn(f, fcf, fcb,  x, y, w, h, "MR")
  mk_bn(f, ncf, ncb, -1, y, w, h, "4")
  mk_bn(f, ncf, ncb, -1, y, w, h, "5")
  mk_bn(f, ncf, ncb, -1, y, w, h, "6")
  mk_bn(f, ocf, ocb, -1, y, w, h, "*")
  mk_bn(f, ocf, ocb, -1, y, w, h, "/")

  y += h
  mk_bn(f, fcf, fcb,  x, y, w, h, "MS")
  mk_bn(f, ncf, ncb, -1, y, w, h, "1")
  mk_bn(f, ncf, ncb, -1, y, w, h, "2")
  mk_bn(f, ncf, ncb, -1, y, w, h, "3")
  mk_bn(f, ocf, ocb, -1, y, w, h, "(")
  mk_bn(f, ocf, ocb, -1, y, w, h, ")")

  y += h
  mk_bn(f, fcf, fcb,  x, y, w, h, "EXIT")
  mk_bn(f, ncf, ncb, -1, y, w, h, "0")
  mk_bn(f, ncf, ncb, -1, y, w, h, ".")
  mk_bn(f, fcf, fcb, -1, y, w, h, "BS")
  mk_bn(f, fcf, fcb, -1, y, w, h, "CE")
  mk_bn(f, ocf, ocb, -1, y, w, h, "=")

  f = form(f)
  createForm = f
end

func showResult(result)
  local bgnd = 3
  local w = 362
  local h = 30
 
  rect out_x, out_y STEP w, h,  COLOR bgnd FILLED
  rect out_x - 1, out_y - 1 STEP w + 1, h + 1,  COLOR 2

  local out_str = chr(27) + "[15 C" + result
  local out_str_w = textwidth(result)
  local x = (out_x + w) - out_str_w - textwidth("0")

  if (x < out_x) then
    x = out_x
    out_str =  " ERR"
  fi
  
  color 15, bgnd
  at x, out_y + 7
  print cat(1) + out_str
end

sub main
  local result = "0"
  local mem = ""

  rect 0, 0, xmax, ymax, 1 FILLED
  f = createForm
  showResult result  
  
  while 1
    f.doEvents()
    k = inkey
    form_var = f.value
    if (len(k) == 1) then
      if (asc(k) == 8) then
        form_var = "BS"
      else if (asc(k) == 127) then
        form_var = "CE"
      else
        form_var = k
      fi
    fi
    select case form_var
    case "CE"
      ' clear all
      result = "0"
    case "BS"
      ' back space
      if len(result) > 1 then
        result = left(result, len(result)-1)
      else 
        result = "0"
      fi
    case "MC"
      mem = ""
    case "MR"
      if (mem != "") then
        result = mem
      fi
    case "MS"
      mem = result
    case "EXIT"
      exit loop
    case "."
      if len(result) > 0 && right(result, 1) != "." then
        result += "."
      fi
    case "="
      if (instr(result, "=") == 0) then
        result = result + form_var + eval(result)
      fi
    case else
      ' expression input
      if (result = "0") then 
        result = "" 
      fi
      if (instr(result, "=") > 0) then
        result = ""
      fi
      result = result + form_var
    end select
    showResult result
  wend
end

main
