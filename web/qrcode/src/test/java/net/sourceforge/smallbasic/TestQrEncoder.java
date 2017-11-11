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
    String fileText = " 'quote comments\r\n# hash comments\nREM remarks"
        + "\r\n\r\n   ?    'hello'\n       ?   10\r\n\r\n\r\n?";
    String expectedText = "? 'hello'\n? 10\n?";
    String result = inst.cleanupCode(fileText);
    Assert.assertEquals(expectedText, result);
  }

  @Test
  public void testEqualsCleanup() {
    QrEncoder inst = new QrEncoder();
    String fileText = "a = b if a == b then blah if a < b a + b a != b a += b";
    String expectedText = "a=b if a==b then blah if a<b a+b a!=b a+=b";
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
