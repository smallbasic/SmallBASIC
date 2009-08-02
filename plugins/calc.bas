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

func createForm
  local w = 60
  local h = 60
  local y = (ymax / 2) - (h * 2)
  local x = (xmax / 2) - (w * 3)
  local fcb = 3
  local ncb = 4
  local ocb = 5

  local fcf = 1
  local ncf = 15
  local ocf = 7

  out_x = x
  out_y = y - h
  
  color fcf, fcb: button  x, y, w, h, b_mc,   "MC"
  color ncf, ncb: button -1, y, w, h, b_7,    "7"
  color ncf, ncb: button -1, y, w, h, b_8,    "8"
  color ncf, ncb: button -1, y, w, h, b_9,    "9"
  color ocf, ocb: button -1, y, w, h, b_add,  "+"
  color ocf, ocb: button -1, y, w, h, b_sub,  "-"

  y += h
  color fcf, fcb: button  x, y, w, h, b_mr,   "MR"
  color ncf, ncb: button -1, y, w, h, b_4,    "4"
  color ncf, ncb: button -1, y, w, h, b_5,    "5"
  color ncf, ncb: button -1, y, w, h, b_6,    "6"
  color ocf, ocb: button -1, y, w, h, b_mul,  "*"
  color ocf, ocb: button -1, y, w, h, b_div,  "/"

  y += h
  color fcf, fcb: button  x, y, w, h, b_ms,   "MS"
  color ncf, ncb: button -1, y, w, h, b_1,    "1"
  color ncf, ncb: button -1, y, w, h, b_2,    "2"
  color ncf, ncb: button -1, y, w, h, b_3,    "3"
  color ocf, ocb: button -1, y, w, h, b_ob,   "("
  color ocf, ocb: button -1, y, w, h, b_cb,   ")"

  y += h
  color fcf, fcb: button  x, y, w, h, b_ex,   "EXIT"
  color ncf, ncb: button -1, y, w, h, b_0,    "0"
  color ncf, ncb: button -1, y, w, h, b_dot,  "."
  color fcf, fcb: button -1, y, w, h, b_bs,   "BS
  color fcf, fcb: button -1, y, w, h, b_ce,   "CE"
  color ocf, ocb: button -1, y, w, h, b_eq,   "="
end

func showResult(result)
  at out_x, out_y: ? chr(27) + "[K" + chr(27) + "[40 C" + result
end

sub main
  local result = ""
  local mem = ""
    
  createForm
  showResult result

  color 1,8
  cls

  while 1
    doform form_var
    select case form_var
    case b_ce
      ' clear all
      result = 0
    case b_bs
      ' back space
      if len(result) > 1 then
        result = left(result, len(result)-1)
      else 
        result = "0"
      fi
    case b_mc
      mem = ""
    case b_mr
      if (mem != "") then
        result = mem
      fi
    case b_ms
      mem = result
    case b_ex
      exit loop
    case b_dot
      if len(result) > 0 && right(result, 1) != "." then
        result += "."
      fi
    case b_eq
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
