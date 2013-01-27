/** 
 * Convert BASIC programs of various dialects into SmallBASIC programs
 *
 * Copyright(C) 2012 Chris Warren-Smith.
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 *
 */

package net.sourceforge.smallbasic.translator;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.antlr.runtime.ANTLRStringStream;
import org.antlr.runtime.CharStream;
import org.antlr.runtime.RecognitionException;
import org.antlr.runtime.TokenRewriteStream;
import org.antlr.runtime.TokenStream;

@SuppressWarnings("serial")
public class SBTranslatorServlet extends HttpServlet {
  public void doGet(HttpServletRequest request, HttpServletResponse response)
      throws IOException {
    displayPage(request, response);
  }

  public void doPost(HttpServletRequest request, HttpServletResponse response)
      throws IOException {
    String input = request.getParameter("input");
    request.setAttribute("input", input);
    try {
      input = input.replaceAll("\\r\\n", "\n");
      request.setAttribute("output", translate(input));
    } catch (RecognitionException e) {
      e.printStackTrace();
    }
    displayPage(request, response);
  }

  private String translate(String source) throws IOException, RecognitionException {
    final String result;
    System.err.println("translate entered");
    CharStream input = new ANTLRStringStream(source);
    SBLexer lex = new SBLexer(input);
    TokenStream tokens = new TokenRewriteStream(lex);
    ServletSBParser parser = new ServletSBParser(tokens);
    parser.program();
    if (parser.hasError()) {
      result = parser.getError();
    } else {
      result = tokens.toString(); 
    }
    System.err.println("leaving translate");
    return result;
  }
  
  private void displayPage(HttpServletRequest request, HttpServletResponse response) 
    throws IOException {
    response.setContentType("text/plain");    
    try {
      request.getRequestDispatcher("/index.jsp").forward(request, response);
    } catch (ServletException e) {
      e.printStackTrace();
    }
  }
  
  public class ServletSBParser extends SBParser {
    private String error;
    
    public ServletSBParser(TokenStream input) {
      super(input);
      error = "";
    }

    @Override
    public void emitErrorMessage(String msg) {
      error = msg;
    }

    public String getError() {
      return error;
    }
    
    public boolean hasError() {
      return state.syntaxErrors > 0;
    }
  }
}
