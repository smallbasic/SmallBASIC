package net.sourceforge.smallbasic;

import androidx.annotation.NonNull;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class HttpConnection implements Runnable {
  private static final int TIMEOUT_MILLIS = 30000;
  private static final String ERROR_PREFIX = "error: [";
  private static final String ERROR_POSTFIX = "]";

  private final String endPoint;
  private final String data;
  private final String apiKey;
  private final CountDownLatch latch;
  private String result;

  public HttpConnection(String endPoint, String data, String apiKey) {
    this.endPoint = endPoint;
    this.data = data;
    this.apiKey = apiKey;
    this.latch = new CountDownLatch(1);
  }

  public String invoke() {
    new Thread(this).start();
    try {
      if (!latch.await(TIMEOUT_MILLIS, TimeUnit.MILLISECONDS)) {
        result = buildError("timeout");
      }
    } catch (InterruptedException e) {
      result = buildError(e.toString());
    }
    return result;
  }

  @Override
  public void run() {
    HttpURLConnection conn = null;
    try {
      conn = getHttpURLConnection(endPoint, (data == null || data.isEmpty()) ? "GET" : "POST", apiKey);
      if (data != null && !data.isEmpty()) {
        conn.setRequestProperty("Content-Length", "" + data.getBytes().length);
        conn.setDoInput(true);
        OutputStream os = conn.getOutputStream();
        os.write(data.getBytes(StandardCharsets.UTF_8));
        os.flush();
        os.close();
      }
      int responseCode = conn.getResponseCode();
      if (responseCode == HttpURLConnection.HTTP_OK) {
        result = getText(conn.getInputStream());
      } else if (responseCode >= HttpURLConnection.HTTP_BAD_REQUEST && responseCode < HttpURLConnection.HTTP_INTERNAL_ERROR) {
        result = buildError(getText(conn.getErrorStream()));
      } else {
        result = buildError(String.valueOf(responseCode));
      }
    } catch (Exception e) {
      result = buildError(e.toString());
    } finally {
      if (conn != null) {
        conn.disconnect();
      }
    }
    latch.countDown();
  }

  @NonNull
  private static String buildError(String message) {
    return ERROR_PREFIX + message + ERROR_POSTFIX;
  }

  @NonNull
  private HttpURLConnection getHttpURLConnection(String endPoint, String method, String apiKey) throws IOException {
    URL url = new URL(endPoint);
    HttpURLConnection result = (HttpURLConnection) url.openConnection();
    result.setRequestMethod(method);
    result.setRequestProperty("User-Agent", "SmallBASIC");
    result.setConnectTimeout(TIMEOUT_MILLIS);
    result.setInstanceFollowRedirects(true);
    if (apiKey != null && !apiKey.isEmpty()) {
      result.setRequestProperty("Accept", "application/json");
      result.setRequestProperty("Content-Type", "application/json");
      result.setRequestProperty("Authorization", "Bearer " + apiKey);
    } else {
      result.setRequestProperty("Accept", "*/*");
    }
    return result;
  }

  private String getText(InputStream stream) throws IOException {
    BufferedReader in = new BufferedReader(new InputStreamReader(stream));
    String inputLine;
    StringBuilder response = new StringBuilder();
    while ((inputLine = in.readLine()) != null) {
      response.append(inputLine);
    }
    in.close();
    return response.toString();
  }
}
