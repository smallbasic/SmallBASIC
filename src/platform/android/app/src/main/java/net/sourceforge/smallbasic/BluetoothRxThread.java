package net.sourceforge.smallbasic;

import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Thread for receiving data on a bluetooth connection
 */
public class BluetoothRxThread extends Thread {
  private static final String TAG = "smallbasic";
  private static final int QUEUE_SIZE = 100;
  private static final int READ_BUFFER_SIZE = 1024;
  private final AtomicBoolean _running;
  private final InputStream _inputStream;
  private final BlockingQueue<byte[]> _queue;

  public BluetoothRxThread(BluetoothSocket socket) throws IOException {
    this._inputStream = socket.getInputStream();
    this._queue = new ArrayBlockingQueue<>(QUEUE_SIZE);
    this._running = new AtomicBoolean(true);
    start();
  }

  public boolean isRunning() {
    return _running.get();
  }

  /**
   * Returns any data from the queue without blocking
   */
  public String read() {
    String result;
    byte[] data = _queue.poll();
    if (data != null && data.length > 0) {
      result = new String(data, StandardCharsets.UTF_8);
    } else {
      result = "";
    }
    return result;
  }

  @Override
  public void run() {
    byte[] buffer = new byte[READ_BUFFER_SIZE];
    int ticks = 0;
    try {
      while (_running.get() && !Thread.currentThread().isInterrupted()) {
        ticks++;
        // read() is blocking
        int size = _inputStream.read(buffer);
        if (size > 0) {
          byte[] data = Arrays.copyOf(buffer, size);
          // blocks if the queue is full
          _queue.put(data);
        }
      }
    } catch (Exception e) {
      Log.d(TAG, "Run failed:", e);
    } finally {
      _running.set(false);
    }
    Log.d(TAG, "Bluetooth RX thread terminated with: " + ticks);
  }

  public void stopThread() {
    _running.set(false);
    interrupt();
  }
}
