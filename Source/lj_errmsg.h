/*
** VM error messages.
** Copyright (C) 2005-2014 Mike Pall. See Copyright Notice in luajit.h
*/

/* This file may be included multiple times with different ERRDEF macros. */
/* This file has been modified to conform with Garry's Mod strings. */

#define LUA_QL( s ) "'" s "'"
#define LUA_QS LUA_QL( "%s" )

/* Basic error handling. */
ERRDEF(ERRMEM,	"not enough memory")
ERRDEF(ERRERR,	"error in error handling")
ERRDEF(ERRCPP,	"C++ exception")

/* Allocations. */
ERRDEF(STROV,	"string length overflow")
ERRDEF(UDATAOV,	"userdata length overflow")
ERRDEF(STKOV,	"stack overflow")
ERRDEF(STKOVM,	"stack overflow (%s)")
ERRDEF(TABOV,	"table overflow")

/* Table indexing. */
ERRDEF(NANIDX,	"table index is NaN")
ERRDEF(NILIDX,	"table index is nil")
ERRDEF(NEXTIDX,	"invalid key to " LUA_QL("next"))

/* Metamethod resolving. */
ERRDEF(BADCALL,	"attempt to call a %s value")
ERRDEF(BADOPRT,	"attempt to %s %s " LUA_QS " (a %s value)")
ERRDEF(BADOPRV,	"attempt to %s a %s value")
ERRDEF(BADCMPT,	"attempt to compare %s with %s")
ERRDEF(BADCMPV,	"attempt to compare two %s values")
ERRDEF(GETLOOP,	"loop in gettable")
ERRDEF(SETLOOP,	"loop in settable")
ERRDEF(OPCALL,	"call")
ERRDEF(OPINDEX,	"index")
ERRDEF(OPARITH,	"perform arithmetic on")
ERRDEF(OPCAT,	"concatenate")
ERRDEF(OPLEN,	"get length of")

/* Type checks. */
ERRDEF(BADSELF,	"calling " LUA_QS " on bad self (%s)")
ERRDEF(BADARG,	"bad argument #%d to " LUA_QS " (%s)")
ERRDEF(BADTYPE,	"%s expected, got %s")
ERRDEF(BADVAL,	"invalid value")
ERRDEF(NOVAL,	"value expected")
ERRDEF(NOCORO,	"coroutine expected")
ERRDEF(NOTABN,	"nil or table expected")
ERRDEF(NOLFUNC,	"Lua function expected")
ERRDEF(NOFUNCL,	"function or level expected")
ERRDEF(NOSFT,	"string/function/table expected")
ERRDEF(NOPROXY,	"boolean or proxy expected")
ERRDEF(FORINIT,	LUA_QL("for") " initial value must be a number")
ERRDEF(FORLIM,	LUA_QL("for") " limit must be a number")
ERRDEF(FORSTEP,	LUA_QL("for") " step must be a number")

/* C API checks. */
ERRDEF(NOENV,	"no calling environment")
ERRDEF(CYIELD,	"attempt to yield across C-call boundary")
ERRDEF(BADLU,	"bad light userdata pointer")
ERRDEF(NOGCMM,	"bad action while in __gc metamethod")
#ifdef _WIN32
ERRDEF(BADFPU,	"bad FPU precision (use D3DCREATE_FPU_PRESERVE with DirectX)")
#endif

/* Standard library function errors. */
ERRDEF(ASSERT,	"assertion failed!")
ERRDEF(PROTMT,	"cannot change a protected metatable")
ERRDEF(UNPACK,	"too many results to unpack")
ERRDEF(RDRSTR,	"reader function must return a string")
ERRDEF(PRTOSTR,	LUA_QL("tostring") " must return a string to " LUA_QL("print"))
ERRDEF(IDXRNG,	"index out of range")
ERRDEF(BASERNG,	"base out of range")
ERRDEF(LVLRNG,	"level out of range")
ERRDEF(INVLVL,	"invalid level")
ERRDEF(INVOPT,	"invalid option")
ERRDEF(INVOPTM,	"invalid option " LUA_QS)
ERRDEF(INVFMT,	"invalid format")
ERRDEF(SETFENV,	LUA_QL("setfenv") " cannot change environment of given object")
ERRDEF(CORUN,	"cannot resume running coroutine")
ERRDEF(CODEAD,	"cannot resume dead coroutine")
ERRDEF(COSUSP,	"cannot resume non-suspended coroutine")
ERRDEF(TABINS,	"wrong number of arguments to " LUA_QL("insert"))
ERRDEF(TABCAT,	"invalid value (%s) at index %d in table for " LUA_QL("concat"))
ERRDEF(TABSORT,	"invalid order function for sorting")
ERRDEF(IOCLFL,	"attempt to use a closed file")
ERRDEF(IOSTDCL,	"standard file is closed")
ERRDEF(OSUNIQF,	"unable to generate a unique filename")
ERRDEF(OSDATEF,	"field " LUA_QS " missing in date table")
ERRDEF(STRDUMP,	"unable to dump given function")
ERRDEF(STRSLC,	"string slice too long")
ERRDEF(STRPATB,	"missing " LUA_QL("[") " after " LUA_QL("%f") " in pattern")
ERRDEF(STRPATC,	"invalid pattern capture")
ERRDEF(STRPATE,	"malformed pattern (ends with " LUA_QL("%") ")")
ERRDEF(STRPATM,	"malformed pattern (missing " LUA_QL("]") ")")
ERRDEF(STRPATU,	"unbalanced pattern")
ERRDEF(STRPATX,	"pattern too complex")
ERRDEF(STRCAPI,	"invalid capture index")
ERRDEF(STRCAPN,	"too many captures")
ERRDEF(STRCAPU,	"unfinished capture")
ERRDEF(STRFMTO,	"invalid option " LUA_QL("%%%c") " to " LUA_QL("format"))
ERRDEF(STRFMTR,	"invalid format (repeated flags)")
ERRDEF(STRFMTW,	"invalid format (width or precision too long)")
ERRDEF(STRGSRV,	"invalid replacement value (a %s)")
ERRDEF(BADMODN,	"name conflict for module " LUA_QS)
ERRDEF(NOJIT,	"JIT compiler disabled, CPU does not support SSE2")
ERRDEF(JITOPT,	"unknown or malformed optimization flag " LUA_QS)

/* Lexer/parser errors. */
ERRDEF(XMODE,	"attempt to load chunk with wrong mode")
ERRDEF(XNEAR,	"%s near " LUA_QS)
ERRDEF(XELEM,	"lexical element too long")
ERRDEF(XLINES,	"chunk has too many lines")
ERRDEF(XLEVELS,	"chunk has too many syntax levels")
ERRDEF(XNUMBER,	"malformed number")
ERRDEF(XLSTR,	"unfinished long string")
ERRDEF(XLCOM,	"unfinished long comment")
ERRDEF(XSTR,	"unfinished string")
ERRDEF(XESC,	"invalid escape sequence")
ERRDEF(XLDELIM,	"invalid long string delimiter")
ERRDEF(XTOKEN,	LUA_QS " expected")
ERRDEF(XJUMP,	"control structure too long")
ERRDEF(XSLOTS,	"function or expression too complex")
ERRDEF(XLIMC,	"chunk has more than %d local variables")
ERRDEF(XLIMM,	"main function has more than %d %s")
ERRDEF(XLIMF,	"function at line %d has more than %d %s")
ERRDEF(XMATCH,	LUA_QS " expected (to close " LUA_QS " at line %d)")
ERRDEF(XFIXUP,	"function too long for return fixup")
ERRDEF(XPARAM,	"<name> or " LUA_QL("...") " expected")
ERRDEF(XAMBIG,	"ambiguous syntax (function call x new statement)")
ERRDEF(XFUNARG,	"function arguments expected")
ERRDEF(XSYMBOL,	"unexpected symbol")
ERRDEF(XDOTS,	"cannot use " LUA_QL("...") " outside a vararg function")
ERRDEF(XSYNTAX,	"syntax error")
ERRDEF(XFOR,	LUA_QL("=") " or " LUA_QL("in") " expected")
ERRDEF(XBREAK,	"no loop to break")
ERRDEF(XLUNDEF,	"undefined label " LUA_QS)
ERRDEF(XLDUP,	"duplicate label " LUA_QS)
ERRDEF(XGSCOPE,	"<goto %s> jumps into the scope of local " LUA_QS)

/* Bytecode reader errors. */
ERRDEF(BCFMT,	"cannot load incompatible bytecode")
ERRDEF(BCBAD,	"cannot load malformed bytecode")

#undef ERRDEF

/* Detecting unused error messages:
   awk -F, '/^ERRDEF/ { gsub(/ERRDEF./, ""); printf "grep -q LJ_ERR_%s *.[ch] || echo %s\n", $1, $1}' lj_errmsg.h | sh
*/