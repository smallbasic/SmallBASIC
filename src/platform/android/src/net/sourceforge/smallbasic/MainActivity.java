package net.sourceforge.smallbasic;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.DialogInterface;
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

  public int getUnicodeChar(int keyCode, int metaState) {
    return InputDevice.getDevice(0).getKeyCharacterMap().get(keyCode, metaState); 
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
}
