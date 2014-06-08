package net.sourceforge.smallbasic;

import android.annotation.TargetApi;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.util.Log;

/**
 * Support for BEEP and PLAY commands
 * 
 * @author chrisws
 */
@TargetApi(Build.VERSION_CODES.GINGERBREAD)
public class Sound {
  static final int AUDIO_SAMPLE_RATE = 8000;
  private byte[] sound;
  private float volume;
  private int dur;
  
  public Sound(int frq, int dur, int vol) {
    this.sound = generateTone(frq, dur);
    this.volume = vol * AudioTrack.getMaxVolume() / 100;
    this.dur = dur;
  }
  
  public void play() {
    AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
        AUDIO_SAMPLE_RATE, AudioFormat.CHANNEL_OUT_MONO,
        AudioFormat.ENCODING_PCM_16BIT, sound.length, AudioTrack.MODE_STATIC);
    audioTrack.setStereoVolume(volume, volume);
    if (audioTrack.write(sound, 0, sound.length) == sound.length) {
      playTrack(audioTrack);
    } else {
      Log.i(MainActivity.TAG, "Failed to write audio: " + sound.length);
    }
  }

  /**
   * http://stackoverflow.com/questions/2413426/playing-an-arbitrary-tone-with-android
   */
  private final byte[] generateTone(int freqOfTone, int durationMillis) {
    int numSamples = durationMillis * AUDIO_SAMPLE_RATE / 1000;
    double sample[] = new double[numSamples];
    byte result[] = new byte[2 * numSamples];

    for (int i = 0; i < numSamples; ++i) {
      // Fill the sample array
      sample[i] = Math.sin(freqOfTone * 2 * Math.PI * i / AUDIO_SAMPLE_RATE);
    }

    // convert to 16 bit pcm sound array assumes the sample buffer is normalised.
    int idx = 0;
    int i = 0;

    // Amplitude ramp as a percent of sample count
    int ramp = numSamples / 20;

    while (i < ramp) {
      // Ramp amplitude up (to avoid clicks)
      double dVal = sample[i];
      // Ramp up to maximum
      final short val = (short) ((dVal * 32767 * i / ramp));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }

    while (i < numSamples - ramp) {
      // Max amplitude for most of the samples
      double dVal = sample[i];
      // scale to maximum amplitude
      final short val = (short) ((dVal * 32767));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }

    while (i < numSamples) {
      // Ramp amplitude down
      double dVal = sample[i];
      // Ramp down to zero
      final short val = (short) ((dVal * 32767 * (numSamples - i) / ramp));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }
    return result;
  }

  private void playTrack(AudioTrack audioTrack) {
    int frame;
    int frames = sound.length / 2;
    audioTrack.play();
    do {
      try {
        Thread.sleep(dur / 2);
      } catch (InterruptedException e) {
        Log.e(MainActivity.TAG, "Sleep failed: ", e);
      }
      frame = audioTrack.getPlaybackHeadPosition();
    } while (frame < frames);
    audioTrack.release();
  }
}
