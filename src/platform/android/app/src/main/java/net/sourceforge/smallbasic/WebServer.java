package net.sourceforge.smallbasic;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
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
   * Returns the HTTP headers from the given stream
   */
  private List<String> getHeaders(InputStream stream) throws IOException {
    List<String> result = new ArrayList<>();
    ByteArrayOutputStream line = new ByteArrayOutputStream(LINE_SIZE);
    final char[] endHeader = {'\r', '\n', '\r', '\n'};
    int index = 0;
    for (int b = stream.read(); b != -1; b = stream.read()) {
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
  private Map<String, String> getPostData(InputStream inputStream, final String line) throws IOException {
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
   * Handler for failed token validation
   */
  private Response handleError(String error) throws IOException {
    JsonBuilder out = new JsonBuilder();
    out.append('{');
    out.append("error", error, false);
    out.append('}');
    return new Response(out.getBytes());
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
   * Handler for GET requests
   */
  private void handleGet(Socket socket, String token, List<String> headers, String url,
                         Map<String, Collection<String>> parameters) throws IOException {
    if (url.startsWith("/api/download?")) {
      if (hasTokenCookie(headers, token)) {
        handleDownload(parameters).send(socket, null);
      }
    } else {
      handleWebResponse(url).send(socket, null);
    }
  }

  /**
   * Handler for POST requests
   */
  private void handlePost(Socket socket, String token, String url,
                          Map<String, String> parameters) throws IOException {
    String userToken = parameters.get(TOKEN);
    log("userToken=" + userToken);
    if (token.equals(userToken)) {
      if (url.startsWith("/api/files")) {
        handleFileList().send(socket, token);
      }
      log("Sent POST response");
    } else {
      log("Invalid token");
      handleError("invalid token").send(socket, null);
    }
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
   * Inspects the headers to ensure the token cookie is present
   */
  private boolean hasTokenCookie(List<String> headers, String token) {
    boolean result = false;
    for (String header : headers) {
      if (header.startsWith("Cookie: ") && header.contains(TOKEN) && header.contains(token)) {
        result = true;
        break;
      }
    }
    return result;
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
        List<String> headers = getHeaders(inputStream);
        if (!headers.isEmpty()) {
          String line = headers.get(0);
          log(line);
          String[] fields = line.split("\\s");
          if ("GET".equals(fields[0]) && fields.length > 1) {
            Map<String, Collection<String>> parameters = getParameters(fields[1]);
            handleGet(socket, token, headers, fields[1], parameters);
          } else if ("POST".equals(fields[0]) && fields.length > 1) {
            Map<String, String> parameters = getPostData(inputStream, line);
            handlePost(socket, token, fields[1], parameters);
          } else {
            log("Invalid request");
          }
        }
      } catch (Throwable e) {
        log("Server failed");
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
    private final String fileName;
    private final String date;
    private final long size;

    public FileData(String fileName, String date, long size) {
      this.fileName = fileName;
      this.date = date;
      this.size = size;
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
   * Server response data
   */
  public class Response {
    private final InputStream inputStream;
    private final long length;

    public Response(InputStream inputStream, long length) {
      this.inputStream = inputStream;
      this.length = length;
    }

    public Response(String data) throws IOException {
      this.inputStream = new ByteArrayInputStream(data.getBytes(UTF_8));
      this.length = data.length();
    }

    public Response(byte[] bytes) {
      this.inputStream = new ByteArrayInputStream(bytes);
      this.length = bytes.length;
    }

    InputStream getInputStream() {
      return inputStream;
    }

    long getLength() {
      return length;
    }

    /**
     * Sends the response to the given socket
     */
    void send(Socket socket, String session) throws IOException {
      log("sendResponse() entered");
      String contentLength = "Content-length: " + length + "\r\n";
      BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
      out.write("HTTP/1.0 200 OK\r\n".getBytes());
      out.write("Content-type: text/html\r\n".getBytes());
      out.write(contentLength.getBytes());
      if (session != null) {
        out.write(("Set-Cookie: " + TOKEN + "=" + session + "\r\n").getBytes());
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
