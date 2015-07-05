package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.URLDecoder;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.zip.GZIPInputStream;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.Base64;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.InputDevice;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

/**
 * Extends NativeActivity to provide interface methods for runtime.cpp
 *
 * @author chrisws
 */
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
public class MainActivity extends NativeActivity {
  private static final String TAG = "smallbasic";
  private static final String WEB_BAS = "web.bas";
  private static final String SCHEME_BAS = "qrcode.bas";
  private static final String SCHEME = "smallbasic://x/";
  private static final int BASE_FONT_SIZE = 18;
  private String _startupBas = null;
  private boolean _untrusted = false;
  private ExecutorService _audioExecutor = Executors.newSingleThreadExecutor();
  private Queue<Sound> _sounds = new ConcurrentLinkedQueue<Sound>();
  private String[] _options = null;

  static {
    System.loadLibrary("smallbasic");
  }

  public static native boolean optionSelected(int index);
  public static native void onResize(int width, int height);
  public static native void runFile(String fileName);

  public boolean ask(String prompt, String accept, String cancel) {
    // TODO: fixme
    boolean result = false;
    final Context context = this;
    final Semaphore mutex = new Semaphore(0);
    final Runnable runnable = new Runnable() {
      public void run() {
        new AlertDialog.Builder(activity)
          .setTitle(title).setMessage(message)
          .setPositiveButton(accept, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
            }
          })
          .setPositiveButton(cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {}
          }).show();
        mutex.release();
      }
    };
    runOnUiThread(runnable);
    try {
      mutex.acquire();
    } catch (InterruptedException e) {
      Log.i(TAG, "getClipboardText failed: ", e);
      e.printStackTrace();
    }
    return result;
  }

  public void clearSoundQueue() {
    Log.i(TAG, "clearSoundQueue");
    for (Sound sound : _sounds) {
      sound.setSilent(true);
    }
  }

  public String getClipboardText() {
    final StringBuilder result = new StringBuilder();
    final Context context = this;
    final Semaphore mutex = new Semaphore(0);
    final Runnable runnable = new Runnable() {
      public void run() {
        ClipboardManager clipboard =
          (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = clipboard.getPrimaryClip();
        if (clip != null && clip.getItemCount() > 0) {
          ClipData.Item item = clip.getItemAt(0);
          if (item != null) {
            String data = item.coerceToText(context).toString();
            result.append(data == null ? "" : data);
          }
        }
        mutex.release();
      }
    };
    runOnUiThread(runnable);
    try {
      mutex.acquire();
    } catch (InterruptedException e) {
      Log.i(TAG, "getClipboardText failed: ", e);
      e.printStackTrace();
    }
    return result.toString();
  }

  public String getIPAddress() {
    String result = null;
    try {
      for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
        NetworkInterface intf = en.nextElement();
        for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
          InetAddress inetAddress = enumIpAddr.nextElement();
          if (!inetAddress.isLoopbackAddress()) {
            result = inetAddress.getHostAddress().toString();
          }
        }
      }
    } catch (SocketException e) {
      Log.i(TAG, "getIPAddress failed: ", e);
    }
    Log.i(TAG, "getIPAddress: " + result);
    return result;
  }

  public boolean getSoundPlaying() {
    boolean result = this._sounds.size() > 0;
    Log.i(TAG, "getSoundPlaying = " + result);
    return result;
  }

  public String getStartupBas() {
    return this._startupBas == null ? "" : this._startupBas;
  }

  public int getStartupFontSize() {
    DisplayMetrics metrics = Resources.getSystem().getDisplayMetrics();
    return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, BASE_FONT_SIZE, metrics);
  }

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

  public boolean getUntrusted() {
    Log.i(TAG, "getUntrusted");
    return this._untrusted;
  }

  @Override
  public void onGlobalLayout() {
    super.onGlobalLayout();
    // find the visible coordinates of our view
    Rect rect = new Rect();
    findViewById(android.R.id.content).getWindowVisibleDisplayFrame(rect);
    onResize(rect.width(), rect.height());
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    if (this._options != null) {
      int index = 0;
      while (index < this._options.length) {
        if (_options[index].equals(item.toString())) {
          break;
        }
        index++;
      }
      Log.i(TAG, "item clicked = " + index);
      optionSelected(index);
    }
    return true;
  }
  
  @Override
  public boolean onPrepareOptionsMenu(Menu menu) {
    if (this._options != null) {
      menu.clear();
      for (String option : this._options) {
        menu.add(option);
      }
    }
    return super.onPrepareOptionsMenu(menu);
  }

  public void optionsBox(final String[] items) {
    this._options = items;
    runOnUiThread(new Runnable() {
      public void run() {
        openOptionsMenu();
      }
    });
  }

  public void playTone(int frq, int dur, int vol) {
    float volume = (vol / 100f);
    final Sound sound = new Sound(frq, dur, volume);
    _sounds.add(sound);
    _audioExecutor.execute(new Runnable() {
      @Override
      public void run() {
        sound.play();
        _sounds.remove(sound);
      }
    });
  }

  public void setClipboardText(final String text) {
    runOnUiThread(new Runnable() {
      public void run() {
        ClipboardManager clipboard =
         (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText("text", text);
       clipboard.setPrimaryClip(clip);
      }
    });
  }
  
  public void showAlert(final String title, final String message) {
    final Activity activity = this;
    runOnUiThread(new Runnable() {
      public void run() {
        new AlertDialog.Builder(activity)
          .setTitle(title).setMessage(message)
          .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {}
          }).show();
      }
    });
  }

  public void showKeypad(final boolean show) {
    Log.i(TAG, "showKeypad: " + show);
    final View view = getWindow().getDecorView();
    runOnUiThread(new Runnable() {
      public void run() {
        InputMethodManager imm = (InputMethodManager)
            getSystemService(Context.INPUT_METHOD_SERVICE);
        if (show) {
          imm.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT);
        } else {
          imm.hideSoftInputFromWindow(view.getWindowToken(),
              InputMethodManager.HIDE_NOT_ALWAYS);
        }
      }
    });
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Intent intent = getIntent();
    Uri uri = intent.getData();
    if (uri != null) {
      String data = intent.getDataString();
      if (data.startsWith(SCHEME)) {
        execScheme(data);
        Log.i(TAG, "data="+ data);
      } else {
        _startupBas = uri.getPath();
      }
      Log.i(TAG, "startupBas="+ _startupBas);
    }
    try {
      Properties p = new Properties();
      p.load(getApplication().openFileInput("settings.txt"));
      int socket = Integer.valueOf(p.getProperty("serverSocket", "-1"));
      String token = p.getProperty("serverToken", new Date().toString());
      if (socket != -1) {
        startServer(socket, token);
      } else {
        Log.i(TAG, "Web service disabled");
      }
    } catch (Exception e) {
      Log.i(TAG, "Failed to start web service: ", e);
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

  private String execBuffer(final String buffer, final String name, boolean run) throws IOException {
    File outputFile = getApplication().getFileStreamPath(name);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    output.write(buffer);
    output.close();
    if (run) {
      Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
      runFile(outputFile.getAbsolutePath());
    }
    return outputFile.getAbsolutePath();
  }

  private void execScheme(final String data) {
    try {
      String input = data.substring(SCHEME.length());
      byte[] decodedBytes = Base64.decode(input, Base64.DEFAULT);
      int magic = (decodedBytes[1] & 0xFF) << 8 | decodedBytes[0];
      BufferedReader reader;
      String bas;
      if (magic == GZIPInputStream.GZIP_MAGIC) {
        GZIPInputStream zipStream = new GZIPInputStream(new ByteArrayInputStream(decodedBytes));
        reader = new BufferedReader(new InputStreamReader(zipStream));
        StringBuilder out = new StringBuilder();
        String s;
        while ((s = reader.readLine()) != null) {
          out.append(s).append('\n');
        }
        bas = out.toString();
      } else {
        bas = URLDecoder.decode(input, "utf-8");
      }
      _startupBas = execBuffer(bas, SCHEME_BAS, false);
      _untrusted = true;
    } catch (IOException e) {
      Log.i(TAG, "saveSchemeData failed: ", e);
    }
  }

  private void execStream(String line, DataInputStream inputStream) throws IOException {
    File outputFile = getApplication().getFileStreamPath(WEB_BAS);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    Log.i(TAG, "execStream() entered");
    while (line != null) {
      output.write(line + "\n");
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
    while (line != null && line.length() > 0) {
      if (line.toLowerCase(Locale.ENGLISH).startsWith(lengthHeader)) {
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
      File inputFile = getApplication().getFileStreamPath(WEB_BAS);
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
    int b;
    for (b = inputReader.read(); b != -1 && b != '\n'; b = inputReader.read()) {
      if (b != '\r') {
        out.write(b);
      }
    }
    return b == -1 ? null : out.size() == 0 ? "" : out.toString();
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
                execBuffer(buffer, WEB_BAS, postData.get("run") != null);
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
