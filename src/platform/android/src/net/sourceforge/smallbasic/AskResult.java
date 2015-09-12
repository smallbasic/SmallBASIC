package net.sourceforge.smallbasic;

class AskResult {
  private static final int ASK_YES = 0;
  private static final int ASK_NO = 1;
  private static final int ASK_CANCEL = 2;
  int value;

  AskResult() {
    value = ASK_CANCEL;
  }

  public void setCancel() {
    value = ASK_CANCEL;
  }

  public void setNo() {
    value = ASK_NO;
  }

  public void setYes() {
    value = ASK_YES;
  }

  @Override
  public String toString() {
    switch (value) {
    case ASK_YES:
      return "Yes";
    case ASK_NO:
      return "No";
    default:
      return "cancel";
    }
  }
}

