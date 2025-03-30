package net.sourceforge.smallbasic;

import org.junit.Ignore;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;
import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Base64;
import java.util.Collection;

public class WebServerTest {
  private static final String APP_HOME = "../";
  private static final String BASIC_HOME = APP_HOME + "webui/basic/";
  private static final String ASSET_HOME = APP_HOME + "app/build/";

  @Test
  @Ignore("For separate MUI app testing, comment for app build")
  public void serverTest() throws InterruptedException {
    WebServer webServer = new WebServer() {
      @Override
      protected byte[] decodeBase64(String data) {
        return Base64.getDecoder().decode(data);
      }

      @Override
      protected void deleteFile(String hostName, String fileName) throws IOException {
        Files.delete(Paths.get(BASIC_HOME, fileName));
      }

      @Override
      protected void execStream(String remoteHost, InputStream inputStream) {
        try {
          byte[] data = inputStream.readAllBytes();
          log(new String(data));
        }
        catch (IOException e) {
          throw new RuntimeException(e);
        }
      }

      @Override
      protected Response getFile(String hostName, String path, boolean asset) throws IOException {
        String prefix = asset ? ASSET_HOME : BASIC_HOME;
        File file = new File(prefix + path);
        return new Response(Files.newInputStream(file.toPath()), file.length());
      }

      @Override
      protected Collection<FileData> getFileData(String hostName) throws IOException {
        final File folder = new File(BASIC_HOME);
        Collection<FileData> result = new ArrayList<>();
        File[] fileList = folder.listFiles();
        if (fileList == null) {
          log("Error: no files in: " + folder.getAbsolutePath());
          fileList = new File[0];
        }
        for (final File fileEntry : fileList) {
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
      protected void log(String message) {
        System.err.println(message);
      }

      @Override
      protected void log(String message, Exception exception) {
        System.err.println(message);
        exception.printStackTrace();
      }

      @Override
      protected void renameFile(String hostName, String from, String to) throws IOException {
        if (to == null) {
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
      protected void saveFile(String hostName, String fileName, byte[] content) throws IOException {
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
    webServer.run(8080, "ABC123").join();
  }
}
