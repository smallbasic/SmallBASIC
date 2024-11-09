package net.sourceforge.smallbasic;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.util.Log;

/**
 * Support for BEEP and PLAY commands
 *
 * @author chrisws
 */
class Sound {
  private static final String TAG = "smallbasic";
  private static final int AUDIO_SAMPLE_RATE = 8000;
  private static final int WAVE_MAX = 32000;
  private final byte[] _sound;
  private final float _volume;
  private final int _dur;
  private boolean _silent;

  Sound(int frq, int dur, float vol) {
    this._sound = frq == 0 ? null : generateTone(frq, dur);
    this._volume = vol;
    this._dur = dur;
    this._silent = false;
  }

  void play() {
    if (!_silent) {
      if (_sound == null) {
        playSilence();
      } else {
        playTrack();
      }
    }
  }

  void setSilent(boolean silent) {
    this._silent = silent;
  }

  /**
   * http://stackoverflow.com/questions/2413426/playing-an-arbitrary-tone-with-android
   */
  private byte[] generateTone(int freqOfTone, int durationMillis) {
    int numSamples = Math.max(1, durationMillis * AUDIO_SAMPLE_RATE / 1000);
    double[] sample = new double[numSamples];
    byte[] result = new byte[2 * numSamples];

    for (int i = 0; i < numSamples; ++i) {
      // Fill the sample array
      sample[i] = Math.sin(freqOfTone * 2 * Math.PI * i / AUDIO_SAMPLE_RATE);
    }

    // convert to 16 bit pcm sound array assumes the sample buffer is normalised.
    int idx = 0;
    int i = 0;

    // Amplitude ramp as a percent of sample count
    int ramp = numSamples / 2;

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

  private AudioTrack getAudioTrack() {
    AudioAttributes audioAttributes = new AudioAttributes.Builder()
        .setUsage(AudioAttributes.USAGE_MEDIA)
        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
        .build();
    return new AudioTrack.Builder()
        .setAudioAttributes(audioAttributes)
        .setTransferMode(AudioTrack.MODE_STATIC)
        .setAudioFormat(new AudioFormat.Builder()
                            .setSampleRate(AUDIO_SAMPLE_RATE)
                            .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                            .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .build())
        .setBufferSizeInBytes(_sound.length)
        .build();
  }

  private void playSilence() {
    try {
      Thread.sleep(_dur);
    }
    catch (InterruptedException e) {
      Log.i(TAG, "failed to sleep: ", e);
    }
  }

  private void playTrack() {
    try {
      AudioTrack audioTrack = getAudioTrack();
      if (audioTrack.write(_sound, 0, _sound.length) == _sound.length) {
        audioTrack.setVolume(_volume);
        playTrack(audioTrack);
      } else {
        Log.i(TAG, "Failed to write audio: " + _sound.length);
      }
    } catch (Exception e) {
      Log.i(TAG, "play failed: ", e);
    }
  }

  private void playTrack(AudioTrack audioTrack) throws InterruptedException {
    int frame;
    int frames = _sound.length / 2;
    audioTrack.play();
    do {
      Thread.sleep(_dur);
      frame = audioTrack.getPlaybackHeadPosition();
    } while (frame < frames);
    Thread.sleep(1);
    audioTrack.release();
  }
}
