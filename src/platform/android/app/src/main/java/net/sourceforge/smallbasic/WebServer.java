package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Minimal AppServer with adequate performance for a single user.
 * Uses some older java constructs to support SDK-16
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
  private static final ExecutorService THREAD_POOL = Executors.newCachedThreadPool();
  private static final String TEXT_HTML = "text/html";

  public WebServer() {
    super();
  }

  /**
   * Runs the WebServer in a new thread
   */
  public Thread run(final int portNum, final String token) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          runServer(portNum, token);
        } catch (Exception e) {
          log("Server failed to start: ", e);
        }
      }
    });
    socketThread.start();
    return socketThread;
  }

  protected abstract void deleteFile(String remoteHost, String fileName) throws IOException;
  protected abstract void execStream(String remoteHost, InputStream inputStream) throws IOException;
  protected abstract Response getFile(String remoteHost, String path, boolean asset) throws IOException;
  protected abstract Collection<FileData> getFileData(String remoteHost) throws IOException;
  protected abstract byte[] decodeBase64(String data);
  protected abstract void log(String message);
  protected abstract void log(String message, Exception exception);
  protected abstract void renameFile(String remoteHost, String from, String to) throws IOException;
  protected abstract void saveFile(String remoteHost, String fileName, byte[] content) throws IOException;

  /**
   * WebServer main loop to be run in a separate thread
   */
  private void runServer(final int portNum, final String token) throws IOException {
    log("Listening :" + portNum);
    log("Token :" + token);
    ServerSocket serverSocket;
    try {
      serverSocket = new ServerSocket(portNum);
    } catch (IllegalArgumentException e) {
      log("Failed to start server: ", e);
      throw new IOException(e);
    }
    while (true) {
      try {
        Socket socket = serverSocket.accept();
        log("Accepted connection from " + socket.getRemoteSocketAddress().toString());
        THREAD_POOL.submit(new Request(socket, token));
      } catch (Exception e) {
        log("Server failed", e);
        break;
      }
    }
  }

  /**
   * Server Request base class
   */
  public abstract class AbstractRequest {
    final Socket socket;
    final String method;
    final String url;
    final String requestToken;
    final String tokenKey;
    final List<String> headers;
    final InputStream inputStream;
    final String remoteHost;

    public AbstractRequest(Socket socket, String tokenKey) throws IOException {
      this.socket = socket;
      this.tokenKey = tokenKey;
      this.inputStream = socket.getInputStream();
      this.headers = getHeaders();
      this.requestToken = getToken(headers);
      this.remoteHost = ((InetSocketAddress) socket.getRemoteSocketAddress()).getHostName();
      String first = !headers.isEmpty() ? headers.get(0) : null;
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

    /**
     * Parses HTTP GET parameters with the given name
     */
    protected Map<String, Collection<String>> getParameters(String url) throws IOException {
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
    protected Map<String, FormField> getPostData(InputStream inputStream) throws IOException {
      String postData = getLine(inputStream);
      String[] fields = postData.split("&");
      Map<String, FormField> result = new HashMap<>();
      for (String nextField : fields) {
        int eq = nextField.indexOf("=");
        if (eq != -1) {
          String key = nextField.substring(0, eq);
          String value = nextField.substring(eq + 1);
          result.put(key, new FormField(key, value));
        }
      }
      return result;
    }

    /**
     * Returns the HTTP headers from the given stream
     */
    private List<String> getHeaders() throws IOException {
      List<String> result = new ArrayList<>();
      ByteArrayOutputStream line = new ByteArrayOutputStream(LINE_SIZE);
      final char[] endHeader = {'\r', '\n', '\r', '\n'};
      int index = 0;
      int firstChar;
      for (int b = firstChar = inputStream.read(); b != -1; b = inputStream.read()) {
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
      final DateFormat format = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z", Locale.ENGLISH);
      format.setLenient(false);
      format.setTimeZone(TimeZone.getTimeZone("UTC"));
      this.fileName = file.getName();
      this.date = format.format(file.lastModified());
      this.size = file.length();
    }
  }

  /**
   * Holder for POST form data
   */
  public class FormField {
    private static final String BASE_64_PREFIX = ";base64,";
    private final String string;
    private final byte[] bytes;

    public FormField(String key, String value) throws IOException {
      int index = value.indexOf(BASE_64_PREFIX);
      if (index != -1 && "data".equals(key)) {
        ByteArrayOutputStream data = new ByteArrayOutputStream();
        String base64Value = value.substring(index + BASE_64_PREFIX.length());
        data.write(decodeBase64(base64Value));
        this.string = null;
        this.bytes = data.toByteArray();
      } else {
        this.string = URLDecoder.decode(value, UTF_8);
        this.bytes = null;
      }
    }

    @Override
    public String toString() {
      return string;
    }

    public byte[] toByteArray() {
      return bytes;
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
  public class Request extends AbstractRequest implements Runnable {
    public Request(Socket socket, String tokenKey) throws IOException {
      super(socket, tokenKey);
    }

    @Override
    public void run() {
      try {
        if (!headers.isEmpty()) {
          log("URL: " + url);
          if ("GET".equals(method)) {
            handleGet(getParameters(url));
          } else if ("POST".equals(method)) {
            handlePost(getPostData(inputStream));
          } else if ("RUN".equals(method)) {
            handleRun(inputStream);
          } else {
            log("Invalid request");
          }
        }
      } catch (Exception e) {
        log("Request failed", e);
      }
      finally {
        afterRun();
      }
    }

    private void afterRun() {
      log("Socket cleanup");
      try {
        if (socket != null) {
          socket.close();
        }
        if (inputStream != null) {
          inputStream.close();
        }
      } catch (IOException e) {
        log("Socket cleanup failed: ", e);
      }
    }

    /**
     * Return all file names
     */
    private Collection<String> getAllFileNames() throws IOException {
      Collection<String> result = new ArrayList<>();
      for (FileData fileData : getFileData(remoteHost)) {
        result.add(fileData.fileName);
      }
      return result;
    }

    private byte[] getData(Map<String, FormField> data) {
      FormField field = data.get("data");
      return field == null ? null : field.toByteArray();
    }

    private String getString(Map<String, FormField> data, String key) {
      FormField field = data.get(key);
      return field == null ? null : field.toString();
    }

    /**
     * Download files button handler
     */
    private Response handleDownload(Map<String, Collection<String>> data) throws IOException {
      Collection<String> fileNames = data.get("f");
      if (fileNames == null || fileNames.isEmpty()) {
        fileNames = getAllFileNames();
      }
      Response result;
      if (fileNames.isEmpty()) {
        result = handleStatus(false, "File list is empty");
      } else if (fileNames.size() == 1) {
        // plain text download single file
        result = getFile(remoteHost, fileNames.iterator().next(), false);
      } else {
        // download multiple as zip
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        ZipOutputStream zipOutputStream = new ZipOutputStream(outputStream);
        Set<String> fileNameSet = new HashSet<>();
        for (String fileName : fileNames) {
          Response response = getFile(remoteHost, fileName, false);
          // de-duplicate the fileName entry to avoid a duplicate entry/zip exception
          int index = 1;
          String originalName = fileName;
          while (fileNameSet.contains(fileName)) {
            int dot = originalName.lastIndexOf('.');
            if (dot != -1) {
              String extension = originalName.substring(dot + 1);
              String name = originalName.substring(0, dot);
              fileName = String.format(Locale.ENGLISH, "%s (%d).%s", name, index, extension);
            } else {
              fileName = String.format(Locale.ENGLISH, "%s (%d)", originalName, index);
            }
            index++;
          }
          fileNameSet.add(fileName);
          ZipEntry entry = new ZipEntry(fileName);
          zipOutputStream.putNextEntry(entry);
          response.toStream(zipOutputStream);
          zipOutputStream.closeEntry();
        }
        zipOutputStream.finish();
        zipOutputStream.close();
        result = new Response(new ByteArrayInputStream(outputStream.toByteArray()), outputStream.size());
      }
      return result;
    }

    /**
     * Handler for files API
     */
    private Response handleFileList() throws IOException {
      log("Creating file list");
      JsonBuilder builder = new JsonBuilder();
      builder.append('[');
      long id = 0;
      char comma = 0;
      Response result;
      try {
        for (FileData nextFile : getFileData(remoteHost)) {
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
        result = new Response(new ByteArrayInputStream(json), json.length);
      } catch (Exception e) {
        result = handleStatus(false, e.getMessage());
      }
      return result;
    }

    /**
     * Handler for GET requests
     */
    private void handleGet(Map<String, Collection<String>> parameters) throws IOException {
      if (url.startsWith("/api/download?")) {
        if (tokenKey.equals(requestToken)) {
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
    private void handlePost(Map<String, FormField> data) throws IOException {
      String userToken = getString(data, TOKEN);
      if (userToken == null) {
        userToken = requestToken;
      }
      log("UserToken=" + userToken);
      if (tokenKey.equals(userToken)) {
        if (url.startsWith("/api/login")) {
          handleFileList().send(socket, userToken);
        } else if (url.startsWith("/api/files")) {
          handleFileList().send(socket, null);
        } else if (url.startsWith("/api/upload")) {
          handleUpload(data).send(socket, null);
        } else if (url.startsWith("/api/rename")) {
          handleRename(data).send(socket, null);
        } else if (url.startsWith("/api/delete")) {
          handleDelete(data).send(socket, null);
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

    /**
     * Handler for File delete
     */
    private Response handleDelete(Map<String, FormField> data) throws IOException {
      String fileName = getString(data, "fileName");
      Response result;
      try {
        deleteFile(remoteHost, fileName);
        log("Deleted " + fileName);
        result = handleFileList();
      } catch (Exception e) {
        result = handleStatus(false, e.getMessage());
      }
      return result;
    }

    /**
     * Handler for File rename operations
     */
    private Response handleRename(Map<String, FormField> data) throws IOException {
      String from = getString(data, "from");
      String to = getString(data, "to");
      Response result;
      try {
        renameFile(remoteHost, from, to);
        result = handleStatus(true, "File renamed");
      } catch (Exception e) {
        result = handleStatus(false, e.getMessage());
      }
      return result;
    }

    private void handleRun(InputStream inputStream) throws IOException {
      if (tokenKey.equals(requestToken)) {
        execStream(remoteHost, inputStream);
      } else {
        log("Invalid token");
      }
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
    private Response handleUpload(Map<String, FormField> data) throws IOException {
      String fileName = getString(data, "fileName");
      byte[] content = getData(data);
      Response result;
      try {
        if (fileName == null || content == null) {
          result = handleStatus(false, "Invalid input");
        } else {
          saveFile(remoteHost, fileName, content);
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
        result = getFile(remoteHost, path, true);
      } catch (Exception e) {
        log("Error: " + e);
        result = null;
      }
      return result != null ? result : new Response(SC_NOT_FOUND);
    }
  }

  /**
   * Server response data
   */
  public class Response {
    private final InputStream inputStream;
    private final long length;
    private final int errorCode;
    private final String contentType;

    public Response(InputStream inputStream, long length, String contentType) {
      this.inputStream = inputStream;
      this.length = length;
      this.errorCode = SC_OKAY;
      this.contentType = contentType;
    }

    public Response(InputStream inputStream, long length) {
      this.inputStream = inputStream;
      this.length = length;
      this.errorCode = SC_OKAY;
      this.contentType = TEXT_HTML;
    }

    public Response(byte[] bytes) {
      this.inputStream = new ByteArrayInputStream(bytes);
      this.length = bytes.length;
      this.errorCode = SC_OKAY;
      this.contentType = TEXT_HTML;
    }

    public Response(int errorCode) throws IOException {
      byte[] bytes = ("Error: " + errorCode).getBytes(UTF_8);
      this.inputStream = new ByteArrayInputStream(bytes);
      this.length = bytes.length;
      this.errorCode = errorCode;
      this.contentType = TEXT_HTML;
    }

    /**
     * Sends the response to the given socket
     */
    void send(Socket socket, String cookie) throws IOException {
      log("Sending response");
      String contentLength = "Content-length: " + length + "\r\n";
      BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
      String code = errorCode == SC_OKAY ? SC_OKAY + " OK" : String.valueOf(errorCode);
      out.write(("HTTP/1.0 " + code + "\r\n").getBytes(UTF_8));
      out.write(("Content-type: " + contentType + "\r\n").getBytes(UTF_8));
      out.write(contentLength.getBytes(UTF_8));
      if (cookie != null) {
        out.write(("Set-Cookie: " + TOKEN + "=" + cookie + "\r\n").getBytes(UTF_8));
      }
      out.write("Server: SmallBASIC for Android\r\n\r\n".getBytes(UTF_8));
      socket.setSendBufferSize(SEND_SIZE);
      int sent = toStream(out);
      log("Sent " + sent + " bytes");
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
