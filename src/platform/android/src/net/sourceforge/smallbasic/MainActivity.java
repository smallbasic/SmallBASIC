package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.InputDevice;
import android.view.inputmethod.InputMethodManager;

/**
 * Extends NativeActivity to provide interface methods for runtime.cpp
 *
 * @author chrisws
 */
public class MainActivity extends NativeActivity {
  private static final String BUFFER_BAS = "web.bas";
  private static final String TAG = "smallbasic";

  static {
    System.loadLibrary("smallbasic");
  }

  public static native boolean optionSelected(int eventBuffer);
  public static native void runFile(String fileName);

  public int getUnicodeChar(int keyCode, int metaState) {
    int result = 0;
    InputDevice device = InputDevice.getDevice(InputDevice.getDeviceIds()[0]);
    if (device != null) {
      result = device.getKeyCharacterMap().get(keyCode, metaState);
    } else {
      Log.i(TAG, "Device not found");
    }
    return result;
  }

  public void optionsBox(final String[] items) {
    final Activity activity = this;
    runOnUiThread(new Runnable() {
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setItems(items, new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int index) {
            Log.i(TAG, "items clicked = " + index);
            optionSelected(index);
          }
        });
        builder.create().show();
      }
    });
  }

  public void showKeypad() {
    runOnUiThread(new Runnable() {
      public void run() {
        InputMethodManager imm = (InputMethodManager) 
            getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(getWindow().getDecorView(),
            InputMethodManager.SHOW_IMPLICIT);
      }
    });
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    try {
      Properties p = new Properties();
      p.load(getApplication().openFileInput("settings.txt"));
      int socket = Integer.valueOf(p.getProperty("serverSocket", "-1"));
      String token = p.getProperty("serverToken", new Date().toString());
      if (socket != -1) {
        startServer(socket, token);
      }
    } catch (Exception e) {
      Log.i(TAG, "Failed: " + e.toString());
    }
  }

  private String buildRunForm(String buffer, String token) {
    String response = new StringBuilder()
      .append("<form method=post>")
      .append("<input type=hidden name=token value='")
      .append(token)
      .append("'><textarea cols=60 rows=30 name=src>")
      .append(buffer)
      .append("</textarea>")
      .append("<input value=Run name=run type=submit style='vertical-align:top'>")
      .append("<input value=Save name=save type=submit style='vertical-align:top'>")
      .append("</form>").toString();
    return response;
  }

  private String buildTokenForm() {
    String response = new StringBuilder()
      .append("<p>Enter access token:</p>")
      .append("<form method=post>")
      .append("<input type=text name=token>")
      .append("<input value=OK name=okay type=submit style='vertical-align:top'>")
      .append("</form>").toString();
    return response;
  }

  private void execBuffer(String buffer, boolean run) throws IOException {
    File outputFile = getApplication().getFileStreamPath(BUFFER_BAS);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    output.write(buffer);
    output.close();
    if (run) {
      Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
      runFile(outputFile.getAbsolutePath());
    }
  }

  private void execStream(String line, DataInputStream inputStream) throws IOException {
    File outputFile = getApplication().getFileStreamPath(BUFFER_BAS);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    while (line != null) {
      output.write(line);  
      line = readLine(inputStream);
    }
    output.close();
    Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
    runFile(outputFile.getAbsolutePath());
  }
  
  private Map<String, String> getPostData(DataInputStream inputStream, String line)
      throws IOException, UnsupportedEncodingException {
    int length = 0;
    final String lengthHeader = "content-length: ";
    while (line != null) {
      if (line.toLowerCase().startsWith(lengthHeader)) {
        length = Integer.valueOf(line.substring(lengthHeader.length()));
      }
      line = readLine(inputStream);
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
    Map<String, String> result = new HashMap<String, String>();
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

  private String readBuffer() {
    StringBuilder result = new StringBuilder();
    try {
      File inputFile = getApplication().getFileStreamPath(BUFFER_BAS);
      BufferedReader input = new BufferedReader(new FileReader(inputFile));
      String line = input.readLine();
      while (line != null) {
        result.append(line).append("\n");
        line = input.readLine();
      }
      input.close();
    } catch (FileNotFoundException e) {
      Log.i(TAG, "Failed: " + e.toString());
    } catch (IOException e) {
      Log.i(TAG, "Failed: " + e.toString());
    }
    return result.toString();
  }

  private String readLine(DataInputStream inputReader) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream(128);
    for (int b = inputReader.read(); b != -1 && b != '\n'; b = inputReader.read()) {
      if (b != '\r') {
        out.write(b);
      }
    }
    return out.size() == 0 ? null : out.toString();
  }

  private void runServer(final int socketNum, final String token) throws IOException {
    ServerSocket serverSocket = new ServerSocket(socketNum);
    Log.i(TAG, "Listening :" + socketNum);
    Log.i(TAG, "Token :" + token);
    while (true) {
      Socket socket = null;
      DataInputStream inputStream = null;
      try {
        socket = serverSocket.accept();
        Log.i(TAG, "Accepted connection from " + socket.getRemoteSocketAddress().toString());
        inputStream = new DataInputStream(socket.getInputStream());
        String line = readLine(inputStream);
        if (line != null) {
          String[] fields = line.split("\\s");
          if ("GET".equals(fields[0])) {
            Log.i(TAG, line);
            sendResponse(socket, buildTokenForm());
          } else if ("POST".equals(fields[0])) {
            Map<String, String> postData = getPostData(inputStream, line);
            String userToken = postData.get("token");
            Log.i(TAG, "userToken="+ userToken);
            if (token.equals(userToken)) {
              String buffer = postData.get("src");
              if (buffer != null) {
                execBuffer(buffer, postData.get("run") != null);
                sendResponse(socket, buildRunForm(buffer, token));
              } else { 
                sendResponse(socket, buildRunForm(readBuffer(), token));
              }
            } else {
              // invalid token
              sendResponse(socket, buildTokenForm());
            }
            Log.i(TAG, "Sent POST response");
          } else if (line.indexOf(token) != -1) {
            execStream(line, inputStream);
          } else {
            Log.i(TAG, "Invalid request");
          }
        }
      }
      catch (IOException e) {
        Log.i(TAG, "Failed: " + e.toString());
      }
      finally {
        Log.i(TAG, "socket cleanup");
        socket.close();
        if (inputStream != null) {
          inputStream.close();
        }
      }
    }
  }
  private void sendResponse(Socket socket, String content) throws IOException {
    Log.i(TAG, "sendResponse() entered");
    String contentLength ="Content-length: " + content.length() + "\r\n"; 
    BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());
    out.write("HTTP/1.0 200 OK\r\n".getBytes());
    out.write("Content-type: text/html\r\n".getBytes());
    out.write(contentLength.getBytes());
    out.write("Server: SmallBASIC for Android\r\n\r\n".getBytes());
    out.write(content.getBytes());
    out.flush();
    out.close();
  }
  
  private void startServer(final int socketNum, final String token) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          runServer(socketNum, token);
        }
        catch (IOException e) {
          Log.i(TAG, "Failed: " + e.toString());
        }
      }
    });
    socketThread.start();
  }
}
