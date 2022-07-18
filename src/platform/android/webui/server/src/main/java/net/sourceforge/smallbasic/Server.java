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
  private static final String BASIC_HOME = "../basic";

  public static void main(String[] args ) {
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
          throw new IOException("Invalid New File Name: " + to);
        }
        File toFile = new File(BASIC_HOME, to);
        if (toFile.exists()) {
          throw new IOException("New File Name already exists");
        }
        File fromFile = new File(BASIC_HOME, from);
        if (!fromFile.exists()) {
          throw new IOException("Old File Name does not exist");
        }
        if (!fromFile.renameTo(new File(BASIC_HOME, to))) {
          throw new IOException("File rename failed");
        }
      }

      @Override
      protected void saveFile(String fileName, byte[] content) throws IOException {
        throw new IOException("Failed to save file: " + fileName);
      }

    };
    webServer.run(8080, "ABC123");
  }
}
