// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditTypes.pas' rev: 6.00

#ifndef SynEditTypesHPP
#define SynEditTypesHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synedittypes
{
//-- type declarations -------------------------------------------------------
typedef Set<char, 0, 255>  TSynIdentChars;

#pragma option push -b-
enum TSynSelectionMode { smNormal, smLine, smColumn };
#pragma option pop

typedef TSynSelectionMode *PSynSelectionMode;

//-- var, const, procedure ---------------------------------------------------
#define TSynSpecialChars (System::Set<char, 0, 255> () << '\xc0' << '\xc1' << '\xc2' << '\xc3' << '\xc4' << '\xc5' << '\xc6' << '\xc7' << '\xc8' << '\xc9' << '\xca' << '\xcb' << '\xcc' << '\xcd' << '\xce' << '\xcf' << '\xd0' << '\xd1' << '\xd2' << '\xd3' << '\xd4' << '\xd5' << '\xd6' << '\xd8' << '\xd9' << '\xda' << '\xdb' << '\xdc' << '\xdd' << '\xde' << '\xdf' << '\xe0' << '\xe1' << '\xe2' << '\xe3' << '\xe4' << '\xe5' << '\xe6' << '\xe7' << '\xe8' << '\xe9' << '\xea' << '\xeb' << '\xec' << '\xed' << '\xee' << '\xef' << '\xf0' << '\xf1' << '\xf2' << '\xf3' << '\xf4' << '\xf5' << '\xf6' << '\xf8' << '\xf9' << '\xfa' << '\xfb' << '\xfc' << '\xfd' << '\xfe' << '\xff' )
#define TSynValidStringChars System::Set<char, 0, 255> () 
#define TSynWordBreakChars (System::Set<char, 0, 255> () << '\x21' << '\x22' << '\x27' << '\x28' << '\x29' << '\x2a' << '\x2b' << '\x2c' << '\x2d' << '\x2e' << '\x2f' << '\x3a' << '\x3b' << '\x3d' << '\x3f' << '\x41' << '\x42' << '\x47' << '\x48' << '\x49' << '\x4a' << '\x4b' << '\x4c' << '\x4d' << '\x4e' << '\x4f' << '\x5a' << '\x5b' << '\x5d' << '\x5f' << '\x61' << '\x62' << '\x67' << '\x68' << '\x69' << '\x6a' << '\x6b' << '\x6c' << '\x6d' << '\x6e' << '\x6f' << '\x7a' << '\x7b' << '\x7d' << '\x7f' )
static const char TSynTabChar = '\x9';
static const char SynTabGlyph = '\xbb';
static const char SynSoftBreakGlyph = '\xac';
static const char SynLineBreakGlyph = '\xb6';
static const char SynSpaceGlyph = '\xb7';

}	/* namespace Synedittypes */
using namespace Synedittypes;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditTypes
