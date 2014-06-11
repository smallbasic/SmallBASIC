package net.sourceforge.smallbasic;

import java.io.IOException;

import org.junit.Assert;
import org.junit.Test;

/**
 * Tests for QrEncoder
 * 
 * @author chrisws
 */
public class TestQrEncoder {
  @Test
  public void testCleanup() {
    QrEncoder inst = new QrEncoder();
    String fileText = " 'quote comments\r\n# hash comments\nREM remarks\r\n? 'hello'\n? 10";
    String expectedText = "? 'hello'\n? 10";
    String result = inst.cleanupCode(fileText);
    Assert.assertEquals(expectedText, result);
  }

  @Test
  public void testZipBase64() throws IOException {
    QrEncoder inst = new QrEncoder();
    String fileText = "? 'hello'\n? 10";
    String expectedText = "H4sIAAAAAAAAALNXUM9IzcnJV+eyVzA0AAAgB4JODgAAAA==";
    String result = inst.zipBase64(fileText);
    Assert.assertEquals(expectedText, result);
  }

}
