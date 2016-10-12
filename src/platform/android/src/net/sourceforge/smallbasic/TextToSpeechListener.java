package net.sourceforge.smallbasic;

import java.util.Locale;

import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.util.Log;

public class TextToSpeechListener implements OnInitListener {
  private static final String TAG = "smallbasic";
  private TextToSpeech _tts;
  private boolean _ready;
  private String _text;

  public TextToSpeechListener(Context context, String text) {
    _tts = new TextToSpeech(context, this);
    _text = text;
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
      if (result != TextToSpeech.LANG_MISSING_DATA &&
          result != TextToSpeech.LANG_NOT_SUPPORTED) {
        _ready = true;
        speak(_text);
      }
    }
    Log.i(TAG, "Tts init: " + _ready);
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
