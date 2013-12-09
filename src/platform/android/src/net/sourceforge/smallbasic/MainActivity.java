package net.sourceforge.smallbasic;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
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
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
      }
    });
  }

  protected void createListenSocket(final int socketNum) {
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        try {
          ServerSocket serverSocket = new ServerSocket(socketNum);
          Log.i(TAG, "Listening :" + socketNum);
          while (true) {
            Socket socket = null;
            DataInputStream inputStream = null;
            try {
              socket = serverSocket.accept();
              Log.i(TAG, "Accepted connection from " + socket.getRemoteSocketAddress().toString());
              inputStream = new DataInputStream(socket.getInputStream());
              File outputFile = getApplication().getFileStreamPath("download.bas");
              BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(outputFile));
              readStream(inputStream, output);
              Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
              runFile(outputFile.getAbsolutePath());
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
        catch (IOException e) {
          Log.i(TAG, "Failed: " + e.toString());
        }
      }
    });
    socketThread.start();
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    try {
      Properties p = new Properties();
      p.load(getApplication().openFileInput("settings.txt"));
      int socket = Integer.valueOf(p.getProperty("listenSocket", "8888"));
      if (socket != -1) {
        createListenSocket(socket);
      }
    } catch (Exception e) {
      Log.i(TAG, "Failed: " + e.toString());
    }
  }

  private void readStream(InputStream input, OutputStream output) throws IOException {
    while (true) {
      byte [] buffer = new byte[1024];
      int read = input.read(buffer);
      if (read != -1) {
        output.write(buffer, 0, read);
      }
      else {
        break;
      }
    }
    output.close();
  }
}
