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
    init();
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
    server.start();
  }
}
