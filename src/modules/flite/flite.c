// -*- c-file-style: "java" -*-
// $Id: flite.c,v 1.1 2004-07-01 23:50:02 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include <extlib.h>
#include <stdio.h>

#include "flite.h"
#include "flite_version.h"
#include "voxdefs.h"

cst_voice *v;
cst_voice *REGISTER_VOX(const char *voxdir);

void speak(const char* dialog) {
    flite_text_to_speech(dialog, v, "play");
}

// prints a var_t
void speak_var_t(int level, var_t *param) {
    int   i;
    var_t *element_p;
    char text[1024];

    // print var_t
    switch (param->type)  {
    case  V_STR:
        sprintf(text, "%s", param->v.p.ptr);
        speak(text);
        break;
    case  V_INT:
        sprintf(text, "%ld", param->v.i);
        speak(text);
        break;
    case  V_REAL:
        sprintf(text, "%.2f",param->v.n);
        speak(text);
        break;
    case  V_ARRAY:
        //dev_printf("ARRAY of %d 'var_t' elements\n", param->v.a.size);
        for ( i = 0; i < param->v.a.size; i ++ )  {
            element_p = (var_t *) (param->v.a.ptr + sizeof(var_t) * i);
            speak_var_t(level+1, element_p);
        }
        break;
    }
}

// this procedure justs speeks all of its parameters
static int doProcedure(int param_count, slib_par_t *params, var_t *retval) {
    int   i;
    var_t *param;

    for ( i = 0; i < param_count; i ++ )  {
        param = params[i].var_p;
        speak_var_t(0, param);
    }
    return 1; // success
}

///// SmallBASIC interface /////////////////////////////////////////////////////

// returns the number of the procedures
int sblib_proc_count(void) {
    v = REGISTER_VOX(NULL);
    //feat_copy_into(extra_feats,v->features);
    //ef_set(extra_feats,argv[i+1],"string");
    // set flite_main.c for examples
    return 1;
}

// returns the supported procedure names
int sblib_proc_getname(int index, char *proc_name) {
    switch (index)  {
    case  0:
        strcpy(proc_name, "SPEAK");
        return 1; // success
    }
    return 0; // error
}

// execute procedures
int  sblib_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
    int success = 0;
    switch (index)  {
    case 0:
        success = doProcedure(param_count, params, retval);
        break;
    default:
        return 1;
    }
    return success;
}

// returns the number of the functions
int sblib_func_count(void) {
    return 0;
}

// returns the function names
int sblib_func_getname(int index, char *proc_name) {
    return 1; // success
}

// execute functions
int  sblib_func_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
    return 0;
}

