package net.sourceforge.smallbasic;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresPermission;
import androidx.core.app.ActivityCompat;

import java.io.IOException;
import java.util.UUID;

/**
 * Bluetooth (non BLE) communications
 */
public class BluetoothConnection extends BroadcastReceiver {
  private static final String TAG = "smallbasic";
  private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
  private static final int CONNECT_PERMISSION = 1000;

  private final BluetoothAdapter _bluetoothAdapter;
  private final Context _context;
  private final String _deviceName;
  private BluetoothTxThread _txThread;
  private BluetoothRxThread _rxThread;
  private BluetoothSocket _socket;
  private boolean _error;

  public BluetoothConnection(Activity activity, String deviceName) throws IOException {
    this._context = activity;
    this._deviceName = deviceName;
    this._bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    this._error = false;
    if (_bluetoothAdapter == null) {
      throw createIoException(activity, R.string.BLUETOOTH_ERROR);
    }

    if (checkPermission(activity, Manifest.permission.BLUETOOTH_CONNECT)) {
      requestPermission(activity, Manifest.permission.BLUETOOTH_CONNECT);
      throw createIoException(activity, R.string.PERMISSION_ERROR);
    }

    if (checkPermission(activity, Manifest.permission.BLUETOOTH_SCAN)) {
      requestPermission(activity, Manifest.permission.BLUETOOTH_SCAN);
      throw createIoException(activity, R.string.PERMISSION_ERROR);
    }

    if (!_bluetoothAdapter.isEnabled()) {
      Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      activity.startActivity(enableBtIntent);
    }

    // Start discovery of nearby Bluetooth devices
    IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
    activity.registerReceiver(this, filter);
    _bluetoothAdapter.startDiscovery();
  }

  /**
   * Closes the connection
   */
  public void close() {
    _bluetoothAdapter.cancelDiscovery();
    unregisterReceiver();
    closeSocket();
    stopTxThread();
    stopRxThread();
    Log.d(TAG, "BT connection closed");
  }

  /**
   * Returns information about the connected device
   */
  @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
  public String getDescription() {
    String result;
    if (_socket != null) {
      result = String.format("Remote %s [%s]",
                             _socket.getRemoteDevice().getName(),
                             _socket.getRemoteDevice().getAddress());
    } else {
      result = String.format("Local: %s [%s] Waiting for %s",
                             _bluetoothAdapter.getName(),
                             _bluetoothAdapter.getAddress(),
                             _deviceName);
    }
    return result;
  }

  /**
   * Returns whether the bluetooth connection is open
   */
  public boolean isConnected() {
    return _socket != null && _socket.isConnected();
  }

  /**
   * Whether a connection error has occurred
   */
  public boolean isError() {
    return _error;
  }

  /**
   * Handles receiving the request permission response event
   */
  @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
  @Override
  public void onReceive(Context context, Intent intent) {
    Log.d(TAG, "onReceive entered");
    String action = intent.getAction();
    if (BluetoothDevice.ACTION_FOUND.equals(action)) {
      BluetoothDevice discoveredDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
      if (discoveredDevice != null) {
        String deviceName = discoveredDevice.getName();
        String deviceAddress = discoveredDevice.getAddress();
        Log.d(TAG, "Found device: " + deviceName + " at " + deviceAddress);
        if (_deviceName.equals(deviceName)) {
          _bluetoothAdapter.cancelDiscovery();
          unregisterReceiver();
          connectToDevice(discoveredDevice);
        }
      }
    }
  }

  /**
   * Receives the next packet of data from the connection
   */
  public String receive(String strDefault) {
    String result;
    if (_rxThread != null) {
      result = _rxThread.read();
    } else {
      result = strDefault;
    }
    return result;
  }

  /**
   * Sends the given data to the connection
   */
  public boolean send(String data) {
    boolean result;
    if (_txThread != null) {
      result = _txThread.send(data);
    } else {
      result = false;
    }
    return result;
  }

  /**
   * Returns whether the given permission is granted
   */
  private boolean checkPermission(Activity activity, String permission) {
    return ActivityCompat.checkSelfPermission(activity, permission) != PackageManager.PERMISSION_GRANTED;
  }

  /**
   * Close the socket at termination of the thread
   */
  private void closeSocket() {
    try {
      if (_socket != null) {
        _socket.close();
        _socket = null;
        Log.d(TAG, "BT socket closed.");
      }
    }
    catch (Exception e) {
      Log.e(TAG, "Error closing socket", e);
    }
  }

  /**
   * Connects to the target device and commences communication
   */
  @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
  private void connectToDevice(BluetoothDevice device) {
    try {
      _socket = device.createRfcommSocketToServiceRecord(SPP_UUID);
      _socket.connect();
      Log.d(TAG, "Connected to device: " + device.getName());
      _txThread = new BluetoothTxThread(_socket);
      _rxThread = new BluetoothRxThread(_socket);
    } catch (Exception e) {
      _error = true;
      Log.e(TAG, "Connection failed", e);
    }
  }

  /**
   * Builds an Exception with the given resource string
   */
  @NonNull
  private static IOException createIoException(Context context, int resourceId) {
    return new IOException(context.getResources().getString(resourceId));
  }

  /**
   * Invokes the display of a permission prompt
   */
  private void requestPermission(Activity activity, String permission) {
    new Handler(Looper.getMainLooper()).post(() -> {
      String[] permissions = {permission};
      ActivityCompat.requestPermissions(activity, permissions, CONNECT_PERMISSION);
    });
    Log.d(TAG, "requesting permission: " + permission);
  }

  /**
   * Stops the receive thread
   */
  private void stopRxThread() {
    if (_rxThread != null) {
      _rxThread.stopThread();
      _rxThread = null;
    }
  }

  /**
   * Stops the transmit thread
   */
  private void stopTxThread() {
    if (_txThread != null) {
      _txThread.stopThread();
      _txThread = null;
    }
  }

  /**
   * Disconnects our BroadcastReceiver listener
   */
  private void unregisterReceiver() {
    try {
      _context.unregisterReceiver(this);
    } catch (IllegalArgumentException e) {
      // ignored
    }
  }
}
