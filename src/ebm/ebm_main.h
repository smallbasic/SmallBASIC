#ifndef EBM_MAIN_HH
#define EBM_MAIN_HH

#include <gui.h>
#include <pkg.h>
#include "AnsiWindow.h"

struct SBWindow : public AnsiWindow {
    SBWindow();
    ~SBWindow() {}
    
    bool wasBreakEv(void) {return runState==rsBreak;}
    bool isTurbo(void) {return bIsTurbo;}
    bool isMenuActive(void) {return menuActive;}
    bool isRunning(void) {return runState==rsBrun;}
    bool isInShell(void) {return runState==rsShell;}

    void run(const char* file);
    void doShell();
    void resetPen() {
        penX = 0;
        penY = 0;
        penDownX = 0;
        penDownY = 0;
        penDown = false;
        penUpdate = 0;
        penState = 0;
        menuActive = 0;
    }

    S16 penX;
    S16 penY;
    S16 penDownX;
    S16 penDownY;
    S16 penUpdate;
    S16 penState;
    bool penDown;
    bool menuActive;

    private:
    CMenuBar *runMenu;
    CMenuBar *shellMenu;
    bool bIsTurbo;
    bool bGenExe;
    const PKG *pkg;
    enum {rsIdle, rsShell, rsShellCmd, 
          rsBrun, rsBreak, rsSelFile, rsAbout} runState;

    S32 MsgHandler(MSG_TYPE type, CViewable *from, S32 data);
    void doKey(S32 key);
    void doAbout();
    void doAboutBasFile();
    void doHelp();
    void doKeyboard();
    void doList();
    void sendKeys(const char* s);
};

#endif
