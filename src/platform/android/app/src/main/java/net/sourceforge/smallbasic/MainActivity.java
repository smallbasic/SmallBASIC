package net.sourceforge.smallbasic;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Rect;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Base64;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collection;
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

import dalvik.system.BaseDexClassLoader;

/**
 * Extends NativeActivity to provide interface methods for runtime.cpp
 *
 * @author Chris Warren-Smith
 */
public class MainActivity extends NativeActivity {
  private static final String TAG = "smallbasic";
  private static final String WEB_BAS = "web.bas";
  private static final String SCHEME_BAS = "qrcode.bas";
  private static final String SCHEME = "smallbasic://x/";
  private static final String CP1252 = "Cp1252";
  private static final int BASE_FONT_SIZE = 18;
  private static final long LOCATION_INTERVAL = 1000;
  private static final float LOCATION_DISTANCE = 1;
  private static final int REQUEST_STORAGE_PERMISSION = 1;
  private static final int REQUEST_LOCATION_PERMISSION = 2;
  private static final String FOLDER_NAME = "SmallBASIC";
  private static final int COPY_BUFFER_SIZE = 1024;
  private String _startupBas = null;
  private boolean _untrusted = false;
  private final ExecutorService _audioExecutor = Executors.newSingleThreadExecutor();
  private final Queue<Sound> _sounds = new ConcurrentLinkedQueue<>();
  private final Handler _keypadHandler = new Handler(Looper.getMainLooper());
  private String[] _options = null;
  private MediaPlayer _mediaPlayer = null;
  private LocationAdapter _locationAdapter = null;
  private TextToSpeechAdapter _tts;

  static {
    System.loadLibrary("smallbasic");
  }

  public static native void onActivityPaused(boolean paused);
  public static native void onResize(int width, int height);
  public static native void onUnicodeChar(int ch);
  public static native boolean optionSelected(int index);
  public static native void runFile(String fileName);
  public static native void setenv(String name, String value);

  public void addShortcut(final byte[] pathBytes) {
    final String path = getString(pathBytes);
    Intent shortcut = new Intent(getApplicationContext(), MainActivity.class);
    shortcut.setAction(Intent.ACTION_MAIN);
    shortcut.setData(Uri.parse("smallbasic://" + path));
    Intent intent = new Intent();
    int index = path.lastIndexOf('/');
    String name = (index == -1) ? path : path.substring(index + 1);
    intent.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcut);
    intent.putExtra(Intent.EXTRA_SHORTCUT_NAME, name);
    intent.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE,
                    Intent.ShortcutIconResource.fromContext(getApplicationContext(),
                                                            R.drawable.ic_launcher));
    intent.putExtra("duplicate", false);
    intent.setAction("com.android.launcher.action.INSTALL_SHORTCUT");
    getApplicationContext().sendBroadcast(intent);
  }

  public int ask(final byte[] titleBytes, final byte[] promptBytes, final boolean cancel) {
    final String title = getString(titleBytes);
    final String prompt = getString(promptBytes);
    final AskResult result = new AskResult();
    final Activity activity = this;
    final Semaphore mutex = new Semaphore(0);
    final Runnable runnable = new Runnable() {
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setTitle(title).setMessage(prompt);
        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int which) {
            result.setYes();
            mutex.release();
          }
        });
        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int which) {
            result.setNo();
            mutex.release();
          }
        });
        builder.setOnCancelListener(new OnCancelListener() {
          @Override
          public void onCancel(DialogInterface dialog) {
            result.setCancel();
            mutex.release();
          }
        });
        if (cancel) {
          builder.setNeutralButton("Cancel", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
              result.setCancel();
              mutex.release();
            }
          });
        }
        builder.show();
      }
    };
    runOnUiThread(runnable);
    try {
      mutex.acquire();
    } catch (InterruptedException e) {
      Log.i(TAG, "ask failed: ", e);
    }
    Log.i(TAG, "ask result=" + result);
    return result.value;
  }

  public void browseFile(final byte[] pathBytes) {
    try {
      String url = new String(pathBytes, CP1252);
      if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
      }
      Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
      startActivity(browserIntent);
    } catch (Exception e) {
      Log.i(TAG, "browseFile failed: ", e);
    }
  }

  public void clearSoundQueue() {
    Log.i(TAG, "clearSoundQueue");
    for (Sound sound : _sounds) {
      sound.setSilent(true);
    }
    if (_mediaPlayer != null) {
      _mediaPlayer.release();
      _mediaPlayer = null;
    }
  }

  public boolean closeLibHandlers() {
    if (_tts != null) {
      _tts.stop();
    }
    return removeLocationUpdates();
  }

  public byte[] getClipboardText() {
    final StringBuilder text = new StringBuilder();
    final Context context = this;
    final Semaphore mutex = new Semaphore(0);
    final Runnable runnable = new Runnable() {
      public void run() {
        ClipboardManager clipboard =
          (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = clipboard != null ? clipboard.getPrimaryClip() : null;
        if (clip != null && clip.getItemCount() > 0) {
          ClipData.Item item = clip.getItemAt(0);
          if (item != null) {
            CharSequence data = item.coerceToText(context);
            text.append(data == null ? "" : data);
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
    }
    byte[] result;
    try {
      result = text.toString().getBytes(CP1252);
    } catch (UnsupportedEncodingException e) {
      Log.i(TAG, "getClipboardText failed: ", e);
      result = null;
    }
    return result;
  }

  public String getIpAddress() {
    String result = "";
    try {
      for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
        NetworkInterface network = en.nextElement();
        if (!network.getDisplayName().startsWith("dummy")) {
          for (Enumeration<InetAddress> address = network.getInetAddresses(); address.hasMoreElements();) {
            InetAddress inetAddress = address.nextElement();
            if (!inetAddress.isLoopbackAddress() && inetAddress instanceof Inet4Address) {
              result = inetAddress.getHostAddress();
              Log.i(TAG, "getIPAddress: " + inetAddress);
            }
          }
        }
      }
    } catch (SocketException e) {
      Log.i(TAG, "getIPAddress failed: ", e);
    }
    Log.i(TAG, "getIPAddress: " + result);
    return result;
  }

  @SuppressLint("MissingPermission")
  public String getLocation() {
    StringBuilder result = new StringBuilder("{");
    if (permitted(Manifest.permission.ACCESS_FINE_LOCATION)) {
      LocationManager locationService = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
      Location location = _locationAdapter != null ? _locationAdapter.getLocation() : null;
      if (locationService != null) {
        if (location == null) {
          location = locationService.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        }
        if (location == null) {
          location = locationService.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
        }
        if (location == null) {
          location = locationService.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
        }
      }
      if (location != null) {
        result.append("\"accuracy\":").append(location.getAccuracy()).append(",")
          .append("\"altitude\":").append(location.getAltitude()).append(",")
          .append("\"bearing\":").append(location.getBearing()).append(",")
          .append("\"latitude\":").append(location.getLatitude()).append(",")
          .append("\"longitude\":").append(location.getLongitude()).append(",")
          .append("\"speed\":").append(location.getSpeed()).append(",")
          .append("\"provider\":\"").append(location.getProvider()).append("\"");
      }
    }
    result.append("}");
    return result.toString();
  }

  public String getModulePath() {
    String result;
    if (getClassLoader() instanceof BaseDexClassLoader) {
      result = ((BaseDexClassLoader) getClassLoader()).findLibrary("smallbasic");
      int lastSlash = result.lastIndexOf('/');
      if (lastSlash != -1) {
        result = result.substring(0, lastSlash);
      }
    } else {
      result = "";
    }
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

  public int getWindowHeight() {
    Rect rect = new Rect();
    findViewById(android.R.id.content).getWindowVisibleDisplayFrame(rect);
    return rect.height();
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
  public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
    char ch = event.getCharacters().charAt(0);
    if (repeatCount == 0 && ch > 126 && ch < 256) {
      onUnicodeChar(ch);
    }
    return super.onKeyMultiple(keyCode, repeatCount, event);
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

  @Override
  public void onRequestPermissionsResult(int requestCode,
                                         @NonNull String[] permissions,
                                         @NonNull int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    if (requestCode == REQUEST_STORAGE_PERMISSION && grantResults[0] != PackageManager.PERMISSION_DENIED) {
      setupStorageEnvironment(true);
    }
  }

  public void optionsBox(final String[] items) {
    this._options = items;
    runOnUiThread(new Runnable() {
      public void run() {
        invalidateOptionsMenu();
        Configuration config = getResources().getConfiguration();
        // https://stackoverflow.com/questions/9996333/openoptionsmenu-function-not-working-in-ics/17903128#17903128
        if ((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) > Configuration.SCREENLAYOUT_SIZE_LARGE) {
          int originalScreenLayout = config.screenLayout;
          config.screenLayout = Configuration.SCREENLAYOUT_SIZE_LARGE;
          openOptionsMenu();
          config.screenLayout = originalScreenLayout;
        } else {
          openOptionsMenu();
        }
      }
    });
  }

  public void playAudio(final byte[] pathBytes) {
    new Thread(new Runnable() {
      public void run() {
        try {
          Uri uri = Uri.parse("file://" + new String(pathBytes, CP1252));
          if (_mediaPlayer == null) {
            _mediaPlayer = new MediaPlayer();
          } else {
            _mediaPlayer.reset();
          }
          _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
          _mediaPlayer.setDataSource(getApplicationContext(), uri);
          _mediaPlayer.prepare();
          _mediaPlayer.start();
        }
        catch (IOException e) {
          Log.i(TAG, "playAudio failed: ", e);
        }
      }
    }).start();
  }

  public void playTone(int frq, int dur, int vol, boolean bgplay) {
    float volume = (vol / 100f);
    final Sound sound = new Sound(frq, dur, volume);
    if (bgplay) {
      _sounds.add(sound);
      _audioExecutor.execute(new Runnable() {
        @Override
        public void run() {
          sound.play();
          _sounds.remove(sound);
        }
      });
    } else {
      sound.play();
    }
  }

  public boolean removeLocationUpdates() {
    boolean result = false;
    if (_locationAdapter != null) {
      LocationManager locationService = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
      if (locationService != null) {
        // requires coarse location permission
        //locationService.removeUpdates(_locationAdapter);
        _locationAdapter = null;
        result = true;
      }
    }
    Log.i(TAG, "removeRuntimeHandlers=" + result);
    return result;
  }

  public boolean requestLocationUpdates() {
    final LocationManager locationService = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
    boolean result = false;
    if (!permitted(Manifest.permission.ACCESS_FINE_LOCATION)) {
      checkPermission(Manifest.permission.ACCESS_FINE_LOCATION, REQUEST_LOCATION_PERMISSION);
    } else if (locationService != null) {
      final Criteria criteria = new Criteria();
      final String provider = locationService.getBestProvider(criteria, true);
      if (_locationAdapter == null && provider != null &&
        locationService.isProviderEnabled(provider)) {
        _locationAdapter = new LocationAdapter();
        result = true;
        runOnUiThread(new Runnable() {
          @SuppressLint("MissingPermission")
          public void run() {
            locationService.requestLocationUpdates(provider, LOCATION_INTERVAL,
              LOCATION_DISTANCE, _locationAdapter);
          }
        });
      }
    }
    Log.i(TAG, "requestLocationUpdates=" + result);
    return result;
  }

  public void setClipboardText(final byte[] textBytes) {
    try {
      final String text = new String(textBytes, CP1252);
      runOnUiThread(new Runnable() {
        public void run() {
          ClipboardManager clipboard =
            (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
          if (clipboard != null) {
            ClipData clip = ClipData.newPlainText("text", text);
            clipboard.setPrimaryClip(clip);
          }
        }
      });
    } catch (UnsupportedEncodingException e) {
      Log.i(TAG, "setClipboardText failed: ", e);
    }
  }

  public void setTtsLocale(final byte[] localeBytes) {
    if (_tts == null) {
      _tts = new TextToSpeechAdapter(this, "");
    }
    try {
      final String locale = new String(localeBytes, CP1252);
      _tts.setLocale(new Locale(locale));
    } catch (Exception e) {
      Log.i(TAG, "setTtsLocale failed:", e);
    }
  }

  public void setTtsPitch(float pitch) {
    if (_tts == null) {
      _tts = new TextToSpeechAdapter(this, "");
    }
    if (pitch != 0) {
      _tts.setPitch(pitch);
    }
  }

  public boolean setTtsQuiet() {
    boolean result;
    if (_tts != null) {
      _tts.stop();
      result = true;
    } else {
      result = false;
    }
    return result;
  }

  public void setTtsRate(float speechRate) {
    if (_tts == null) {
      _tts = new TextToSpeechAdapter(this, "");
    }
    if (speechRate != 0) {
      _tts.setSpeechRate(speechRate);
    }
  }

  public void share(final byte[] pathBytes) {
    File file = new File(getString(pathBytes));
    String buffer = readBuffer(file);
    if (!buffer.isEmpty()) {
      Intent sendIntent = new Intent();
      sendIntent.setAction(Intent.ACTION_SEND);
      sendIntent.putExtra(Intent.EXTRA_TEXT, buffer);
      sendIntent.putExtra(Intent.EXTRA_STREAM, getSharedFile(file));
      sendIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
      sendIntent.putExtra(Intent.EXTRA_SUBJECT, file.getName());
      sendIntent.setType("text/plain");
      startActivity(Intent.createChooser(sendIntent, "Share"));
    }
  }

  public void showAlert(final byte[] titleBytes, final byte[] messageBytes) {
    final Activity activity = this;
    final String title = getString(titleBytes);
    final String message = getString(messageBytes);
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
    _keypadHandler.removeCallbacksAndMessages(null);
    _keypadHandler.postDelayed(new Runnable() {
      @Override
      public void run() {
        runOnUiThread(new Runnable() {
          public void run() {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null) {
              if (show) {
                imm.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT);
              } else {
                imm.hideSoftInputFromWindow(view.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
              }
            }
          }
        });
      }
    }, 100);
  }

  public void showToast(final byte[] messageBytes) {
    final Activity activity = this;
    final String message = getString(messageBytes);
    runOnUiThread(new Runnable() {
      public void run() {
        Toast.makeText(activity, message.trim(), Toast.LENGTH_LONG).show();
      }
    });
  }

  public void speak(final byte[] textBytes) {
    final String text = getString(textBytes);
    if (_tts == null) {
      _tts = new TextToSpeechAdapter(this, text);
    } else {
      _tts.speak(text);
    }
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    processIntent();
    processSettings();
    checkFilePermission();
  }

  @Override
  protected void onPause() {
    super.onPause();
    onActivityPaused(true);
  }

  @Override
  protected void onResume() {
    super.onResume();
    onActivityPaused(false);
  }

  @Override
  protected void onStop() {
    super.onStop();
    if (_mediaPlayer != null) {
      _mediaPlayer.release();
      _mediaPlayer = null;
    }
    if (_tts != null) {
      _tts.close();
      _tts = null;
    }
  }

  private void checkFilePermission() {
    if (!permitted(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
      setupStorageEnvironment(false);
      Runnable handler = new Runnable() {
        @Override
        public void run() {
          checkPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE, REQUEST_STORAGE_PERMISSION);
        }
      };
      new Handler().postDelayed(handler, 250);
    } else {
      setupStorageEnvironment(true);
    }
  }

  private void checkPermission(final String permission, final int result) {
    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        String[] permissions = {permission};
        ActivityCompat.requestPermissions(MainActivity.this, permissions, result);
      }
    });
  }

  private void copy(InputStream in, OutputStream out) throws IOException {
    try {
      byte[] buf = new byte[COPY_BUFFER_SIZE];
      int len;
      while ((len = in.read(buf)) > 0) {
        out.write(buf, 0, len);
      }
    } finally {
      out.close();
      in.close();
    }
  }

  private void copy(File src, File dst) throws IOException {
    copy(new FileInputStream(src), new FileOutputStream(dst));
  }

  private String execBuffer(final String buffer, final String name, boolean run) throws IOException {
    File outputFile = new File(getInternalStorage(), name);
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

  private void execStream(InputStream inputStream) throws IOException {
    File outputFile = new File(getInternalStorage(), WEB_BAS);
    BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
    Log.i(TAG, "execStream() entered");
    String line = readLine(inputStream);
    while (line != null) {
      output.write(line + "\n");
      line = readLine(inputStream);
    }
    output.close();
    Log.i(TAG, "invoke runFile: " + outputFile.getAbsolutePath());
    runFile(outputFile.getAbsolutePath());
  }

  private String getExternalStorage() {
    String result;
    String path = Environment.getExternalStorageDirectory().getAbsolutePath();
    if (isPublicStorage(path)) {
      File sb = new File(path, FOLDER_NAME);
      if ((sb.isDirectory() && sb.canWrite()) || sb.mkdirs()) {
        result = path + "/" + FOLDER_NAME;
      } else {
        result = path;
      }
    } else if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      // https://commonsware.com/blog/2019/06/07/death-external-storage-end-saga.html
      File[] dirs = getExternalMediaDirs();
      result = dirs != null && dirs.length > 0 ? dirs[0].getAbsolutePath() : getInternalStorage();
    } else {
      result = getInternalStorage();
    }
    return result;
  }

  private String getInternalStorage() {
    return getFilesDir().getAbsolutePath();
  }

  private Uri getSharedFile(File file) {
    Uri result;
    try {
      File sharesPath = new File(getExternalFilesDir(null), "shares");
      if (sharesPath.mkdirs()) {
        Log.i(TAG, "created folder: " + sharesPath.toString());
      }
      File shareFile = new File(sharesPath, file.getName());
      copy(file, sharesPath);
      result = FileProvider.getUriForFile(this, "net.sourceforge.smallbasic.provider", shareFile);
    } catch (Exception e) {
      result = null;
      Log.e(TAG, "failed to create shared file", e);
    }
    return result;
  }

  private String getString(final byte[] bytes) {
    try {
      return new String(bytes, CP1252);
    } catch (UnsupportedEncodingException e) {
      Log.i(TAG, "getString failed: ", e);
      return "";
    }
  }

  private boolean isPublicStorage(String dir) {
    boolean result;
    if (dir == null || dir.isEmpty()) {
      result = false;
    } else {
      File file = new File(dir);
      result = file.isDirectory() && file.canRead() && file.canWrite();
    }
    return result;
  }

  private void migrateFiles(File fromDir, File toDir) {
    FilenameFilter filter = new FilenameFilter() {
      @Override
      public boolean accept(File dir, String name) {
        return name != null && name.endsWith(".bas");
      }
    };
    File[] toFiles = toDir.listFiles(filter);
    File[] fromFiles = fromDir.listFiles(filter);
    if (fromFiles != null && (toFiles == null || toFiles.length == 0)) {
      // only attempt file copy into a clean destination folder
      for (File file : fromFiles) {
        try {
          copy(file, new File(toDir, file.getName()));
        } catch (IOException e) {
          Log.d(TAG, "failed to copy: ", e);
        }
      }
    }
  }

  private boolean permitted(String permission) {
    return (ContextCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED);
  }

  private void processIntent() {
    Intent intent = getIntent();
    Uri uri = intent.getData();
    if (uri != null) {
      String data = intent.getDataString();
      if (data != null && data.startsWith(SCHEME)) {
        execScheme(data);
        Log.i(TAG, "data=" + data);
      } else {
        _startupBas = uri.getPath();
      }
      Log.i(TAG, "startupBas=" + _startupBas);
    }
  }

  private void processSettings() {
    try {
      Properties p = new Properties();
      p.load(getApplication().openFileInput("settings.txt"));
      int socket = Integer.parseInt(p.getProperty("serverSocket", "-1"));
      String token = p.getProperty("serverToken", new Date().toString());
      if (socket > 1023 && socket < 65536) {
        WebServer webServer = new WebServerImpl();
        webServer.run(socket, token);
      } else {
        Log.i(TAG, "Web service disabled");
      }
    } catch (Exception e) {
      Log.i(TAG, "Failed to start web service: ", e);
    }
  }

  private String readBuffer(File inputFile) {
    StringBuilder result = new StringBuilder();
    try {
      BufferedReader input = new BufferedReader(new FileReader(inputFile));
      String line = input.readLine();
      while (line != null) {
        result.append(line).append("\n");
        line = input.readLine();
      }
      input.close();
    } catch (FileNotFoundException e) {
      Log.i(TAG, "readBuffer failed: ", e);
    } catch (IOException e) {
      Log.i(TAG, "readBuffer failed: ", e);
    }
    return result.toString();
  }

  private String readLine(InputStream inputReader) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream(128);
    int b;
    for (b = inputReader.read(); b != -1 && b != '\n'; b = inputReader.read()) {
      if (b != '\r') {
        out.write(b);
      }
    }
    return b == -1 ? null : out.size() == 0 ? "" : out.toString();
  }

  private void setupStorageEnvironment(boolean external) {
    setenv("INTERNAL_DIR", getInternalStorage());
    if (external) {
      String externalDir = getExternalStorage();
      File files = getExternalFilesDir(null);
      if (files != null) {
        String externalFiles = files.getAbsolutePath();
        if (!externalDir.equals(externalFiles) && isPublicStorage(externalFiles)) {
          migrateFiles(new File(externalDir), files);
          setenv("LEGACY_DIR", externalDir);
          externalDir = externalFiles;
        }
      }
      setenv("EXTERNAL_DIR", externalDir);
    }
  }

  private static class BasFileFilter implements FilenameFilter {
    @Override
    public boolean accept(File dir, String name) {
      return name.endsWith(".bas");
    }
  }

  private class WebServerImpl extends WebServer {
    private final Map<String, Long> fileLengths = new HashMap<>();

    @Override
    protected void execStream(InputStream inputStream) throws IOException {
      MainActivity.this.execStream(inputStream);
    }

    @Override
    protected Response getFile(String path, boolean asset) throws IOException {
      Response result;
      if (asset) {
        String name = "webui/" + path;
        long length = getFileLength(name);
        log("Opened " + name + " " + length + " bytes");
        result = new Response(getAssets().open(name), length);
      } else {
        File file = getFile(getExternalStorage(), path);
        if (file == null) {
          file = getFile(getInternalStorage(), path);
        }
        if (file != null) {
          result = new Response(new FileInputStream(file), file.length());
        } else {
          throw new IOException("file not found");
        }
      }
      return result;
    }

    @Override
    protected Collection<FileData> getFileData() throws IOException {
      Collection<FileData> result = new ArrayList<>();
      result.addAll(getFiles(new File(getExternalStorage())));
      result.addAll(getFiles(new File(getInternalStorage())));
      return result;
    }

    @Override
    protected void log(String message, Exception exception) {
      Log.i(TAG, message, exception);
    }

    @Override
    protected void log(String message) {
      Log.i(TAG, message);
    }

    @Override
    protected void renameFile(String from, String to) throws IOException {
      if (to == null || !to.endsWith(".bas")) {
        throw new IOException("Invalid New File Name: " + to);
      }
      File toFile = getFile(getInternalStorage(), to);
      if (toFile == null) {
        toFile = getFile(getExternalStorage(), to);
      }
      if (toFile != null) {
        throw new IOException("New File Name already exists");
      }
      File fromFile = getFile(getInternalStorage(), from);
      if (fromFile == null) {
        fromFile = getFile(getExternalStorage(), from);
      }
      if (fromFile == null) {
        throw new IOException("Old File Name does not exist");
      }
      if (!fromFile.renameTo(new File(getExternalStorage(), to))) {
        throw new IOException("File rename failed");
      }
    }

    @Override
    protected void saveFile(String fileName, byte[] content) throws IOException {
      File file = new File(getExternalStorage(), fileName);
      if (file.exists()) {
        throw new IOException("File already exists");
      } else if (file.isDirectory()) {
        throw new IOException("Invalid File Name: " + fileName);
      }
      copy(new ByteArrayInputStream(content), new FileOutputStream(file));
    }

    private File getFile(String parent, String path) {
      File result = new File(parent, path);
      if (!result.exists() || !result.canRead() || result.isDirectory()) {
        result = null;
      }
      return result;
    }

    private long getFileLength(String name) throws IOException {
      Long length = fileLengths.get(name);
      if (length == null) {
        length = 0L;
        InputStream inputStream = getAssets().open(name);
        while (inputStream.available() > 0) {
          int unused = inputStream.read();
          length++;
        }
        inputStream.close();
        fileLengths.put(name, length);
      }
      return length;
    }

    private Collection<FileData> getFiles(File path) {
      Collection<FileData> result = new ArrayList<>();
      if (path.isDirectory() && path.canRead()) {
        File[] files = path.listFiles(new BasFileFilter());
        if (files != null) {
          for (File file : files) {
            result.add(new FileData(file));
          }
        }
      }
      return result;
    }
  };
}
