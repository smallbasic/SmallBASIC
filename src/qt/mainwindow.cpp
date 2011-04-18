// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QCompleter>
#include <QDesktopServices>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QSetIterator>
#include <QSettings>
#include <QTextStream>
#include <QUrl>

#include "mainwindow.h"
#include "config.h"
#include "sbapp.h"

const char* aboutText =
 "QT Version " VERSION "\n\n"
 "Copyright (c) 2002-2011 Chris Warren-Smith. \n"
 "Copyright (c) 2000-2006 Nicholas Christopoulos\n\n"
 "http://smallbasic.sourceforge.net\n\n"
 "SmallBASIC comes with ABSOLUTELY NO WARRANTY.\n"
 "This program is free software; you can use it\n"
 "redistribute it and/or modify it under the terms of the\n"
 "GNU General Public License version 2 as published by\n"
 "the Free Software Foundation.";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow) {
  ui->setupUi(this);
  wnd = this;
  out = ui->ansiWidget;
  out->setMouseListener(this);

  // accept keyboard input
  setFocusPolicy(Qt::ClickFocus);
  setAcceptDrops(true);

  // setup the URL input widget
  textInput = new QLineEdit();
  QCompleter* completer = new QCompleter(this);
  QFileSystemModel* fsModel = new QFileSystemModel(completer);
  fsModel->setRootPath("");
  fsModel->setNameFilters(QStringList("*.bas"));
  completer->setModel(fsModel);
  textInput->setCompleter(completer);
  ui->toolBar->addWidget(textInput);
  ui->toolBar->addAction(ui->actionStart);

  // setup dialogs
  logDialog = new QDialog();
  errorUi = new Ui::ErrorConsole();
  errorUi->setupUi(logDialog);
  errorUi->plainTextEdit->setReadOnly(true);

  sourceDialog = new QDialog();
  sourceUi = new Ui::SourceDialog();
  sourceUi->setupUi(sourceDialog);
  sourceUi->plainTextEdit->setReadOnly(true);

  // connect signals and slots
  connect(ui->actionBookmarkProgram, SIGNAL(triggered()), 
          this, SLOT(bookmarkProgram()));
  connect(ui->actionExit, SIGNAL(triggered()), 
          this, SLOT(close()));
  connect(ui->actionOpen, SIGNAL(triggered()), 
          this, SLOT(fileOpen()));
  connect(ui->actionAbout, SIGNAL(triggered()), 
          this, SLOT(helpAbout()));
  connect(ui->actionCopy, SIGNAL(triggered()), 
          ui->ansiWidget, SLOT(copySelection()));
  connect(ui->actionFind, SIGNAL(triggered()), 
          ui->ansiWidget, SLOT(findText()));
  connect(ui->actionFindAgain, SIGNAL(triggered()), 
          ui->ansiWidget, SLOT(findNextText()));
  connect(ui->actionSelectAll, SIGNAL(triggered()), 
          ui->ansiWidget, SLOT(selectAll()));
  connect(ui->actionPreferences, SIGNAL(triggered()), 
          this, SLOT(viewPreferences()));
  connect(ui->actionHomePage, SIGNAL(triggered()), 
          this, SLOT(helpHomePage()));
  connect(ui->actionNewWindow, SIGNAL(triggered()), 
          this, SLOT(newWindow()));
  connect(ui->actionRefresh, SIGNAL(triggered()), 
          this, SLOT(runRestart()));
  connect(ui->actionBreak, SIGNAL(triggered()), 
          this, SLOT(runBreak()));
  connect(ui->actionHome, SIGNAL(triggered()), 
          this, SLOT(runHome()));
  connect(ui->actionStart, SIGNAL(triggered()), 
          this, SLOT(runStart()));
  connect(textInput, SIGNAL(returnPressed()), 
          this, SLOT(runStart()));
  connect(ui->actionViewBookmarks, SIGNAL(triggered()), 
          this, SLOT(viewBookmarks()));
  connect(ui->actionProgramSource, SIGNAL(triggered()), 
          this, SLOT(viewProgramSource()));
  connect(ui->actionErrorConsole, SIGNAL(triggered()), 
          this, SLOT(viewErrorConsole()));

  // setup state
  resourceApp = false;
  runMode = init_state;
  opt_ide = IDE_NONE;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;
  opt_interactive = true;
  opt_verbose = false;
  opt_quiet = true;
  os_graphics = 1;

  ui->statusbar->addWidget(&status);
  showStatus(false);

  // restore settings
  QCoreApplication::setOrganizationName("SmallBASIC");
  QCoreApplication::setApplicationName("SmallBASIC");
  QCoreApplication::setOrganizationDomain("sourceforge.net");

  QSettings settings;
  settings.beginGroup("MainWindow");
  resize(settings.value("size", size()).toSize());
  move(settings.value("pos", pos()).toPoint());
  settings.endGroup();
}

MainWindow::~MainWindow() {
  delete ui;
}

// return whether the break key was pressed 
bool MainWindow::isBreakExec() {
  return (runMode == break_state || runMode == quit_state);
}

// return whether a smallbasic program is running
bool MainWindow::isRunning() {
  return (runMode == run_state || runMode == modal_state);
}

// return whether a smallbasic program is running in modal mode
bool MainWindow::isRunModal() {
  return (runMode == modal_state);
}

// append to the log window
void MainWindow::logWrite(const char* msg) {
  QString buffer = errorUi->plainTextEdit->toPlainText();
  buffer.append(msg);
  errorUi->plainTextEdit->setPlainText(buffer);
}

// set the program modal state
void MainWindow::setRunModal(bool modal) {
  runMode = modal ? modal_state : run_state;  
}

// end the program modal state
void MainWindow::endModal() {
  runMode = run_state;
}

// cause the basicMain loop to end
void MainWindow::runQuit() {
  runBreak();
  close();
}

// ensure any running program is terminated upon closing
void MainWindow::closeEvent(QCloseEvent*) {
  if (runMode == run_state || runMode == modal_state) {
    brun_break();
    runMode = quit_state;
  }

  QSettings settings;
  settings.beginGroup("MainWindow");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();
}

// handle file drag and drop from a desktop file manager
void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  QString path = dropFile(event->mimeData());
  if (path.length() > 0) {
    event->accept();
  }
  else {
    event->ignore();
  }
}

// handle file drag and drop from a desktop file manager
void MainWindow::dropEvent(QDropEvent* event) {
  QString path = dropFile(event->mimeData());
  if (path.length() > 0) {
    loadPath(path);
  }
}

// launch home page program
bool MainWindow::event(QEvent* event) {
  if (event->type() == QEvent::ShowToParent) {
    QStringList args = QCoreApplication::arguments();
    if (args.count() == 2) {
      loadPath(args.value(1));
    }
    else {
      runHome();
    }
  }
  return QMainWindow::event(event);
}

// adds the current program to the bookmark list
void MainWindow::bookmarkProgram() {
  if (programPath.length() > 0 && !programPath.contains("qt_temp")) {
    QSettings settings;
    
    QSet<QString> paths;
    int size = settings.beginReadArray("bookmarks");
    for (int i = 0; i < size; i++) {
      settings.setArrayIndex(i);
      paths << settings.value("path").toString();
    }
    paths << programPath;
    settings.endArray();

    settings.beginWriteArray("bookmarks");
    QSetIterator<QString> iter(paths);
    int i = 0;
    while (iter.hasNext()) {
      settings.setArrayIndex(i++);
      settings.setValue("path", iter.next());
    }
    settings.endArray();
  }
}

// open a new file system program file
void MainWindow::fileOpen() {
  QString path = QFileDialog::getOpenFileName(this, tr("Open Program"),
                                              QString(), 
                                             tr("BASIC Files (*.bas)"));
  if (QFileInfo(path).isFile() && QString::compare(programPath, path) != 0) {
    loadPath(path);
  }
}

// display the about box
void MainWindow::helpAbout() {
  QMessageBox::information(this, tr("SmallBASIC"),
                           tr(aboutText), QMessageBox::Ok);
}

// show our home page
void MainWindow::helpHomePage() {
  QDesktopServices::openUrl(QUrl("http://smallbasic.sourceforge.net"));
}

// opens a new program window
void MainWindow::newWindow() {
  QProcess::startDetached(QCoreApplication::applicationFilePath());
}

// program break button handler
void MainWindow::runBreak() {
  if (runMode == run_state || runMode == modal_state) {
    brun_break();
    runMode = break_state;
  }
}

// program home button handler
void MainWindow::runHome() {
  loadResource("home", ":/res/home.bas");
}

// program restart button handler
void MainWindow::runRestart() {
  switch (runMode) {
  case init_state:
    runStart();
    break;
  case run_state:
    runMode = restart_state;
    brun_break();
    break;
  default:
    break;
  }
}

// program start button handler
void MainWindow::runStart() {
  loadPath(textInput->text());
}

// view bookmarks
void MainWindow::viewBookmarks() {
  loadResource("bookmarks", ":/res/bookmarks.bas");
}

// view the error console
void MainWindow::viewErrorConsole() {
  logDialog->show();
  logDialog->raise();
}

// view the preferences dialog
void MainWindow::viewPreferences() {
  loadResource("bookmarks", ":/res/settings.bas");
}

// view the program source code
void MainWindow::viewProgramSource() {
  sourceDialog->show();
  sourceDialog->raise();
}

// convert mouse press event into a smallbasic mouse press event
void MainWindow::mousePressEvent() {
  if (isRunning()) {
    keymap_invoke(SB_KEY_MK_PUSH);
  }
}

// convert mouse release event into a smallbasic mouse release event
void MainWindow::mouseReleaseEvent() {
  if (isRunning()) {
    keymap_invoke(SB_KEY_MK_RELEASE);
  }
}

// convert mouse move event into a smallbasic mouse move event
void MainWindow::mouseMoveEvent(bool down) {
  if (isRunning()) {
    keymap_invoke(down ? SB_KEY_MK_DRAG : SB_KEY_MK_MOVE);
  }
}

// convert keystrokes into smallbasic key events
void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (isRunning()) {
    switch (event->key()) {
    case Qt::Key_Tab:
      dev_pushkey(SB_KEY_TAB);
      break;
    case Qt::Key_Home:
      dev_pushkey(SB_KEY_KP_HOME);
      break;
    case Qt::Key_End:
      dev_pushkey(SB_KEY_END);
      break;
    case Qt::Key_Insert:
      dev_pushkey(SB_KEY_INSERT);
      break;
    case Qt::Key_Menu:
      dev_pushkey(SB_KEY_MENU);
      break;
    case Qt::Key_multiply:
      dev_pushkey(SB_KEY_KP_MUL);
      break;
    case Qt::Key_Plus:
      dev_pushkey(SB_KEY_KP_PLUS);
      break;
    case Qt::Key_Minus:
      dev_pushkey(SB_KEY_KP_MINUS);
      break;
    case Qt::Key_Slash:
      dev_pushkey(SB_KEY_KP_DIV);
      break;
    case Qt::Key_F1:       
      dev_pushkey(SB_KEY_F(1));
      break;
    case Qt::Key_F2:
      dev_pushkey(SB_KEY_F(2));
      break;
    case Qt::Key_F3:
      dev_pushkey(SB_KEY_F(3));
      break;
    case Qt::Key_F4:
      dev_pushkey(SB_KEY_F(4));
      break;
    case Qt::Key_F5:
      dev_pushkey(SB_KEY_F(5));
      break;
    case Qt::Key_F6:
      dev_pushkey(SB_KEY_F(6));
      break;
    case Qt::Key_F7:
      dev_pushkey(SB_KEY_F(7));
      break;
    case Qt::Key_F8:
      dev_pushkey(SB_KEY_F(8));
      break;
    case Qt::Key_F9:
      dev_pushkey(SB_KEY_F(9));
      break;
    case Qt::Key_F10:
      dev_pushkey(SB_KEY_F(10));
      break;
    case Qt::Key_F11:
      dev_pushkey(SB_KEY_F(11));
      break;
    case Qt::Key_F12:
      dev_pushkey(SB_KEY_F(12));
      break;
    case Qt::Key_PageUp:
      dev_pushkey(SB_KEY_PGUP);
      break;
    case Qt::Key_PageDown:
      dev_pushkey(SB_KEY_PGDN);
      break;
    case Qt::Key_Up:
      dev_pushkey(SB_KEY_UP);
      break;
    case Qt::Key_Down:
      dev_pushkey(SB_KEY_DN);
      break;
    case Qt::Key_Left:
      dev_pushkey(SB_KEY_LEFT);
      break;
    case Qt::Key_Right:
      dev_pushkey(SB_KEY_RIGHT);
      break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
      dev_pushkey(SB_KEY_BACKSPACE);
      break;
    case Qt::Key_Return:
      dev_pushkey(13);
      break;
    case 'b':
      if (event->modifiers() & Qt::ControlModifier) {
        runBreak();
        break;
      }
      dev_pushkey(event->key());
      break;
    case 'q':
      if (event->modifiers() & Qt::ControlModifier) {
        runQuit();
        break;
      }
      dev_pushkey(event->key());
      break;
    
    default:
      dev_pushkey(event->key());
      break;
    }
  }
}

// main basic program loop
void MainWindow::basicMain(QString path) {
  programPath = path;

  opt_pref_width = 0;
  opt_pref_height = 0;
  bool success = false;

  do {
    runMode = run_state;
    showStatus(false);

    // start in the directory of the bas program
    QString path = programPath.replace("\\", "/");
    int index = path.lastIndexOf("/");
    if (index != -1) {
      if (!chdir(path.left(index).toUtf8().data())) {
        path = path.right(path.length() - index - 1);
      }
    }
    success = sbasic_main(path.toUtf8().data());
  }
  while (runMode == restart_state);

  if (runMode == quit_state) {
    exit(0);
  }

  runMode = init_state;
  showStatus(!success);
}

// return any new .bas program filename from mimeData 
QString MainWindow::dropFile(const QMimeData* mimeData) {
  QString result;
  if (mimeData->hasText()) {
    QString path = mimeData->text().trimmed();
    if (path.startsWith("file://")) {
      path = path.remove(0, 7);
    }
    if (QFileInfo(path).isFile() &&
        path.endsWith(".bas") &&
        QString::compare(path, this->programPath) != 0) {
      result = path;
    }
  }
  return result;
}

// loads and runs a resource program
void MainWindow::loadResource(QString key, QString path) {
  QSettings settings;
  QString homePath = settings.value(key).toString();
  resourceApp = true;

  if (!homePath.length()) {
    // show the default home page
    QTemporaryFile tmpFile;
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      tmpFile.open();
      tmpFile.write(file.readAll());
      tmpFile.close();
      file.close();
    }

    homePath = tmpFile.fileName();
    loadPath(homePath, false);
  }
  else {
    loadPath(homePath);
  }
  resourceApp = false;
}

// resolve the path to a local file then call basicMain
void MainWindow::loadPath(QString path, bool showPath) {
  runBreak();

  if (showPath) {
    textInput->setText(path);
  }

  if (QFileInfo(path).isFile()) {
    // populate the source view window
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
      QTextStream stream(&file);
      sourceUi->plainTextEdit->setPlainText(stream.readAll());
    }
    setFocus();
    basicMain(path);
  }
  else if (path.startsWith("http://")) {
    new HttpFile(this, path);
  }
}

// called from HttpFile when a loading error occurs
void MainWindow::loadError(QString message) {
  out->print(message.toUtf8().data());
}

// display the status depending on the current state
void MainWindow::showStatus(bool error) {
  if (error) {
    status.setText(gsb_last_errmsg);
  }
  else {
    switch (runMode) {
    case init_state:
      status.setText("Ready");
      break;
    case run_state:
      status.setText("RUN");
      break;
    default:
      break;
    }
  }
}

// End of "$Id$".
