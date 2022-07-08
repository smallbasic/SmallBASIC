package net.sourceforge.smallbasic;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;

public class App {
  public static void main( String[] args ) {
    WebServer webServer = new WebServer() {
      @Override
      protected void execStream(String line, InputStream inputStream) {
      }

      @Override
      protected void log(String message, Exception exception) {
        System.err.println(message);
        exception.printStackTrace();
      }

      @Override
      protected void log(String message) {
        System.err.println(message);
      }

      @Override
      protected Reader openAsset(String path) throws IOException {
        String prefix = "../build/";
        return new BufferedReader(new FileReader(prefix + path));
      }
    };
    webServer.startServer(8080, "ABC123");
  }
}
