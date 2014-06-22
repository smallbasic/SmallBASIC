package net.sourceforge.smallbasic;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.GZIPOutputStream;

import javax.imageio.ImageIO;
import javax.xml.bind.DatatypeConverter;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;

/**
 * Encode SmallBASIC program into a QrCode
 * 
 * @author chrisws
 */
public class QrEncoder {
  private static final int DEF_SIZE = 500;

  /**
   * @param args
   * @throws IOException 
   * @throws WriterException 
   */
  public static void main(String[] args) throws IOException, WriterException {
    File input = new File(args[0]);
    if (!input.exists() || !input.canRead() || !input.isFile()) {
      System.err.println("Invalid input: " + input.toString());
    } else {
      int size = DEF_SIZE;
      if (args.length > 1) {
        size = Integer.valueOf(args[1]);
      }
      new QrEncoder().process(input, size);
    }
  }

  protected String cleanupCode(String fileText) {
    // convert DOS line endings to UNIX
    String result = fileText.replaceAll("\\r\\n", "\n");

    // remove quotes
    result = fileText.replaceAll("(?mi)^\\s*?rem.*[\\r\\n]", "");
    result = result.replaceAll("(?m)^\\s*?'.*[\\r\\n]", "");
    result = result.replaceAll("(?m)^\\s*?#.*[\\r\\n]", "");

    // remove leading white space
    result = result.replaceAll("(?m)^\\s*", "");
    
    // remove double spaces
    result = result.replaceAll("[ \t]{2,}", " ");

    // remove empty blank lines
    result = result.replaceAll("[\\r\\n]{2,}", "\n");
    
    return result.trim();
  }
  
  /**
   * Creates a QR code image using the given text and dimensions 
   */
  protected byte[] createQRCode(String qrText, int size) throws WriterException, IOException {
    Map<EncodeHintType, Object> hintMap = new HashMap<EncodeHintType, Object>();
    hintMap.put(EncodeHintType.ERROR_CORRECTION, ErrorCorrectionLevel.L);
    QRCodeWriter qrCodeWriter = new QRCodeWriter();
    BitMatrix byteMatrix = qrCodeWriter.encode(qrText, BarcodeFormat.QR_CODE, size, size, hintMap);
    int crunchifyWidth = byteMatrix.getWidth();
    BufferedImage image = new BufferedImage(crunchifyWidth, crunchifyWidth, BufferedImage.TYPE_INT_RGB);
    image.createGraphics();
    Graphics2D graphics = (Graphics2D) image.getGraphics();
    graphics.setColor(Color.WHITE);
    graphics.fillRect(0, 0, crunchifyWidth, crunchifyWidth);
    graphics.setColor(Color.BLACK);
    for (int i = 0; i < crunchifyWidth; i++) {
      for (int j = 0; j < crunchifyWidth; j++) {
        if (byteMatrix.get(i, j)) {
          graphics.fillRect(i, j, 1, 1);
        }
      }
    }
    ByteArrayOutputStream output = new ByteArrayOutputStream();
    ImageIO.write(image, "png", output);
    return output.toByteArray();
  }

  protected String getFileText(File input) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    try (DataInputStream is = new DataInputStream(new FileInputStream(input));) {
      byte[] buf = new byte[1024];
      int len = is.read(buf);
      while (len != -1) {
        out.write(buf, 0, len);
        len = is.read(buf);
      }
    }
    return out.toString("utf-8");
  }

  protected void process(File input, int size) throws IOException,
      FileNotFoundException, WriterException {

    // load the file text
    String fileText = getFileText(input);
    
    // remove comments and empty white space
    fileText = cleanupCode(fileText);

    // gzip the result
    String zipText = zipBase64(fileText);

    // encode the zipped data into a qrcode
    byte[] out = createQRCode("smallbasic://x/" + zipText, size);
    
    writeOutput(out, input + ".png");
  }

  protected void writeOutput(byte[] bytes, String fileName) throws FileNotFoundException, IOException {
    try (DataOutputStream out = new DataOutputStream(new FileOutputStream(fileName));) {
      out.write(bytes, 0, bytes.length);
    }    
  }

  protected String zipBase64(String fileText) throws IOException {
    ByteArrayOutputStream buffer = new ByteArrayOutputStream();
    DataOutputStream stream = new DataOutputStream(new GZIPOutputStream(buffer));
    stream.write(fileText.getBytes());
    stream.close();
    return DatatypeConverter.printBase64Binary(buffer.toByteArray());
  }
}
