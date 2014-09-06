url = "http://ajax.googleapis.com/ajax/services/search/web?rsz=large&q=" + trim(command) + "&v=1.0"
open url as #1
if (eof(1)) then
  throw "Connection failed: " + url
fi

dim results
tload #1, results
json = array(results)
num_results = len(json.responseData.results)
for i = 0 to num_results - 1
  print json.responseData.results(i).titleNoFormatting
  print "  "; json.responseData.results(i).unescapedUrl
next i
