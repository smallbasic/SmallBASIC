/** 
 * Convert BASIC programs of various dialects into SmallBASIC programs
 *
 * Copyright(C) 2009 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 *
 */

import org.antlr.runtime.*;

public class Translator {
  public static void main(String[] args) throws Exception {
    CharStream input = new ANTLRFileStream(args[0]);
    SBLexer lex = new SBLexer(input);
    TokenStream tokens = new TokenRewriteStream(lex);
    SBParser parser = new SBParser(tokens);
    parser.program();
    System.out.println(tokens);
  }
}
