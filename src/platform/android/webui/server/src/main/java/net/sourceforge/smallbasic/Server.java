package net.sourceforge.smallbasic;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.util.Collection;

public class Server {
  public static void main( String[] args ) {
    // ln -s ../../../../../../../../app/src/main/java/net/sourceforge/smallbasic/WebServer.java .
    WebServer webServer = new WebServer() {
      @Override
      protected void execStream(String line, InputStream inputStream) {
      }

      @Override
      protected Collection<FileData> getFileData() throws IOException {
        return null;
      }

      @Override
      protected Response getResponse(String path, boolean asset) throws IOException {
        String prefix = asset ? "../build/" : "../basic/";
        File file = new File(prefix + path);
        return new Response(Files.newInputStream(file.toPath()), file.length());
      }

      @Override
      protected void log(String message) {
        System.err.println(message);
      }

      @Override
      protected void log(String message, Exception exception) {
        System.err.println(message);
        exception.printStackTrace();
      }
    };
    webServer.run(8080, "ABC123");
  }
}
