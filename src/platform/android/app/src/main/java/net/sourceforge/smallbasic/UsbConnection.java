package net.sourceforge.smallbasic;

import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbConstants;
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

import androidx.annotation.NonNull;

import java.io.IOException;
import java.nio.charset.StandardCharsets;

/**
 * Usb host mode communications
 */
public class UsbConnection extends BroadcastReceiver {
  private static final String TAG = "smallbasic";
  private static final String ACTION_USB_PERMISSION = "smallbasic.android.USB_PERMISSION";
  private static final int TIMEOUT_MILLIS = 5000;

  private final UsbDevice _usbDevice;
  private final UsbManager _usbManager;
  private UsbDeviceConnection _connection;
  private UsbInterface _dataInterface;
  private UsbInterface _controlInterface;
  private UsbEndpoint _endpointIn;
  private UsbEndpoint _endpointOut;
  private Context _context;
  private final int _bufferSize;
  private final int _timeoutMillis;
  private final int _baud;

  /**
   * Constructs a new UsbConnection
   */
  public UsbConnection(Context context, int vendorId, int baud, int timeout) throws IOException {
    _context = context;
    _usbManager = getUsbManager(context);
    _usbDevice = getDevice(_usbManager, vendorId);
    _timeoutMillis = timeout < 0 ? TIMEOUT_MILLIS : timeout;

    if (_usbDevice == null) {
      throw createIoException(context, R.string.USB_DEVICE_NOT_FOUND);
    }

    if (!_usbManager.hasPermission(_usbDevice)) {
      requestPermission(context);
      throw createIoException(context, R.string.PERMISSION_ERROR);
    }

    _connection = _usbManager.openDevice(_usbDevice);
    if (_connection == null) {
      throw createIoException(context, R.string.USB_CONNECTION_ERROR);
    }

    // Find the CDC interfaces and endpoints
    UsbInterface controlInterface = null;
    UsbInterface dataInterface = null;
    UsbEndpoint endpointIn = null;
    UsbEndpoint endpointOut = null;

    // Look for CDC control interface
    for (int i = 0; i < _usbDevice.getInterfaceCount(); i++) {
      UsbInterface anInterface = _usbDevice.getInterface(i);
      Log.i(TAG, "Interface " + i + " - Class: " + anInterface.getInterfaceClass());
      if (anInterface.getInterfaceClass() == UsbConstants.USB_CLASS_COMM) {
        controlInterface = anInterface;
      } else if (anInterface.getInterfaceClass() == UsbConstants.USB_CLASS_CDC_DATA) {
        dataInterface = anInterface;
        for (int j = 0; j < anInterface.getEndpointCount(); j++) {
          UsbEndpoint ep = anInterface.getEndpoint(j);
          Log.i(TAG, "  Endpoint " + j + " - Type: " + ep.getType() +
                     ", Direction: " + ep.getDirection());
          if (ep.getType() == UsbConstants.USB_ENDPOINT_XFER_BULK) {
            if (ep.getDirection() == UsbConstants.USB_DIR_IN) {
              endpointIn = ep;
            } else {
              endpointOut = ep;
            }
          }
        }
      }
    }

    if (controlInterface == null) {
      throw createIoException(context, R.string.USB_CONTROL_NOT_FOUND);
    }
    if (endpointIn == null) {
      throw createIoException(context, R.string.ENDPOINT_IN_ERROR);
    }
    if (endpointOut == null) {
      throw createIoException(context, R.string.ENDPOINT_OUT_ERROR);
    }

    _endpointIn = endpointIn;
    _endpointOut = endpointOut;
    _controlInterface = controlInterface;
    _dataInterface = dataInterface;
    _bufferSize = calcBufferSize(_endpointIn);

    // Claim interfaces
    if (!_connection.claimInterface(controlInterface, true)) {
      throw createIoException(context, R.string.USB_CONTROL_NOT_CLAIMED);
    }
    if (!_connection.claimInterface(dataInterface, true)) {
      throw createIoException(context, R.string.USB_DATA_NOT_CLAIMED);
    }

    byte baudLsb;
    byte baudMsb;

    if (baud >= 300) {
      baudLsb = (byte) (baud);
      baudMsb = (byte) (baud >> 8 & 0xff);
      _baud = baud;
    } else {
      // defaults to 19200 (little endian)
      baudLsb = 0;
      baudMsb = 0x4b;
      _baud = 19200;
    }

    // Set line coding (baud, 8N1)
    byte[] lineCoding = new byte[] {
      baudLsb,
      baudMsb,
      0, 0,
      0, // 1 stop bit
      0, // No parity
      8  // 8 data bits
    };

    if (_connection.controlTransfer(
        0x21,  // bmRequestType (0x21)
        0x20,  // bRequest (SET_LINE_CODING)
        0,     // wValue
        _controlInterface.getId(), // wIndex
        lineCoding,
        lineCoding.length,
        TIMEOUT_MILLIS) < 0) {
      throw createIoException(context, R.string.USB_ENCODING_ERROR);
    }

    if (_connection.controlTransfer(
        0x21,  // bmRequestType (0x21)
        0x22,  // SET_CONTROL_LINE_STATE
        0x03,  // wValue Enable DTR (bit 0) and RTS (bit 1)
        _controlInterface.getId(),
        null,
        0,
        TIMEOUT_MILLIS) < 0) {
      throw createIoException(context, R.string.USB_LINE_STATE_ERROR);
    }
  }

  /**
   * Closes the USB connection
   */
  public void close() {
    if (_context != null) {
      try {
        _context.unregisterReceiver(this);
      } catch (IllegalArgumentException e) {
        // ignored
      }
      _context = null;
    }
    if (_connection != null) {
      if (_controlInterface != null) {
        _connection.releaseInterface(_controlInterface);
      }
      if (_dataInterface != null) {
        _connection.releaseInterface(_dataInterface);
      }
      _connection.close();
      _connection = null;
    }

    _dataInterface = null;
    _controlInterface = null;
    _endpointIn = null;
    _endpointOut = null;
  }

  /**
   * Returns information about the connected USB device
   */
  @SuppressLint("DefaultLocale")
  public String getDescription() {
    String result;
    if (_usbDevice != null) {
      result = String.format("%s %s %s %s [%d bps]",
                             _usbDevice.getDeviceName(),
                             _usbDevice.getProductName(),
                             _usbDevice.getManufacturerName(),
                             _usbDevice.getSerialNumber(),
                             _baud);
    } else {
      result = "";
    }
    return result;
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
      final String message = name + " access " + (permitted ? "permitted" : "denied");
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
    byte[] dataIn = new byte[_bufferSize];
    int bytesRead = _connection.bulkTransfer(_endpointIn, dataIn, dataIn.length, _timeoutMillis);
    if (bytesRead > 0) {
      result = new String(dataIn, 0, bytesRead, StandardCharsets.UTF_8);
    } else {
      result = "";
    }
    return result;
  }

  /**
   * Sends the given data to the usb connection
   */
  public int send(String data) {
    byte[] dataOut = data.getBytes(StandardCharsets.UTF_8);
    return _connection.bulkTransfer(_endpointOut, dataOut, dataOut.length, TIMEOUT_MILLIS);
  }

  /**
   * Calculates the optimum receive buffer size
   */
  private int calcBufferSize(UsbEndpoint endpointIn) {
    // Get the maximum packet size from the endpoint
    int maxPacketSize = endpointIn.getMaxPacketSize();

    // Calculate buffer size as a multiple of the maximum packet size
    // 4 is a good multiplier for most applications
    int result = maxPacketSize * 4;

    // Ensure minimum reasonable size
    result = Math.max(result, 1024);

    // Optional: Cap at a maximum size if memory is a concern
    result = Math.min(result, 16384); // 16KB max

    return result;
  }

  @NonNull
  private static IOException createIoException(Context context, int resourceId) {
    return new IOException(context.getResources().getString(resourceId));
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
