package net.sourceforge.smallbasic;

import sun.misc.IOUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Objects;

public class Server {
  public static void main( String[] args ) {
    // ln -s ../../../../../../../../app/src/main/java/net/sourceforge/smallbasic/WebServer.java .
    WebServer webServer = new WebServer() {
      @Override
      protected void execStream(InputStream inputStream) {
        try {
          byte[] data = IOUtils.readAllBytes(inputStream);
          log(new String(data));
        }
        catch (IOException e) {
          throw new RuntimeException(e);
        }
      }

      @Override
      protected Collection<FileData> getFileData() throws IOException {
        final File folder = new File("../basic");
        Collection<FileData> result = new ArrayList<>();
        for (final File fileEntry : Objects.requireNonNull(folder.listFiles())) {
          BasicFileAttributes attr = Files.readAttributes(fileEntry.toPath(), BasicFileAttributes.class);
          if (!attr.isDirectory()) {
            FileTime lastModifiedTime = attr.lastModifiedTime();
            DateFormat dateFormat = DateFormat.getDateInstance(DateFormat.DEFAULT);
            String fileName = fileEntry.getName();
            String date = dateFormat.format(lastModifiedTime.toMillis());
            long size = attr.size();
            result.add(new FileData(fileName, date, size));
          }
        }
        return result;
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

      @Override
      protected boolean renameFile(String from, String to) {
        log("rename " + from + " to " + to);
        return true;
      }

      @Override
      protected boolean saveFile(String fileName, String content) {
        return true;
      }
    };
    webServer.run(8080, "ABC123");
  }
}
