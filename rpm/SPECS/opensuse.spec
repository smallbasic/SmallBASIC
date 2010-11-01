# see: http://www.ibm.com/developerworks/library/l-rpm1/

%define _topdir	/home/chris/src/smallbasic/rpm
%define name	smallbasic
%define release	1
%define version 0.10.8
%define buildroot %{_topdir}/%{name}-%{version}-root

BuildRoot:	%{buildroot}
Summary: 	SmallBASIC
License: 	GPL
Name: 		%{name}
Version: 	%{version}
Release: 	%{release}
Source: 	%{name}-%{version}.tar.gz
Prefix: 	/usr
Group: 		Development/Tools

%description
SmallBASIC is a fast and easy to learn BASIC language interpreter 
ideal for everyday calculations, scripts and prototypes. 
SmallBASIC includes trigonometric, matrices and algebra functions, 
a built in IDE, a powerful string library, system, sound, and graphic
commands along with structured programming syntax. 
SmallBASIC is licensed under the GPL. 

%prep
%setup -q

%build
./configure --enable-fltk --prefix=/usr
make -s

%install
make install prefix=$RPM_BUILD_ROOT/usr

%files
%defattr(-,root,root)
/usr/bin/sbasici
/usr/share/applications/smallbasic.desktop
/usr/share/icons/hicolor/32x32/apps/sb-desktop-32x32.png
/usr/share/smallbasic/ide/small-basic-mode.el
/usr/share/smallbasic/ide/smallbasic.lang
/usr/share/smallbasic/ide/smallbasic.syn
/usr/share/smallbasic/plugins/comment_in.bas
/usr/share/smallbasic/plugins/comment_out.bas
/usr/share/smallbasic/plugins/dos2unix.bas
/usr/share/smallbasic/plugins/indent.bas
/usr/share/smallbasic/plugins/publish.bas
/usr/share/smallbasic/plugins/help.bas
/usr/share/smallbasic/sbasic_ref.csv
