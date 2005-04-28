######################################################################
# $Id: calc.bas,v 1.1 2005-04-28 23:22:37 zeeb90au Exp $
# calc.bas Copyright (c) Chris Warren-Smith April 2005
# Demonstration for html user interfaces
#
# Version 1.0
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#
######################################################################

func compute(byref operator, lval, rval)
    local result = 0
    if (operator = "+") then
        result = lval + rval
    elseif (operator = "-") then
        result = lval - rval
    elseif (operator = "/") then
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

# evaluate the expression string
func eval(byref expr)
    local strl,num,result,operator,inner,endb,i,j,nbracket

    strl = len(expr)
    num = 0
    result = 0
    operator = ""

    for i = 0 to strl
        if (i < strl)
            ch = mid(expr, i+1, 1)
        else
            # end of last operand
            eval = compute(operator, result, num)
            exit func
        fi
        if isnumber(ch) then
            num = num*10+ch
        else 
            if ch = "(" then
                # find the matching )
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
            elseif ch = ")" then
                eval = result 
                exit func
            else
                # operator character
                if (len(operator) = 0) then
                    # first operand
                    result = num
                else
                    # resolve the current expression
                    result = compute(operator, result, num)
                endif
                operator = ch
            fi
            num = 0
        fi
    next i
    eval = result
end

sub main
    # install the application help file
    env("APP-HELP=calc_help.html")
    cls

    # display the user interface if not already displayed
    # the form-active field is a hidden form variable  
    if env("form-active") != "true" then
        chdir env("BAS-HOME")
        html "file:calc.html"
    fi

    if len(command) = 0 then
        exit sub
    fi

    # process button clicks - the command variable takes the  
    # value of the onclick argument
    if command = "CE" then
        env("out=0")
        exit sub
    fi

    curval= env("out")
    if instr(curval, "=") > 0 then
        exit sub
    fi

    out = ""
    if command = "BS" then
       if  len(curval) > 1 then
           out = left(curval, len(curval)-1)
       else 
           out = "0"
       fi
    elseif command = "mc" then
    elseif command = "mr" then
    elseif command = "ms" then
    elseif command = "m+" then
    elseif command = "dot" then
        out = curval+"."
    elseif command = "=" then
        out = curval+command+eval(curval)
    else
        # numeric input
        if (curval = "0") then 
            curval = "" 
        fi
        out = curval+command
    fi 
    env("out="+out)
end

main
