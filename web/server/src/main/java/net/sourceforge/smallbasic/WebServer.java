package net.sourceforge.smallbasic;

import java.nio.ByteBuffer;

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
  static {
    System.loadLibrary("smallbasic");
  }

  public static native String execute(String basName);
  public static native void init();

  public static void main(final String[] args) {
    int port = 8080;
    String host = "localhost";
    for (int i = 0; i < args.length; i++) {
      if ("--port".equals(args[i]) && i + 1 < args.length) {
        port = Integer.valueOf(args[++i].trim());
      } else if ("--host".equals(args[i]) && i + 1 < args.length) {
        host = args[++i].trim();
      }
    }
    System.err.println("Starting server [" + host + ":" + port + "]");
    Undertow server = Undertow.builder().addHttpListener(port, host).setHandler(new HttpHandler() {
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
            exchange.getResponseSender().send(do_execute(basName));
          } else {
            exchange.getResponseSender().send("error: file not found");
          }
        }
      }
      private synchronized String do_execute(String basName) {
        return execute(basName);
      }
    }).build();
    init();
    server.start();
  }
}
