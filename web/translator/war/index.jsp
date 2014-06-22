<!DOCTYPE>
<html>
  <head>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
    <title>SmallBASIC Translator</title>
    <style>
    body {
      color: #000;
      background-color: #19c;
      font: 76% Verdana,Arial,Helvetica,sans-serif;
    }
    h1,div {
      color: white;
    }
    textarea {
      background-color: #eee;
    }
    </style>
  </head>

  <body>
    <h1>SmallBASIC Translator</h1>
    <div>
      <form name="convert" method="post" action="sbtranslator">
        Input:
        <textarea id="input" name="input" cols="" rows="25"
                  style="width:100%; height:40%">${requestScope.input}</textarea>
        <input type="submit" name="submit" value="Translate" title="translate" />
      </form>
      Output:
      <textarea id="output" name="output" cols="" rows="25"
                style="width:100%; height:40%">${requestScope.output}</textarea>
    </div>
  </body>
</html>
