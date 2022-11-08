package net.sourceforge.smallbasic;

import sun.misc.IOUtils;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;
import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Objects;

public class Server {
  private static final String BASIC_HOME = "../basic/";

  public static void main(String[] args ) throws IOException {


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
        final File folder = new File(BASIC_HOME);
        Collection<FileData> result = new ArrayList<>();
        for (final File fileEntry : Objects.requireNonNull(folder.listFiles())) {
          BasicFileAttributes attr = Files.readAttributes(fileEntry.toPath(), BasicFileAttributes.class);
          if (!attr.isDirectory()) {
            FileTime lastModifiedTime = attr.lastModifiedTime();
            String fileName = fileEntry.getName();
            ZonedDateTime zonedDateTime = Instant.ofEpochMilli(lastModifiedTime.toMillis()).atZone(ZoneId.of("UTC"));
            String date = DateTimeFormatter.RFC_1123_DATE_TIME.format(zonedDateTime);
            long size = attr.size();
            result.add(new FileData(fileName, date, size));
          }
        }
        return result;
      }

      @Override
      protected Response getFile(String path, boolean asset) throws IOException {
        String prefix = asset ? "../build/" : BASIC_HOME;
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
      protected void renameFile(String from, String to) throws IOException {
        if (to == null || !to.endsWith(".bas")) {
          throw new IOException("Invalid File Name: " + to);
        }
        File toFile = new File(BASIC_HOME, to);
        if (toFile.exists()) {
          throw new IOException("File Name already exists");
        }
        File fromFile = new File(BASIC_HOME, from);
        if (!fromFile.exists()) {
          throw new IOException("Previous File Name does not exist");
        }
        if (!fromFile.renameTo(new File(BASIC_HOME, to))) {
          throw new IOException("File rename failed");
        }
      }

      @Override
      protected void saveFile(String fileName, byte[] content) throws IOException {
        File file = new File(BASIC_HOME, fileName);
        if (file.exists()) {
          throw new IOException("File already exists: " + fileName);
        } else if (file.isDirectory()) {
          throw new IOException("Invalid File Name: " + fileName);
        }
        copy(new ByteArrayInputStream(content), Files.newOutputStream(file.toPath()));
      }

      private void copy(InputStream in, OutputStream out) throws IOException {
        try {
          byte[] buf = new byte[1024];
          int len;
          while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
          }
        } finally {
          out.close();
          in.close();
        }
      }
    };
    webServer.run(8080, "ABC123");
  }
}
