//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl.h		-	The clone configuration language headerfile. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

#ifndef __CCL_H__
#define __CCL_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifdef USE_GUILE
#  include <guile/gh.h>
#  define get_c_string(lisp)     CclConvertToString(lisp)
#  define try_get_c_string(lisp) CclConvertToString(lisp)
#  define symbol_value(x, env)   scm_variable_ref(scm_lookup(x))
#  define NIL                    SCM_EOL
#  define cons(a, b)             gh_cons(a, b)
#  define symbol_boundp(x, env)  (!SCM_UNBNDP(x))
#  define fast_load(s_filename, bogus)  scm_primitive_load(s_filename)
#  define cons_array              gh_make_vector
#  define gh_eval(expr, env)      scm_primitive_eval(expr)
#  define setvar(sym, value, env) scm_define(sym,value)
#  define vload(buf,cflag,rflag)  gh_load(buf)
#  define errl(message, value)    { fputs(message, stdout); gh_display(value); putchar('\n'); }
// FIXME : support for guile lprin1CL needed
#  define lprin1CL(var, file)
#  define gh_new_procedureN(name, proc) gh_new_procedure(name, proc, 0, 0, 1)
#  define aset1(array, pos, value)      gh_vector_set_x(array, pos, value)
#  define repl_c_string(msg, a, b, c  ) gh_eval_str(msg)
#  define print_welcome()         
#  define user_gc(a)              scm_gc()
#  define gh_scm2newstr(scm, lenp) \
  (gh_symbol_p(scm) ? gh_symbol2newstr(scm, lenp) : gh_scm2newstr(scm,lenp))
#  define gh_scm2int(val) \
  (gh_inexact_p(val) ? (int)gh_scm2double(val) : gh_scm2int(val))
#  define gh_scm2long(val) \
  (gh_inexact_p(val) ? (long)gh_scm2double(val) : gh_scm2long(val))

extern int siod_verbose_level;
struct gen_printio* f;
typedef scm_t_bits ccl_smob_type_t;

#else
#  include <string.h>
#  include "siod.h"
#  include "siodp.h"

extern LISP fast_load(LISP lfname,LISP noeval);
/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

//
//	Macros for compatibility with guile high level interface.
//

#define SCM LISP
#define SCM_UNSPECIFIED NIL
#define gh_null_p(lisp) NULLP(lisp)

#define gh_eq_p(lisp1,lisp2)	EQ(lisp1,lisp2)

#define gh_list_p(lisp)		CONSP(lisp)
#define gh_car(lisp)		car(lisp)
#define gh_cdr(lisp)		cdr(lisp)
#define gh_caar(lisp)		caar(lisp)
#define gh_cadr(lisp)		cadr(lisp)
#define gh_cddr(lisp)		cddr(lisp)
#define gh_length(lisp)		nlength(lisp)

#define gh_set_car_x(pair, val) setcar(pair, val)
#define gh_set_cdr_x(pair, val) setcdr(pair, val)

#define gh_exact_p(lisp)	TYPEP(lisp,tc_flonum)
#define gh_scm2int(lisp)	(long)FLONM(lisp)
#define gh_scm2long(lisp)	(long)FLONM(lisp)
#define gh_int2scm(num)		flocons(num)

#define gh_string_p(lisp)	TYPEP(lisp,tc_string)
#define	gh_scm2newstr(lisp,str) strdup(get_c_string(lisp))
#define	gh_str02scm(str) strcons(strlen(str),str)

#define	gh_vector_p(lisp)	\
	(TYPE(lisp)>=tc_string && TYPE(lisp)<=tc_byte_array)
#define	gh_vector_length(lisp)	nlength(lisp)
#define	gh_vector_ref(lisp,n)	aref1(lisp,n)

#define gh_boolean_p(lisp)	(EQ(lisp,sym_t) || NULLP(lisp))
#define gh_scm2bool(lisp)	(NNULLP(lisp))
#define gh_bool2scm(n)		((n) ? SCM_BOOL_T : SCM_BOOL_F)

#define gh_symbol_p(lisp)	SYMBOLP(lisp)
#define gh_symbol2scm(str)	cintern(str)

#define gh_define(str,val)	setvar(rintern((str)),(val),NIL)

#define gh_display(lisp)	lprin1f(lisp,stdout)
#define gh_newline()		fprintf(stdout,"\n")

#define gh_load(str)	        vload(str,0,0)

#define gh_apply(proc,args)	lapply(proc,args)
#define gh_eval(proc,env)	leval(proc,env)

#define gh_new_procedure0_0	init_subr_0
#define gh_new_procedure1_0	init_subr_1
#define gh_new_procedure2_0	init_subr_2
#define gh_new_procedure3_0	init_subr_3
#define gh_new_procedure4_0	init_subr_4
#define gh_new_procedure5_0	init_subr_5
#define gh_new_procedureN	init_lsubr

#define SCM_BOOL_T	sym_t
#define SCM_BOOL_F	NIL

#define gh_vector_set_x(array, pos, value) aset1(array, pos, value) 

extern LISP sym_t;
typedef long ccl_smob_type_t;

#endif // !USE_GUILE

#include "iolib.h"

//extern SCM CclEachSecond;		/// Scheme function called each second

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char* CclStartFile;		/// CCL start file
extern int CclInConfigFile;		/// True while config file parsing

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern char*           CclConvertToString(SCM scm);
extern ccl_smob_type_t CclMakeSmobType(const char* name);
extern SCM             CclMakeSmobObj(ccl_smob_type_t tag, void* ptr);
extern void*           CclGetSmobData(SCM smob);
extern ccl_smob_type_t CclGetSmobType(SCM smob);

extern void CclGcProtect(SCM obj);	/// Protect scm object for GC
extern void CclGcUnprotect(SCM obj);	/// Unprotect scm object for GC
extern void CclFlushOutput();		/// Flush ccl output
extern void InitCcl(void);		/// Initialise ccl
extern void LoadCcl(void);		/// Load ccl config file
extern void SaveCcl(CLFile* file);	/// Save CCL module
extern void SavePreferences(void);	/// Save user preferences
extern void CclCommand(const char*);	/// Execute a ccl command
extern void CclFree(void*);		/// Save free
extern void CleanCclCredits();		/// Free Ccl Credits Memory

//@}

#endif	// !__CCL_H__
