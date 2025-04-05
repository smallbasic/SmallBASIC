package net.sourceforge.smallbasic;

import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Thread for sending data on a bluetooth connection
 */
public class BluetoothTxThread extends Thread {
  private static final String TAG = "smallbasic";
  private static final int QUEUE_SIZE = 100;
  private final AtomicBoolean _running;
  private final BlockingQueue<byte[]> _queue;
  private final OutputStream _outputStream;

  public BluetoothTxThread(BluetoothSocket socket) throws IOException {
    this._outputStream = socket.getOutputStream();
    this._queue = new ArrayBlockingQueue<>(QUEUE_SIZE);
    this._running = new AtomicBoolean(true);
    start();
  }

  public boolean isRunning() {
    return _running.get();
  }

  @Override
  public void run() {
    int ticks = 0;
    try {
      while (_running.get() && !Thread.currentThread().isInterrupted()) {
        ticks++;
        byte[] data = _queue.take();
        _outputStream.write(data);
        _outputStream.flush();
      }
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    } catch (Exception e) {
      Log.d(TAG, "Run failed:", e);
    } finally {
      _running.set(false);
    }
    Log.d(TAG, "Bluetooth TX thread terminated with: " + ticks);
  }

  /**
   * Add data to the send queue without blocking
   */
  public boolean send(String data) {
    boolean result;
    if (data == null || data.isEmpty()) {
      result  = false;
    } else {
      result = _queue.offer(data.getBytes(StandardCharsets.UTF_8));
    }
    return result;
  }

  public void stopThread() {
    _running.set(false);
    interrupt();
  }
}
