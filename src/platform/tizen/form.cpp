// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/tizen/form.h"
#include "platform/common/utils.h"

using namespace Tizen::App;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Base::Utility;
using namespace Tizen::Ui::Controls;

#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// converts a Tizen (wchar) String into a StringLib (char) string
//
String fromString(const Tizen::Base::String &in) {
  Tizen::Base::ByteBuffer *buf = StringUtil::StringToUtf8N(in);
  String result((const char*)buf->GetPointer());
  delete buf;
  return result;
}

//
// AppForm
//
AppForm::AppForm() :
  _editField(NULL),
  _contextMenu(NULL),
  _display(NULL),
  _runtime(NULL) {
}

AppForm::~AppForm() {
  logEntered();
  if (_runtime) {
    if (_runtime->isActive()) {
      // push an exit message onto the thread event queue
      _runtime->setExit(true);

      // block while thread ends
      AppLog("Waiting for runtime shutdown");
      for (int i = 0; i < EXIT_SLEEP_STEP && _runtime->isClosing(); i++) {
        Thread::Sleep(EXIT_SLEEP);
      }

      _runtime->Stop();
      _runtime->Join();
    }
    delete _runtime;
    _runtime = NULL;
  }

  logLeaving();
}

result AppForm::Construct(int w, int h) {
  logEntered();
  result r = Form::Construct(FORM_STYLE_NORMAL);
  String appRootPath = fromString(App::GetInstance()->GetAppRootPath());

  if (!IsFailed(r)) {
    _display = new FormViewable();
    if (_display && _display->Construct(appRootPath, w, h) == E_SUCCESS &&
        AddControl(_display) == E_SUCCESS) {
      SetOrientation(ORIENTATION_AUTOMATIC);
    } else {
      AppLog("Failed to create FormViewable");
      r = E_OUT_OF_MEMORY;
    }
  }

  if (!IsFailed(r)) {
    _runtime = new RuntimeThread(w, h);
    r = _runtime != NULL ? _runtime->Construct(appRootPath) : E_OUT_OF_MEMORY;
  }

  if (!IsFailed(r)) {
    _editField = new EditField();
    r = _editField == NULL ? E_OUT_OF_MEMORY :
        _editField->Construct(Rectangle(0, 0, 1, 1),
                              EDIT_FIELD_STYLE_NORMAL_SMALL,
                              INPUT_STYLE_OVERLAY,
                              EDIT_FIELD_TITLE_STYLE_NONE,
                              true, 100, GROUP_STYLE_SINGLE);
    if (!IsFailed(r)) {
      r = AddControl(_editField);
    }
  }

  if (!IsFailed(r)) {
    _contextMenu = new ContextMenu();
    r = _contextMenu == NULL ? E_OUT_OF_MEMORY :
        _contextMenu->Construct(Point(0, h), CONTEXT_MENU_STYLE_GRID,
                                CONTEXT_MENU_ANCHOR_DIRECTION_UPWARD);
  }

  if (IsFailed(r)) {
    AppLog("Form startup failed");
    delete _display;
    delete _runtime;
    delete _editField;
    delete _contextMenu;
    _runtime = NULL;
    _display = NULL;
    _editField = NULL;
    _contextMenu = NULL;
  }
  return r;
}

void AppForm::OnActionPerformed(const Control &source, int actionId) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED;
  maEvent.optionsBoxButtonIndex = actionId;
  _runtime->pushEvent(maEvent);
}

result AppForm::OnDraw() {
  return _display->OnDraw();
}

void AppForm::OnFormBackRequested(Form &source) {
  logEntered();
  _runtime->setBack();
}

void AppForm::OnFormMenuRequested(Form &source) {
  logEntered();
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_KEY_PRESSED;
  maEvent.nativeKey = KEY_CONTEXT_MENU;
  _runtime->pushEvent(maEvent);
}

result AppForm::OnInitializing(void) {
  logEntered();

  AddOrientationEventListener(*this);
  SetFormBackEventListener(this);
  SetFormMenuEventListener(this);

  _editField->AddKeyEventListener(*this);
  _editField->SetKeypadEnabled(true);
  _editField->SetKeypadAction(KEYPAD_ACTION_DONE);
  _editField->SetEnabled(true);
  _editField->SetShowState(false);
  _contextMenu->AddActionEventListener(*this);

  // set focus to enable receiving key events
  _display->AddTouchEventListener(*this);
  SetEnabled(true);
  SetFocusable(true);
  SetFocus();

  if (_runtime->isInitial()) {
    _runtime->Start();
  }

  logLeaving();
  return E_SUCCESS;
}

void AppForm::OnKeyReleased(const Control &source, KeyCode keyCode) {
  MAEvent maEvent;
  wchar_t ch = 0;
  maEvent.type = EVENT_TYPE_KEY_PRESSED;
  maEvent.nativeKey = keyCode;
  if (_editField->GetTextLength() > 0 &&
      _editField->GetText().GetCharAt(0, ch) == E_SUCCESS) {
    maEvent.key = ch;
  }
  _editField->Clear();
  _runtime->pushEvent(maEvent);
  if (keyCode == KEY_ENTER) {
    _editField->HideKeypad();
    _editField->RequestRedraw();
  }
}

void AppForm::OnOrientationChanged(const Control &source,
                                   OrientationStatus orientationStatus) {
  logEntered();
}

void AppForm::OnTouchMoved(const Control &source,
                           const Point &currentPosition,
                           const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_DRAGGED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

void AppForm::OnTouchPressed(const Control &source,
                             const Point &currentPosition,
                             const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_PRESSED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

void AppForm::OnTouchReleased(const Control &source,
                              const Point &currentPosition,
                              const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_RELEASED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

void AppForm::showAlert(ArrayList *args) {
  int modalResult = 0;
  MessageBox messageBox;
  Tizen::Base::String *title = (Tizen::Base::String *)args->GetAt(0);
  Tizen::Base::String *text = (Tizen::Base::String *)args->GetAt(1);
  messageBox.Construct(*title, *text, MSGBOX_STYLE_OK);
  messageBox.ShowAndWait(modalResult);
}

void AppForm::showKeypad() {
  _editField->RequestRedraw();
  _editField->SetShowState(true);
  _editField->ShowKeypad();
  _editField->SetFocus();
}

void AppForm::showMenu(ArrayList *args) {
  _contextMenu->RemoveAllItems();
  int len = args->GetCount();
  for (int i = 0; i < len; i++) {
    Tizen::Base::String *text = (Tizen::Base::String *)args->GetAt(i);
    _contextMenu->AddItem(*text, i);
  }
  _contextMenu->SetShowState(true);
  _contextMenu->Show();
}

