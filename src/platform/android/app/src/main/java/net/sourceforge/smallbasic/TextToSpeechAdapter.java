package net.sourceforge.smallbasic;

import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.util.Log;

import java.util.Locale;

class TextToSpeechAdapter implements OnInitListener {
  private static final String TAG = "smallbasic";
  private final TextToSpeech _tts;
  private boolean _ready;
  private String _text;
  private float _pitch;
  private float _speechRate;
  private Locale _locale;

  public TextToSpeechAdapter(Context context, String text) {
    _tts = new TextToSpeech(context, this);
    _text = text;
    _pitch = 1f;
    _speechRate = 1f;
    _locale = Locale.ENGLISH;
  }

  public void close() {
    _tts.stop();
    _tts.shutdown();
    _ready = false;
  }

  @Override
  public void onInit(int status) {
    if (status == TextToSpeech.SUCCESS) {
      int result = _tts.setLanguage(_locale);
      if (result != TextToSpeech.LANG_MISSING_DATA &&
          result != TextToSpeech.LANG_NOT_SUPPORTED) {
        _tts.setPitch(_pitch);
        _tts.setSpeechRate(_speechRate);
        _ready = true;
        if (_text != null) {
          speak(_text);
        }
      }
    }
    Log.i(TAG, "Tts init: " + _ready);
  }

  public void setLocale(Locale locale) {
    this._locale = locale;
    if (_ready) {
      int result = _tts.setLanguage(_locale);
      if (result == TextToSpeech.LANG_MISSING_DATA ||
          result == TextToSpeech.LANG_NOT_SUPPORTED) {
        Log.i(TAG, "invalid locale: " + locale);
        _locale = Locale.ENGLISH;
        _tts.setLanguage(_locale);
      }
    }
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
      _tts.speak(text, TextToSpeech.QUEUE_ADD, null, null);
    } else {
      _text = text;
    }
  }

  public void stop() {
    _tts.stop();
  }
}
