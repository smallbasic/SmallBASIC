package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Minimal AppServer with adequate performance for a single user
 *
 * @author chrisws
 */
public abstract class WebServer {
  private static final int BUFFER_SIZE = 32768;
  private static final int SEND_SIZE = BUFFER_SIZE / 4;
  private static final int LINE_SIZE = 128;
  private static final String UTF_8 = "utf-8";
  private static final String TOKEN = "token";
  private static final int SC_OKAY = 200;
  private static final int SC_NOT_AUTHORISED = 401;
  private static final int SC_NOT_FOUND = 404;

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

  protected abstract void execStream(InputStream inputStream) throws IOException;
  protected abstract Response getFile(String path, boolean asset) throws IOException;
  protected abstract Collection<FileData> getFileData() throws IOException;
  protected abstract void log(String message);
  protected abstract void log(String message, Exception exception);
  protected abstract void renameFile(String from, String to) throws IOException;
  protected abstract void saveFile(String fileName, byte[] content) throws IOException;

  /**
   * Download files button handler
   */
  private Response handleDownload(Map<String, Collection<String>> data) throws IOException {
    Collection<String> fileNames = data.get("f");
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    ZipOutputStream zipOutputStream = new ZipOutputStream(outputStream);
    if (fileNames == null) {
      fileNames = Collections.emptyList();
    }

    for (String fileName : fileNames) {
      Response response = getFile(fileName, false);
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
    log("sending file list");
    JsonBuilder builder = new JsonBuilder();
    builder.append('[');
    long id = 0;
    char comma = 0;
    for (FileData nextFile : getFileData()) {
      builder.append(comma);
      builder.append('{');
      builder.append("id", id++, true);
      builder.append("fileName", nextFile.fileName, true);
      builder.append("size", nextFile.size, true);
      builder.append("date", nextFile.date, false);
      builder.append('}');
      comma = ',';
    }
    builder.append(']');
    byte[] json = builder.getBytes();
    return new Response(new ByteArrayInputStream(json), json.length);
  }

  /**
   * Handler for File rename operations
   */
  private Response handleRename(Map<String, String> data) throws IOException {
    String from = data.get("from");
    String to = data.get("to");
    Response result;
    try {
      renameFile(from, to);
      result = handleStatus(true, "File renamed");
    } catch (IOException e) {
      result = handleStatus(false, e.getMessage());
    }
    return result;
  }

  /**
   * Handler to indicate operational status
   */
  private Response handleStatus(boolean success, String message) throws IOException {
    JsonBuilder out = new JsonBuilder();
    out.append('{');
    if (success) {
      out.append("success", message, false);
    } else {
      out.append("error", message, false);
    }
    out.append('}');
    return new Response(out.getBytes());
  }

  /**
   * Handler for file uploads
   */
  private Response handleUpload(Map<String, String> data) throws IOException {
    String fileName = data.get("fileName");
    String content = data.get("data");
    Response result;
    try {
      if (fileName == null || content == null) {
        result = handleStatus(false, "Invalid input");
      } else {
        saveFile(fileName, content.getBytes(UTF_8));
        result = handleStatus(true, "File saved");
      }
    } catch (Exception e) {
      result = handleStatus(false, e.getMessage());
    }
    return result;
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
    Response result;
    try {
      result = getFile(path, true);
    } catch (Exception e) {
      log("error: " + e);
      result = null;
    }
    return result != null ? result : new Response(SC_NOT_FOUND);
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
      InputStream inputStream = null;
      try {
        socket = serverSocket.accept();
        log("Accepted connection from " + socket.getRemoteSocketAddress().toString());
        inputStream = socket.getInputStream();
        Request request = new Request(socket, inputStream);
        request.invoke(inputStream, token);
      } catch (Exception e) {
        log("Server failed", e);
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
  }

  /**
   * BASIC file details
   */
  public static class FileData {
    private final String fileName;
    private final String date;
    private final long size;

    public FileData(String fileName, String date, long size) {
      this.fileName = fileName;
      this.date = date;
      this.size = size;
    }

    public FileData(File file) {
      DateFormat dateFormat = DateFormat.getDateInstance(DateFormat.DEFAULT);
      this.fileName = file.getName();
      this.date = dateFormat.format(file.lastModified());
      this.size = file.length();
    }
  }

  /**
   * Build JSON string
   */
  public static class JsonBuilder {
    private final StringBuilder json;

    JsonBuilder() {
      json = new StringBuilder();
    }

    void append(char chr) {
      if (chr > 0) {
        json.append(chr);
      }
    }

    void append(String field, String value, boolean nextField, boolean quote) {
      json.append("\"")
          .append(field)
          .append("\":")
          .append(quote ? "\"" : "")
          .append(value)
          .append(quote ? "\"" : "");
      if (nextField) {
        json.append(",");
      }
    }

    void append(String field, String value, boolean nextField) {
      this.append(field, value, nextField, true);
    }

    void append(String field, long value, boolean nextField) {
      this.append(field, String.valueOf(value), nextField, false);
    }

    byte[] getBytes() throws UnsupportedEncodingException {
      return json.toString().getBytes(UTF_8);
    }
  }

  /**
   * Server Request
   */
  public class Request {
    final Socket socket;
    final String method;
    final String url;
    final String token;
    final List<String> headers;

    public Request(Socket socket, InputStream inputStream) throws IOException {
      this.headers = getHeaders(inputStream);
      this.socket = socket;
      this.token = getToken(headers);
      String first = headers.get(0);
      String[] fields;
      if (first != null) {
        fields = first.split("\\s");
      } else {
        fields = null;
      }
      if (fields != null && fields.length > 1) {
        this.method = fields[0];
        this.url = fields[1];
      } else {
        this.method = null;
        this.url = null;
      }
    }

    public void invoke(InputStream inputStream, String tokenKey) throws IOException {
      if (!headers.isEmpty()) {
        log("URL: " + url);
        if ("GET".equals(method)) {
          handleGet(getParameters(url), tokenKey);
        } else if ("POST".equals(method)) {
          handlePost(getPostData(inputStream), tokenKey);
        } else if ("RUN".equals(method)) {
          handleRun(inputStream, tokenKey);
        } else {
          log("Invalid request");
        }
      }
    }

    /**
     * Returns the HTTP headers from the given stream
     */
    private List<String> getHeaders(InputStream stream) throws IOException {
      List<String> result = new ArrayList<>();
      ByteArrayOutputStream line = new ByteArrayOutputStream(LINE_SIZE);
      final char[] endHeader = {'\r', '\n', '\r', '\n'};
      int index = 0;
      int firstChar = 0;
      for (int b = firstChar = stream.read(); b != -1; b = stream.read()) {
        if (b == endHeader[index]) {
          index++;
          if (index == endHeader.length) {
            // end of headers
            break;
          } else if (index == 2) {
            // end of line
            result.add(line.toString());
            line.reset();
          }
        } else if (b == '\n' && ((char)firstChar) == '#') {
          // export from SmallBASIC
          result.add("RUN /import");
          result.add("cookie: " + TOKEN + "=" + line.toString().substring(2));
          break;
        } else {
          line.write(b);
          index = 0;
        }
      }
      return result;
    }

    /**
     * Reads a line of text from the given stream
     */
    private String getLine(InputStream stream) throws IOException {
      StringBuilder result = new StringBuilder();
      while (stream.available() != 0) {
        int b = stream.read();
        if (b == -1 || b == 10 || b == 13) {
          break;
        } else {
          result.append(Character.toChars(b));
        }
      }
      return result.toString();
    }

    /**
     * Parses HTTP GET parameters with the given name
     */
    private Map<String, Collection<String>> getParameters(String url) throws IOException {
      Map<String, Collection<String>> result = new HashMap<>();
      try {
        String[] args = URLDecoder.decode(url, UTF_8).split("[?]");
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
    private Map<String, String> getPostData(InputStream inputStream) throws IOException {
      String postData = getLine(inputStream);
      String[] fields = postData.split("&");
      Map<String, String> result = new HashMap<>();
      for (String nextField : fields) {
        int eq = nextField.indexOf("=");
        if (eq != -1) {
          String key = nextField.substring(0, eq);
          String value = URLDecoder.decode(nextField.substring(eq + 1), UTF_8);
          result.put(key, value);
        }
      }
      return result;
    }

    private String getToken(List<String> headers) {
      String result = null;
      for (String header : headers) {
        if (header.toLowerCase().startsWith("cookie:")) {
          String[] fields = header.split(":");
          if (fields.length == 2) {
            fields = fields[1].split("=");
            if (fields.length == 2 && TOKEN.equals(fields[0].trim())) {
              result = fields[1];
              break;
            }
          }
        }
      }
      return result;
    }

    /**
     * Handler for GET requests
     */
    private void handleGet(Map<String, Collection<String>> parameters, String tokenKey) throws IOException {
      if (url.startsWith("/api/download?")) {
        if (tokenKey.equals(token)) {
          handleDownload(parameters).send(socket, null);
        } else {
          log("Invalid token");
          new Response(SC_NOT_AUTHORISED).send(socket, null);
        }
      } else {
        handleWebResponse(url).send(socket, null);
      }
    }

    /**
     * Handler for POST requests
     */
    private void handlePost(Map<String, String> data, String tokenKey) throws IOException {
      String userToken = data.get(TOKEN);
      if (userToken == null) {
        userToken = token;
      }
      log("userToken=" + userToken);
      if (tokenKey.equals(userToken)) {
        if (url.startsWith("/api/login")) {
          handleFileList().send(socket, userToken);
        } else if (url.startsWith("/api/files")) {
          handleFileList().send(socket, null);
        } else if (url.startsWith("/api/upload")) {
          handleUpload(data).send(socket, null);
        } else if (url.startsWith("/api/rename")) {
          handleRename(data).send(socket, null);
        } else {
          new Response(SC_NOT_FOUND).send(socket, null);
        }
        log("Sent POST response");
      } else {
        log("Invalid token");
        if (url.startsWith("/api/login")) {
          handleStatus(false, "Invalid token").send(socket, null);
        } else {
          new Response(SC_NOT_AUTHORISED).send(socket, null);
        }
      }
    }

    private void handleRun(InputStream inputStream, String tokenKey) throws IOException {
      if (tokenKey.equals(token)) {        execStream(inputStream);
      } else {
        log("Invalid token");
      }
    }
  }

  /**
   * Server response data
   */
  public class Response {
    private final InputStream inputStream;
    private final long length;
    private final int errorCode;

    public Response(InputStream inputStream, long length) {
      this.inputStream = inputStream;
      this.length = length;
      this.errorCode = SC_OKAY;
    }

    public Response(byte[] bytes) {
      this.inputStream = new ByteArrayInputStream(bytes);
      this.length = bytes.length;
      this.errorCode = SC_OKAY;
    }

    public Response(int errorCode) throws IOException {
      byte[] bytes = ("Error: " + errorCode).getBytes(UTF_8);
      this.inputStream = new ByteArrayInputStream(bytes);
      this.length = bytes.length;
      this.errorCode = errorCode;
    }

    /**
     * Sends the response to the given socket
     */
    void send(Socket socket, String cookie) throws IOException {
      log("sendResponse() entered");
      String contentLength = "Content-length: " + length + "\r\n";
      BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
      String code = errorCode == SC_OKAY ? SC_OKAY + " OK" : String.valueOf(errorCode);
      out.write(("HTTP/1.0 " + code + "\r\n").getBytes());
      out.write("Content-type: text/html\r\n".getBytes());
      out.write(contentLength.getBytes());
      if (cookie != null) {
        out.write(("Set-Cookie: " + TOKEN + "=" + cookie + "\r\n").getBytes());
      }
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
      int sent = 0;
      try {
        byte[] bytes = new byte[BUFFER_SIZE];
        int read;
        while ((read = inputStream.read(bytes)) != -1) {
          outputStream.write(bytes, 0, read);
          outputStream.flush();
          sent += read;
        }
      } finally {
        inputStream.close();
      }
      return sent;
    }
  }
}
