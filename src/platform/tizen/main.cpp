// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <FBase.h>
#include <FApp.h>
#include <FSystem.h>

#include "platform/tizen/application.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Collection;

/**
 * The entry function of tizen application called by the operating system.
 */
extern "C" _EXPORT_ int OspMain(int argc, char *pArgv[]) {
	result r = E_SUCCESS;

	AppLog("Application started.");
	ArrayList args(SingleObjectDeleter);
	args.Construct();
	for (int i = 0; i < argc; i++) {
		args.Add(new (std::nothrow) String(pArgv[i]));
	}

	r = Tizen::App::UiApp::Execute(TizenApp::createInstance, &args);
	TryLog(r == E_SUCCESS, "[%s] Application execution failed", GetErrorMessage(r));
	AppLog("Application finished.");

	return static_cast<int>(r);
}

