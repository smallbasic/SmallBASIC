#!/usr/local/bin/sbasic -q
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#
# $Id: texi2html.bas,v 1.1 2004-08-07 01:10:30 zeeb90au Exp $
# text2html.bas Copyright (c) Chris Warren-Smith July 2004 
# Version 1.0
#
######################################################################

local g_fontSize
local g_excludeChapters
local g_title
local g_fileName

map_tbl = [&
"command", "CODE";&
"var", "SPAN";&
"strong", "B";&
"uref", "U";&
"samp", "SAMP";&
"code", "CODE";&
"dfn", "I"]
map_cont = 0
map_rpl = ""
map_len = (len(map_tbl)/2)-1
map_skip = 0

g_fontSize = 2

func toString(ar, index)
    local s, arLen, i

    arLen = len(ar)-1
    if (index <= arLen) then
        s = ar(index)
    fi
    for i = index+1 to arLen
        s = s + " " + ar(i)
    next i
    toString = s
end

func replaceAll(s, find, repl)
    local i, s_len, s_end, f_len

    i = 1
    s_len = len(s)
    f_len = len(find)
    
    while i < s_len
        i = instr(i, s, find)
        if (i = 0) then
            exit loop
        fi
        if (i+f_len < len(s)) then
            s_end = mid(s, i+f_len)
        else
            s_end = ""
        fi
        s = left(s, i-1) + repl + s_end
        i++
    wend
    replaceAll = s
end

func doReplaceMap(s, i_beg, byref i_end, level)
    local i_at, i, offs, s_blk, s_cmd, s_beg, s_end, s_out, s_mod, out_len

    s_out = s
    i_at = 0
    i = i_beg
    s_mod = 0

    while i < len(s)
        ch = mid(s, i+1, 1)
        if (ch = "@") then
            i_at = i

        elseif (ch = "{") then
            ## offset position of i_at in s for position in s_out
            offs = len(s_out)-len(s)
            s_beg = mid(s_out, i_beg+1, i_at-i_beg+offs)
            s_cmd = mid(s, i_at+2, i-i_at-1)
            s_blk = doReplaceMap(s, i+1, i_end, level+1)

            if (i_end < len(s)) then
                s_end = mid(s, i_end+1)
            else
                s_end = ""
            fi

            i = i_end
           
            map_rpl = ""
            for m = 0 to map_len
                if (map_tbl(m, 0) = s_cmd) then
                    map_rpl = map_tbl(m, 1)
                    exit for
                fi
            next m

            if (len(map_rpl) > 0) then
                if (map_cont) then
                    s_out = s_beg+"<"+map_rpl+">"+s_blk+s_end
                else
                    s_out = s_beg+"<"+map_rpl+">"+s_blk+"</"+map_rpl+">"+s_end
                fi
            else
                s_out = s_beg+s_blk+s_end
            fi
            
            s_mod = 1
            i_beg = 0
            
        elseif (ch = "}") then
            i_end = i+1

            if (s_mod) then
                out_len = instr(s_out, "}")-1
            else
                out_len = i-i_beg
            fi
            
            if (mid(s, i, 1) = "@") then
                out_len--
            fi

            map_cont = 0
            if (level > 0) then
                doReplaceMap = mid(s_out, i_beg+1, out_len)
                exit func
            else
                ## continued from previous line
                s_beg = left(s, i)
                if (i_end = len(s)) then
                    s_end = ""
                else
                    s_end = mid(s, i_end+1)
                fi
                s_out = s_beg+"</"+map_rpl+">"+s_end
            fi
        fi
        i++
    wend

    map_cont = 1
    i_end = i
    doReplaceMap = mid(s_out, i_beg+1)
end

func replaceMap(s)
    local i_end, i
    i_end = 0
    i = 0

    if (instr(s, "@verbatim") != 0) then
        map_skip = 1
    elseif (instr(s, "@end verbatim") != 0) then
        map_skip = 0
    fi

    if (map_skip = 0 && instr(s, "{") != 0) then
        replaceMap = doReplaceMap(s, i, i_end, 0)
    else
        replaceMap = s
    fi
end

func isExcludeChapter(chapter)
   local c
   for c in g_excludeChapters
       if (instr(chapter, c) = 1) then
           isExcludeChapter = 1
           exit func
       fi
   next
   isExcludeChapter = 0
end

func makeHTM(s)
    local t
    t = squeeze(s)
    t = replaceAll(t, "&", "&amp;")
    t = replaceAll(t, "<", "&lt;")
    t = replaceAll(t, ">", "&gt;")
    makeHTM = t
end

sub addLineEnding(byref out, ending)
    local s,t

    s = out(len(out)-2)
    if (len(s) > 0) then
        t = right(s, 4)
        if (t = "<hr>" || t = "<br>" || t = "</p>") then
            exit sub
        fi
    fi
    
    out << ending
end

sub writeHeader(byref buf, byref chapters, chapterId)
    local t, nav, title, index

    nav = "<font size="+g_fontSize+"><title>"+g_title+"</title>"
    index = 0
    for chapter in chapters
        if (isExcludeChapter(chapter) = 0) then
            if (index = chapterId) then
                ' current section
                title = "<b>" + chapter + "</b>"
            else
            title = chapter
            fi
            nav += "<a href=" + index + ".html>" + title + "</a> | "
        fi
        index++        
    next
    nav += "<hr>"
    buf << nav
end

sub readHeader(byref buffer, byref i)
    local bufferLen

    bufferLen = len(buffer)-1
    for i =0 to bufferLen
        split buffer(i), " ", v() use trim(x)
        if (len(v) > 0)
            if (v(0) = "@node") then
                i++
                exit sub
            elseif (v(0) = "@c") then
                # comment
            elseif (v(0) = "@settitle") then
                g_title = toString(v,1)
            elseif (v(0) = "@setfilename") then
                g_fileName = toString(v,1)
            elseif (v(0) = "@set") then
                s = v(1) + "=" + toString(v, 2)
                env(s)
            fi
        fi
    next i
end

func readChapterTitles(byref buffer, byref chapters, byref i)
    local bufferLen

    bufferLen = len(buffer)-1
    while (i < bufferLen)
        s = buffer(i)
        if (instr(s, "@node") = 1) then
            i++
            exit loop
        elseif (instr(s, "@chapter") = 1 || &
                instr(s, "@top") = 1 || &
                instr(s, "@appendix") = 1) then
            chapters << rightof(s, " ")
        elseif (instr(s, "@bye") = 1) then
            readChapterTitles = -1
            exit func
        fi
        i++
    wend
    readChapterTitles = i
end

sub readSectionTitles(byref buffer, i, byref sections)
    bufferLen = len(buffer)-1
    while (i < bufferLen)
        s = buffer(i)
        if (instr(s, "@section") = 1) then
            sections << replaceMap(rightof(s, " "))
        elseif (instr(s, "@node") = 1 || &
                instr(s, "@chapter") = 1 || &
                instr(s, "@bye") = 1) then
            exit sub
        fi
        i++
    wend
end

func readNextSection(byref buffer, byref i, byref out, byref title)
    local bufferLen, s, inTable, inItem, s_line

    inTable = 0
    inItem = 0
    bufferLen = len(buffer)-1
    while (i < bufferLen)
        s = buffer(i)
        s = replaceMap(s)
        
        if (instr(s, "@node") = 1 || &
            instr(s, "@bye") = 1) then
            readNextSection = -1
            addLineEnding out, "</p>"
            exit func
        elseif (instr(s, "@section") = 1) then
            # end of this section -- another section follows
            title = rightof(s, " ")
            i++
            exit loop
        elseif (instr(s, "@subs") = 1) then
            #subsection or subsubsection
            title = rightof(s, " ")            
            readNextSection = i
            exit func
        fi

        if (instr(s, "@c") = 1) then
            # comment
        elseif (instr(s, "@menu") = 1) then
            out << "<br>"
            i++
            while (i < bufferLen)
                s = buffer(i)
                if (instr(s, "@end") = 1) then
                    exit loop
                else
                    #menu not supported
                    #out << s +"<br>"
                fi
                i++
            wend
        elseif (instr(s, "@itemize") = 1 || &
                instr(s, "@enumerate") = 1) then
            out << "<ul>"
        elseif (instr(s, "@item") = 1) then
            inItem = 1
            if (inTable = 1) then
                s_line = rightof(s, "@item")
                if (len(squeeze(leftof(s_line, "@tab"))) > 0) then
                    out << "<tr>"
                    out << "<td>" + makeHTM(leftof(s_line, "@tab")) + "</td>"
                    out << "<td>" + makeHTM(rightof(s_line, "@tab")) + "</td>"
                    out << "</tr>"
                fi
            else
                out << "<li>" + rightof(s, " ")
            fi
        elseif (instr(s, "@end itemize") = 1 || &
                instr(s, "@end enumerate") = 1) then
            out << "</ul>"
        elseif (instr(s, "@verbatim") = 1 || &
                instr(s, "@example") = 1) then
            out << "<pre>"
        elseif (instr(s, "@end verbatim") = 1 || &
                instr(s, "@end example") = 1) then
            out << "</pre>"
        elseif (instr(s, "@deffn") = 1) then
            split s, " ", v() use trim(x)
            addLineEnding out, "<br>"
            out << "<u>" + v(1) + "</u>:"
            out << "<b>" + v(2) + "</b>"
            out << "<i>" + toString(v, 3)+ "</i>"
        elseif (instr(s, "@end deffn") = 1) then
            #####out << "<br>"
        elseif (instr(s, "@table") = 1) then
            out << "<ul>"            
        elseif (instr(s, "@end table") = 1) then
            out << "</ul>"
        elseif (instr(s, "@multitable") = 1) then
            out << "<table>"
            inTable = 1
        elseif (instr(s, "@end multitable") = 1) then
            out << "</table>"
            inTable = 0
            inItem = 0
        elseif (instr(s, "@tab") = 1) then
            out << "&nbsp;"
        elseif (instr(s, "@") = 1) then
            # un-supported tag
        else
            out << s
        fi
        i++
    wend
    out << "</p>"
    readNextSection = i
end

func skipChapter(byref buffer, byref i)
    local s

    # skip past initial @chapter    
    i+=2
    
    while (i < bufferLen)
        s = buffer(i)
        if (instr(s, "@bye") = 1) then
            exit loop
        elseif (instr(s, "@node") = 1) then
            skipChapter = i
            exit func
        fi
        i++
    wend
    skipChapter = -1    
end

func processChapter(byref buffer, byref i, byref chapters, byref chapterId)
    local s, sections, out, index, result, bufferLen, fname, title, nav
    local sectionTitle, sstitle, ssout, ssindex, sstitle
    
    # skip past initial @chapter
    i+=2

    # create the section index page
    writeHeader out, chapters, chapterId
    readSectionTitles buffer, i, sections

    # read the chapter preface
    result = readNextSection(buffer, i, out, title)
    addLineEnding out, "<br>"
   
    index = 0
    for section in sections
        out << "<br><a href="+chapterId+"_"+index+".html>"+section+"</a>"
        index++
    next
    tsave chapterId+".html", out

    index = 0
    while (result != -1)
        out = 0
        writeHeader out, chapters, chapterId
        out << "<b>"+title+"</b><br><br>"
        sectionTitle = title
        result = readNextSection(buffer, i, out, title)

        ssindex = 0
        while (result != -1 && instr(buffer(i), "@subs") = 1)
            ssout = 0
            ssindex ++
            sstitle = rightof(buffer(i), " ")
            fname = chapterId+"_"+index+"_"+ssindex+".html"
            out << "<p><a href="+fname+">"+sstitle+"</a>"
            i++
            writeHeader ssout, chapters, chapterId
            ssout << "<b>"+sectionTitle+" > "+sstitle+"</b><br><br>"
            result = readNextSection(buffer, i, ssout, title)
            ssout << "<hr><a href="+chapterId+"_"+index+".html>[Up]</a>"
            tsave fname, ssout
        wend

        # section navigation
        nav = "<hr>"
        if (index > 0) then
            nav += "<a href="+chapterId+"_"
            nav += (index-1)
            nav += ".html>[Prev]</a> "
        else
            nav += "[Prev] "
        fi
        if (index+1 < len(sections)) then
            nav += "<a href="+chapterId+"_"
            nav += (index+1)
            nav += ".html>[Next]</a>"
        else
            nav += "[Next]"
        fi
        out << nav
        
        tsave chapterId+"_"+index+".html", out
        index++
    wend

    if (i >= bufferLen || instr(buffer(i), "@bye") = 1) then
        processChapter = -1
    else
        processChapter = i
    fi
end

sub execute(inputFile)
    local i, ibegin, result, chapters, buffer, index, bufferLen
    
    tload inputFile, buffer

    bufferLen = len(buffer)-1    
    readHeader buffer, ibegin

    result = 0
    i = ibegin
    
    while result != -1
        result = readChapterTitles(buffer, chapters, i)
    wend

    index = 0
    result = 0
    i = ibegin

    while result != -1
        if (isExcludeChapter(chapters(index)) = 1) then
            result = skipChapter(buffer, i)
        else
            result = processChapter(buffer, i, chapters, index)
        fi
        index++
    wend
   
  # ? title
  # ? env("SUBTITLE")
  
end

if (len(command$) = 0) then
    ? "Usage: texi2html file.texi"
else
    split command, " ", args() use trim(x)
    fileIndex = 0
    argLen = len(args)-1
    for i = 0 to argLen
        if (args(i) = "/x") then
            i++
            if (i < len(args)) then
                g_excludeChapters << replaceAll(args(i), "_", " ")
            fi
        fi
    next i

    execute args(argLen)
fi

