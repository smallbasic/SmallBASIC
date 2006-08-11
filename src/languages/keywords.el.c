/*
*	SB Keywords in Greek - ISO-8859-7
*
*	Rules:
*	* Names can be 15 chars long. (defined on scan.h)
* 	* Names must defined always with capitals. lower-case means invisible keywords.
*	* Spaces are not allowed. Only letters, digits and the character '_'.
*	* Alias supported just repeat the keyword.
*	  Example:
*		{ "GOTO",		kwGOTO	},
*		{ "GOTHERE",	kwGOTO	},
*	  This means the command GOTO it is known to the compiler with the names "GOTO" and "GOTHERE".
*/

/*
*	GENERIC KEYWORDS (basic bc-types & oldest code)
*
*	This table is limited to 256 elements
*/
struct keyword_s keyword_table[] = {
//{ Command-Name, Command-Code },
///1234567890123456
{ "топий╧", 		kwLOCAL	},
{ "диадийас╨а", 		kwPROC	},
{ "сум╤ятгсг", 		kwFUNC	},
{ "DEF", 		kwFUNC	},
{ "BYREF",		kwBYREF	},
{ "DECLARE",	kwDECLARE	},
{ "IMPORT",		kwIMPORT	},
{ "EXPORT",		kwEXPORT	},
{ "UNIT",		kwUNIT	},

{ "LET", 		kwLET	},
{ "CONST", 		kwCONST	},
{ "п╨майас", 		kwDIM	},
{ "REDIM", 		kwREDIM	},
{ "стал╤та", kwSTOP	},
{ "т╦кос",		kwEND	},
{ "т╬пысе",		kwPRINT	},
{ "SPRINT",		kwSPRINT },
{ "ф╧тгсе",		kwINPUT	},
{ "SINPUT",		kwSINPUT	},
{ "сгл",		kwREM	},
{ "CHAIN",		kwCHAIN	},
{ "ON",			kwON	},

{ "LABEL",		kwLABEL	},
{ "п╧цаиме",		kwGOTO	},
{ "ам",			kwIF	},
{ "т╪те",		kwTHEN	},
{ "акки©с",		kwELSE	},
{ "ELIF",		kwELIF	},
{ "ELSEIF",		kwELIF	},
{ "т╦костоуам",		kwENDIF	},
{ "т╦косам",		kwENDIF	},
{ "там",			kwENDIF	},
{ "циа",		kwFOR	},
{ "╦ыс",			kwTO	},
{ "STEP",		kwSTEP	},
{ "еп╪лемо",		kwNEXT	},
{ "еж╪сом",		kwWHILE	},
{ "теж",		kwWEND	},
{ "епам╦кабе",	kwREPEAT	},
{ "л╦вяи",		kwUNTIL	},
{ "SELECT",     kwSELECT },
{ "CASE",       kwCASE },
{ "GOSUB",		kwGOSUB	},
{ "RETURN",		kwRETURN	},
{ "READ",		kwREAD	},
{ "DATA",		kwDATA	},
{ "RESTORE",	kwRESTORE	},
{ "бцес",		kwEXIT	},

{ "ERASE", 		kwERASE	},
{ "USE",		kwUSE	},
{ "USING",		kwUSING		},
{ "USG",		kwUSING		},

{ "LINE",		kwLINE	},
{ "COLOR",		kwCOLOR	},

{ "RUN",		kwRUN	},
{ "EXEC",		kwEXEC	},

{ "OPEN", 		kwOPEN	},
{ "APPEND",		kwAPPEND	},
{ "AS",			kwAS },		// OPEN's args
{ "CLOSE", 		kwCLOSE  },
{ "LINEINPUT", 	kwLINEINPUT },		// The QB's keyword is "LINE INPUT"
{ "LINPUT", 	kwLINEINPUT },		// The QB's keyword is "LINE INPUT"
{ "SEEK", 		kwSEEK	},
{ "WRITE", 		kwFILEWRITE	},
//{ "READ", 	kwFILEREAD	},
//{ "INPUT", 		kwFINPUT },	// not needed

{ "INSERT",		kwINSERT },
{ "DELETE",		kwDELETE },

/* DEBUG */
{ "TRON",		kwTRON	},
{ "TROFF",		kwTROFF	},
{ "OPTION",		kwOPTION },

{ "BG",		kwBACKG },

/* for debug */
/* by using small letters, */
/* the keywords are invisible by compiler */
{ "$i32", 		kwTYPE_INT		},
{ "$r64",		kwTYPE_NUM		},
{ "$str",		kwTYPE_STR		},
{ "$log",		kwTYPE_LOGOPR	},
{ "$cmp",		kwTYPE_CMPOPR	},
{ "$add",		kwTYPE_ADDOPR	},
{ "$mul",		kwTYPE_MULOPR	},
{ "$pow",		kwTYPE_POWOPR	},
{ "$unr",		kwTYPE_UNROPR	},
{ "$var",		kwTYPE_VAR	},
{ "$tln",		kwTYPE_LINE	},
{ "$lpr",		kwTYPE_LEVEL_BEGIN	},
{ "$rpr",		kwTYPE_LEVEL_END	},
{ "$crv",		kwTYPE_CRVAR	},
{ "$sep",		kwTYPE_SEP		},
{ "$biF",		kwTYPE_CALLF	},
{ "$biP",		kwTYPE_CALLP	},
{ "$exF",		kwTYPE_CALLEXTF	},
{ "$exP",		kwTYPE_CALLEXTP	},
{ "$ret",		kwTYPE_RET	},
{ "$udp",		kwTYPE_CALL_UDP	},
{ "$udf",		kwTYPE_CALL_UDF	},

{ "", 0 }
};

/* ----------------------------------------------------------------------------------------------------------------------- */

/*
*	OPERATORS (not the symbols)
*/
struct opr_keyword_s opr_table[] = {
{ "йаи",   kwTYPE_LOGOPR, '&' 		 },
{ "╧",    kwTYPE_LOGOPR, '|' 		 },
{ "BAND",  kwTYPE_LOGOPR, OPLOG_BAND },
{ "BOR",   kwTYPE_LOGOPR, OPLOG_BOR	 },
{ "XOR",   kwTYPE_LOGOPR, '~'        },
{ "NOT",   kwTYPE_UNROPR, '!'        },
{ "уп╪коипо",   kwTYPE_MULOPR, OPLOG_MOD	 },
{ "MDL",   kwTYPE_MULOPR, OPLOG_MDL	 },
{ "EQV",   kwTYPE_LOGOPR, OPLOG_EQV	 },
{ "IMP",   kwTYPE_LOGOPR, OPLOG_IMP	 },
{ "NAND",  kwTYPE_LOGOPR, OPLOG_NAND },
{ "NOR",   kwTYPE_LOGOPR, OPLOG_NOR	 },
{ "XNOR",  kwTYPE_LOGOPR, OPLOG_NOR	 },
{ "ам╧йеи",    kwTYPE_CMPOPR, OPLOG_IN	 },
{ "LIKE",  kwTYPE_CMPOPR, OPLOG_LIKE },

{ "", 0, 0 }
};

/* ----------------------------------------------------------------------------------------------------------------------- */

/*
*	SPECIAL SEPERATORS
*
*	This keywords are used on commands but are not commands nor operators
*
*	example:
*	print USING ...
*	for f IN x
*	open x FOR INPUT ...
*/
struct spopr_keyword_s spopr_table[] = {
{ "левя©ла",		kwCOLOR 	},
{ "цел╤то",		kwFILLED	},
{ "FOR",		kwFORSEP	},
{ "INPUT",		kwINPUTSEP	},
{ "OUTPUT",		kwOUTPUTSEP	},
{ "APPEND",		kwAPPENDSEP	},
{ "ACCESS",		kwACCESS	},
{ "USING",		kwUSING		},
{ "USG",		kwUSING		},
{ "SHARED",		kwSHARED	},
{ "AS",			kwAS		},
{ "╦ыс",		kwTO		},
{ "DO",			kwDO		},
{ "STEP",		kwSTEP		},
{ "т╪те",		kwTHEN		},
{ "SUB", 		kwPROCSEP	},
{ "FUNC", 		kwFUNCSEP	},
{ "DEF", 		kwFUNCSEP	},
{ "LOOP", 		kwLOOPSEP	},
{ "ON",			kwON		},
{ "OFF",		kwOFF		},
{ "USE",		kwUSE		},
{ "BG",			kwBACKG 	},

{ "", 0 }
};

/* ----------------------------------------------------------------------------------------------------------------------- */

/*
*	BUILDIN-FUNCTIONS
*/
struct func_keyword_s func_table[] = {
///1234567890123456
{ "ASC",			kwASC },
{ "аяихл",			kwVAL },
{ "CHR",			kwCHR },
{ "йе╨л",			kwSTR },
{ "OCT",			kwOCT },
{ "HEX",			kwHEX },
{ "LCASE",			kwLCASE },
{ "LOWER",			kwLCASE },
{ "UCASE",			kwUCASE },
{ "UPPER",			kwUCASE },
{ "LTRIM",			kwLTRIM },
{ "RTRIM",			kwRTRIM },
{ "SPACE",			kwSPACE }, 
{ "SPC",			kwSPACE }, 
{ "TAB",			kwTAB },
{ "CAT",			kwCAT },
{ "ENVIRON",		kwENVIRONF },
{ "ENV",			kwENVIRONF },
{ "TRIM",			kwTRIM },
{ "STRING",			kwSTRING },
{ "SQUEEZE",		kwSQUEEZE },
{ "LEFT",			kwLEFT },
{ "RIGHT",			kwRIGHT },
{ "LEFTOF",			kwLEFTOF },
{ "RIGHTOF",		kwRIGHTOF },
{ "LEFTOFLAST",		kwLEFTOFLAST },
{ "RIGHTOFLAST",	kwRIGHTOFLAST },
{ "MID",			kwMID },
{ "REPLACE",		kwREPLACE },
{ "RUN",			kwRUNF },
{ "INKEY",			kwINKEY },
{ "TIME",			kwTIME },
{ "DATE",			kwDATE },
{ "INSTR",			kwINSTR },
{ "RINSTR",			kwINSTR },
{ "LBOUND",			kwLBOUND },
{ "UBOUND",			kwUBOUND },
{ "LEN",			kwLEN },
{ "EMPTY",			kwEMPTY },
{ "ISARRAY",		kwISARRAY },
{ "ISNUMBER",		kwISNUMBER },
{ "ISSTRING",		kwISSTRING },
{ "ATAN2",			kwATAN2 },
{ "POW",			kwPOW },
{ "ROUND",			kwROUND },
{ "COS",			kwCOS },
{ "SIN",			kwSIN },
{ "TAN",			kwTAN },
{ "COSH",			kwCOSH },
{ "SINH",			kwSINH },
{ "TANH",			kwTANH },
{ "ACOS",			kwACOS },
{ "ASIN",			kwASIN },
{ "ATAN",			kwATAN },
{ "ATN",			kwATAN },
{ "ACOSH",			kwACOSH },
{ "ASINH",	   		kwASINH },
{ "ATANH",			kwATANH },

{ "SEC",			kwSEC },
{ "ASEC",			kwASEC },
{ "SECH",			kwSECH },
{ "ASECH",			kwASECH },

{ "CSC",			kwCSC },
{ "ACSC",			kwACSC },
{ "CSCH",			kwCSCH },
{ "ACSCH",			kwACSCH },

{ "COT",			kwCOT },
{ "ACOT",			kwACOT },
{ "COTH",			kwCOTH },
{ "ACOTH",			kwACOTH },

{ "SQR",	   		kwSQR },
{ "ABS",			kwABS },
{ "EXP",			kwEXP },
{ "LOG",			kwLOG },
{ "LOG10",			kwLOG10 },
{ "FIX",			kwFIX },
{ "ай╦яаиос",			kwINT },
{ "CDBL",			kwCDBL },
{ "CREAL",			kwCDBL },
{ "DEG",			kwDEG },
{ "RAD",			kwRAD },
{ "PEN",			kwPENF },
{ "FLOOR",			kwFLOOR },
{ "CEIL",			kwCEIL },
{ "FRAC",			kwFRAC },
{ "FRE",			kwFRE },
{ "SGN",			kwSGN },
{ "CINT",			kwCINT },
{ "EOF",			kwEOF },
{ "SEEK",			kwSEEKF },
{ "LOF",			kwLOF },
{ "тува╨ос",			kwRND },
{ "MAX",			kwMAX },
{ "MIN",			kwMIN },
{ "ABSMAX",			kwABSMAX },
{ "ABSMIN",			kwABSMIN },
{ "SUM",			kwSUM },
{ "SUMSQ",			kwSUMSV },
{ "STATMEAN",		kwSTATMEAN },
{ "STATMEANDEV",	kwSTATMEANDEV },
{ "STATSPREADS",	kwSTATSPREADS },
{ "STATSPREADP",	kwSTATSPREADP },
{ "SEGCOS",			kwSEGCOS },
{ "SEGSIN",			kwSEGSIN },
{ "SEGLEN",			kwSEGLEN },
{ "POLYAREA",		kwPOLYAREA },
{ "POLYCENT",		kwPOLYCENT },
{ "PTDISTSEG",		kwPTDISTSEG },
{ "PTSIGN",	   		kwPTSIGN },
{ "PTDISTLN",		kwPTDISTLN },
{ "POINT",			kwPOINT },
{ "XPOS",			kwXPOS },
{ "YPOS",			kwYPOS },
{ "INPUT",			kwINPUTF },
{ "ARRAY",			kwCODEARRAY }, 
{ "LINEQN",			kwGAUSSJORDAN },
{ "FILES",			kwFILES },
{ "INVERSE",		kwINVERSE },
{ "DETERM",			kwDETERM },
{ "JULIAN",			kwJULIAN },
{ "DATEFMT",		kwDATEFMT },
{ "WEEKDAY",		kwWDAY },
{ "IF",				kwIFF },
{ "IFF",			kwIFF },
{ "FORMAT",			kwFORMAT },
{ "FREEFILE",		kwFREEFILE },
{ "TICKS",			kwTICKS },
{ "TICKSPERSEC",	kwTICKSPERSEC },
{ "TIMER", 			kwTIMER }, 
{ "PROGLINE",		kwPROGLINE },
{ "RUN",			kwRUNF	},
{ "пк╤тосй",		kwTEXTWIDTH },
{ "╬ьосй",			kwTEXTHEIGHT },
{ "EXIST",			kwEXIST },
{ "ISFILE",			kwISFILE },
{ "ISDIR",			kwISDIR },
{ "ISLINK",			kwISLINK },
{ "ACCESS",			kwACCESSF },
{ "RGB",			kwRGB },
{ "RGBF",			kwRGBF },
{ "BIN",			kwBIN },
{ "ENCLOSE",		kwENCLOSE },
{ "DISCLOSE",		kwDISCLOSE },
{ "TRANSLATE",		kwTRANSLATEF },
{ "CHOP",			kwCHOP },
{ "BGETC",			kwBGETC },
{ "BALLOC",			kwBALLOC },
{ "MALLOC",			kwBALLOC },
{ "PEEK32",			kwPEEK32 },
{ "PEEK16",			kwPEEK16 },
{ "PEEK",			kwPEEK },
{ "VADR",			kwVADDR },

{ "SEQ",			kwSEQ },

{ "CBS",			kwCBS },
{ "BCS",			kwBCS },

{ "LOADLIB",		kwLOADLIB },
{ "CALL",			kwCALLCF },

{ "IMAGEW",			kwIMGW },
{ "IMAGEH",			kwIMGH },

{ "", 0 }
};

/* ----------------------------------------------------------------------------------------------------------------------- */

/*
*	BUILD-IN PROCEDURES
*/
struct proc_keyword_s proc_table[] = {
///1234567890123456
{ "CLS",		kwCLS	},
{ "RTE",		kwRTE	},
//kwSHELL,
{ "ENVIRON",	kwENVIRON },
{ "ENV",		kwENVIRON },
{ "LOCATE",		kwLOCATE	},
{ "AT",			kwAT		},
{ "PEN",		kwPEN	},
{ "DATEDMY",	kwDATEDMY },
{ "BEEP",		kwBEEP	},
{ "SOUND",		kwSOUND	},
{ "NOSOUND",	kwNOSOUND	},
{ "PSET",		kwPSET	},
{ "RECT",		kwRECT	},
{ "CIRCLE",		kwCIRCLE	},
{ "RANDOMIZE",	kwRANDOMIZE	},
{ "SPLIT",		kwSPLIT },
{ "WSPLIT",		kwWSPLIT },
{ "JOIN",		kwWJOIN	},
{ "PAUSE",		kwPAUSE	},
{ "DELAY",		kwDELAY	},
{ "ARC",		kwARC	},
{ "DRAW",		kwDRAW	},
{ "PAINT",		kwPAINT	},
{ "PLAY",		kwPLAY	},
{ "SORT",		kwSORT	},
{ "SEARCH",		kwSEARCH },
{ "ROOT",		kwROOT },
{ "DIFFEQN",	kwDIFFEQ },
{ "CHART",		kwCHART	},
{ "WINDOW",		kwWINDOW },
{ "VIEW",		kwVIEW },
{ "DRAWPOLY",	kwDRAWPOLY },
{ "M3IDENT",	kwM3IDENT },
{ "M3ROTATE",	kwM3ROTATE },
{ "M3SCALE", 	kwM3SCALE },
{ "M3TRANS",	kwM3TRANSLATE },
{ "M3APPLY",	kwM3APPLY },
{ "INTERSECT",	kwSEGINTERSECT },
{ "POLYEXT",	kwPOLYEXT },
{ "DERIV",		kwDERIV },
{ "KILL", 		kwKILL	 },
{ "RENAME", 	kwRENAME	},
{ "COPY", 		kwCOPY	},
{ "CHDIR", 		kwCHDIR	},
{ "MKDIR", 		kwMKDIR	},
{ "RMDIR", 		kwRMDIR	},
{ "TLOAD",	 	kwLOADLN },
{ "TSAVE",		kwSAVELN },
{ "LOCK",		kwFLOCK },
{ "CHMOD",		kwCHMOD },
{ "PLOT2",		kwPLOT2 },
{ "PLOT",		kwPLOT },
{ "LOGPRINT",	kwLOGPRINT	},
{ "SWAP",		kwSWAP	},
{ "BUTTON",		kwBUTTON	},
{ "TEXT",		kwTEXT	},
{ "DOFORM",		kwDOFORM	},
{ "DIRWALK",	kwDIRWALK	},
{ "BPUTC",		kwBPUTC	},
{ "POKE32",		kwPOKE32 },
{ "POKE16",		kwPOKE16 },
{ "POKE",		kwPOKE },
{ "BCOPY",		kwBCOPY },
{ "BLOAD",		kwBLOAD },
{ "BSAVE",		kwBSAVE },
{ "IMGGET",		kwIMGGET },
{ "IMGPUT",		kwIMGPUT },
{ "TIMEHMS",	kwTIMEHMS },
{ "EXPRSEQ",	kwEXPRSEQ },
{ "UNLOADLIB",	kwUNLOADLIB },
{ "CALL",		kwCALLCP },
{ "HTML",		kwHTML },
{ "IMAGE",		kwIMAGE },

#if !defined(OS_LIMITED)
{ "STKDUMP",	kwSTKDUMP	},
#endif

{ "", 0 }
};

/*
*	in some cases (preprocessor) there is needed the texts 
*	(single-line IFs, option keyword, include, fast-cut of 'rem's. '@' -> byref, procedures declarations, etc)
*
*	_WS = With spaces (one left and one right)
*	_WRS = With one space at right
*/
#define LCN_PRINT		"т╬пысе"
#define LCN_REM			"сгл"
#define LCN_THEN_WS		" т╪те "
#define LCN_GOTO_WRS	"п╧цаиме "
#define LCN_GOTO_WS		" п╧цаиме "
#define LCN_ELSE		"акки©с"
#define LCN_UICS_WRS	"UICS "
#define LCN_CHARS		"CHARS"
#define LCN_PIXELS		"PIXELS"
#define LCN_BASE_WRS	"BASE "
#define LCN_PCRE_CASELESS "MATCH PCRE CASELESS"
#define LCN_PCRE		"MATCH PCRE"
#define LCN_SIMPLE		"MATCH SIMPLE"
#define LCN_PREDEF_WRS	"PREDEF "
#define LCN_IMPORT_WRS	"IMPORT "
#define LCN_UNIT_WRS	"UNIT "
#define LCN_BYREF_WRS	"BYREF "
#define LCN_END			"т╦кос"
#define LCN_ENDIF		"т╦косам"
#define LCN_GOSUB_WS	" GOSUB "
#define LCN_DO_WS		" DO "
#define LCN_NEXT		"еп╪лемо"
#define LCN_IN_WS		" IN "
#define LCN_WEND		"теж"
#define LCN_IF			"ам"
#define LCN_SELECT		"SELECT"
#define LCN_CASE		"CASE"
#define LCN_INPUT_WRS	"ф╧тгсе "
#define LCN_OPTION		"OPTION"
#define LCN_PREDEF		"PREDEF"
#define LCN_QUIET		"QUIET"
#define LCN_GRMODE		"GRMODE"
#define LCN_TEXTMODE	"TEXTMODE"
#define LCN_CSTR		"CSTR"
#define LCN_UNIT_PATH	"#UNIT-PATH:"
#define LCN_COMMAND		"COMMAND"
#define LCN_INC			"#INC:"
#define	LCN_SUB_WRS		"диадийас╨а "
#define	LCN_FUNC_WRS	"сум╤ятгсг "
#define	LCN_DEF_WRS		"DEF "
#define LCN_END_WRS		"т╦кос "
#define LCN_END_WNL		"т╦кос\n"

/* system variables */
#define LCN_SV_OSVER	"OSVER"
#define LCN_SV_OSNAME	"OSNAME"
#define LCN_SV_SBVER	"SBVER"
#define LCN_SV_PI		"п"
#define LCN_SV_XMAX		"л╦цистов"
#define LCN_SV_YMAX		"л╦цистоь"
#define LCN_SV_BPP		"BPP"	// Bits Per Pixel
#define LCN_SV_TRUE		"TRUE"
#define LCN_SV_FALSE	"FALSE"
#define LCN_SV_LINECHART	"LINECHART"
#define LCN_SV_BARCHART	"BARCHART"
#define LCN_SV_CWD		"CWD"	// Current Working Directory
#define LCN_SV_HOME		"HOME"	// home directory (user's personal directory)
#define LCN_SV_COMMAND	"COMMAND"

// x,y,z (USE keyword parameters)
#define LCN_SV_X		"X"
#define LCN_SV_Y		"Y"
#define LCN_SV_Z		"Z"

#define LCN_SV_VADR		"VIDADR" // video-ram address

// fast cut of comments (pp)
#define LCN_REM_1		":СГЛ "
#define LCN_REM_2		":СГЛ\t"
#define LCN_REM_3		"СГЛ "
#define LCN_REM_4		"СГЛ\n"

#define SYS_MAIN_SECTION_NAME	"йЕМТЯИЙч"
