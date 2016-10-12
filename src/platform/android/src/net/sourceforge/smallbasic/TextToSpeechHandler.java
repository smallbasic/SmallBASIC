package net.sourceforge.smallbasic;

import java.util.Locale;

import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.util.Log;

public class TextToSpeechHandler implements OnInitListener {
  private static final String TAG = "smallbasic";
  private TextToSpeech _tts;
  private boolean _ready;
  private String _text;
  private float _pitch;
  private float _speechRate;

  public TextToSpeechHandler(Context context, String text) {
    _tts = new TextToSpeech(context, this);
    _text = text;
    _pitch = 1f;
    _speechRate = 1f;
  }

  public void close() {
    _tts.stop();
    _tts.shutdown();
    _ready = false;
  }

  @Override
  public void onInit(int status) {
    if (status == TextToSpeech.SUCCESS) {
      int result = _tts.setLanguage(Locale.ENGLISH);
      _tts.setPitch(_pitch);
      _tts.setSpeechRate(_speechRate);
      if (result != TextToSpeech.LANG_MISSING_DATA &&
          result != TextToSpeech.LANG_NOT_SUPPORTED) {
        _ready = true;
        if (_text != null) {
          speak(_text);
        }
      }
    }
    Log.i(TAG, "Tts init: " + _ready);
  }

  public void setPitch(float pitch) {
    this._pitch = pitch;
    if (_ready) {
      _tts.setPitch(pitch);
    }
  }

  public void setSpeechRate(float speechRate) {
    this._speechRate = speechRate;
    if (_ready) {
      _tts.setSpeechRate(speechRate);
    }
  }

  public void speak(final String text) {
    if (_ready) {
      _tts.speak(text, TextToSpeech.QUEUE_ADD, null);
    } else {
      _text = text;
    }
  }

  public void stop() {
    _tts.stop();
  }
}
