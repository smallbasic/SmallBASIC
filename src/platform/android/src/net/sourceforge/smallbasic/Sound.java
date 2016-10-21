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
@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class Sound {
  private static final String TAG = "smallbasic";
  private static final int AUDIO_SAMPLE_RATE = 8000;
  private static final int WAVE_MAX = 32000;
  private byte[] _sound;
  private float _volume;
  private int _dur;
  private boolean _silent;
  
  public Sound(int frq, int dur, float vol) {
    this._sound = generateTone(frq, dur);
    this._volume = vol;
    this._dur = dur;
    this._silent = false;
  }
  
  public boolean isSilent() {
    return _silent;
  }

  public void play() {
    if (!_silent) {
      try {
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
            AUDIO_SAMPLE_RATE, AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_16BIT, _sound.length, AudioTrack.MODE_STATIC);
        if (audioTrack.write(_sound, 0, _sound.length) == _sound.length) {
          audioTrack.setStereoVolume(_volume, _volume);
          playTrack(audioTrack);
        } else {
          Log.i(TAG, "Failed to write audio: " + _sound.length);
        }
      } catch (Exception e) {
        Log.i(TAG, "play failed: ", e);
      }
    }
  }

  public void setSilent(boolean silent) {
    this._silent = silent;
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
      final short val = (short) ((dVal * WAVE_MAX * i / ramp));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }

    while (i < numSamples - ramp) {
      // Max amplitude for most of the samples
      double dVal = sample[i];
      // scale to maximum amplitude
      final short val = (short) ((dVal * WAVE_MAX));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }

    while (i < numSamples) {
      // Ramp amplitude down
      double dVal = sample[i];
      // Ramp down to zero
      final short val = (short) ((dVal * WAVE_MAX * (numSamples - i) / ramp));
      // in 16 bit wav PCM, first byte is the low order byte
      result[idx++] = (byte) (val & 0x00ff);
      result[idx++] = (byte) ((val & 0xff00) >>> 8);
      i++;
    }
    return result;
  }

  private void playTrack(AudioTrack audioTrack) throws InterruptedException {
    int frame;
    int frames = _sound.length / 2;
    audioTrack.play();
    do {
      Thread.sleep(_dur / 2);
      frame = audioTrack.getPlaybackHeadPosition();
    } while (frame < frames);
    audioTrack.release();
  }
}
