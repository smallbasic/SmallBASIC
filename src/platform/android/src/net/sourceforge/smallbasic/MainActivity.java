package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
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
  private String buffer = "";

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
      int socket = Integer.valueOf(p.getProperty("listenSocket", "-1"));
      if (socket != -1) {
        startServer(socket);
      }
    } catch (Exception e) {
      Log.i(TAG, "Failed: " + e.toString());
    }
  }

  private String buildResponse() {
    String response = new StringBuilder()
      .append("<form method=post>")
      .append("<textarea cols=60 rows=30 name=src>")
      .append(this.buffer)
      .append("</textarea>")
      .append("<input value=Run type=submit style='vertical-align:top'>")
      .append("</form>").toString();
    return response;
  }

  private void execBuffer() throws IOException {
    File outputFile = getApplication().getFileStreamPath(BUFFER_BAS);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    output.write(this.buffer);
    output.close();
    Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
    runFile(outputFile.getAbsolutePath());
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
  
  private String getPostData(DataInputStream inputStream, String line, String field)
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
    String result = "";
    for (String nextField : fields) {
      int eq = nextField.indexOf(field + "=");
      if (eq != -1) {
        result = nextField.substring(eq + field.length() + 1);
      }
    }
    return URLDecoder.decode(result, "utf-8");
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

  private void runServer(final int socketNum) throws IOException {
    ServerSocket serverSocket = new ServerSocket(socketNum);
    Log.i(TAG, "Listening :" + socketNum);
    while (true) {
      Socket socket = null;
      DataInputStream inputStream = null;
      try {
        socket = serverSocket.accept();
        Log.i(TAG, "Accepted connection from " + socket.getRemoteSocketAddress().toString());
        inputStream = new DataInputStream(socket.getInputStream());
        String line = readLine(inputStream);
        String[] fields = line.split("\\s");
        if ("GET".equals(fields[0])) {
          Log.i(TAG, line);
          sendResponse(socket, buildResponse());
        } else if ("POST".equals(fields[0])) {
          this.buffer = getPostData(inputStream, line, "src");
          execBuffer();
          sendResponse(socket, buildResponse());
          Log.i(TAG, "Sent POST response");
        } else {
          execStream(line, inputStream);
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
  
  private void startServer(final int socketNum) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          runServer(socketNum);
        }
        catch (IOException e) {
          Log.i(TAG, "Failed: " + e.toString());
        }
      }
    });
    socketThread.start();
  }
}
