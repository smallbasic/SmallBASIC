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

using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

// round down small Y touch values to 1 to allow the
// cursor to be positioned at the top of the screen
#define MIN_TOUCH_Y 20

// block for up to 2.5 seconds during shutdown to
// allow the game thread to exit gracefully.
#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// TizenAppForm
//
TizenAppForm::TizenAppForm() :
	_execThread(NULL),
	_state(kInitState),
	_buttonState(kLeftButton),
	_shortcut(kEscapeKey) {
}

result TizenAppForm::Construct() {
	result r = Form::Construct(FORM_STYLE_NORMAL);

	if (!IsFailed(r)) {
		_execThread = new Runtime();
		r = _execThread != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
	}
	if (!IsFailed(r)) {
    Rectangle rc = GetClientAreaBounds();
		r = _execThread->Construct(rc.width, rc.height);
	}
	if (!IsFailed(r)) {
    _display = new FormViewable();
    if (_display) {
      AddControl(_display);
      SetOrientation(ORIENTATION_AUTOMATIC);
      AddOrientationEventListener(*this);
    } else {
      r = E_OUT_OF_MEMORY;
    }
  }
  return r;
}

TizenAppForm::~TizenAppForm() {
	logEntered();

	if (_execThread && _state != kErrorState) {
		terminate();

		_execThread->Stop();
		if (_state != kErrorState) {
			_execThread->Join();
		}

		delete _execThread;
		_execThread = NULL;
	}

	logLeaving();
}

//
// abort the game thread
//
void TizenAppForm::terminate() {
	if (_state == kActiveState) {
		// block while thread ends
		AppLog("waiting for shutdown");
		for (int i = 0; i < EXIT_SLEEP_STEP && _state == kClosingState; i++) {
			Thread::Sleep(EXIT_SLEEP);
		}

		if (_state == kClosingState) {
			// failed to terminate - Join() will freeze
			_state = kErrorState;
		}
	}
}

void TizenAppForm::exitSystem() {
	_state = kErrorState;

	if (_execThread) {
		_execThread->Stop();
		delete _execThread;
		_execThread = NULL;
	}
}

result TizenAppForm::OnInitializing(void) {
	logEntered();

	AddOrientationEventListener(*this);
	AddTouchEventListener(*this);
	SetMultipointTouchEnabled(true);
	SetFormBackEventListener(this);
	SetFormMenuEventListener(this);

	// set focus to enable receiving key events
	SetEnabled(true);
	SetFocusable(true);
	SetFocus();

	return E_SUCCESS;
}

void TizenAppForm::OnOrientationChanged(const Control &source, OrientationStatus orientationStatus) {
	logEntered();
	if (_state == kInitState) {
		_state = kActiveState;
		_execThread->Start();
	}
}

/*
bool TizenAppForm::pollEvent(Common::Event &event) {
	bool result = false;

	_eventQueueLock->Acquire();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}
	if (_osdMessage) {
		TizenSystem *system = (TizenSystem *)g_system;
		TizenGraphicsManager *graphics = system->getGraphics();
		if (graphics) {
			graphics->displayMessageOnOSD(_osdMessage);
			_osdMessage = NULL;
		}
	}
	_eventQueueLock->Release();

	return result;
}
*/

//void TizenAppForm::pushEvent(Common::EventType type, const Point &currentPosition) {
  /*
	TizenSystem *system = (TizenSystem *)g_system;
	TizenGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		// graphics could be NULL at startup or when
		// displaying the system error screen
		Common::Event e;
		e.type = type;
		e.mouse.x = currentPosition.x;
		e.mouse.y = currentPosition.y > MIN_TOUCH_Y ? currentPosition.y : 1;

		bool moved = graphics->moveMouse(e.mouse.x, e.mouse.y);

		_eventQueueLock->Acquire();

		if (moved && type != Common::EVENT_MOUSEMOVE) {
			Common::Event moveEvent;
			moveEvent.type = Common::EVENT_MOUSEMOVE;
			moveEvent.mouse = e.mouse;
			_eventQueue.push(moveEvent);
		}

		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
  */
//}

/*
void TizenAppForm::pushKey(Common::KeyCode keycode) {
	if (_eventQueueLock) {
		Common::Event e;
		e.synthetic = false;
		e.kbd.keycode = keycode;
		e.kbd.ascii = keycode;
		e.kbd.flags = 0;

		_eventQueueLock->Acquire();
		e.type = Common::EVENT_KEYDOWN;
		_eventQueue.push(e);
		e.type = Common::EVENT_KEYUP;
		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
}

void TizenAppForm::setButtonShortcut() {
	switch (_buttonState) {
	case kLeftButton:
		setMessage(_s("Right Click Once"));
		_buttonState = kRightButtonOnce;
		break;
	case kRightButtonOnce:
		setMessage(_s("Right Click"));
		_buttonState = kRightButton;
		break;
	case kRightButton:
		setMessage(_s("Move Only"));
		_buttonState = kMoveOnly;
		break;
	case kMoveOnly:
		setMessage(_s("Left Click"));
		_buttonState = kLeftButton;
		break;
	}
}


*/
int TizenAppForm::getTouchCount() {
	Tizen::Ui::TouchEventManager *touch = Tizen::Ui::TouchEventManager::GetInstance();
	IListT<TouchEventInfo *> *touchList = touch->GetTouchInfoListN();
	int touchCount = touchList->GetCount();
	touchList->RemoveAll();
	delete touchList;
	return touchCount;
}

void TizenAppForm::OnTouchDoublePressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (_buttonState != kMoveOnly) {
		//pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
		//					currentPosition);
	}
}

void TizenAppForm::OnTouchFocusIn(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
}

void TizenAppForm::OnTouchFocusOut(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
}

void TizenAppForm::OnTouchLongPressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	logEntered();
	if (_buttonState != kLeftButton) {
		//pushKey(Common::KEYCODE_RETURN);
	}
}

void TizenAppForm::OnTouchMoved(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {

	  //pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);

}

void TizenAppForm::OnTouchPressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (getTouchCount() > 1) {
	} else if (_buttonState != kMoveOnly) {
		//pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
    //							currentPosition);
	}
}

void TizenAppForm::OnTouchReleased(const Control &source,
                                   const Point &currentPosition, 
                                   const TouchEventInfo &touchInfo) {
  int touchCount = getTouchCount();
  if (touchCount == 1) {
    //setShortcut();
  } else {
    if (touchCount == 2) {
      //invokeShortcut();
    }
  }
  if (_buttonState != kMoveOnly) {
    //pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
		//					currentPosition);
		if (_buttonState == kRightButtonOnce) {
			_buttonState = kLeftButton;
		}
		// flick to skip dialog
		if (touchInfo.IsFlicked()) {
			//pushKey(Common::KEYCODE_PERIOD);
		}
	}
}

void TizenAppForm::OnFormBackRequested(Form &source) {
	logEntered();
	if (_state == kActiveState) {
		//invokeShortcut();
	}
}

void TizenAppForm::OnFormMenuRequested(Form &source) {
	logEntered();
	if (_state == kActiveState) {
		//setShortcut();
	}
}
