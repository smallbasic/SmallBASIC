// This file is part of SmallBASIC
//
//  Generic keywords  : BC size = 1 byte = 255-2 names
//  Buildin functions : BC size = 2 (4 on Unix) bytes = 32K-~1K names
//  Buildin procedures: BC size = 2 (4 on Unix) bytes = 32K-~1K names
//  I had changed the starting values for debuging purposes
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sb_kw_h)
#define _sb_kw_h

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *       operators
 */
#define OPLOG_GT        '>'
#define OPLOG_LT        '<'
#define OPLOG_EQ        '='
#define OPLOG_GE        11      // >=
#define OPLOG_LE        12      // <=
#define OPLOG_NE        14      // <>
#define OPLOG_AND       '&'     // AND
#define OPLOG_OR        '|'     // OR
#define OPLOG_NOT       '!'     // NOT
#define OPLOG_XOR       '~'     // XOR
#define OPLOG_BAND      'A'     // AND (bit)
#define OPLOG_BOR       'B'     // OR (bit)
#define OPLOG_INV       'C'     // Invert (bit)
#define OPLOG_EQV       'D'     // EQV (bit)
#define OPLOG_IMP       'E'     // IMP (bit)
#define OPLOG_NAND      'F'     // NAND (bit)
#define OPLOG_NOR       'G'     // NOR (bit)
#define OPLOG_XNOR      'H'     // XNOR (bit)
#define OPLOG_IN        'I'     // IN (list)
#define OPLOG_MOD       'M'     // MOD (remain)
#define OPLOG_MDL       'L'     // MDL (modulus)
#define OPLOG_LIKE      'W'     // LIKE wc
#define OPLOG_LSHIFT    'X'     // LSHIFT
#define OPLOG_RSHIFT    'Y'     // RSHIFT

/**
 * @ingroup sys
 * @enum keyword
 *
 * Generic keyword codes
 *
 * special commands like PRINT, INPUT, LINE (ex: LINE INPUT)
 * special seperators too like INPUT, APPEND, FORSEP
 * and, of course, root commands like REPEAT-UNTIL, IF, etc
 */
enum keyword {                // line 50
  kwTYPE_INT = 0x1, /* 32b Integer */
  kwTYPE_NUM, /* 64b Real */
  kwTYPE_STR, /* String */
  kwTYPE_LOGOPR, /* Logical operator */
  kwTYPE_CMPOPR, /* Comparation operator */
  kwTYPE_ADDOPR, /* ADD/SUB operator */
  kwTYPE_MULOPR, /* MUL/DIV/IDIV operator */
  kwTYPE_POWOPR, /* POW(x,y) operator */
  kwTYPE_UNROPR, /* Unary operator */
  kwTYPE_VAR, /* Variable */
  kwTYPE_UDS_EL, /* Structure element */
  kwTYPE_SEP, /* Separator */
  kwTYPE_LINE, /* Debug info: SOURCE LINE */
  kwTYPE_LEVEL_BEGIN, /* Parenthesis ( */
  kwTYPE_LEVEL_END, /* Parenthesis ) */
  kwTYPE_EOC, /* End-Of-Command mark */
  kwTYPE_EVPUSH, /* PUSH R */
  kwTYPE_EVPOP, /* POP L */
  kwTYPE_EVAL_SC, /* Evalulation short-circuit begin */
  kwTYPE_CALLF, /* Call a build-in function */
  kwTYPE_CALLP, /* Call a build-in procedure */
  kwTYPE_CALL_UDF, /* Call user defined function */
  kwTYPE_CALL_UDP, /* Call user defined procedure */
  kwTYPE_CALL_PTR, /* Call user defined procedure or function from address pointer */
  kwTYPE_CALL_VFUNC, /* Call virtual function */
  kwTYPE_CALLEXTF, /* Call an external function */
  kwTYPE_CALLEXTP, /* Call an external procedure */
  kwTYPE_CRVAR, /* Create dynamic variable (PARAMETERS OR LOCALS) */
  kwTYPE_RET, /* Return from UDF|UDP */
  kwTYPE_PARAM, /* Parameters */
  kwTYPE_PTR, /* Address pointer, eg f=@foo */
  kwLOCAL, /* Create local variables */
  kwFUNC, /* USER DEFINED FUNCTION */
  kwPROC, /* USER DEFINED PROCEDURE */
  kwBYREF,
  kwDECLARE,
  kwIMPORT,
  kwEXPORT,
  kwUNIT,
  kwLET,
  kwCONST,
  kwEND,
  kwSTOP,
  kwPRINT,
  kwUSING,
  kwINPUT,
  kwSINPUT,
  kwINPUTSEP,
  kwLOOPSEP,
  kwPROCSEP,
  kwFUNCSEP,
  kwREM,
  kwLABEL,
  kwGOTO,
  kwIF,
  kwTHEN,
  kwELSE,
  kwELIF,
  kwENDIF,
  kwFOR,
  kwTO,
  kwSTEP,
  kwIN,
  kwNEXT,
  kwWHILE,
  kwWEND,
  kwREPEAT,
  kwUNTIL,
  kwGOSUB,
  kwRETURN,
  kwEXIT,
  kwLOOP,
  kwDIM,
  kwREDIM,
  kwCHAIN,
  kwREAD,
  kwRESTORE,
  kwDATA,
  kwCOLOR,
  kwFILLED,
  kwLINE,
  kwON,
  kwOFF,
  kwTRON,
  kwTROFF,
  kwONJMP,
  kwRUN,
  kwEXEC,
  kwERASE,
  kwUSE,
  kwFORSEP,
  kwOUTPUTSEP,
  kwAPPEND,
  kwINSERT,
  kwDELETE,
  kwAPPENDSEP,
  kwOPEN,
  kwAS,
  kwFILEPRINT,
  kwLINEINPUT,
  kwFILEINPUT,
  kwFILEWRITE,
  kwFILEREAD,
  kwCLOSE,
  kwSCRMODE,
  kwSEEK,
  kwTYPE,
  kwSPRINT,
  kwDO,
  kwOPTION,
  kwBACKG,
  kwLOGPRINT,
  kwSELECT,
  kwCASE,
  kwCASE_ELSE,
  kwENDSELECT,
  kwTRY,
  kwCATCH,
  kwENDTRY,
  kwNULL
};

/**
 * @ingroup sys
 * @enum proc_keywords
 *
 * buildin procedures - keyword codes
 */
enum proc_keywords {
  kwCLS = 0x100,              // 256 (generic keywords)
  kwSHELL,
  kwENVIRON,
  kwLOCATE,
  kwAT,
  kwPEN,
  kwDATEDMY,
  kwBEEP,
  kwSOUND,
  kwNOSOUND,
  kwPSET,
  kwRECT,
  kwCIRCLE,
  kwRANDOMIZE,
  kwSPLIT,
  kwWJOIN,
  kwPAUSE,
  kwDELAY,
  kwARC,
  kwDRAW,
  kwPAINT,
  kwPLAY,
  kwSORT,
  kwSEARCH,
  kwROOT,
  kwDIFFEQ,
  kwCHART,
  kwWINDOW,
  kwVIEW,
  kwDRAWPOLY,
  kwM3IDENT,
  kwM3ROTATE,
  kwM3SCALE,
  kwM3TRANSLATE,
  kwM3APPLY,
  kwSEGINTERSECT,
  kwPOLYEXT,
  kwDERIV,
  kwLOADLN,
  kwSAVELN,
  kwKILL,
  kwRENAME,
  kwCOPY,
  kwCHDIR,
  kwMKDIR,
  kwRMDIR,
  kwFLOCK,
  kwCHMOD,
  kwPLOT,
  kwSTKDUMP,
  kwSWAP,
  kwDIRWALK,
  kwBPUTC,
  kwBLOAD,
  kwBSAVE,
  kwTIMEHMS,
  kwEXPRSEQ,
  kwCALLCP,
  kwDEFINEKEY,
  kwSHOWPAGE,
  kwTHROW,
  kwNULLPROC
};

/**
 * @ingroup sys
 * @enum func_keywords
 *
 * buildin functions - keyword codes
 */
enum func_keywords {
  kwASC = 0x200,
  kwVAL,
  kwCHR,
  kwSTR,
  kwOCT,
  kwHEX,
  kwLCASE,
  kwUCASE,
  kwLTRIM,
  kwRTRIM,
  kwSPACE,
  kwTAB,
  kwCAT,
  kwENVIRONF,
  kwTRIM,
  kwSTRING,
  kwSQUEEZE,
  kwLEFT,
  kwRIGHT,
  kwLEFTOF,
  kwRIGHTOF,
  kwLEFTOFLAST,
  kwRIGHTOFLAST,
  kwMID,
  kwREPLACE,
  kwRUNF,
  kwINKEY,
  kwTIME,
  kwDATE,
  kwINSTR,
  kwRINSTR,
  kwLBOUND,
  kwUBOUND,
  kwLEN,
  kwEMPTY,
  kwISARRAY,
  kwISNUMBER,
  kwISSTRING,
  kwISMAP,
  kwISREF,
  kwATAN2,
  kwPOW,
  kwROUND,
  kwSIN,
  kwASIN,
  kwSINH,
  kwASINH,
  kwCOS,
  kwACOS,
  kwCOSH,
  kwACOSH,
  kwTAN,
  kwATAN,
  kwTANH,
  kwATANH,
  kwSEC,
  kwASEC,
  kwSECH,
  kwASECH,
  kwCSC,
  kwACSC,
  kwCSCH,
  kwACSCH,
  kwCOT,
  kwACOT,
  kwCOTH,
  kwACOTH,
  kwSQR,
  kwABS,
  kwEXP,
  kwLOG,
  kwLOG10,
  kwFIX,
  kwINT,
  kwCDBL,
  kwDEG,
  kwRAD,
  kwPENF,
  kwFLOOR,
  kwCEIL,
  kwFRAC,
  kwFRE,
  kwSGN,
  kwCINT,
  kwEOF,
  kwSEEKF,
  kwLOF,
  kwRND,
  kwMAX,
  kwMIN,
  kwABSMAX,
  kwABSMIN,
  kwSUM,
  kwSUMSV,
  kwSTATMEAN,
  kwSTATMEANDEV,
  kwSTATSPREADS,
  kwSTATSPREADP,
  kwSEGCOS,
  kwSEGSIN,
  kwSEGLEN,
  kwPOLYAREA,
  kwPOLYCENT,
  kwPTDISTSEG,
  kwPTSIGN,
  kwPTDISTLN,
  kwPOINT,
  kwCODEARRAY,
  kwGAUSSJORDAN,
  kwFILES,
  kwINVERSE,
  kwDETERM,
  kwJULIAN,
  kwDATEFMT,
  kwWDAY,
  kwIFF,
  kwFORMAT,
  kwFREEFILE,
  kwTICKS,
  kwTIMER,
  kwPROGLINE,
  kwINPUTF,
  kwTEXTWIDTH,
  kwTEXTHEIGHT,
  kwEXIST,
  kwISFILE,
  kwISDIR,
  kwISLINK,
  kwACCESSF,
  kwXPOS,
  kwYPOS,
  kwRGB,
  kwRGBF,
  kwBIN,
  kwENCLOSE,
  kwDISCLOSE,
  kwSEARCHF,
  kwTRANSLATEF,
  kwCHOP,
  kwBGETC,
  kwSEQ,
  kwCBS,
  kwBCS,
  kwCALLCF,
  kwARRAY,
  kwIMAGE,
  kwFORM,
  kwTIMESTAMP,
  kwNULLFUNC
};

/**
 * @ingroup sys
 *
 * checks if the 'code' belongs to the codes that contained in table
 *
 * @param code the code to search for
 * @param table the table of codes to scan
 * @return non-zero on success
 */
int kw_check(code_t *table, code_t code);

/**
 * @ingroup sys
 *
 * returns true if the 'code' is valid code for end-of-expression
 *
 *       @return non-zero if the 'code' is valid code for end-of-expression
 */
int kw_check_evexit(code_t code);

/**
 * @ingroup sys
 *
 * returns the name which is assigned to code
 *
 * @param code is the code
 * @param dest is the buffer to store the keyword
 * @return non-zero on success
 */
int kw_getcmdname(code_t code, char *dest);

/**
 * @ingroup sys
 *
 * returns the name which is assigned to build-in function code
 *
 * @param code is the code
 * @param dest is the buffer to store the keyword
 * @return non-zero on success
 */
int kw_getfuncname(fcode_t code, char *dest);

/**
 * @ingroup sys
 *
 * returns the name which is assigned to build-in procedure code
 *
 * @param code is the code
 * @param dest is the buffer to store the keyword
 * @return non-zero on success
 */
int kw_getprocname(pcode_t code, char *dest);

/**
 * @ingroup sys
 *
 * returns true if the code is a function without parameters
 *
 * @param code is the code
 * @return non-zero if 'code' is a function that does not requires parameters
 */
int kw_noarg_func(fcode_t code);

/**
 */
int kw_iscommand(const char *name);

/**
 */
int kw_isproc(const char *name);

/**
 */
void prcmd(code_t code);

/**
 */
void prfunc(long code);

#define OPTION_BASE                     1
#define OPTION_UICS                     2
#define OPTION_GRMODE                   3
#define OPTION_MATCH                    4

#define OPTION_UICS_PIXELS              0
#define OPTION_UICS_CHARS               1

#if defined(__cplusplus)
}
#endif
#endif
