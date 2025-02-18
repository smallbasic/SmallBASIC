package net.sourceforge.smallbasic;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;

/**
 * Usb host mode communications
 */
public class UsbConnection extends BroadcastReceiver {
  private static final String TAG = "smallbasic";
  private static final String ACTION_USB_PERMISSION = "smallbasic.android.USB_PERMISSION";
  private static final String PERMISSION_ERROR = "Not permitted";
  private static final String USB_DEVICE_NOT_FOUND = "Usb device not found";
  private static final int TIMEOUT_MILLIS = 1000;
  private static final int BUFFER_SIZE = 64;

  private final UsbDeviceConnection _connection;
  private final UsbInterface _usbInterface;
  private final UsbDevice _usbDevice;
  private final UsbManager _usbManager;
  private final UsbEndpoint _endpointIn;
  private final UsbEndpoint _endpointOut;

  /**
   * Constructs a new UsbConnection
   */
  public UsbConnection(Context context, int vendorId) throws IOException {
    _usbManager = getUsbManager(context);
    _usbDevice = getDevice(_usbManager, vendorId);
    if (_usbDevice == null) {
      throw new IOException(USB_DEVICE_NOT_FOUND);
    }

    if (!_usbManager.hasPermission(_usbDevice)) {
      requestPermission(context);
      throw new IOException(PERMISSION_ERROR);
    }

    _usbInterface = _usbDevice.getInterface(0);
    _endpointIn = _usbInterface.getEndpoint(0);
    _endpointOut = _usbInterface.getEndpoint(1);
    _connection = _usbManager.openDevice(_usbDevice);
    _connection.claimInterface(_usbInterface, true);
  }

  /**
   * Closes the USB connection
   */
  public void close() {
    if (_connection != null && _usbInterface != null) {
      _connection.releaseInterface(_usbInterface);
      _connection.close();
    }
  }

  /**
   * Handles receiving the request permission response event
   */
  @Override
  public void onReceive(Context context, Intent intent) {
    Log.d(TAG, "onReceive entered");
    if (ACTION_USB_PERMISSION.equals(intent.getAction())) {
      boolean permitted = _usbManager.hasPermission(_usbDevice);
      String name = _usbDevice.getProductName();
      final String message = "USB connection [" + name + "] access " + (permitted ? "permitted" : "denied");
      final BroadcastReceiver receiver = this;
      new Handler(Looper.getMainLooper()).post(() -> {
        Toast.makeText(context, message, Toast.LENGTH_LONG).show();
        context.unregisterReceiver(receiver);
      });
    }
  }

  /**
   * Receives the next packet of data from the usb connection
   */
  public String receive() {
    String result;
    byte[] dataIn = new byte[BUFFER_SIZE];
    int bytesRead = _connection.bulkTransfer(_endpointIn, dataIn, dataIn.length, TIMEOUT_MILLIS);
    if (bytesRead > 0) {
      result = new String(dataIn, 0, bytesRead);
    } else {
      result = "";
    }
    return result;
  }

  /**
   * Sends the given data to the usb connection
   */
  public void send(byte[] dataOut) {
    _connection.bulkTransfer(_endpointOut, dataOut, dataOut.length, TIMEOUT_MILLIS);
  }

  /**
   * Returns the UsbDevice matching the given vendorId
   */
  private UsbDevice getDevice(UsbManager usbManager, int vendorId) {
    UsbDevice result = null;
    for (UsbDevice device : usbManager.getDeviceList().values()) {
      if (device.getVendorId() == vendorId) {
        result = device;
        break;
      }
    }
    return result;
  }

  /**
   * Returns the UsbManager
   */
  private static UsbManager getUsbManager(Context context) {
    return (UsbManager) context.getSystemService(Context.USB_SERVICE);
  }

  /**
   * Invokes the display of a permission prompt
   */
  private void requestPermission(Context context) {
    new Handler(Looper.getMainLooper()).post(() -> {
      IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
      filter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY - 1);
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
        context.registerReceiver(this, filter, Context.RECEIVER_NOT_EXPORTED);
      }
      int flags = PendingIntent.FLAG_IMMUTABLE;
      Intent intent = new Intent(ACTION_USB_PERMISSION);
      PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, flags);
      _usbManager.requestPermission(_usbDevice, pendingIntent);
    });
    Log.d(TAG, "requesting permission");
  }
}
