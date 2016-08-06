package net.sourceforge.smallbasic;

import io.undertow.Undertow;
import io.undertow.server.HttpHandler;
import io.undertow.server.HttpServerExchange;
import io.undertow.util.Headers;

/**
 * SmallBASIC web server
 * 
 * @author chrisws
 * @see http://undertow.io
 */
public class WebServer {
  /**
     to locate the shared library:
     Call System.load to load the .so from an explicitly specified absolute path.
     Copy the shared library to one of the paths already listed in java.library.path
     Modify the LD_LIBRARY_PATH environment variable to include the directory where the shared library is located.
     Specify the java.library.path on the command line by using the -D option.
  */
  
  static {
    System.loadLibrary("smallbasic");
  }

  public static native String execute(String basName);
  
  public static void main(final String[] args) {
    Undertow server = Undertow.builder()
        .addHttpListener(8080, "localhost")
        .setHandler(new HttpHandler() {
      @Override
      public void handleRequest(final HttpServerExchange exchange) throws Exception {
        if (exchange.isInIoThread()) {
          exchange.dispatch(this);
        } else {
          exchange.getResponseHeaders().put(Headers.CONTENT_TYPE, "text/plain");
          String requestPath = exchange.getRequestPath();
          int lastIndex =  requestPath.lastIndexOf('/');
          if (lastIndex != -1 && requestPath.endsWith(".bas")) {
            String basName = requestPath.substring(lastIndex + 1);
            exchange.getResponseSender().send(execute(basName)); 
          } else {
            exchange.getResponseSender().send("error: file not found");
          }
        }
      }
    }).build();
    server.start();
  }
}
