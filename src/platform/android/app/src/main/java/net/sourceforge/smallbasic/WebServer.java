package net.sourceforge.smallbasic;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * WebServer
 *
 * @author chrisws
 */
public abstract class WebServer {
  private static final int BUFFER_SIZE = 32768;
  private static final int SEND_SIZE = BUFFER_SIZE / 4;

  public WebServer() {
    super();
  }

  /**
   * Runs the WebServer in a new thread
   */
  public void run(final int socketNum, final String token) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          runServer(socketNum, token);
        } catch (IOException e) {
          log("startServer failed: ", e);
        }
      }
    });
    socketThread.start();
  }

  protected abstract void execStream(String line, InputStream inputStream) throws IOException;
  protected abstract Collection<FileData> getFileData() throws IOException;
  protected abstract Response getResponse(String path, boolean asset) throws IOException;
  protected abstract void log(String message, Exception exception);
  protected abstract void log(String message);

  /**
   * Appends a string field to the given JSON string
   */
  private void appendJson(StringBuilder json, String field, String value, boolean nextField) {
    json.append(field)
        .append(":'")
        .append(value)
        .append("'");
    if (nextField) {
      json.append(",");
    }
  }

  /**
   * Appends an int field to the given JSON string
   */
  private void appendJson(StringBuilder json, String field, long value, boolean nextField) {
    json.append(field)
        .append(":'")
        .append(value)
        .append("'");
    if (nextField) {
      json.append(",");
    }
  }

  /**
   * Parses HTTP GET parameters with the given name
   */
  private Map<String, Collection<String>> getParameters(String url) throws IOException {
    Map<String, Collection<String>> result = new HashMap<>();
    try {
      String[] args = URLDecoder.decode(url, "utf-8").split("[?]");
      if (args.length == 2) {
        for (String arg : args[1].split("&")) {
          String[] field = arg.split("=");
          Collection<String> values = result.get(field[0]);
          if (values == null) {
            values = new ArrayList<>();
            result.put(field[0], values);
          }
          values.add(field[1]);
        }
      }
    } catch (Exception e) {
      throw new IOException(e);
    }
    return result;
  }

  /**
   * Parses HTTP POST data from the given input stream
   */
  private Map<String, String> getPostData(InputStream inputStream, final String line) throws IOException {
    int length = 0;
    final String lengthHeader = "content-length: ";
    String nextLine = line;
    while (nextLine != null && nextLine.length() > 0) {
      if (nextLine.toLowerCase(Locale.ENGLISH).startsWith(lengthHeader)) {
        length = Integer.parseInt(nextLine.substring(lengthHeader.length()));
      }
      nextLine = readLine(inputStream);
    }
    StringBuilder postData = new StringBuilder();
    for (int i = 0; i < length; i++) {
      int b = inputStream.read();
      if (b == -1) {
        break;
      } else {
        postData.append(Character.toChars(b));
      }
    }
    String[] fields = postData.toString().split("&");
    Map<String, String> result = new HashMap<>();
    for (String nextField : fields) {
      int eq = nextField.indexOf("=");
      if (eq != -1) {
        String key = nextField.substring(0, eq);
        String value = URLDecoder.decode(nextField.substring(eq + 1), "utf-8");
        result.put(key, value);
      }
    }
    return result;
  }

  /**
   * Download files button handler
   */
  private Response handleDownload(Map<String, Collection<String>> parameters) throws IOException {
    Collection<String> fileNames = parameters.get("f");
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    ZipOutputStream zipOutputStream = new ZipOutputStream(outputStream);
    if (fileNames == null) {
      fileNames = Collections.emptyList();
    }

    for (String fileName : fileNames) {
      Response response = getResponse(fileName, false);
      ZipEntry entry = new ZipEntry(fileName);
      zipOutputStream.putNextEntry(entry);
      response.toStream(zipOutputStream);
      zipOutputStream.closeEntry();
    }

    zipOutputStream.finish();
    zipOutputStream.close();
    return new Response(new ByteArrayInputStream(outputStream.toByteArray()), outputStream.size());
  }

  /**
   * Handler for files API
   */
  private Response handleFileList() throws IOException {
    StringBuilder json = new StringBuilder("[");
    long id = 0;
    String comma = "";
    for (FileData nextFile : getFileData()) {
      json.append(comma);
      json.append("{");
      appendJson(json, "id", id++, true);
      appendJson(json, "fileName", nextFile.fileName, true);
      appendJson(json, "size", nextFile.size, true);
      appendJson(json, "date", nextFile.date, false);
      json.append("}");
      comma = ",";
    }
    json.append("]");
    return new Response(new ByteArrayInputStream(json.toString().getBytes("utf-8")), json.length());
  }

  /**
   * Handler for HTTP GET
   */
  private void handleGet(Socket socket, String url) throws IOException {
    Map<String, Collection<String>> parameters = getParameters(url);
    if (url.startsWith("/api/download?")) {
      handleDownload(parameters).send(socket);
    } else {
      handleWebResponse(url).send(socket);
    }
  }

  /**
   * Handler for HTTP POST
   */
  private void handlePost(Socket socket, Map<String, String> postData, String url) throws IOException {
    if (url.startsWith("/api/files")) {
      handleFileList().send(socket);
    }
    log("Sent POST response");
  }

  /**
   * Opens the given web ui file
   */
  private Response handleWebResponse(String asset) throws IOException {
    String path;
    if (asset.equals("/")) {
      path = "index.html";
    } else if (asset.startsWith("/")) {
      path = asset.substring(1);
    } else {
      path = asset;
    }
    return getResponse(path, true);
  }

  /**
   * Reads a line of text from the given stream
   */
  private String readLine(InputStream stream) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream(128);
    int b;
    for (b = stream.read(); b != -1 && b != '\n'; b = stream.read()) {
      if (b != '\r') {
        out.write(b);
      }
    }
    return b == -1 ? null : out.size() == 0 ? "" : out.toString();
  }

  /**
   * WebServer main loop to be run in a separate thread
   */
  private void runServer(final int socketNum, final String token) throws IOException {
    log("Listening :" + socketNum);
    log("Token :" + token);
    ServerSocket serverSocket;
    try {
      serverSocket = new ServerSocket(socketNum);
    } catch (IllegalArgumentException e) {
      log("Failed to start server: ", e);
      throw new IOException(e);
    }
    while (true) {
      Socket socket = null;
      DataInputStream inputStream = null;
      try {
        socket = serverSocket.accept();
        log("Accepted connection from " + socket.getRemoteSocketAddress().toString());
        inputStream = new DataInputStream(socket.getInputStream());
        String line = readLine(inputStream);
        if (line != null) {
          log(line);
          String[] fields = line.split("\\s");
          if ("GET".equals(fields[0]) && fields.length > 1) {
            handleGet(socket, fields[1]);
          } else if ("POST".equals(fields[0])) {
            Map<String, String> postData = getPostData(inputStream, line);
            String userToken = postData.get("token");
            log("userToken=" + userToken);
            if (token.equals(userToken)) {
              handlePost(socket, postData, fields[1]);
            } else {
              log("Invalid token");
            }
          } else {
            log("Invalid request");
          }
        }
      } catch (IOException e) {
        log("Server failed: ", e);
        break;
      } finally {
        log("socket cleanup");
        if (socket != null) {
          socket.close();
        }
        if (inputStream != null) {
          inputStream.close();
        }
      }
    }
    log("server stopped");
  }

  /**
   * BASIC file details
   */
  public static class FileData {
    String fileName;
    String date;
    long size;
  }

  /**
   * Server response data
   */
  public class Response {
    private final InputStream inputStream;
    private final long length;

    public Response(InputStream inputStream, long length) {
      this.inputStream = inputStream;
      this.length = length;
    }

    public InputStream getInputStream() {
      return inputStream;
    }

    public long getLength() {
      return length;
    }

    /**
     * Sends the response to the given socket
     */
    void send(Socket socket) throws IOException {
      log("sendResponse() entered");
      String contentLength = "Content-length: " + length + "\r\n";
      BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
      out.write("HTTP/1.0 200 OK\r\n".getBytes());
      out.write("Content-type: text/html\r\n".getBytes());
      out.write(contentLength.getBytes());
      out.write("Server: SmallBASIC for Android\r\n\r\n".getBytes());
      socket.setSendBufferSize(SEND_SIZE);
      int sent = toStream(out);
      log("sent " + sent + " bytes");
      out.close();
    }

    /**
     * Transfers data from the input stream to the output stream
     */
    int toStream(OutputStream outputStream) throws IOException {
      DataInputStream in = new DataInputStream(new BufferedInputStream(inputStream));
      int sent = 0;
      try {
        byte[] bytes = new byte[BUFFER_SIZE];
        int read;
        while ((read = in.read(bytes)) != -1) {
          outputStream.write(bytes, 0, read);
          outputStream.flush();
          sent += read;
        }
      } finally {
        in.close();
      }
      return sent;
    }
  }
}
