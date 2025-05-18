package net.sourceforge.smallbasic;

import android.app.Activity;
import android.content.Context;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

class FindBar {
  private LinearLayout findBar;
  private EditText findInput;
  private Button findNext, findPrev, findClose;

  public FindBar(Activity activity) {
    FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();

    // Create find bar layout
    findBar = new LinearLayout(activity);
    findBar.setOrientation(LinearLayout.HORIZONTAL);
    findBar.setBackgroundColor(0xFFEEEEEE);
    findBar.setPadding(8, 8, 8, 8);
    findBar.setVisibility(View.GONE);

    // Create EditText
    findInput = new EditText(activity);
    LinearLayout.LayoutParams lpInput = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1);
    findInput.setLayoutParams(lpInput);
    findInput.setHint("Find...");
    findBar.addView(findInput);

    // Create buttons
    findPrev = new Button(activity);
    findPrev.setText("↑");
    findBar.addView(findPrev);

    findNext = new Button(activity);
    findNext.setText("↓");
    findBar.addView(findNext);

    findClose = new Button(activity);
    findClose.setText("✕");
    findBar.addView(findClose);

    // Add find bar to decor view
    FrameLayout.LayoutParams lp =
      new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                                   FrameLayout.LayoutParams.WRAP_CONTENT,
                                   Gravity.BOTTOM);
    decor.addView(findBar, lp);

    // Hook up buttons
    findNext.setOnClickListener(v -> {
        String query = findInput.getText().toString();
        // TODO: call native method or Java function to highlight next
      });
    
    findPrev.setOnClickListener(v -> {
        String query = findInput.getText().toString();
        // TODO: call native method or Java function to highlight previous
      });

    findClose.setOnClickListener(v -> {
        findBar.setVisibility(View.GONE);
        //hideKeyboard(findInput);
      });
  }

  // To show the find bar
  public void show() {
    findBar.setVisibility(View.VISIBLE);
    findInput.requestFocus();
  }
}
