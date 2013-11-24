package net.sourceforge.smallbasic;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.DialogInterface;
import android.util.Log;

/**
 * Extends NativeActivity to provide interface methods for runtime.cpp
 *
 * @author chrisws
 */
public class MainActivity extends NativeActivity {
  private static final String TAG = "smallbasic";

  public static native boolean optionSelected(int eventBuffer);
  
  static {
    System.loadLibrary("smallbasic");
  }
  
  public void optionsBox(final String[] items) {
    final Activity activity = this;
    runOnUiThread(new Runnable() {
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setItems(items, new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int index) {
            Log.v(TAG, "items clicked = " + index);
            optionSelected(index);
          }
        });
        builder.create().show();
      }
    });
  }
}
