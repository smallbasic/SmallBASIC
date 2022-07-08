package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * WebServer
 *
 * @author chrisws
 */
public abstract class WebServer {
  public WebServer() {
    super();
  }

  public void startServer(final int socketNum, final String token) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          runServer(socketNum, token);
        }
        catch (IOException e) {
          log("startServer failed: ", e);
        }
      }
    });
    socketThread.start();
  }

  protected abstract void execStream(String line, InputStream inputStream) throws IOException;
  protected abstract void log(String message);
  protected abstract void log(String message, Exception exception);
  protected abstract Reader openAsset(String path) throws IOException;

  private String buildTokenForm() {
    return "<p>Enter access token:</p><form method=post><input type=text name=token>" +
           "<input value=OK name=okay type=submit style='vertical-align:top'></form>";
  }

  private String getDownload(String field) {
    return null;
  }

  private Map<String, String> getPostData(DataInputStream inputStream, final String line) throws IOException {
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

  private String loadWebAsset(String asset) throws IOException {
    String path;
    if (asset.equals("/")) {
      path = "index.html";
    }
    else if (asset.startsWith("/")) {
      path = asset.substring(1);
    } else {
      path = asset;
    }
    Reader input = openAsset(path);
    String result = readInput(input);
    input.close();
    return result;
  }

  private String loadWebUI(String token) {
    String result = null;
    try {
      result = loadWebAsset("index.html");
      result = result.replace("%TOKEN%", token);
    } catch (IOException e) {
      log("readBuffer failed: ", e);
    }
    return result;
  }

  private String readInput(Reader inputReader) throws IOException {
    StringBuilder out = new StringBuilder();
    int c;
    for (c = inputReader.read(); c != -1; c = inputReader.read()) {
      out.append((char) c);
    }
    return out.toString();
  }

  private String readLine(DataInputStream inputReader) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream(128);
    int b;
    for (b = inputReader.read(); b != -1 && b != '\n'; b = inputReader.read()) {
      if (b != '\r') {
        out.write(b);
      }
    }
    return b == -1 ? null : out.size() == 0 ? "" : out.toString();
  }

  private void runServer(final int socketNum, final String token) throws IOException {
    log("Listening :" + socketNum);
    log("Token :" + token);
    ServerSocket serverSocket;
    try {
      serverSocket = new ServerSocket(socketNum);
    }
    catch (IllegalArgumentException e) {
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
          String[] fields = line.split("\\s");
          if ("GET".equals(fields[0]) && fields.length > 1) {
            log(line);
            if (fields[1].startsWith("/api/download?")) {
              sendResponse(socket, getDownload(fields[1]));
            } else {
              sendResponse(socket, loadWebAsset(fields[1]));
            }
          } else if ("POST".equals(fields[0])) {
            Map<String, String> postData = getPostData(inputStream, line);
            String userToken = postData.get("token");
            log("userToken="+ userToken);
            if (token.equals(userToken)) {
              String buffer = postData.get("src");
              if (buffer != null) {
                //execBuffer(buffer, WEB_BAS, postData.get("run") != null);
                sendResponse(socket, loadWebUI(token));
              } else {
                sendResponse(socket, loadWebUI(token));
              }
            } else {
              // invalid token
              sendResponse(socket, buildTokenForm());
            }
            log("Sent POST response");
          } else if (line.contains(token)) {
            execStream(line, inputStream);
          } else {
            log("Invalid request");
          }
        }
      }
      catch (IOException e) {
        log("Server failed: ", e);
        break;
      }
      finally {
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

  private void sendResponse(Socket socket, String content) throws IOException {
    log("sendResponse() entered");
    String contentLength ="Content-length: " + content.length() + "\r\n";
    BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
    out.write("HTTP/1.0 200 OK\r\n".getBytes());
    out.write("Content-type: text/html\r\n".getBytes());
    out.write(contentLength.getBytes());
    out.write("Server: SmallBASIC for Android\r\n\r\n".getBytes());
    out.write(content.getBytes(Charset.forName("utf-8")));
    out.flush();
    out.close();
  }
}
