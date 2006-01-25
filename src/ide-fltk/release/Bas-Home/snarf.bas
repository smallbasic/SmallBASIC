
#######################################################################
# $Id: snarf.bas,v 1.2 2006-01-25 03:19:22 zeeb90au Exp $
# fetch web pages for off-line reading
#######################################################################

# strip the given tag from the html text
sub removeTag(byref s, tag) 
  local i, i_end, s_len, end_tag
  
  end_tag = "</"+tag+">"
  tag = "<"+tag
  s_len = len(s)
  
  for i = 1 to s_len
    i = instr(i, s, tag)
    if (i = 0) then
      exit for
    fi
    i_end = instr(i, s, end_tag)
    if (i_end = 0) then
      i_end = s_len
    fi
    i_end += len(end_tag)
    s_end = if(i_end<s_len, mid(s, i_end), "")
    s = left(s, i-1) + s_end
    s_len -= (i_end-i)
  next i
end  

# return an array of anchor links from the html text
func getLinks(byref s)
  local i, iend, s_len, links, end_ch, ahref
  
  s_len = len(s)
  i = 1
  
  for i = 1 to s_len
    i = instr(i, s, "<a href")
    if (i = 0) then
      exit for
    fi
    i += 8 'advance past tag
    end_ch = ">"
    if i = instr(i, s, chr(34)) then
      end_ch = chr(34) 'double quote
      i++
    elif i = instr(i, s, chr(39)) then
      end_ch = chr(39) 'single quote
      i++
    fi

    iend = instr(i, s, end_ch)
    if (iend = 0) then
      exit for
    fi
    ahref = mid(s, i, iend-i)
    if (instr(1, ahref, "#") != 1) then
      links << ahref
    fi
    
  next i
  getLinks=links
end func

# connect to the given url and retrive the page text
func snarf(url)
  local s

  open url as #2
  if (eof(2)) then
    ? "Connection failed: "+url
    exit
  fi

  ? "Snarfing " + url
  tload #2, s
  removeTag s, "script"
  removeTag s, "style"
  removeTag s, "noscript"
  close #2
  snarf = s
end

# create the local file system path
sub mkpath(s)
  local s_len, i, path

  s_len = len(s)
  i = 1
  while (i < s_len)
    i = instr(i, s, "/")
    if (i = 0) then
      exit sub
    fi
    path = left(s, i-1)
    if exist(path) = 0 then
      mkdir path
    fi
    i++
  wend
end

# fetch pages from the given site
sub fetch
  cacheHome = ENV("HOME")+"/.smallbasic/cache/smh"
  homeURL = "http://www.smh.com.au"
  indexURL = homeURL+"/handheld/index.html"
  indexPage = cacheHome+"/handheld/index.html"
  
  mkpath indexPage

  'fetch the index page
  open indexPage for output as #1
  s = snarf(indexURL)
  links = getLinks(s)
  print #1, s
  close #1
  
  'fetch the sub-pages
  for href in links
    path = cacheHome+href 
    mkpath path
    open path for output as #1
    print #1, snarf(homeURL+href)
    close #1
  next
  
  ? "Done"
end

# process button clicks - command =value of onclick argument
sub main
  select case command
  case "f_smh"
    fetch
  case "f_age"
  case "r_smh"
    s = "file:"+ENV("HOME")+"/.smallbasic/cache/smh/handheld/index.html"
    html s
    exit sub
  case "r_age"
  end select
  
  'display the menu
  bn="<td><input type=button onclick='!"+env("BAS_HOME")+"snarf.sbx"
  s = "<br><table><tr>"
  s +="<td>Sydney Morning Herald</td>"
  s +=bn+" r_smh' value=Read></td>"
  s +=bn+" f_smh' value=Fetch></td>"
  s +="</tr><tr><td>AGE</td>"
  s +=bn+" r_age' value=Read></td>"
  s +=bn+" f_age' value=Fetch></td>"
  s +="</tr></table>"
  html s
end

main


