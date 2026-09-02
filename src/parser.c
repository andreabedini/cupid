/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 17 "parser.y"

#ifndef lint
#define lint
#endif
#define defTycon(n,l,lhs,rhs,w)	 tyconDefn(intOf(l),lhs,rhs,w); sp-=n
#define sigdecl(l,vs,t)		 ap(SIGDECL,triple(l,vs,t))
#define fixdecl(l,ops,a,p)	 ap(FIXDECL,\
				    triple(l,ops,mkInt(mkSyntax(a,intOf(p)))))
#define grded(gs)		 ap(GUARDED,gs)
#define bang(t)			 ap(BANG,t)
#define only(t)			 ap(ONLY,t)
#define letrec(bs,e)		 (nonNull(bs) ? ap(LETREC,pair(bs,e)) : e)
#define qualify(ps,t)		 (nonNull(ps) ? ap(QUAL,pair(ps,t)) : t)
#define exportSelf()		 singleton(ap(MODULEENT,mkCon(module(currentModule).text)))
#define yyerror(s)		 /* errors handled elsewhere */
#define YYSTYPE			 Cell

#ifdef YYBISON
# if !defined(__GNUC__) || __GNUC__ <= 1
static void __yy_memcpy (char*,char*, unsigned int);
# endif
#endif

#ifdef _MANAGED
static void yymemcpy (char *yyto, const char *yyfrom, size_t yycount);
#endif

static Cell   local gcShadow	 (Int,Cell);
static Void   local syntaxError	 (String);
static String local unexpected	 (Void);
static Cell   local checkPrec	 (Cell);
static Cell   local buildTuple	 (List);
static List   local checkCtxt	 (List);
static Cell   local checkPred	 (Cell);
static Pair   local checkDo	 (List);
static Cell   local desugarGuards (List);
static Cell   local guardMatch	 (Cell,List,Cell);
static Cell   local checkTyLhs	 (Cell);
static Cell   local checkConstr	 (Cell);



/* For the purposes of reasonably portable garbage collection, it is
 * necessary to simulate the YACC stack on the Hugs stack to keep
 * track of all intermediate constructs.  The lexical analyser
 * pushes a token onto the stack for each token that is found, with
 * these elements being removed as reduce actions are performed,
 * taking account of look-ahead tokens as described by gcShadow()
 * below.
 *
 * Of the non-terminals used below, only start, topDecl & begin
 * do not leave any values on the Hugs stack.  The same is true for the
 * terminals EXPR and SCRIPT.  At the end of a successful parse, there
 * should only be one element left on the stack, containing the result
 * of the parse.
 */

#define gc0(e)			gcShadow(0,e)
#define gc1(e)			gcShadow(1,e)
#define gc2(e)			gcShadow(2,e)
#define gc3(e)			gcShadow(3,e)
#define gc4(e)			gcShadow(4,e)
#define gc5(e)			gcShadow(5,e)
#define gc6(e)			gcShadow(6,e)
#define gc7(e)			gcShadow(7,e)


#line 139 "parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    EXPR = 258,                    /* EXPR  */
    CTXT = 259,                    /* CTXT  */
    SCRIPT = 260,                  /* SCRIPT  */
    CASEXP = 261,                  /* CASEXP  */
    OF = 262,                      /* OF  */
    DATA = 263,                    /* DATA  */
    TYPE = 264,                    /* TYPE  */
    IF = 265,                      /* IF  */
    THEN = 266,                    /* THEN  */
    ELSE = 267,                    /* ELSE  */
    WHERE = 268,                   /* WHERE  */
    LET = 269,                     /* LET  */
    IN = 270,                      /* IN  */
    INFIXN = 271,                  /* INFIXN  */
    INFIXL = 272,                  /* INFIXL  */
    INFIXR = 273,                  /* INFIXR  */
    PRIMITIVE = 274,               /* PRIMITIVE  */
    TNEWTYPE = 275,                /* TNEWTYPE  */
    DEFAULT = 276,                 /* DEFAULT  */
    DERIVING = 277,                /* DERIVING  */
    DO = 278,                      /* DO  */
    TCLASS = 279,                  /* TCLASS  */
    TINSTANCE = 280,               /* TINSTANCE  */
    REPEAT = 281,                  /* REPEAT  */
    ALL = 282,                     /* ALL  */
    NUMLIT = 283,                  /* NUMLIT  */
    CHARLIT = 284,                 /* CHARLIT  */
    STRINGLIT = 285,               /* STRINGLIT  */
    VAROP = 286,                   /* VAROP  */
    VARID = 287,                   /* VARID  */
    CONOP = 288,                   /* CONOP  */
    CONID = 289,                   /* CONID  */
    QVAROP = 290,                  /* QVAROP  */
    QVARID = 291,                  /* QVARID  */
    QCONOP = 292,                  /* QCONOP  */
    QCONID = 293,                  /* QCONID  */
    COCO = 294,                    /* COCO  */
    UPTO = 295,                    /* UPTO  */
    FROM = 296,                    /* FROM  */
    ARROW = 297,                   /* ARROW  */
    IMPLIES = 298,                 /* IMPLIES  */
    TMODULE = 299,                 /* TMODULE  */
    IMPORT = 300,                  /* IMPORT  */
    HIDING = 301,                  /* HIDING  */
    QUALIFIED = 302,               /* QUALIFIED  */
    ASMOD = 303,                   /* ASMOD  */
    NEEDPRIMS = 304,               /* NEEDPRIMS  */
    FOREIGN = 305                  /* FOREIGN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_EXPR = 3,                       /* EXPR  */
  YYSYMBOL_CTXT = 4,                       /* CTXT  */
  YYSYMBOL_SCRIPT = 5,                     /* SCRIPT  */
  YYSYMBOL_CASEXP = 6,                     /* CASEXP  */
  YYSYMBOL_OF = 7,                         /* OF  */
  YYSYMBOL_DATA = 8,                       /* DATA  */
  YYSYMBOL_TYPE = 9,                       /* TYPE  */
  YYSYMBOL_IF = 10,                        /* IF  */
  YYSYMBOL_THEN = 11,                      /* THEN  */
  YYSYMBOL_ELSE = 12,                      /* ELSE  */
  YYSYMBOL_WHERE = 13,                     /* WHERE  */
  YYSYMBOL_LET = 14,                       /* LET  */
  YYSYMBOL_IN = 15,                        /* IN  */
  YYSYMBOL_INFIXN = 16,                    /* INFIXN  */
  YYSYMBOL_INFIXL = 17,                    /* INFIXL  */
  YYSYMBOL_INFIXR = 18,                    /* INFIXR  */
  YYSYMBOL_PRIMITIVE = 19,                 /* PRIMITIVE  */
  YYSYMBOL_TNEWTYPE = 20,                  /* TNEWTYPE  */
  YYSYMBOL_DEFAULT = 21,                   /* DEFAULT  */
  YYSYMBOL_DERIVING = 22,                  /* DERIVING  */
  YYSYMBOL_DO = 23,                        /* DO  */
  YYSYMBOL_TCLASS = 24,                    /* TCLASS  */
  YYSYMBOL_TINSTANCE = 25,                 /* TINSTANCE  */
  YYSYMBOL_REPEAT = 26,                    /* REPEAT  */
  YYSYMBOL_ALL = 27,                       /* ALL  */
  YYSYMBOL_NUMLIT = 28,                    /* NUMLIT  */
  YYSYMBOL_CHARLIT = 29,                   /* CHARLIT  */
  YYSYMBOL_STRINGLIT = 30,                 /* STRINGLIT  */
  YYSYMBOL_VAROP = 31,                     /* VAROP  */
  YYSYMBOL_VARID = 32,                     /* VARID  */
  YYSYMBOL_CONOP = 33,                     /* CONOP  */
  YYSYMBOL_CONID = 34,                     /* CONID  */
  YYSYMBOL_QVAROP = 35,                    /* QVAROP  */
  YYSYMBOL_QVARID = 36,                    /* QVARID  */
  YYSYMBOL_QCONOP = 37,                    /* QCONOP  */
  YYSYMBOL_QCONID = 38,                    /* QCONID  */
  YYSYMBOL_COCO = 39,                      /* COCO  */
  YYSYMBOL_40_ = 40,                       /* '='  */
  YYSYMBOL_UPTO = 41,                      /* UPTO  */
  YYSYMBOL_42_ = 42,                       /* '@'  */
  YYSYMBOL_43_ = 43,                       /* '\\'  */
  YYSYMBOL_44_ = 44,                       /* '|'  */
  YYSYMBOL_45_ = 45,                       /* '-'  */
  YYSYMBOL_FROM = 46,                      /* FROM  */
  YYSYMBOL_ARROW = 47,                     /* ARROW  */
  YYSYMBOL_48_ = 48,                       /* '~'  */
  YYSYMBOL_49_ = 49,                       /* '!'  */
  YYSYMBOL_IMPLIES = 50,                   /* IMPLIES  */
  YYSYMBOL_51_ = 51,                       /* '('  */
  YYSYMBOL_52_ = 52,                       /* ','  */
  YYSYMBOL_53_ = 53,                       /* ')'  */
  YYSYMBOL_54_ = 54,                       /* '['  */
  YYSYMBOL_55_ = 55,                       /* ';'  */
  YYSYMBOL_56_ = 56,                       /* ']'  */
  YYSYMBOL_57_ = 57,                       /* '`'  */
  YYSYMBOL_58_ = 58,                       /* '.'  */
  YYSYMBOL_TMODULE = 59,                   /* TMODULE  */
  YYSYMBOL_IMPORT = 60,                    /* IMPORT  */
  YYSYMBOL_HIDING = 61,                    /* HIDING  */
  YYSYMBOL_QUALIFIED = 62,                 /* QUALIFIED  */
  YYSYMBOL_ASMOD = 63,                     /* ASMOD  */
  YYSYMBOL_NEEDPRIMS = 64,                 /* NEEDPRIMS  */
  YYSYMBOL_FOREIGN = 65,                   /* FOREIGN  */
  YYSYMBOL_66_ = 66,                       /* '{'  */
  YYSYMBOL_67_ = 67,                       /* '}'  */
  YYSYMBOL_68___ = 68,                     /* '_'  */
  YYSYMBOL_69_ = 69,                       /* '+'  */
  YYSYMBOL_YYACCEPT = 70,                  /* $accept  */
  YYSYMBOL_start = 71,                     /* start  */
  YYSYMBOL_topModule = 72,                 /* topModule  */
  YYSYMBOL_startMain = 73,                 /* startMain  */
  YYSYMBOL_modname = 74,                   /* modname  */
  YYSYMBOL_modid = 75,                     /* modid  */
  YYSYMBOL_modBody = 76,                   /* modBody  */
  YYSYMBOL_expspec = 77,                   /* expspec  */
  YYSYMBOL_exports = 78,                   /* exports  */
  YYSYMBOL_export = 79,                    /* export  */
  YYSYMBOL_qnames = 80,                    /* qnames  */
  YYSYMBOL_qnames1 = 81,                   /* qnames1  */
  YYSYMBOL_qname = 82,                     /* qname  */
  YYSYMBOL_impDecls = 83,                  /* impDecls  */
  YYSYMBOL_chase = 84,                     /* chase  */
  YYSYMBOL_impDecl = 85,                   /* impDecl  */
  YYSYMBOL_impspec = 86,                   /* impspec  */
  YYSYMBOL_imports = 87,                   /* imports  */
  YYSYMBOL_imports1 = 88,                  /* imports1  */
  YYSYMBOL_import = 89,                    /* import  */
  YYSYMBOL_names = 90,                     /* names  */
  YYSYMBOL_names1 = 91,                    /* names1  */
  YYSYMBOL_name = 92,                      /* name  */
  YYSYMBOL_topDecls = 93,                  /* topDecls  */
  YYSYMBOL_topDecl = 94,                   /* topDecl  */
  YYSYMBOL_tyLhs = 95,                     /* tyLhs  */
  YYSYMBOL_invars = 96,                    /* invars  */
  YYSYMBOL_invar = 97,                     /* invar  */
  YYSYMBOL_constrs = 98,                   /* constrs  */
  YYSYMBOL_pconstr = 99,                   /* pconstr  */
  YYSYMBOL_qconstr = 100,                  /* qconstr  */
  YYSYMBOL_constr = 101,                   /* constr  */
  YYSYMBOL_btype3 = 102,                   /* btype3  */
  YYSYMBOL_bbtype = 103,                   /* bbtype  */
  YYSYMBOL_nconstr = 104,                  /* nconstr  */
  YYSYMBOL_fieldspecs = 105,               /* fieldspecs  */
  YYSYMBOL_fieldspec = 106,                /* fieldspec  */
  YYSYMBOL_deriving = 107,                 /* deriving  */
  YYSYMBOL_derivs0 = 108,                  /* derivs0  */
  YYSYMBOL_derivs = 109,                   /* derivs  */
  YYSYMBOL_prims = 110,                    /* prims  */
  YYSYMBOL_prim = 111,                     /* prim  */
  YYSYMBOL_crule = 112,                    /* crule  */
  YYSYMBOL_irule = 113,                    /* irule  */
  YYSYMBOL_dtypes = 114,                   /* dtypes  */
  YYSYMBOL_dtypes1 = 115,                  /* dtypes1  */
  YYSYMBOL_fds = 116,                      /* fds  */
  YYSYMBOL_fds1 = 117,                     /* fds1  */
  YYSYMBOL_fd = 118,                       /* fd  */
  YYSYMBOL_varids0 = 119,                  /* varids0  */
  YYSYMBOL_topType = 120,                  /* topType  */
  YYSYMBOL_topType0 = 121,                 /* topType0  */
  YYSYMBOL_topType1 = 122,                 /* topType1  */
  YYSYMBOL_polyType = 123,                 /* polyType  */
  YYSYMBOL_bpolyType = 124,                /* bpolyType  */
  YYSYMBOL_varids = 125,                   /* varids  */
  YYSYMBOL_sigType = 126,                  /* sigType  */
  YYSYMBOL_context = 127,                  /* context  */
  YYSYMBOL_type = 128,                     /* type  */
  YYSYMBOL_type1 = 129,                    /* type1  */
  YYSYMBOL_btype = 130,                    /* btype  */
  YYSYMBOL_btype1 = 131,                   /* btype1  */
  YYSYMBOL_btype2 = 132,                   /* btype2  */
  YYSYMBOL_atype = 133,                    /* atype  */
  YYSYMBOL_atype1 = 134,                   /* atype1  */
  YYSYMBOL_btypes2 = 135,                  /* btypes2  */
  YYSYMBOL_typeTuple = 136,                /* typeTuple  */
  YYSYMBOL_gendecl = 137,                  /* gendecl  */
  YYSYMBOL_optDigit = 138,                 /* optDigit  */
  YYSYMBOL_ops = 139,                      /* ops  */
  YYSYMBOL_vars = 140,                     /* vars  */
  YYSYMBOL_decls = 141,                    /* decls  */
  YYSYMBOL_decls0 = 142,                   /* decls0  */
  YYSYMBOL_decls1 = 143,                   /* decls1  */
  YYSYMBOL_decl = 144,                     /* decl  */
  YYSYMBOL_funlhs = 145,                   /* funlhs  */
  YYSYMBOL_funlhs0 = 146,                  /* funlhs0  */
  YYSYMBOL_funlhs1 = 147,                  /* funlhs1  */
  YYSYMBOL_rhs = 148,                      /* rhs  */
  YYSYMBOL_rhs1 = 149,                     /* rhs1  */
  YYSYMBOL_gdrhs = 150,                    /* gdrhs  */
  YYSYMBOL_gddef = 151,                    /* gddef  */
  YYSYMBOL_wherePart = 152,                /* wherePart  */
  YYSYMBOL_lwherePart = 153,               /* lwherePart  */
  YYSYMBOL_ldecls = 154,                   /* ldecls  */
  YYSYMBOL_ldecls0 = 155,                  /* ldecls0  */
  YYSYMBOL_ldecls1 = 156,                  /* ldecls1  */
  YYSYMBOL_ldecl = 157,                    /* ldecl  */
  YYSYMBOL_pat = 158,                      /* pat  */
  YYSYMBOL_pat_npk = 159,                  /* pat_npk  */
  YYSYMBOL_pat0 = 160,                     /* pat0  */
  YYSYMBOL_pat0_vI = 161,                  /* pat0_vI  */
  YYSYMBOL_infixPat = 162,                 /* infixPat  */
  YYSYMBOL_pat10 = 163,                    /* pat10  */
  YYSYMBOL_pat10_vI = 164,                 /* pat10_vI  */
  YYSYMBOL_fpat = 165,                     /* fpat  */
  YYSYMBOL_apat = 166,                     /* apat  */
  YYSYMBOL_apat_vI = 167,                  /* apat_vI  */
  YYSYMBOL_pats2 = 168,                    /* pats2  */
  YYSYMBOL_pats1 = 169,                    /* pats1  */
  YYSYMBOL_patbinds = 170,                 /* patbinds  */
  YYSYMBOL_patbinds1 = 171,                /* patbinds1  */
  YYSYMBOL_patbind = 172,                  /* patbind  */
  YYSYMBOL_exp = 173,                      /* exp  */
  YYSYMBOL_exp_err = 174,                  /* exp_err  */
  YYSYMBOL_exp0 = 175,                     /* exp0  */
  YYSYMBOL_exp0a = 176,                    /* exp0a  */
  YYSYMBOL_exp0b = 177,                    /* exp0b  */
  YYSYMBOL_infixExpa = 178,                /* infixExpa  */
  YYSYMBOL_infixExpb = 179,                /* infixExpb  */
  YYSYMBOL_exp10a = 180,                   /* exp10a  */
  YYSYMBOL_exp10b = 181,                   /* exp10b  */
  YYSYMBOL_then_exp = 182,                 /* then_exp  */
  YYSYMBOL_else_exp = 183,                 /* else_exp  */
  YYSYMBOL_pats = 184,                     /* pats  */
  YYSYMBOL_appExp = 185,                   /* appExp  */
  YYSYMBOL_aexp = 186,                     /* aexp  */
  YYSYMBOL_exps2 = 187,                    /* exps2  */
  YYSYMBOL_alts = 188,                     /* alts  */
  YYSYMBOL_alts1 = 189,                    /* alts1  */
  YYSYMBOL_alt = 190,                      /* alt  */
  YYSYMBOL_altRhs = 191,                   /* altRhs  */
  YYSYMBOL_guardAlts = 192,                /* guardAlts  */
  YYSYMBOL_guardAlt = 193,                 /* guardAlt  */
  YYSYMBOL_stmts = 194,                    /* stmts  */
  YYSYMBOL_stmts1 = 195,                   /* stmts1  */
  YYSYMBOL_stmt = 196,                     /* stmt  */
  YYSYMBOL_fbinds = 197,                   /* fbinds  */
  YYSYMBOL_fbinds1 = 198,                  /* fbinds1  */
  YYSYMBOL_fbind = 199,                    /* fbind  */
  YYSYMBOL_list = 200,                     /* list  */
  YYSYMBOL_zipquals = 201,                 /* zipquals  */
  YYSYMBOL_quals = 202,                    /* quals  */
  YYSYMBOL_qual = 203,                     /* qual  */
  YYSYMBOL_gcon = 204,                     /* gcon  */
  YYSYMBOL_tupCommas = 205,                /* tupCommas  */
  YYSYMBOL_varid = 206,                    /* varid  */
  YYSYMBOL_qconid = 207,                   /* qconid  */
  YYSYMBOL_var = 208,                      /* var  */
  YYSYMBOL_qvar = 209,                     /* qvar  */
  YYSYMBOL_con = 210,                      /* con  */
  YYSYMBOL_qcon = 211,                     /* qcon  */
  YYSYMBOL_varop = 212,                    /* varop  */
  YYSYMBOL_varop_mi = 213,                 /* varop_mi  */
  YYSYMBOL_varop_pl = 214,                 /* varop_pl  */
  YYSYMBOL_varop_mipl = 215,               /* varop_mipl  */
  YYSYMBOL_qvarop = 216,                   /* qvarop  */
  YYSYMBOL_qvarop_mi = 217,                /* qvarop_mi  */
  YYSYMBOL_conop = 218,                    /* conop  */
  YYSYMBOL_qconop = 219,                   /* qconop  */
  YYSYMBOL_op = 220,                       /* op  */
  YYSYMBOL_qop = 221,                      /* qop  */
  YYSYMBOL_begin = 222,                    /* begin  */
  YYSYMBOL_end = 223                       /* end  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3769

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  154
/* YYNRULES -- Number of rules.  */
#define YYNRULES  462
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  802

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   305


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,     2,     2,     2,     2,     2,     2,
      51,    53,     2,    69,    52,    45,    58,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    55,
       2,    40,     2,     2,    42,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    54,    43,    56,     2,    68,    57,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    66,    44,    67,    48,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    41,    46,    47,    50,    59,
      60,    61,    62,    63,    64,    65
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   104,   104,   105,   106,   107,   120,   124,   128,   130,
     132,   138,   141,   143,   144,   152,   153,   154,   155,   156,
     161,   162,   163,   164,   165,   167,   168,   173,   174,   175,
     176,   177,   179,   180,   181,   182,   184,   185,   187,   188,
     193,   194,   195,   197,   208,   210,   213,   216,   219,   221,
     222,   223,   225,   226,   227,   228,   230,   231,   233,   234,
     235,   236,   238,   239,   240,   241,   243,   244,   246,   247,
     252,   253,   254,   255,   256,   261,   262,   265,   266,   269,
     273,   275,   278,   279,   282,   286,   287,   293,   295,   296,
     297,   299,   300,   302,   304,   306,   307,   309,   311,   313,
     314,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     326,   327,   328,   329,   330,   331,   333,   334,   335,   337,
     339,   340,   342,   343,   344,   346,   347,   348,   350,   351,
     353,   354,   359,   361,   362,   363,   365,   366,   371,   373,
     375,   377,   379,   385,   386,   387,   388,   389,   390,   392,
     393,   395,   396,   398,   399,   401,   402,   404,   405,   407,
     408,   410,   411,   413,   414,   419,   421,   423,   424,   426,
     427,   428,   429,   431,   433,   435,   437,   438,   440,   441,
     443,   444,   445,   446,   448,   449,   451,   452,   453,   454,
     455,   457,   458,   460,   461,   463,   464,   466,   467,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     481,   482,   484,   485,   486,   487,   492,   493,   494,   495,
     496,   497,   498,   499,   501,   502,   504,   505,   507,   508,
     510,   511,   513,   514,   515,   517,   519,   520,   521,   524,
     526,   527,   529,   530,   531,   532,   533,   535,   536,   537,
     538,   540,   541,   543,   544,   546,   547,   549,   551,   552,
     557,   558,   561,   562,   565,   566,   567,   570,   572,   577,
     579,   580,   582,   583,   584,   586,   587,   589,   590,   591,
     592,   593,   594,   595,   596,   597,   598,   600,   601,   603,
     604,   606,   607,   609,   610,   611,   613,   614,   615,   616,
     617,   618,   619,   620,   621,   622,   624,   625,   627,   628,
     630,   631,   633,   634,   636,   637,   642,   643,   645,   646,
     648,   649,   651,   652,   654,   655,   657,   658,   659,   660,
     662,   664,   665,   666,   667,   669,   671,   672,   673,   675,
     678,   679,   684,   685,   687,   688,   691,   692,   694,   695,
     697,   698,   699,   700,   701,   702,   703,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   714,   716,   717,   719,
     720,   722,   723,   724,   726,   728,   729,   730,   732,   733,
     735,   738,   739,   741,   742,   743,   746,   747,   749,   751,
     752,   754,   755,   757,   758,   763,   764,   765,   771,   772,
     773,   774,   777,   778,   780,   781,   783,   784,   785,   790,
     791,   792,   793,   795,   796,   798,   799,   800,   801,   803,
     804,   806,   807,   808,   809,   810,   811,   813,   814,   815,
     817,   818,   820,   821,   822,   824,   825,   826,   828,   829,
     831,   832,   834,   835,   836,   837,   839,   840,   842,   843,
     844,   847,   848,   850,   851,   852,   854,   855,   857,   858,
     863,   866,   867
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "EXPR", "CTXT",
  "SCRIPT", "CASEXP", "OF", "DATA", "TYPE", "IF", "THEN", "ELSE", "WHERE",
  "LET", "IN", "INFIXN", "INFIXL", "INFIXR", "PRIMITIVE", "TNEWTYPE",
  "DEFAULT", "DERIVING", "DO", "TCLASS", "TINSTANCE", "REPEAT", "ALL",
  "NUMLIT", "CHARLIT", "STRINGLIT", "VAROP", "VARID", "CONOP", "CONID",
  "QVAROP", "QVARID", "QCONOP", "QCONID", "COCO", "'='", "UPTO", "'@'",
  "'\\\\'", "'|'", "'-'", "FROM", "ARROW", "'~'", "'!'", "IMPLIES", "'('",
  "','", "')'", "'['", "';'", "']'", "'`'", "'.'", "TMODULE", "IMPORT",
  "HIDING", "QUALIFIED", "ASMOD", "NEEDPRIMS", "FOREIGN", "'{'", "'}'",
  "'_'", "'+'", "$accept", "start", "topModule", "startMain", "modname",
  "modid", "modBody", "expspec", "exports", "export", "qnames", "qnames1",
  "qname", "impDecls", "chase", "impDecl", "impspec", "imports",
  "imports1", "import", "names", "names1", "name", "topDecls", "topDecl",
  "tyLhs", "invars", "invar", "constrs", "pconstr", "qconstr", "constr",
  "btype3", "bbtype", "nconstr", "fieldspecs", "fieldspec", "deriving",
  "derivs0", "derivs", "prims", "prim", "crule", "irule", "dtypes",
  "dtypes1", "fds", "fds1", "fd", "varids0", "topType", "topType0",
  "topType1", "polyType", "bpolyType", "varids", "sigType", "context",
  "type", "type1", "btype", "btype1", "btype2", "atype", "atype1",
  "btypes2", "typeTuple", "gendecl", "optDigit", "ops", "vars", "decls",
  "decls0", "decls1", "decl", "funlhs", "funlhs0", "funlhs1", "rhs",
  "rhs1", "gdrhs", "gddef", "wherePart", "lwherePart", "ldecls", "ldecls0",
  "ldecls1", "ldecl", "pat", "pat_npk", "pat0", "pat0_vI", "infixPat",
  "pat10", "pat10_vI", "fpat", "apat", "apat_vI", "pats2", "pats1",
  "patbinds", "patbinds1", "patbind", "exp", "exp_err", "exp0", "exp0a",
  "exp0b", "infixExpa", "infixExpb", "exp10a", "exp10b", "then_exp",
  "else_exp", "pats", "appExp", "aexp", "exps2", "alts", "alts1", "alt",
  "altRhs", "guardAlts", "guardAlt", "stmts", "stmts1", "stmt", "fbinds",
  "fbinds1", "fbind", "list", "zipquals", "quals", "qual", "gcon",
  "tupCommas", "varid", "qconid", "var", "qvar", "con", "qcon", "varop",
  "varop_mi", "varop_pl", "varop_mipl", "qvarop", "qvarop_mi", "conop",
  "qconop", "op", "qop", "begin", "end", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-675)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-432)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     685,  -675,  1981,   244,    13,   143,  -675,  1981,  1981,   125,
     134,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  3261,
    2564,  2719,  1420,   793,  -675,  -675,  -675,  -675,   193,  -675,
    -675,   192,  -675,   636,  -675,   636,  -675,  2719,   181,  -675,
    -675,  -675,   216,  -675,   203,  -675,  -675,   291,  -675,  3668,
    -675,   242,  -675,   224,  -675,   289,    63,  -675,   303,  2257,
    -675,  -675,  -675,  3261,  2802,  2928,  -675,  -675,  -675,  3002,
    -675,   282,   285,  -675,  -675,   181,   304,   309,   318,   324,
    2346,   329,  -675,  -675,   976,   338,   341,    11,   636,   353,
     404,  -675,  -675,  2392,  -675,  2392,  -675,   197,   313,   372,
     125,  -675,  2196,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,  -675,  -675,  2435,  2478,   181,  1241,  2719,  1241,
    -675,  3383,   478,  1771,   459,  -675,  -675,  -675,  -675,  -675,
    -675,   382,  -675,  2620,  2620,   387,  1981,   432,    64,  2065,
      34,  1981,   125,  2257,   416,    48,   410,  -675,  -675,   469,
     304,   309,   324,   886,   329,   338,   341,   415,   420,   441,
    -675,   469,   469,  3261,  -675,   541,  3261,   104,  1535,  -675,
    -675,   107,  1981,  -675,  3261,  1241,  -675,  -675,  -675,  -675,
    -675,  -675,   435,   439,   442,   447,  -675,  -675,  1981,  -675,
    2300,  1981,  -675,  -675,  -675,   436,  -675,   454,  1598,  2024,
    1981,   468,  -675,  -675,  -675,  1050,   481,  -675,   483,  -675,
    -675,  3505,  3409,  -675,  2564,  -675,  -675,  2564,  -675,  -675,
     703,   476,   497,  -675,   513,   519,   181,   489,   175,  -675,
     175,  -675,   511,  1731,  -675,   568,  3313,   593,   602,   626,
    -675,   514,  3531,  3208,   559,    47,    93,   159,   567,   573,
     171,   206,    66,   504,   533,   595,  2844,  2620,   312,    59,
    1088,   508,   526,  -675,   530,  -675,  -675,    23,  -675,   228,
    -675,  3261,    46,   595,   595,  2760,    48,  2965,  -675,  1981,
    1981,   585,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,   303,  -675,  1981,  -675,  2521,   241,  3039,  -675,
    -675,  3261,  -675,  3076,  -675,  2211,  3113,  3150,  -675,  3076,
    -675,  -675,  3187,  3076,  -675,  -675,  -675,   547,   553,  -675,
     513,   583,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,   125,   581,   577,  -675,   592,  2024,   970,   591,
     582,   481,  3339,   639,  2211,  2211,  2211,  -675,  2211,  -675,
    -675,  -675,  -675,   318,   598,  -675,  1241,  1981,  -675,  3668,
    3668,  -675,  2211,  -675,  2211,  -675,  2211,  -675,  2211,  -675,
    -675,  -675,    16,   605,   695,   612,  -675,   322,   649,  -675,
     607,  -675,  -675,    45,  -675,   622,  3435,    42,  -675,   340,
    -675,  -675,   817,  -675,   817,  -675,   817,  -675,   538,   146,
    -675,   645,  -675,   654,  3549,  -675,  1680,  -675,   662,   661,
    3564,  -675,   696,   668,  3564,  -675,   772,  -675,  3076,  -675,
     676,  3224,  2886,  -675,  -675,  -675,   322,   160,  -675,  -675,
    -675,   386,   386,  -675,    33,  -675,  2678,  2138,   386,  -675,
    2211,  1981,  2024,  -675,   696,   687,  -675,  -675,  -675,  3076,
    3076,  -675,  3076,  -675,  3076,  -675,  -675,  2965,    95,    48,
     678,  -675,  -675,  -675,  1981,  -675,  -675,  3261,  -675,  -675,
    -675,  3261,  -675,  3261,  -675,  -675,  3261,  -675,  -675,  -675,
    1241,  3076,   303,  1981,  2024,  1922,   577,   576,  -675,  -675,
     692,   699,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
    3531,  -675,  3531,  -675,  -675,  -675,  2152,  -675,  2572,  -675,
    2620,   161,  1109,  2211,  -675,   516,   683,  -675,  -675,  -675,
     683,   683,   923,   386,  -675,   161,  1109,   698,   704,  -675,
     337,   696,   175,   689,  -675,   175,  -675,  3261,  3261,   548,
     973,   708,   322,  -675,   868,   730,  2678,  -675,  -675,  -675,
    -675,   970,  -675,  -675,  -675,   718,   717,  -675,  3590,  3461,
    -675,    46,  -675,   117,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,  2024,  1981,   696,   724,  -675,  -675,  3076,  -675,
    -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  2196,
    -675,  -675,  -675,   720,  -675,   721,   723,  -675,  -675,  -675,
      48,  -675,   634,  -675,   970,  3668,  1310,   122,  -675,  -675,
    3608,   118,  3487,  3365,   713,   755,   817,  -675,  -675,   848,
    -675,   762,  -675,  2211,  -675,   734,  -675,   844,  -675,  3668,
    -675,  -675,  3668,  -675,  -675,   322,  -675,   737,  -675,   742,
     738,  -675,  -675,   973,    19,   386,   821,   386,   530,   901,
    3686,  3686,  3686,  3686,  -675,  1981,   168,  -675,  -675,  -675,
    -675,  -675,  -675,  -675,  1148,  -675,  1109,   910,   118,  3668,
    3668,   743,   421,  1109,  -675,  3668,  -675,  -675,   764,  3629,
    3629,  3668,  -675,  3629,   358,   386,  -675,  1109,  -675,  -675,
     337,  -675,  -675,  2106,    55,    19,   484,  -675,  1044,   747,
    -675,   766,   386,   923,   773,   774,  3701,  -675,  3647,  -675,
    -675,  -675,  -675,  1981,  -675,   122,  1195,  3629,   735,   175,
    -675,  -675,  -675,  3668,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,   119,  -675,   162,  -675,   759,  -675,   776,   762,  -675,
     970,  -675,  -675,  -675,  -675,  -675,  -675,   765,   253,  -675,
     767,   778,  -675,  -675,  -675,  -675,  -675,   923,   787,  -675,
     923,   923,  -675,  -675,  -675,  1460,  -675,  -675,   782,  3287,
    -675,   775,   785,  -675,  -675,   386,  -675,  1851,   386,   923,
    -675,  -675,  -675,  1307,  -675,   923,  -675,  -675,  2170,  -675,
     175,  -675,  2211,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     5,     0,     0,    11,     0,   317,     0,     0,     0,
       0,   360,   357,   358,   359,   415,   430,   427,   432,     0,
       0,     0,     0,     0,   416,   417,   418,   353,   260,   316,
     319,   320,   321,   322,   324,   323,   325,   338,   349,   354,
     421,   429,   350,   434,   409,   420,   419,     0,     3,   181,
     196,     0,     4,   460,     1,     0,     0,   264,     0,     0,
     293,   299,   300,     0,     0,     0,   301,   347,   295,     0,
     297,   294,   409,   328,   333,   352,   442,   451,   448,   453,
       0,   444,   414,   410,     0,   445,   438,     0,   323,     0,
       0,   450,   439,     0,   455,     0,   411,   395,   396,     0,
       0,     2,     0,   442,   451,   448,   453,   446,   444,   445,
     438,   458,   447,   459,     0,     0,   348,   389,     0,   389,
     180,     0,     0,     0,     0,   209,   195,   197,   199,   198,
      10,    20,    12,    15,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   388,     0,   381,   385,   305,   273,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   271,
     274,   276,   275,   289,   290,     0,   297,   272,     0,   309,
     269,     0,     0,   346,     0,   310,   422,   431,   428,   433,
     424,   425,     0,     0,     0,     0,   426,   423,     0,   361,
       0,     0,   362,   413,   412,     0,   320,     0,     0,     0,
       0,   397,   363,   261,   190,     0,     0,   318,     0,   179,
     184,   186,   185,   194,     0,   327,   332,     0,   330,   335,
       0,     0,   390,   392,   393,     0,   351,     0,     0,   182,
       0,   183,     0,     0,   200,     0,     0,     0,     0,     0,
     208,     0,   185,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   273,     0,    15,     0,     0,
       0,     0,    43,    42,    17,    73,   236,     0,    74,     0,
     240,   241,     0,   276,   275,   272,     0,     0,   343,     0,
       0,     0,   341,   462,   265,   461,   268,   267,   262,   266,
     263,   340,   387,   382,     0,   337,   384,     0,     0,   278,
     277,   287,   288,     0,   302,     0,     0,     0,   291,     0,
     303,   292,     0,     0,   304,   339,   296,     0,   311,   313,
     315,     0,   452,   449,   454,   443,   368,   364,   367,   365,
     366,   398,     0,   407,   403,   405,   368,     0,     0,   200,
       0,   174,     0,     0,     0,     0,     0,   193,     0,   326,
     331,   329,   334,     0,     0,   356,     0,     0,   355,   211,
     210,   201,     0,   202,     0,   203,     0,   205,     0,   206,
     204,   207,   420,   432,     0,     0,    21,     0,     0,    26,
       0,    27,    28,     0,    82,     0,    80,    90,    89,     0,
     217,   224,     0,   219,     0,   221,     0,   135,     0,     0,
     134,   137,    85,     0,   181,   148,     0,   146,   157,     0,
     150,   147,   258,     0,   152,   436,     0,   435,     0,   437,
       0,     0,   272,    16,    48,    14,     0,    49,    13,    87,
      86,     0,     0,     7,    43,    18,    70,     0,     0,   252,
       0,     0,     0,   237,   258,   254,   256,   250,   239,     0,
       0,   440,     0,   249,     0,   441,     6,     0,     0,     0,
     369,   373,   342,   345,     0,   386,   383,     0,   281,   307,
     270,     0,   285,     0,   283,   306,     0,   279,   308,   298,
       0,     0,   408,     0,     0,     0,   402,     0,   177,   175,
     203,   205,   187,   178,   188,   189,   391,   394,   212,   213,
     211,   214,   210,   215,    22,    31,     0,    23,    32,     9,
      15,     0,     0,     0,    88,     0,   216,   456,   457,   227,
     218,   220,     0,     0,   136,     0,     0,     0,   154,   156,
       0,   258,     0,     0,   144,     0,   245,     0,     0,    49,
      52,     0,     0,    44,     0,     0,     0,    40,    71,    72,
     223,     0,   222,   166,   168,     0,     0,   172,   191,   192,
     228,     0,   253,     0,   251,   255,   244,   242,   243,   246,
     370,   377,     0,     0,   258,   375,   379,   336,   372,   344,
     282,   286,   284,   280,   312,   314,   406,   404,   401,     0,
     176,    24,    25,     0,    33,     0,    34,    37,    38,    39,
       0,    90,    81,   109,     0,     0,     0,   125,    96,    98,
     106,     0,     0,   105,     0,    75,     0,   132,   133,     0,
     119,   125,   145,     0,   162,   158,   160,     0,   143,   149,
     232,   259,   151,   247,   248,     0,    47,    59,    53,     0,
      54,    57,    58,    52,    49,     0,     0,     0,    19,     0,
       0,     0,     0,     0,   238,     0,     0,   376,   374,   378,
     371,   173,    29,    30,    35,     8,     0,     0,     0,   191,
     192,     0,     0,     0,    78,     0,   114,   112,     0,     0,
       0,     0,   111,     0,     0,     0,   226,     0,    83,   155,
       0,   163,   164,     0,     0,    49,    62,    51,    55,     0,
      45,     0,     0,     0,     0,     0,     0,   169,   192,   167,
     170,   171,   257,     0,    36,   125,     0,     0,   115,   128,
     126,    95,   113,     0,   104,   118,   117,   102,   110,   103,
     108,     0,   121,     0,   229,    76,    92,    94,   125,   159,
     161,   233,   235,   230,   234,   231,    46,     0,     0,    63,
       0,    64,    67,    68,    69,    56,    50,     0,     0,   139,
       0,     0,   165,   380,    79,     0,    97,   100,     0,   105,
     101,     0,   129,   131,   116,     0,   107,     0,     0,     0,
      84,    60,    61,    65,   138,     0,   141,   142,     0,   127,
       0,   120,     0,   122,   123,    91,    93,    66,   140,    99,
     130,   124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -675,  -675,  -675,  -675,  -675,  -369,  -129,  -675,  -675,   333,
    -675,  -675,   176,  -675,   408,   409,  -521,   202,  -675,   148,
    -675,  -675,    69,   311,   422,  -452,  -675,    85,   199,  -490,
    -675,  -674,  -675,  -622,   180,  -675,    94,  -580,  -675,  -675,
    -675,   350,  -675,  -675,  -675,  -675,  -675,  -675,   178,   187,
    -393,   179,   539,   116,  1094,  -500,   292,     1,  -109,  -103,
    -567,  1214,    -3,  -199,   659,   -41,  -675,  -675,   498,   -80,
    -654,  -675,  -675,  -675,  -136,  -675,   638,   640,  -262,  -675,
    -675,   452,  -425,  -675,   -76,  -675,  -675,  -675,    27,   -51,
    -117,  -675,  -132,   -88,  -102,  -128,     2,   957,  -675,  -675,
    -675,  -675,   424,   286,   -36,   302,   345,  -675,  -675,  -675,
     327,   261,  -675,  -675,  -675,  -675,     8,   879,   455,  -675,
     335,  -675,  -675,   342,   779,  -675,   625,   804,  -675,   571,
    -675,  -675,  -310,   448,  1249,   -11,   377,   566,    14,   -91,
    -300,  1107,   157,  -675,  -675,    17,  -675,   916,  -258,    32,
     328,    20,  -675,  -131
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     5,    52,    53,   131,   427,   261,   244,   378,   379,
     595,   596,   597,   262,   435,   263,   543,   639,   640,   641,
     750,   751,   752,   264,   265,   389,   735,   736,   607,   608,
     766,   609,   610,   724,   621,   731,   732,   674,   771,   772,
     399,   400,   408,   412,   527,   528,   531,   625,   626,   627,
     552,   553,   554,   340,   206,   487,   207,   556,   209,   210,
     557,   211,   242,   126,   213,   237,   238,   266,   392,   516,
     267,   631,   693,   694,   268,   269,   270,   271,   443,   444,
     445,   446,   534,   101,    58,   139,   140,   287,   458,   170,
     159,   160,   161,   300,   162,   163,   302,    68,   165,   171,
     317,   318,   319,   333,    29,    30,    31,    32,    33,    34,
      35,    36,   138,   282,    69,    37,    38,    89,   459,   460,
     461,   574,   575,   576,   145,   146,   147,   221,   222,   223,
      99,   201,   334,   335,    39,   239,    40,    50,    41,    42,
      43,    72,   517,    91,   454,   419,   111,   112,    94,   113,
     519,   114,   134,   288
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,   273,   273,   286,    48,   276,   122,   273,   505,   290,
     448,    90,   347,   158,   295,   241,   272,   272,   636,   564,
     235,    67,   272,   144,   203,   301,   225,   486,   225,    75,
     733,   274,   274,    71,   -41,   283,   620,   274,   668,    92,
     301,   688,   767,   -77,   121,   116,   509,   439,   384,   283,
      92,   649,    92,    90,    95,   115,   283,   539,   727,   602,
     429,   729,   437,   188,   189,   148,   292,   405,  -430,  -430,
     540,   173,    51,   619,   136,   438,   280,    71,   167,   167,
     541,    45,  -430,    71,   321,    46,   441,   430,   -41,   289,
     442,   157,   169,   258,   387,   770,   571,   -77,    47,   212,
     -41,   285,   235,   208,   667,    92,   628,   144,   190,   -77,
     744,   510,   726,   726,   799,   285,   726,   406,   137,   281,
     236,   733,   285,   700,   273,   273,   226,   388,   423,   617,
     235,   224,   563,   224,   518,   764,   518,   104,   518,   572,
     272,   106,   573,    54,   672,   456,   174,   275,   275,   658,
     726,   104,   381,   275,   274,   274,   774,   655,   780,   313,
     390,   297,   601,   314,   343,   308,   673,    71,   311,   484,
     301,   775,   397,   644,   746,   678,   316,    71,   301,   301,
      71,   298,    71,   721,   301,   522,   776,   391,    71,   320,
    -225,    57,  -225,   306,   307,   388,   470,   620,   523,   312,
      59,   777,   342,    15,  -225,   158,   100,   402,  -225,    45,
     468,   540,   614,    46,   438,   713,  -225,  -225,   472,   474,
     484,   541,   398,   542,   477,   359,   614,   360,  -225,   439,
     236,   102,    24,    25,    26,   492,   493,   494,   198,   495,
      45,   199,   386,   130,    46,    90,   385,   117,   404,   200,
     410,   414,   403,   498,   409,   413,   482,    47,   118,   503,
     144,   499,   656,   501,   401,   225,   695,   440,   441,   119,
     422,   275,   442,   447,   432,   182,    45,   453,    45,   184,
      46,    74,    46,   157,   150,    71,   151,   298,    28,    71,
     133,   167,   455,    55,    56,    47,   135,   529,   354,   654,
     549,   536,   154,   308,   273,   306,   307,   312,    87,    97,
     759,   155,    71,   424,   520,    71,   521,   167,   141,   272,
      71,    71,   156,   167,   174,    45,    71,   167,   577,    46,
     469,   561,   566,   567,   274,   568,   475,   569,   624,   301,
     478,    74,   425,   301,   120,   301,    45,    73,   301,    88,
      46,   175,   425,   679,   680,   683,    45,   176,   518,   347,
      46,   500,   177,   502,   784,   191,   614,   786,   787,  -163,
     224,   178,    15,   614,   426,   216,   219,   179,   273,   580,
     513,   600,   181,   581,  -163,   582,   796,   614,   583,   321,
      15,   186,   798,   272,   187,   195,   754,   197,  -163,  -163,
    -163,    24,    25,    26,   615,   191,   192,    73,   274,   398,
     717,   677,   418,   347,   273,   381,   614,   598,    15,    24,
      25,    26,   278,   447,   453,   730,   128,   291,   202,   272,
     449,   450,   167,   243,   559,    71,    71,   398,   196,   455,
     196,   215,   218,   279,   274,   544,   545,    24,    25,    26,
     275,   219,   560,   277,   312,    45,   193,   194,   315,    46,
     204,   185,   294,   167,   167,   296,   167,   303,   167,   665,
     347,   167,   719,   304,   326,   350,   722,   328,   352,   128,
     305,    71,   728,   754,   331,    71,   336,    71,   614,   329,
      71,    15,   322,    45,   320,   167,   323,    46,   128,   324,
     128,   128,   104,   235,   325,   407,   106,   330,   585,   613,
     233,   683,   337,   124,   689,   240,    15,   218,    16,   559,
      24,    25,    26,   613,   275,   747,   297,   125,   344,   629,
     230,   231,   632,   345,   411,   748,   749,   401,    45,   633,
     634,   349,    46,   355,   351,    24,    25,    26,    15,   356,
     182,    71,    71,  -429,   642,    47,   358,   742,   646,   357,
     275,   273,   743,   745,   361,   462,   463,    45,   393,   150,
     371,    46,   383,   598,   395,   433,   272,    24,    25,    26,
     465,   434,   128,   354,    47,   436,   212,   154,   128,   128,
     208,   274,   167,   309,   310,   391,   155,   464,  -225,   540,
    -225,   391,   670,   236,  -225,   480,  -225,   156,    15,   541,
     128,   635,  -225,   128,   479,   129,  -225,   132,  -225,   128,
     362,   363,  -225,   481,  -225,  -225,   103,   483,   104,   484,
    -225,  -225,   106,   485,   589,   489,  -225,    24,    25,    26,
     415,  -180,  -225,   497,   108,   366,   367,   708,   708,   708,
     708,   180,   416,   109,   368,   369,  -419,   642,   508,   701,
     704,   705,   235,   613,   417,   504,    15,   103,   794,   104,
     613,   105,   511,   106,   666,   524,   670,   670,   193,   370,
     670,   107,   128,   801,   613,   108,     1,   129,     2,     3,
       4,   366,   491,    84,   109,    24,    25,    26,   734,   737,
     559,   506,   507,   559,   525,   110,   530,   275,   127,   533,
     753,   532,   642,   769,   670,   488,   758,   768,   535,   128,
     670,   128,   128,   128,   343,   128,   150,   562,   151,   537,
     353,   442,   152,   578,   150,   616,   128,   128,   353,   128,
     354,   128,  -182,   128,   154,   128,   394,   396,   354,  -183,
     579,   622,   154,   155,   559,   630,   623,   559,   559,   643,
     647,   155,   342,   128,   156,   650,   514,   651,   572,   586,
     685,   588,   156,   662,   663,   664,   559,   129,   129,   684,
     127,   128,   559,   128,   672,   613,   690,   128,   696,   734,
     698,   128,   737,   185,     6,   697,   718,   753,   182,     7,
     756,  -431,   129,     8,    15,   757,   182,     9,   129,   380,
     184,   778,   760,   761,   128,   779,    10,   128,   781,    11,
     782,    12,    13,    14,   428,    15,   785,    16,   789,    17,
     783,    18,   788,    24,    25,    26,    19,   790,    20,   592,
     714,    21,   546,   547,    22,   699,   755,    23,   103,    96,
     104,   702,   797,    15,    24,    25,    26,   648,   548,   657,
     703,    27,   415,   795,   590,   715,   108,   738,   739,   791,
     127,   127,   398,   618,   515,   109,    15,   128,   740,   128,
      15,   661,    24,    25,    26,   762,   417,   299,   687,   128,
     128,   691,   185,   793,   420,   127,   421,   565,   645,   128,
      15,   127,    98,   128,   584,    24,    25,    26,   129,    24,
      25,    26,   570,   660,    60,    61,    62,   659,    15,   398,
      16,   466,   293,   227,    18,   129,   129,   496,   488,    24,
      25,    26,   587,    15,    63,   128,   128,    64,    93,   180,
      65,   712,    15,   428,   686,     0,     0,    24,    25,    26,
     551,     0,   129,     0,    66,    15,     0,    45,     0,   706,
       0,    46,    24,    25,    26,     0,   128,     0,   716,     0,
     129,    24,    25,    26,   205,     0,   129,   124,     0,   514,
     129,   488,   128,   128,    24,    25,    26,   128,     0,   128,
     128,   125,   428,     0,     0,     0,   514,     0,     0,   763,
     128,   127,    15,     0,   692,    15,   128,   637,    15,   128,
     182,     0,   183,     0,   184,     0,     0,     0,   127,   127,
       0,   164,   164,     0,   398,   638,   590,   128,   128,   128,
     128,    24,    25,    26,    24,    25,    26,    24,    25,    26,
       0,     0,     0,   128,   590,   127,   128,   128,     0,     0,
     128,   204,   128,     0,     0,     0,   128,   128,   128,     0,
     128,     0,     0,   127,   128,     0,   129,     0,   129,   127,
       0,     0,   380,   127,     0,     0,    15,   338,   637,     0,
     128,     0,    15,   128,    45,   128,     0,     0,    46,     0,
     164,   164,     0,   128,   128,   398,   164,   232,     0,     0,
     128,   233,    82,   339,   124,    24,    25,    26,   428,    44,
     603,    24,    25,    26,    44,    44,     0,   692,   125,     0,
      15,     0,     0,     0,   129,   129,     0,    44,    44,    44,
      44,     0,     0,     0,   128,     0,   604,   128,   128,   398,
       0,    15,   128,   372,    44,     0,   128,    46,   431,    24,
      25,    26,     0,     0,   128,     0,   128,     0,   605,   127,
     606,   127,   128,   124,     0,   128,    44,     0,     0,   128,
      24,    25,    26,     0,     0,     0,   129,   125,   129,   129,
      15,     0,    16,     0,    17,     0,    18,    44,     0,   707,
     709,   710,   711,     0,     0,   129,   603,     0,   129,   374,
      44,   428,    44,     0,     0,     0,     0,     0,     0,    24,
      25,    26,     0,   164,   164,     0,     0,   127,   127,     0,
       0,    44,    44,     0,     0,    44,     0,    15,     0,   372,
       0,     0,     0,    46,   164,   129,   129,     0,   720,     0,
       0,   129,     0,    44,   605,     0,   765,   129,    44,   124,
      44,     0,     0,     0,     0,     0,    24,    25,    26,     0,
     164,     0,     0,   125,     0,     0,   164,     0,    70,   127,
     164,   127,   127,    15,   129,     0,     0,    17,     0,    44,
       0,     0,     0,     0,     0,   773,     0,     0,   127,     0,
       0,   127,   220,     0,     0,    44,     0,    44,    44,   341,
       0,     0,    24,    25,    26,    44,    44,    44,     0,     0,
       0,   204,    70,   166,   166,     0,     0,     0,    70,     0,
       0,    44,     0,     0,    44,     0,     0,   341,   127,   127,
       0,     0,     0,     0,   127,   129,     0,   338,     0,    15,
     127,    16,    15,   671,    45,     0,     0,     0,    46,     0,
     382,     0,     0,     0,     0,     0,   800,   232,   748,     0,
       0,   233,    82,   234,   124,     0,     0,   127,    24,    25,
      26,    24,    25,    26,     0,   164,     0,     0,   125,     0,
       0,     0,   166,   166,     0,     0,    44,    44,   166,     0,
       0,     0,     0,   164,     0,     0,     0,     0,     0,     0,
       0,    44,   166,    44,     0,     0,   164,   164,     0,   164,
       0,   164,    70,     0,   164,    70,     0,   166,     0,     0,
       0,     6,     0,    70,     0,     0,     7,     0,   127,     0,
       8,     0,     0,     0,     9,     0,     0,     0,   164,     0,
       0,     0,     0,    10,    44,     0,    11,     0,    12,    13,
      14,    76,    15,    77,    16,    78,    17,    79,    18,     0,
       0,   204,     0,    19,    44,    80,     0,   164,    21,    81,
       0,    22,    82,    83,    23,     0,     0,    84,    85,     0,
       0,    24,    25,    26,     0,     0,     0,   338,    27,    86,
       0,     0,    15,   671,    45,     0,     0,     0,    46,     0,
       0,     0,     0,   164,     0,   166,   166,   232,     0,     0,
       0,   233,    82,   339,   124,     0,     0,     0,     0,     0,
      70,    24,    25,    26,    70,     0,   166,     0,   125,     0,
       0,   555,     0,     0,     0,   164,   299,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   166,    44,    44,
      70,     0,   166,     0,     0,   166,   166,     0,   166,     0,
       0,   166,   166,    60,    61,    62,     0,    15,     0,    16,
       0,    44,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,     0,    64,     0,     0,    65,
      44,    44,    44,     0,     0,     0,    24,    25,    26,     6,
       0,     0,     0,    66,     7,     0,   611,     0,     8,     0,
       0,     0,     9,   382,     0,   599,   555,     0,     0,     0,
     611,    10,     0,     0,    11,     0,    12,    13,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,    19,     0,    20,     0,     0,    21,     0,     0,    22,
     164,   558,    23,     0,  -400,     0,     0,     0,     0,    24,
      25,    26,     0,     0,     0,     0,    27,   166,     0,     0,
      70,    70,     0,     0,     0,     0,     0,     0,     0,    44,
      44,   204,     0,     0,     0,   166,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   166,   166,
     341,   166,     0,   166,   676,     0,   166,   682,     0,     0,
       0,     0,    15,     0,    45,     0,   166,     0,    46,     0,
     166,     0,   166,     0,     0,   166,   612,     0,     0,     0,
     166,   233,   204,  -153,   124,     0,   558,     0,     0,     0,
     612,    24,    25,    26,   555,   555,   555,   555,   125,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,   166,
     611,     0,    44,    15,     0,    45,     0,   611,     0,    46,
       0,   599,   204,   725,   725,     0,     0,   725,   232,     0,
       0,   611,   233,    82,   234,   124,    70,    70,     0,     0,
       0,     0,    24,    25,    26,   166,     0,   555,     0,   125,
     555,     0,     0,    15,     0,    45,     0,     0,     0,    46,
     611,   725,     0,     0,     0,     0,     0,     0,   232,   669,
      44,     0,   233,    82,   234,   124,     0,   166,     0,     0,
       0,     0,    24,    25,    26,     0,     0,     0,     0,   125,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   555,   204,     0,   555,   555,     0,     0,     0,   341,
       0,     0,     0,   682,   558,   558,   558,   558,     0,     0,
       0,   341,     0,   555,     0,     0,     0,     0,   338,   555,
     612,     0,   611,    15,     0,    45,     0,   612,     0,    46,
       0,     0,     0,   669,   669,     0,     0,   669,     0,     0,
     792,   612,   233,     0,     0,   124,     0,     0,     0,     0,
       0,     0,    24,    25,    26,     0,     0,   558,     0,   125,
     558,     0,     0,     6,     0,     0,     0,     0,     7,     0,
     612,   669,     8,     0,     0,     0,     9,   669,     0,     0,
       0,     0,   166,     0,     0,    10,     0,     0,    11,     0,
      12,    13,    14,     0,    15,     0,    16,     0,    17,     0,
      18,     0,     0,     0,     0,    19,     0,    20,     0,     0,
      21,   558,     0,    22,   558,   558,    23,     0,  -399,     0,
       0,     0,     6,    24,    25,    26,     0,     7,     0,     0,
      27,     8,     0,   558,     0,     9,     0,     0,     0,   558,
       0,     0,   612,     0,    10,     0,     0,    11,     0,    12,
      13,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,    19,     6,    20,     0,     0,    21,
       7,     0,    22,     0,     8,    23,     0,     0,   332,     0,
       0,     0,    24,    25,    26,     0,     0,    10,     0,    27,
      11,     0,    12,    13,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,     0,     0,   283,    19,     0,    20,
       0,     0,    21,     0,     0,    22,     0,     0,    23,     0,
       0,   247,   248,   249,     0,    24,    25,    26,     0,     0,
       0,     0,    27,   255,    61,    62,     0,    15,     0,    16,
       0,     0,     0,    18,     0,     0,     0,   283,     0,     0,
     168,     0,     0,    63,     0,     0,   256,     0,     0,    65,
     284,     0,   247,   248,   249,     0,    24,    25,    26,     0,
       0,     0,   285,    66,   255,    61,    62,     0,    15,   550,
      16,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,   168,     0,     0,    63,     0,     0,   256,     0,     0,
      65,   741,     0,     0,     0,   551,     0,    24,    25,    26,
      15,   603,    45,   285,    66,     0,    46,     0,     0,     0,
       0,     0,     0,     0,    15,     0,   372,     0,    17,   205,
     373,     0,   124,     0,     0,     0,     0,   204,     0,    24,
      25,    26,    15,   374,   372,   591,   125,     0,    46,     0,
       0,   377,   204,    24,    25,    26,     0,     0,     0,   605,
       0,   606,     0,     0,   124,     0,     0,     0,    15,     0,
      45,    24,    25,    26,    46,     0,     0,     0,   125,     0,
       0,     0,     0,    15,     0,    45,     0,   205,     0,    46,
     124,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,     0,   233,     7,   125,   124,     0,     8,     0,     0,
       0,   142,    24,    25,    26,     0,     0,     0,     0,   125,
      10,     0,     0,    11,     0,    12,    13,    14,     0,    15,
       0,    16,     0,    17,     0,    18,     0,     0,     0,     0,
      19,     0,    20,     0,     0,    21,     7,     0,    22,     0,
       8,    23,   143,     0,     9,     0,     0,     0,    24,    25,
      26,     0,     0,    10,     0,    27,    11,     0,    12,    13,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
       0,     0,     0,    19,     0,   217,     0,     0,    21,     0,
       0,    22,     7,   327,    23,     0,     8,     0,     0,     0,
       9,    24,    25,    26,     0,     0,     0,     0,    27,    10,
       0,     0,    11,     0,    12,    13,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,     0,     0,     0,    19,
       0,     0,     0,     0,    21,     0,     0,    22,     7,   180,
      23,     0,     8,     0,     0,     0,     9,    24,    25,    26,
       0,     0,     0,     0,    27,    10,     0,     0,    11,     0,
      12,    13,    14,     0,    15,     0,    16,     0,    17,     0,
      18,     0,     0,     0,     0,    19,     0,    20,     0,     0,
      21,     7,     0,    22,     0,     8,    23,     0,     0,     9,
       0,     0,     0,    24,    25,    26,     0,     0,    10,     0,
      27,    11,     0,    12,    13,    14,     0,    15,     0,    16,
       0,    17,     0,    18,     0,     0,     0,     0,    19,     0,
     214,     0,     0,    21,     7,     0,    22,     0,     8,    23,
       0,     0,     9,     0,     0,     0,    24,    25,    26,     0,
       0,    10,     0,    27,    11,     0,    12,    13,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,    19,     0,   217,     0,     0,    21,     7,     0,    22,
       0,     8,    23,     0,     0,   142,     0,     0,     0,    24,
      25,    26,     0,     0,    10,     0,    27,    11,     0,    12,
      13,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,    19,     0,    20,     0,     0,    21,
       7,     0,    22,     0,     8,    23,     0,     0,     9,     0,
       0,     0,    24,    25,    26,     0,     0,    10,     0,    27,
      11,     0,    12,    13,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,    15,     0,    16,    19,    17,     0,
      18,     0,    21,   593,     0,    22,     0,     0,    23,     0,
       0,     0,     0,   374,   594,    24,    25,    26,   245,   246,
       0,     0,    27,    24,    25,    26,   247,   248,   249,   250,
     251,   252,     0,     0,   253,   254,     0,     0,   255,    61,
      62,     0,    15,     0,    16,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,   168,     0,     0,    63,     0,
       0,   256,     0,     0,    65,   257,     0,     0,     0,     0,
     258,    24,    25,    26,   259,   260,   245,   246,    66,     0,
       0,     0,     0,     0,   247,   248,   249,   250,   251,   252,
       0,     0,   253,   254,     0,     0,   255,    61,    62,     0,
      15,     0,    16,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,   168,     0,     0,    63,     0,     0,   256,
       0,     0,    65,     0,     0,     0,     0,     0,     0,    24,
      25,    26,   259,   260,     0,    11,    66,    12,    13,    14,
       0,    15,     0,    16,     0,    17,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
      22,     0,     0,    23,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,     0,     0,     0,    27,    60,    61,
      62,   103,    15,   104,    16,     0,     0,   106,    18,  -229,
       0,     0,   174,     0,     0,   451,     0,     0,    63,   108,
       0,    64,  -229,     0,    65,     0,     0,   416,   109,     0,
       0,    24,    25,    26,     0,     0,     0,     0,    66,   452,
     149,    61,    62,   150,    15,   151,    16,     0,     0,   152,
      18,     0,     0,     0,     0,     0,     0,   153,     0,     0,
      63,   154,     0,    64,    82,    83,    65,     0,     0,     0,
     155,     0,     0,    24,    25,    26,     0,     0,     0,     0,
      66,   156,   255,    61,    62,   150,    15,   151,    16,     0,
       0,   152,    18,     0,     0,     0,     0,     0,     0,   153,
       0,     0,    63,   154,     0,   256,    82,    83,    65,     0,
       0,     0,   155,     0,     0,    24,    25,    26,     0,     0,
       0,     0,    66,   156,    60,    61,    62,   103,    15,   104,
      16,     0,     0,   106,    18,     0,     0,     0,   174,     0,
       0,   451,     0,     0,    63,   108,     0,    64,     0,     0,
      65,     0,     0,   416,   109,     0,     0,    24,    25,    26,
       0,     0,     0,     0,    66,   452,   149,    61,    62,     0,
      15,     0,    16,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,   168,     0,     0,    63,     0,     0,    64,
       0,     0,    65,     0,    96,     0,     0,     0,     0,    24,
      25,    26,     0,   149,    61,    62,    66,    15,     0,    16,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
     168,     0,     0,    63,     0,     0,    64,     0,     0,    65,
     457,     0,     0,     0,     0,     0,    24,    25,    26,     0,
      60,    61,    62,    66,    15,     0,    16,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,   172,
      63,     0,     0,    64,     0,     0,    65,     0,     0,     0,
       0,     0,     0,    24,    25,    26,     0,    60,    61,    62,
      66,    15,     0,    16,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,   467,     0,     0,    63,     0,     0,
      64,     0,     0,    65,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,   149,    61,    62,    66,    15,     0,
      16,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,   168,     0,     0,    63,     0,     0,    64,     0,     0,
      65,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,    60,    61,    62,    66,    15,     0,    16,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,   471,     0,
       0,    63,     0,     0,    64,     0,     0,    65,     0,     0,
       0,     0,     0,     0,    24,    25,    26,     0,    60,    61,
      62,    66,    15,     0,    16,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,   473,     0,     0,    63,     0,
       0,    64,     0,     0,    65,     0,     0,     0,     0,     0,
       0,    24,    25,    26,     0,    60,    61,    62,    66,    15,
       0,    16,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,   476,     0,     0,    63,     0,     0,    64,     0,
      15,    65,   372,     0,    17,     0,   373,     0,    24,    25,
      26,     0,    60,    61,    62,    66,    15,     0,    16,   374,
     375,   376,    18,     0,     0,     0,     0,   377,     0,    24,
      25,    26,    63,     0,     0,    64,     0,   538,    65,     0,
       0,     0,     0,     0,     0,    24,    25,    26,     0,    60,
      61,    62,    66,    15,     0,    16,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    63,
       0,     0,    64,     0,     0,    65,     0,     0,     0,    15,
     104,    45,    24,    25,    26,    46,     0,     0,     0,    66,
       0,     0,     0,     0,     0,     0,   681,  -181,   233,     0,
       0,   124,     0,     0,   678,    15,     0,    45,    24,    25,
      26,    46,     0,     0,     0,   125,     0,     0,     0,     0,
     348,     0,     0,     0,   123,   364,   365,   124,     0,     0,
       0,    15,     0,    45,    24,    25,    26,    46,     0,     0,
       0,   125,     0,     0,     0,     0,   348,     0,     0,     0,
     123,   364,   490,   124,     0,     0,     0,    15,   104,    45,
      24,    25,    26,    46,     0,     0,     0,   125,     0,     0,
       0,     0,     0,     0,   681,    15,   233,    45,     0,   124,
       0,    46,   678,     0,     0,     0,    24,    25,    26,     0,
       0,     0,     0,   125,   123,   228,   229,   124,     0,     0,
       0,    15,     0,    45,    24,    25,    26,    46,     0,     0,
       0,   125,     0,     0,     0,     0,   348,     0,     0,  -181,
     123,     0,     0,   124,     0,     0,     0,    15,     0,    45,
      24,    25,    26,    46,     0,   512,     0,   125,     0,     0,
       0,     0,     0,     0,     0,  -181,   123,     0,     0,   124,
       0,     0,     0,    15,     0,    45,    24,    25,    26,    46,
       0,     0,     0,   125,     0,     0,     0,     0,   653,     0,
       0,  -181,   123,     0,     0,   124,     0,     0,     0,    15,
     104,    45,    24,    25,    26,    46,     0,     0,     0,   125,
       0,     0,     0,     0,     0,     0,     0,    15,   123,    45,
       0,   124,     0,    46,   678,     0,     0,     0,    24,    25,
      26,     0,   346,     0,     0,   125,   123,     0,     0,   124,
       0,     0,     0,    15,     0,    45,    24,    25,    26,    46,
       0,     0,     0,   125,     0,     0,     0,     0,   348,     0,
       0,    15,   123,    45,     0,   124,     0,    46,     0,   526,
       0,     0,    24,    25,    26,     0,    15,     0,    45,   125,
     123,     0,    46,   124,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,  -181,   123,     0,   125,   124,     0,
       0,     0,    15,     0,    45,    24,    25,    26,    46,     0,
       0,     0,   125,     0,     0,     0,     0,   652,     0,     0,
      15,   123,    45,     0,   124,     0,    46,     0,     0,     0,
       0,    24,    25,    26,     0,     0,     0,   675,   125,   233,
       0,    15,   124,    45,     0,     0,     0,    46,     0,    24,
      25,    26,     0,     0,     0,     0,   125,     0,   723,    15,
     233,    45,     0,   124,     0,    46,     0,     0,     0,     0,
      24,    25,    26,     0,   653,     0,     0,   125,   123,     0,
      15,   124,    45,     0,     0,     0,    46,     0,    24,    25,
      26,     0,     0,     0,     0,   125,     0,     0,    15,   123,
      45,     0,   124,     0,    46,     0,     0,     0,     0,    24,
      25,    26,     0,    15,     0,    45,   125,   233,     0,    46,
     124,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,     0,   205,     0,   125,   124,     0,     0,     0,     0,
       0,     0,    24,    25,    26,     0,     0,     0,     0,   125
};

static const yytype_int16 yycheck[] =
{
       3,   133,   134,   139,     3,   134,    47,   139,   377,   140,
     272,    22,   211,    64,   145,   124,   133,   134,   539,   444,
     123,    19,   139,    59,   100,   153,   117,   337,   119,    21,
     684,   133,   134,    19,     1,     1,   526,   139,   605,    22,
     168,   621,   716,     1,    47,    37,     1,     1,     1,     1,
      33,   551,    35,    64,    22,    35,     1,   426,   680,   511,
       1,   683,    39,    52,    53,    63,   142,     1,    52,    53,
      51,    69,    59,   525,    11,    52,    12,    63,    64,    65,
      61,    34,    66,    69,   175,    38,    40,    28,    55,    55,
      44,    64,    65,    60,     1,   717,     1,    55,    51,   102,
      67,    67,   205,   102,   604,    88,   531,   143,    88,    67,
      55,    66,   679,   680,   788,    67,   683,    51,    55,    55,
     123,   775,    67,   644,   256,   257,   118,    34,   257,   522,
     233,   117,   442,   119,   392,   715,   394,    33,   396,    44,
     257,    37,    47,     0,    22,   276,    42,   133,   134,   574,
     717,    33,   243,   139,   256,   257,   723,    40,   738,    52,
       1,    57,     1,    56,   205,   163,    44,   153,   166,    52,
     298,    52,     1,   542,   695,    57,   174,   163,   306,   307,
     166,   149,   168,   673,   312,    39,    67,    28,   174,   175,
      31,    66,    33,   161,   162,    34,   305,   687,    52,   167,
      66,    39,   205,    32,    45,   256,    13,     1,    49,    34,
     298,    51,   512,    38,    52,    47,    57,    58,   306,   307,
      52,    61,    51,    63,   312,   228,   526,   230,    69,     1,
     233,    39,    61,    62,    63,   344,   345,   346,    41,   348,
      34,    44,   245,     1,    38,   256,   245,    66,   251,    52,
     253,   254,   251,   362,   253,   254,   332,    51,    42,   368,
     296,   364,   572,   366,   250,   356,   635,    39,    40,    66,
     256,   257,    44,   271,   260,    34,    34,   275,    34,    38,
      38,    20,    38,   256,    31,   271,    33,   255,     2,   275,
      66,   277,   275,     7,     8,    51,     7,   406,    45,   561,
     436,   418,    49,   301,   436,   273,   274,   275,    22,    23,
     703,    58,   298,     1,   394,   301,   396,   303,    15,   436,
     306,   307,    69,   309,    42,    34,   312,   313,   459,    38,
     303,   440,   449,   450,   436,   452,   309,   454,     1,   467,
     313,    80,    30,   471,    53,   473,    34,    20,   476,    22,
      38,    66,    30,   611,   612,   613,    34,    53,   616,   558,
      38,   364,    53,   366,   757,    52,   666,   760,   761,    32,
     356,    53,    32,   673,    62,   114,   115,    53,   510,   467,
      40,   510,    53,   471,    47,   473,   779,   687,   476,   480,
      32,    53,   785,   510,    53,    93,   696,    95,    61,    62,
      63,    61,    62,    63,   513,    52,    53,    80,   510,    51,
     668,   610,   255,   612,   546,   506,   716,   508,    32,    61,
      62,    63,   136,   421,   422,    67,    49,   141,    56,   546,
     273,   274,   418,    51,   437,   421,   422,    51,    93,   422,
      95,   114,   115,    11,   546,   431,   432,    61,    62,    63,
     436,   190,   438,    66,   422,    34,    52,    53,   172,    38,
       1,    84,    46,   449,   450,    55,   452,    52,   454,   600,
     669,   457,    51,    53,   188,   214,   675,   191,   217,   102,
      39,   467,   681,   783,   198,   471,   200,   473,   788,    53,
     476,    32,    57,    34,   480,   481,    57,    38,   121,    57,
     123,   124,    33,   606,    57,     1,    37,    53,   481,   512,
      51,   769,    44,    54,   623,    56,    32,   190,    34,   522,
      61,    62,    63,   526,   510,    41,    57,    68,    47,   532,
      52,    53,   535,    50,     1,    51,    52,   523,    34,   537,
     538,   214,    38,    67,   217,    61,    62,    63,    32,    52,
      34,   537,   538,    40,   540,    51,    67,   693,   544,    40,
     546,   693,   693,   694,    53,   279,   280,    34,     1,    31,
      56,    38,    13,   664,     1,    67,   693,    61,    62,    63,
     294,    55,   205,    45,    51,    55,   589,    49,   211,   212,
     589,   693,   578,    52,    53,    28,    58,    12,    31,    51,
      33,    28,   605,   606,    31,    52,    33,    69,    32,    61,
     233,    63,    45,   236,    67,    49,    49,    51,    45,   242,
      52,    53,    49,    40,    57,    58,    31,    46,    33,    52,
      57,    58,    37,    41,    58,    53,    69,    61,    62,    63,
      45,    50,    69,   357,    49,    52,    53,   650,   651,   652,
     653,    53,    57,    58,    52,    53,    51,   643,    51,   645,
     646,   647,   765,   666,    69,    53,    32,    31,   777,    33,
     673,    35,    50,    37,    40,    30,   679,   680,    52,    53,
     683,    45,   305,   792,   687,    49,     1,   121,     3,     4,
       5,    52,    53,    57,    58,    61,    62,    63,   684,   685,
     703,    52,    53,   706,    50,    69,    44,   693,    49,    13,
     696,    50,   698,   716,   717,   338,   702,   716,    50,   342,
     723,   344,   345,   346,   765,   348,    31,   441,    33,    53,
      35,    44,    37,    55,    31,    52,   359,   360,    35,   362,
      45,   364,    50,   366,    49,   368,   248,   249,    45,    50,
     464,    53,    49,    58,   757,    66,    52,   760,   761,    51,
      30,    58,   765,   386,    69,    47,   389,    50,    44,   483,
      15,   485,    69,    53,    53,    52,   779,   211,   212,    66,
     121,   404,   785,   406,    22,   788,    52,   410,    51,   775,
      52,   414,   778,   416,     1,    53,    53,   783,    34,     6,
      53,    66,   236,    10,    32,    39,    34,    14,   242,   243,
      38,    52,    39,    39,   437,    39,    23,   440,    53,    26,
      53,    28,    29,    30,   258,    32,    39,    34,    53,    36,
      52,    38,    50,    61,    62,    63,    43,    52,    45,   506,
     664,    48,   434,   434,    51,   643,   698,    54,    31,    56,
      33,    30,   783,    32,    61,    62,    63,   546,   436,   573,
      39,    68,    45,   778,   487,   666,    49,   687,   690,   775,
     211,   212,    51,   523,    57,    58,    32,   500,   691,   502,
      32,   589,    61,    62,    63,   706,    69,     1,    40,   512,
     513,    47,   515,   777,   256,   236,   256,   445,    30,   522,
      32,   242,    23,   526,   480,    61,    62,    63,   342,    61,
      62,    63,   457,   578,    28,    29,    30,   575,    32,    51,
      34,   296,   143,   119,    38,   359,   360,   356,   551,    61,
      62,    63,   484,    32,    48,   558,   559,    51,    22,    53,
      54,   655,    32,   377,   616,    -1,    -1,    61,    62,    63,
      27,    -1,   386,    -1,    68,    32,    -1,    34,    -1,    58,
      -1,    38,    61,    62,    63,    -1,   589,    -1,    58,    -1,
     404,    61,    62,    63,    51,    -1,   410,    54,    -1,   602,
     414,   604,   605,   606,    61,    62,    63,   610,    -1,   612,
     613,    68,   426,    -1,    -1,    -1,   619,    -1,    -1,   713,
     623,   342,    32,    -1,   627,    32,   629,    34,    32,   632,
      34,    -1,    36,    -1,    38,    -1,    -1,    -1,   359,   360,
      -1,    64,    65,    -1,    51,    52,   649,   650,   651,   652,
     653,    61,    62,    63,    61,    62,    63,    61,    62,    63,
      -1,    -1,    -1,   666,   667,   386,   669,   670,    -1,    -1,
     673,     1,   675,    -1,    -1,    -1,   679,   680,   681,    -1,
     683,    -1,    -1,   404,   687,    -1,   500,    -1,   502,   410,
      -1,    -1,   506,   414,    -1,    -1,    32,    27,    34,    -1,
     703,    -1,    32,   706,    34,   708,    -1,    -1,    38,    -1,
     133,   134,    -1,   716,   717,    51,   139,    47,    -1,    -1,
     723,    51,    52,    53,    54,    61,    62,    63,   542,     2,
       1,    61,    62,    63,     7,     8,    -1,   740,    68,    -1,
      32,    -1,    -1,    -1,   558,   559,    -1,    20,    21,    22,
      23,    -1,    -1,    -1,   757,    -1,    27,   760,   761,    51,
      -1,    32,   765,    34,    37,    -1,   769,    38,    60,    61,
      62,    63,    -1,    -1,   777,    -1,   779,    -1,    49,   500,
      51,   502,   785,    54,    -1,   788,    59,    -1,    -1,   792,
      61,    62,    63,    -1,    -1,    -1,   610,    68,   612,   613,
      32,    -1,    34,    -1,    36,    -1,    38,    80,    -1,   650,
     651,   652,   653,    -1,    -1,   629,     1,    -1,   632,    51,
      93,   635,    95,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,   256,   257,    -1,    -1,   558,   559,    -1,
      -1,   114,   115,    -1,    -1,   118,    -1,    32,    -1,    34,
      -1,    -1,    -1,    38,   277,   669,   670,    -1,   672,    -1,
      -1,   675,    -1,   136,    49,    -1,    51,   681,   141,    54,
     143,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
     303,    -1,    -1,    68,    -1,    -1,   309,    -1,    19,   610,
     313,   612,   613,    32,   708,    -1,    -1,    36,    -1,   172,
      -1,    -1,    -1,    -1,    -1,   719,    -1,    -1,   629,    -1,
      -1,   632,    51,    -1,    -1,   188,    -1,   190,   191,   205,
      -1,    -1,    61,    62,    63,   198,   199,   200,    -1,    -1,
      -1,     1,    63,    64,    65,    -1,    -1,    -1,    69,    -1,
      -1,   214,    -1,    -1,   217,    -1,    -1,   233,   669,   670,
      -1,    -1,    -1,    -1,   675,   769,    -1,    27,    -1,    32,
     681,    34,    32,    33,    34,    -1,    -1,    -1,    38,    -1,
     243,    -1,    -1,    -1,    -1,    -1,   790,    47,    51,    -1,
      -1,    51,    52,    53,    54,    -1,    -1,   708,    61,    62,
      63,    61,    62,    63,    -1,   418,    -1,    -1,    68,    -1,
      -1,    -1,   133,   134,    -1,    -1,   279,   280,   139,    -1,
      -1,    -1,    -1,   436,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   294,   153,   296,    -1,    -1,   449,   450,    -1,   452,
      -1,   454,   163,    -1,   457,   166,    -1,   168,    -1,    -1,
      -1,     1,    -1,   174,    -1,    -1,     6,    -1,   769,    -1,
      10,    -1,    -1,    -1,    14,    -1,    -1,    -1,   481,    -1,
      -1,    -1,    -1,    23,   337,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,     1,    -1,    43,   357,    45,    -1,   510,    48,    49,
      -1,    51,    52,    53,    54,    -1,    -1,    57,    58,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    27,    68,    69,
      -1,    -1,    32,    33,    34,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,   546,    -1,   256,   257,    47,    -1,    -1,
      -1,    51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
     271,    61,    62,    63,   275,    -1,   277,    -1,    68,    -1,
      -1,   437,    -1,    -1,    -1,   578,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   298,   441,   442,
     301,    -1,   303,    -1,    -1,   306,   307,    -1,   309,    -1,
      -1,   312,   313,    28,    29,    30,    -1,    32,    -1,    34,
      -1,   464,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,
     483,   484,   485,    -1,    -1,    -1,    61,    62,    63,     1,
      -1,    -1,    -1,    68,     6,    -1,   512,    -1,    10,    -1,
      -1,    -1,    14,   506,    -1,   508,   522,    -1,    -1,    -1,
     526,    23,    -1,    -1,    26,    -1,    28,    29,    30,    -1,
      32,    -1,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,
      -1,    43,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
     693,   437,    54,    -1,    56,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    -1,    -1,    -1,    68,   418,    -1,    -1,
     421,   422,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   572,
     573,     1,    -1,    -1,    -1,   436,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   449,   450,
     606,   452,    -1,   454,   610,    -1,   457,   613,    -1,    -1,
      -1,    -1,    32,    -1,    34,    -1,   467,    -1,    38,    -1,
     471,    -1,   473,    -1,    -1,   476,   512,    -1,    -1,    -1,
     481,    51,     1,    53,    54,    -1,   522,    -1,    -1,    -1,
     526,    61,    62,    63,   650,   651,   652,   653,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,   510,
     666,    -1,   655,    32,    -1,    34,    -1,   673,    -1,    38,
      -1,   664,     1,   679,   680,    -1,    -1,   683,    47,    -1,
      -1,   687,    51,    52,    53,    54,   537,   538,    -1,    -1,
      -1,    -1,    61,    62,    63,   546,    -1,   703,    -1,    68,
     706,    -1,    -1,    32,    -1,    34,    -1,    -1,    -1,    38,
     716,   717,    -1,    -1,    -1,    -1,    -1,    -1,    47,   605,
     713,    -1,    51,    52,    53,    54,    -1,   578,    -1,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   757,     1,    -1,   760,   761,    -1,    -1,    -1,   765,
      -1,    -1,    -1,   769,   650,   651,   652,   653,    -1,    -1,
      -1,   777,    -1,   779,    -1,    -1,    -1,    -1,    27,   785,
     666,    -1,   788,    32,    -1,    34,    -1,   673,    -1,    38,
      -1,    -1,    -1,   679,   680,    -1,    -1,   683,    -1,    -1,
      49,   687,    51,    -1,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,   703,    -1,    68,
     706,    -1,    -1,     1,    -1,    -1,    -1,    -1,     6,    -1,
     716,   717,    10,    -1,    -1,    -1,    14,   723,    -1,    -1,
      -1,    -1,   693,    -1,    -1,    23,    -1,    -1,    26,    -1,
      28,    29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,
      48,   757,    -1,    51,   760,   761,    54,    -1,    56,    -1,
      -1,    -1,     1,    61,    62,    63,    -1,     6,    -1,    -1,
      68,    10,    -1,   779,    -1,    14,    -1,    -1,    -1,   785,
      -1,    -1,   788,    -1,    23,    -1,    -1,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,
      -1,    -1,    -1,    -1,    43,     1,    45,    -1,    -1,    48,
       6,    -1,    51,    -1,    10,    54,    -1,    -1,    14,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,    23,    -1,    68,
      26,    -1,    28,    29,    30,    -1,    32,    -1,    34,    -1,
      36,    -1,    38,    -1,    -1,    -1,     1,    43,    -1,    45,
      -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,    -1,
      -1,    16,    17,    18,    -1,    61,    62,    63,    -1,    -1,
      -1,    -1,    68,    28,    29,    30,    -1,    32,    -1,    34,
      -1,    -1,    -1,    38,    -1,    -1,    -1,     1,    -1,    -1,
      45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,
      55,    -1,    16,    17,    18,    -1,    61,    62,    63,    -1,
      -1,    -1,    67,    68,    28,    29,    30,    -1,    32,     1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,
      54,    55,    -1,    -1,    -1,    27,    -1,    61,    62,    63,
      32,     1,    34,    67,    68,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,    34,    -1,    36,    51,
      38,    -1,    54,    -1,    -1,    -1,    -1,     1,    -1,    61,
      62,    63,    32,    51,    34,    53,    68,    -1,    38,    -1,
      -1,    59,     1,    61,    62,    63,    -1,    -1,    -1,    49,
      -1,    51,    -1,    -1,    54,    -1,    -1,    -1,    32,    -1,
      34,    61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    32,    -1,    34,    -1,    51,    -1,    38,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,
      -1,    -1,    51,     6,    68,    54,    -1,    10,    -1,    -1,
      -1,    14,    61,    62,    63,    -1,    -1,    -1,    -1,    68,
      23,    -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,
      -1,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,     6,    -1,    51,    -1,
      10,    54,    55,    -1,    14,    -1,    -1,    -1,    61,    62,
      63,    -1,    -1,    23,    -1,    68,    26,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,
      -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,    -1,
      -1,    51,     6,    53,    54,    -1,    10,    -1,    -1,    -1,
      14,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    23,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    -1,
      34,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,     6,    53,
      54,    -1,    10,    -1,    -1,    -1,    14,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    23,    -1,    -1,    26,    -1,
      28,    29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,
      48,     6,    -1,    51,    -1,    10,    54,    -1,    -1,    14,
      -1,    -1,    -1,    61,    62,    63,    -1,    -1,    23,    -1,
      68,    26,    -1,    28,    29,    30,    -1,    32,    -1,    34,
      -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    43,    -1,
      45,    -1,    -1,    48,     6,    -1,    51,    -1,    10,    54,
      -1,    -1,    14,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    23,    -1,    68,    26,    -1,    28,    29,    30,    -1,
      32,    -1,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,
      -1,    43,    -1,    45,    -1,    -1,    48,     6,    -1,    51,
      -1,    10,    54,    -1,    -1,    14,    -1,    -1,    -1,    61,
      62,    63,    -1,    -1,    23,    -1,    68,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,
      -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,
       6,    -1,    51,    -1,    10,    54,    -1,    -1,    14,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,    23,    -1,    68,
      26,    -1,    28,    29,    30,    -1,    32,    -1,    34,    -1,
      36,    -1,    38,    -1,    32,    -1,    34,    43,    36,    -1,
      38,    -1,    48,    41,    -1,    51,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    51,    52,    61,    62,    63,     8,     9,
      -1,    -1,    68,    61,    62,    63,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    -1,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    -1,
      -1,    51,    -1,    -1,    54,    55,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,     8,     9,    68,    -1,
      -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    -1,    -1,    28,    29,    30,    -1,
      32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    64,    65,    -1,    26,    68,    28,    29,    30,
      -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    -1,    -1,    -1,    68,    28,    29,
      30,    31,    32,    33,    34,    -1,    -1,    37,    38,    39,
      -1,    -1,    42,    -1,    -1,    45,    -1,    -1,    48,    49,
      -1,    51,    52,    -1,    54,    -1,    -1,    57,    58,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    69,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    -1,    51,    52,    53,    54,    -1,    -1,    -1,
      58,    -1,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,
      68,    69,    28,    29,    30,    31,    32,    33,    34,    -1,
      -1,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    -1,    51,    52,    53,    54,    -1,
      -1,    -1,    58,    -1,    -1,    61,    62,    63,    -1,    -1,
      -1,    -1,    68,    69,    28,    29,    30,    31,    32,    33,
      34,    -1,    -1,    37,    38,    -1,    -1,    -1,    42,    -1,
      -1,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    -1,
      54,    -1,    -1,    57,    58,    -1,    -1,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    69,    28,    29,    30,    -1,
      32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
      -1,    -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    28,    29,    30,    68,    32,    -1,    34,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,
      55,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      28,    29,    30,    68,    32,    -1,    34,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,    51,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    62,    63,    -1,    28,    29,    30,
      68,    32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,
      51,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    28,    29,    30,    68,    32,    -1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,
      -1,    28,    29,    30,    68,    32,    -1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    -1,    -1,    51,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    62,    63,    -1,    28,    29,
      30,    68,    32,    -1,    34,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    -1,
      -1,    51,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    63,    -1,    28,    29,    30,    68,    32,
      -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,
      32,    54,    34,    -1,    36,    -1,    38,    -1,    61,    62,
      63,    -1,    28,    29,    30,    68,    32,    -1,    34,    51,
      52,    53,    38,    -1,    -1,    -1,    -1,    59,    -1,    61,
      62,    63,    48,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,    28,
      29,    30,    68,    32,    -1,    34,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    51,    -1,    -1,    54,    -1,    -1,    -1,    32,
      33,    34,    61,    62,    63,    38,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    57,    32,    -1,    34,    61,    62,
      63,    38,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,
      -1,    32,    -1,    34,    61,    62,    63,    38,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,
      51,    52,    53,    54,    -1,    -1,    -1,    32,    33,    34,
      61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    32,    51,    34,    -1,    54,
      -1,    38,    57,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    51,    52,    53,    54,    -1,    -1,
      -1,    32,    -1,    34,    61,    62,    63,    38,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    -1,    -1,    54,    -1,    -1,    -1,    32,    -1,    34,
      61,    62,    63,    38,    -1,    40,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    54,
      -1,    -1,    -1,    32,    -1,    34,    61,    62,    63,    38,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,    32,
      33,    34,    61,    62,    63,    38,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    51,    34,
      -1,    54,    -1,    38,    57,    -1,    -1,    -1,    61,    62,
      63,    -1,    47,    -1,    -1,    68,    51,    -1,    -1,    54,
      -1,    -1,    -1,    32,    -1,    34,    61,    62,    63,    38,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    32,    51,    34,    -1,    54,    -1,    38,    -1,    40,
      -1,    -1,    61,    62,    63,    -1,    32,    -1,    34,    68,
      51,    -1,    38,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    50,    51,    -1,    68,    54,    -1,
      -1,    -1,    32,    -1,    34,    61,    62,    63,    38,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      32,    51,    34,    -1,    54,    -1,    38,    -1,    -1,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    49,    68,    51,
      -1,    32,    54,    34,    -1,    -1,    -1,    38,    -1,    61,
      62,    63,    -1,    -1,    -1,    -1,    68,    -1,    49,    32,
      51,    34,    -1,    54,    -1,    38,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    47,    -1,    -1,    68,    51,    -1,
      32,    54,    34,    -1,    -1,    -1,    38,    -1,    61,    62,
      63,    -1,    -1,    -1,    -1,    68,    -1,    -1,    32,    51,
      34,    -1,    54,    -1,    38,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    32,    -1,    34,    68,    51,    -1,    38,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,
      -1,    -1,    51,    -1,    68,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,    71,     1,     6,    10,    14,
      23,    26,    28,    29,    30,    32,    34,    36,    38,    43,
      45,    48,    51,    54,    61,    62,    63,    68,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   185,   186,   204,
     206,   208,   209,   210,   211,    34,    38,    51,   127,   132,
     207,    59,    72,    73,     0,   173,   173,    66,   154,    66,
      28,    29,    30,    48,    51,    54,    68,   166,   167,   184,
     204,   208,   211,   180,   181,   186,    31,    33,    35,    37,
      45,    49,    52,    53,    57,    58,    69,   173,   180,   187,
     205,   213,   215,   217,   218,   219,    56,   173,   187,   200,
      13,   153,    39,    31,    33,    35,    37,    45,    49,    58,
      69,   216,   217,   219,   221,   221,   186,    66,    42,    66,
      53,   132,   135,    51,    54,    68,   133,   134,   206,   207,
       1,    74,   207,    66,   222,     7,    11,    55,   182,   155,
     156,    15,    14,    55,   174,   194,   195,   196,   166,    28,
      31,    33,    37,    45,    49,    58,    69,   158,   159,   160,
     161,   162,   164,   165,   167,   168,   204,   208,    45,   158,
     159,   169,    47,   166,    42,    66,    53,    53,    53,    53,
      53,    53,    34,    36,    38,   206,    53,    53,    52,    53,
     221,    52,    53,    52,    53,   175,   176,   175,    41,    44,
      52,   201,    56,   154,     1,    51,   124,   126,   127,   128,
     129,   131,   132,   134,    45,   180,   181,    45,   180,   181,
      51,   197,   198,   199,   208,   209,   186,   197,    52,    53,
      52,    53,    47,    51,    53,   129,   132,   135,   136,   205,
      56,   128,   132,    51,    77,     8,     9,    16,    17,    18,
      19,    20,    21,    24,    25,    28,    51,    55,    60,    64,
      65,    76,    83,    85,    93,    94,   137,   140,   144,   145,
     146,   147,   160,   162,   164,   208,    76,    66,   173,    11,
      12,    55,   183,     1,    55,    67,   144,   157,   223,    55,
     223,   173,   154,   194,    46,   223,    55,    57,   219,     1,
     163,   165,   166,    52,    53,    39,   219,   219,   166,    52,
      53,   166,   219,    52,    56,   173,   166,   170,   171,   172,
     208,   209,    57,    57,    57,    57,   173,    53,   173,    53,
      53,   173,    14,   173,   202,   203,   173,    44,    27,    53,
     123,   124,   132,   135,    47,    50,    47,   133,    47,   180,
     181,   180,   181,    35,    45,    67,    52,    40,    67,   132,
     132,    53,    52,    53,    52,    53,    52,    53,    52,    53,
      53,    56,    34,    38,    51,    52,    53,    59,    78,    79,
     207,   209,   211,    13,     1,   127,   132,     1,    34,    95,
       1,    28,   138,     1,   138,     1,   138,     1,    51,   110,
     111,   208,     1,   127,   132,     1,    51,     1,   112,   127,
     132,     1,   113,   127,   132,    45,    57,    69,   212,   215,
     146,   147,   208,    76,     1,    30,    62,    75,   207,     1,
      28,    60,   208,    67,    55,    84,    55,    39,    52,     1,
      39,    40,    44,   148,   149,   150,   151,   166,   148,   212,
     212,    45,    69,   166,   214,   215,   223,    55,   158,   188,
     189,   190,   173,   173,    12,   173,   196,    45,   163,   158,
     128,    45,   163,    45,   163,   158,    45,   163,   158,    67,
      52,    40,   154,    46,    52,    41,   202,   125,   206,    53,
      53,    53,   128,   128,   128,   128,   199,   173,   128,   129,
     132,   129,   132,   128,    53,    75,    52,    53,    51,     1,
      66,    50,    40,    40,   206,    57,   139,   212,   218,   220,
     139,   139,    39,    52,    30,    50,    40,   114,   115,   128,
      44,   116,    50,    13,   152,    50,   160,    53,    53,    75,
      51,    61,    63,    86,   208,   208,    84,    85,    94,   144,
       1,    27,   120,   121,   122,   124,   127,   130,   131,   132,
     208,   128,   173,   202,   152,   151,   160,   160,   160,   160,
     188,     1,    44,    47,   191,   192,   193,   223,    55,   173,
     163,   163,   163,   163,   172,   158,   173,   203,   173,    58,
     206,    53,    79,    41,    52,    80,    81,    82,   209,   211,
      76,     1,    95,     1,    27,    49,    51,    98,    99,   101,
     102,   124,   131,   132,   210,   128,    52,   120,   111,    95,
      99,   104,    53,    52,     1,   117,   118,   119,   152,   132,
      66,   141,   132,   166,   166,    63,    86,    34,    52,    87,
      88,    89,   208,    51,    75,    30,   208,    30,    93,   125,
      47,    50,    47,    47,   148,    40,   202,   173,   152,   193,
     190,   126,    53,    53,    52,   223,    40,   125,   130,   131,
     132,    33,    22,    44,   107,    49,   124,   133,    57,   218,
     218,    49,   124,   218,    66,    15,   220,    40,   107,   128,
      52,    47,   206,   142,   143,    75,    51,    53,    52,    87,
      86,   208,    30,    39,   208,   208,    58,   122,   132,   122,
     122,   122,   173,    47,    82,    98,    58,   218,    53,    51,
     207,    99,   133,    49,   103,   124,   130,   103,   133,   103,
      67,   105,   106,   140,   208,    96,    97,   208,   104,   118,
     119,    55,   144,   223,    55,   223,    86,    41,    51,    52,
      90,    91,    92,   208,   210,    89,    53,    39,   208,   120,
      39,    39,   121,   173,   107,    51,   100,   101,   127,   132,
     103,   108,   109,   207,   130,    52,    67,    39,    52,    39,
     107,    53,    53,    52,   120,    39,   120,   120,    50,    53,
      52,   106,    49,   123,   128,    97,   120,    92,   120,   101,
     207,   128
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    70,    71,    71,    71,    71,    72,    72,    72,    72,
      72,    73,    74,    75,    75,    76,    76,    76,    76,    76,
      77,    77,    77,    77,    77,    78,    78,    79,    79,    79,
      79,    79,    80,    80,    80,    80,    81,    81,    82,    82,
      83,    83,    83,    84,    85,    85,    85,    85,    85,    86,
      86,    86,    87,    87,    87,    87,    88,    88,    89,    89,
      89,    89,    90,    90,    90,    90,    91,    91,    92,    92,
      93,    93,    93,    93,    93,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    95,    95,
      95,    96,    96,    97,    97,    98,    98,    99,    99,   100,
     100,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     102,   102,   102,   102,   102,   102,   103,   103,   103,   104,
     105,   105,   106,   106,   106,   107,   107,   107,   108,   108,
     109,   109,    94,   110,   110,   110,   111,   111,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,   112,
     112,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     117,   118,   118,   119,   119,   120,   120,   121,   121,   122,
     122,   122,   122,   123,   123,   124,   125,   125,   126,   126,
     127,   127,   127,   127,   128,   128,   129,   129,   129,   129,
     129,   130,   130,   131,   131,   132,   132,   133,   133,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     135,   135,   136,   136,   136,   136,   137,   137,   137,   137,
     137,   137,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   142,   143,   144,   144,   144,   144,
     145,   145,   146,   146,   146,   146,   146,   147,   147,   147,
     147,   148,   148,   149,   149,   150,   150,   151,   152,   152,
     153,   153,   154,   154,   155,   155,   155,   156,   157,   158,
     159,   159,   160,   160,   160,   161,   161,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   163,   163,   164,
     164,   165,   165,   166,   166,   166,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   168,   168,   169,   169,
     170,   170,   171,   171,   172,   172,   173,   173,   174,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   178,   178,
     178,   179,   179,   179,   179,   179,   180,   180,   180,   181,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   186,   187,   187,   188,
     188,   189,   189,   189,   190,   191,   191,   191,   192,   192,
     193,   194,   194,   195,   195,   195,   196,   196,   196,   197,
     197,   198,   198,   199,   199,   200,   200,   200,   200,   200,
     200,   200,   201,   201,   202,   202,   203,   203,   203,   204,
     204,   204,   204,   205,   205,   206,   206,   206,   206,   207,
     207,   208,   208,   208,   208,   208,   208,   209,   209,   209,
     210,   210,   211,   211,   211,   212,   212,   212,   213,   213,
     214,   214,   215,   215,   215,   215,   216,   216,   217,   217,
     217,   218,   218,   219,   219,   219,   220,   220,   221,   221,
     222,   223,   223
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     2,     2,     1,     4,     4,     7,     5,
       2,     0,     1,     1,     1,     0,     2,     1,     2,     4,
       0,     2,     3,     3,     4,     3,     1,     1,     1,     4,
       4,     2,     0,     1,     1,     2,     3,     1,     1,     1,
       3,     2,     1,     0,     3,     5,     6,     4,     2,     0,
       4,     3,     0,     1,     1,     2,     3,     1,     1,     1,
       4,     4,     0,     1,     1,     2,     3,     1,     1,     1,
       2,     3,     3,     1,     1,     4,     6,     2,     5,     7,
       2,     4,     2,     5,     7,     2,     2,     2,     2,     1,
       1,     3,     1,     3,     1,     3,     1,     4,     1,     3,
       1,     4,     3,     3,     3,     1,     1,     4,     3,     1,
       3,     2,     2,     3,     2,     3,     2,     1,     1,     1,
       3,     1,     3,     3,     4,     0,     2,     4,     0,     1,
       3,     1,     4,     3,     1,     1,     2,     1,     7,     6,
       8,     7,     7,     4,     3,     4,     2,     2,     2,     3,
       1,     3,     1,     0,     1,     3,     1,     0,     2,     3,
       1,     3,     1,     0,     2,     4,     1,     3,     1,     3,
       3,     3,     1,     4,     1,     3,     2,     1,     3,     1,
       2,     1,     3,     3,     1,     1,     1,     3,     3,     3,
       1,     1,     1,     2,     1,     2,     1,     1,     1,     1,
       2,     3,     3,     3,     3,     3,     3,     3,     2,     1,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     2,
       3,     2,     3,     3,     1,     0,     3,     1,     3,     1,
       3,     3,     0,     2,     2,     2,     1,     2,     4,     2,
       1,     1,     3,     3,     3,     3,     3,     4,     4,     2,
       2,     2,     1,     2,     1,     2,     1,     4,     0,     2,
       0,     2,     3,     3,     0,     2,     2,     2,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       4,     3,     4,     3,     4,     3,     4,     1,     1,     1,
       1,     2,     2,     1,     1,     1,     3,     1,     4,     1,
       1,     1,     3,     3,     3,     2,     3,     3,     3,     1,
       0,     1,     3,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     4,     3,     2,     4,
       3,     4,     3,     2,     4,     3,     6,     4,     1,     4,
       4,     4,     3,     2,     3,     2,     2,     1,     2,     1,
       1,     3,     2,     1,     1,     4,     4,     1,     1,     1,
       1,     3,     3,     3,     4,     4,     4,     3,     3,     1,
       2,     3,     2,     1,     3,     1,     2,     1,     2,     1,
       4,     1,     2,     3,     2,     1,     3,     2,     1,     0,
       1,     3,     1,     1,     3,     1,     1,     2,     3,     4,
       2,     5,     3,     2,     3,     1,     3,     1,     2,     1,
       2,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     1,     3,     1,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       0,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: EXPR exp lwherePart  */
#line 104 "parser.y"
                                        {inputExpr = letrec(yyvsp[0],yyvsp[-1]); sp-=2;}
#line 2614 "parser.c"
    break;

  case 3: /* start: CTXT context  */
#line 105 "parser.y"
                                        {inputContext = yyvsp[0];	    sp-=1;}
#line 2620 "parser.c"
    break;

  case 4: /* start: SCRIPT topModule  */
#line 106 "parser.y"
                                        {valDefns  = yyvsp[0];	    sp-=1;}
#line 2626 "parser.c"
    break;

  case 5: /* start: error  */
#line 107 "parser.y"
                                        {syntaxError("input");}
#line 2632 "parser.c"
    break;

  case 6: /* topModule: startMain begin modBody end  */
#line 120 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc3(yyvsp[-1]);
					}
#line 2641 "parser.c"
    break;

  case 7: /* topModule: startMain '{' modBody '}'  */
#line 124 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc4(yyvsp[-1]);
					}
#line 2650 "parser.c"
    break;

  case 8: /* topModule: TMODULE modname expspec WHERE '{' modBody end  */
#line 129 "parser.y"
                                        {setExportList(yyvsp[-4]);   yyval = gc7(yyvsp[-1]);}
#line 2656 "parser.c"
    break;

  case 9: /* topModule: TMODULE modname expspec WHERE error  */
#line 131 "parser.y"
                                        {syntaxError("declaration");}
#line 2662 "parser.c"
    break;

  case 10: /* topModule: TMODULE error  */
#line 132 "parser.y"
                                        {syntaxError("module definition");}
#line 2668 "parser.c"
    break;

  case 11: /* startMain: %empty  */
#line 138 "parser.y"
                                        {startModule(conMain); 
					 yyval = gc0(NIL);}
#line 2675 "parser.c"
    break;

  case 12: /* modname: qconid  */
#line 141 "parser.y"
                                        {startModule(mkCon(mkNestedQual(yyvsp[0]))); yyval = gc1(NIL);}
#line 2681 "parser.c"
    break;

  case 13: /* modid: qconid  */
#line 143 "parser.y"
                                        {yyval = mkCon(mkNestedQual(yyvsp[0]));}
#line 2687 "parser.c"
    break;

  case 14: /* modid: STRINGLIT  */
#line 144 "parser.y"
                                        { String modName = findPathname(textToStr(textOf(yyvsp[0])));
					  if (modName) { /* fillin pathname if known */
					      yyval = mkStr(findText(modName));
					  } else {
					      yyval = yyvsp[0];
					  }
					}
#line 2699 "parser.c"
    break;

  case 15: /* modBody: %empty  */
#line 152 "parser.y"
                                        {yyval = gc0(NIL); }
#line 2705 "parser.c"
    break;

  case 16: /* modBody: ';' modBody  */
#line 153 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 2711 "parser.c"
    break;

  case 17: /* modBody: topDecls  */
#line 154 "parser.y"
                                        {yyval = gc1(yyvsp[0]);}
#line 2717 "parser.c"
    break;

  case 18: /* modBody: impDecls chase  */
#line 155 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2723 "parser.c"
    break;

  case 19: /* modBody: impDecls ';' chase topDecls  */
#line 156 "parser.y"
                                        {yyval = gc4(yyvsp[0]);}
#line 2729 "parser.c"
    break;

  case 20: /* expspec: %empty  */
#line 161 "parser.y"
                                        {yyval = gc0(exportSelf());}
#line 2735 "parser.c"
    break;

  case 21: /* expspec: '(' ')'  */
#line 162 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2741 "parser.c"
    break;

  case 22: /* expspec: '(' ',' ')'  */
#line 163 "parser.y"
                                        {yyval = gc3(NIL);}
#line 2747 "parser.c"
    break;

  case 23: /* expspec: '(' exports ')'  */
#line 164 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2753 "parser.c"
    break;

  case 24: /* expspec: '(' exports ',' ')'  */
#line 165 "parser.y"
                                        {yyval = gc4(yyvsp[-2]);}
#line 2759 "parser.c"
    break;

  case 25: /* exports: exports ',' export  */
#line 167 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2765 "parser.c"
    break;

  case 26: /* exports: export  */
#line 168 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2771 "parser.c"
    break;

  case 27: /* export: qvar  */
#line 173 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2777 "parser.c"
    break;

  case 28: /* export: qcon  */
#line 174 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2783 "parser.c"
    break;

  case 29: /* export: qconid '(' UPTO ')'  */
#line 175 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 2789 "parser.c"
    break;

  case 30: /* export: qconid '(' qnames ')'  */
#line 176 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 2795 "parser.c"
    break;

  case 31: /* export: TMODULE modid  */
#line 177 "parser.y"
                                        {yyval = gc2(ap(MODULEENT,yyvsp[0]));}
#line 2801 "parser.c"
    break;

  case 32: /* qnames: %empty  */
#line 179 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2807 "parser.c"
    break;

  case 33: /* qnames: ','  */
#line 180 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2813 "parser.c"
    break;

  case 34: /* qnames: qnames1  */
#line 181 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2819 "parser.c"
    break;

  case 35: /* qnames: qnames1 ','  */
#line 182 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2825 "parser.c"
    break;

  case 36: /* qnames1: qnames1 ',' qname  */
#line 184 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2831 "parser.c"
    break;

  case 37: /* qnames1: qname  */
#line 185 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2837 "parser.c"
    break;

  case 38: /* qname: qvar  */
#line 187 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2843 "parser.c"
    break;

  case 39: /* qname: qcon  */
#line 188 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2849 "parser.c"
    break;

  case 40: /* impDecls: impDecls ';' impDecl  */
#line 193 "parser.y"
                                        {imps = cons(yyvsp[0],imps); yyval=gc3(NIL);}
#line 2855 "parser.c"
    break;

  case 41: /* impDecls: impDecls ';'  */
#line 194 "parser.y"
                                        {yyval   = gc2(NIL); }
#line 2861 "parser.c"
    break;

  case 42: /* impDecls: impDecl  */
#line 195 "parser.y"
                                        {imps = singleton(yyvsp[0]); yyval=gc1(NIL);}
#line 2867 "parser.c"
    break;

  case 43: /* chase: %empty  */
#line 197 "parser.y"
                                        {if (chase(imps)) {
					     clearStack();
					     onto(imps);
					     done();
					     closeAnyInput();
					     return 0;
					 }
					 yyval = gc0(NIL);
					}
#line 2881 "parser.c"
    break;

  case 44: /* impDecl: IMPORT modid impspec  */
#line 208 "parser.y"
                                        {addUnqualImport(yyvsp[-1],NIL,yyvsp[0]);
					 yyval = gc3(yyvsp[-1]);}
#line 2888 "parser.c"
    break;

  case 45: /* impDecl: IMPORT modid ASMOD modid impspec  */
#line 211 "parser.y"
                                        {addUnqualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc5(yyvsp[-3]);}
#line 2895 "parser.c"
    break;

  case 46: /* impDecl: IMPORT QUALIFIED modid ASMOD modid impspec  */
#line 214 "parser.y"
                                        {addQualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc6(yyvsp[-3]);}
#line 2902 "parser.c"
    break;

  case 47: /* impDecl: IMPORT QUALIFIED modid impspec  */
#line 217 "parser.y"
                                        {addQualImport(yyvsp[-1],yyvsp[-1],yyvsp[0]);
					 yyval = gc4(yyvsp[-1]);}
#line 2909 "parser.c"
    break;

  case 48: /* impDecl: IMPORT error  */
#line 219 "parser.y"
                                        {syntaxError("import declaration");}
#line 2915 "parser.c"
    break;

  case 49: /* impspec: %empty  */
#line 221 "parser.y"
                                        {yyval = gc0(DOTDOT);}
#line 2921 "parser.c"
    break;

  case 50: /* impspec: HIDING '(' imports ')'  */
#line 222 "parser.y"
                                        {yyval = gc4(ap(HIDDEN,yyvsp[-1]));}
#line 2927 "parser.c"
    break;

  case 51: /* impspec: '(' imports ')'  */
#line 223 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2933 "parser.c"
    break;

  case 52: /* imports: %empty  */
#line 225 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2939 "parser.c"
    break;

  case 53: /* imports: ','  */
#line 226 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2945 "parser.c"
    break;

  case 54: /* imports: imports1  */
#line 227 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2951 "parser.c"
    break;

  case 55: /* imports: imports1 ','  */
#line 228 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2957 "parser.c"
    break;

  case 56: /* imports1: imports1 ',' import  */
#line 230 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2963 "parser.c"
    break;

  case 57: /* imports1: import  */
#line 231 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2969 "parser.c"
    break;

  case 58: /* import: var  */
#line 233 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2975 "parser.c"
    break;

  case 59: /* import: CONID  */
#line 234 "parser.y"
                                        {yyval = gc1(pair(yyvsp[0],NONE));}
#line 2981 "parser.c"
    break;

  case 60: /* import: CONID '(' UPTO ')'  */
#line 235 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 2987 "parser.c"
    break;

  case 61: /* import: CONID '(' names ')'  */
#line 236 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 2993 "parser.c"
    break;

  case 62: /* names: %empty  */
#line 238 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2999 "parser.c"
    break;

  case 63: /* names: ','  */
#line 239 "parser.y"
                                        {yyval = gc1(NIL);}
#line 3005 "parser.c"
    break;

  case 64: /* names: names1  */
#line 240 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3011 "parser.c"
    break;

  case 65: /* names: names1 ','  */
#line 241 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3017 "parser.c"
    break;

  case 66: /* names1: names1 ',' name  */
#line 243 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3023 "parser.c"
    break;

  case 67: /* names1: name  */
#line 244 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3029 "parser.c"
    break;

  case 68: /* name: var  */
#line 246 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3035 "parser.c"
    break;

  case 69: /* name: con  */
#line 247 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3041 "parser.c"
    break;

  case 70: /* topDecls: topDecls ';'  */
#line 252 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3047 "parser.c"
    break;

  case 71: /* topDecls: topDecls ';' topDecl  */
#line 253 "parser.y"
                                        {yyval = gc2(yyvsp[-2]);}
#line 3053 "parser.c"
    break;

  case 72: /* topDecls: topDecls ';' decl  */
#line 254 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3059 "parser.c"
    break;

  case 73: /* topDecls: topDecl  */
#line 255 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3065 "parser.c"
    break;

  case 74: /* topDecls: decl  */
#line 256 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3071 "parser.c"
    break;

  case 75: /* topDecl: TYPE tyLhs '=' type  */
#line 261 "parser.y"
                                        {defTycon(4,yyvsp[-1],yyvsp[-2],yyvsp[0],SYNONYM);}
#line 3077 "parser.c"
    break;

  case 76: /* topDecl: TYPE tyLhs '=' type IN invars  */
#line 263 "parser.y"
                                        {defTycon(6,yyvsp[-3],yyvsp[-4],
						    ap(yyvsp[-2],yyvsp[0]),RESTRICTSYN);}
#line 3084 "parser.c"
    break;

  case 77: /* topDecl: TYPE error  */
#line 265 "parser.y"
                                        {syntaxError("type declaration");}
#line 3090 "parser.c"
    break;

  case 78: /* topDecl: DATA btype2 '=' constrs deriving  */
#line 267 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(rev(yyvsp[-1]),yyvsp[0]),DATATYPE);}
#line 3097 "parser.c"
    break;

  case 79: /* topDecl: DATA context IMPLIES tyLhs '=' constrs deriving  */
#line 270 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],rev(yyvsp[-1])),
						     yyvsp[0]),DATATYPE);}
#line 3105 "parser.c"
    break;

  case 80: /* topDecl: DATA btype2  */
#line 273 "parser.y"
                                        {defTycon(2,yyvsp[-1],checkTyLhs(yyvsp[0]),
						    ap(NIL,NIL),DATATYPE);}
#line 3112 "parser.c"
    break;

  case 81: /* topDecl: DATA context IMPLIES tyLhs  */
#line 275 "parser.y"
                                        {defTycon(4,yyvsp[-3],yyvsp[0],
						  ap(qualify(yyvsp[-2],NIL),
						     NIL),DATATYPE);}
#line 3120 "parser.c"
    break;

  case 82: /* topDecl: DATA error  */
#line 278 "parser.y"
                                        {syntaxError("data declaration");}
#line 3126 "parser.c"
    break;

  case 83: /* topDecl: TNEWTYPE btype2 '=' nconstr deriving  */
#line 280 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(yyvsp[-1],yyvsp[0]),NEWTYPE);}
#line 3133 "parser.c"
    break;

  case 84: /* topDecl: TNEWTYPE context IMPLIES tyLhs '=' nconstr deriving  */
#line 283 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],yyvsp[-1]),
						     yyvsp[0]),NEWTYPE);}
#line 3141 "parser.c"
    break;

  case 85: /* topDecl: TNEWTYPE error  */
#line 286 "parser.y"
                                        {syntaxError("newtype declaration");}
#line 3147 "parser.c"
    break;

  case 86: /* topDecl: NEEDPRIMS NUMLIT  */
#line 287 "parser.y"
                                        {if (isInt(yyvsp[0])) {
					     needPrims(intOf(yyvsp[0]), NULL);
					 } else {
					     syntaxError("needprims decl");
					 }
					 sp-=2;}
#line 3158 "parser.c"
    break;

  case 87: /* topDecl: NEEDPRIMS error  */
#line 293 "parser.y"
                                        {syntaxError("needprims decl");}
#line 3164 "parser.c"
    break;

  case 88: /* tyLhs: tyLhs varid  */
#line 295 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3170 "parser.c"
    break;

  case 89: /* tyLhs: CONID  */
#line 296 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3176 "parser.c"
    break;

  case 90: /* tyLhs: error  */
#line 297 "parser.y"
                                        {syntaxError("type defn lhs");}
#line 3182 "parser.c"
    break;

  case 91: /* invars: invars ',' invar  */
#line 299 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3188 "parser.c"
    break;

  case 92: /* invars: invar  */
#line 300 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3194 "parser.c"
    break;

  case 93: /* invar: var COCO topType  */
#line 302 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],singleton(yyvsp[-2]),
									yyvsp[0]));}
#line 3201 "parser.c"
    break;

  case 94: /* invar: var  */
#line 304 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3207 "parser.c"
    break;

  case 95: /* constrs: constrs '|' pconstr  */
#line 306 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3213 "parser.c"
    break;

  case 96: /* constrs: pconstr  */
#line 307 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3219 "parser.c"
    break;

  case 97: /* pconstr: ALL varids '.' qconstr  */
#line 309 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3226 "parser.c"
    break;

  case 98: /* pconstr: constr  */
#line 311 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3232 "parser.c"
    break;

  case 99: /* qconstr: context IMPLIES constr  */
#line 313 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3238 "parser.c"
    break;

  case 100: /* qconstr: constr  */
#line 314 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3244 "parser.c"
    break;

  case 101: /* constr: '!' btype conop bbtype  */
#line 316 "parser.y"
                                        {yyval = gc4(ap(ap(yyvsp[-1],bang(yyvsp[-2])),yyvsp[0]));}
#line 3250 "parser.c"
    break;

  case 102: /* constr: btype1 conop bbtype  */
#line 317 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3256 "parser.c"
    break;

  case 103: /* constr: btype2 conop bbtype  */
#line 318 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3262 "parser.c"
    break;

  case 104: /* constr: bpolyType conop bbtype  */
#line 319 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3268 "parser.c"
    break;

  case 105: /* constr: btype2  */
#line 320 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3274 "parser.c"
    break;

  case 106: /* constr: btype3  */
#line 321 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3280 "parser.c"
    break;

  case 107: /* constr: con '{' fieldspecs '}'  */
#line 322 "parser.y"
                                        {yyval = gc4(ap(LABC,pair(yyvsp[-3],rev(yyvsp[-1]))));}
#line 3286 "parser.c"
    break;

  case 108: /* constr: con '{' '}'  */
#line 323 "parser.y"
                                        {yyval = gc3(ap(LABC,pair(yyvsp[-2],NIL)));}
#line 3292 "parser.c"
    break;

  case 109: /* constr: error  */
#line 324 "parser.y"
                                        {syntaxError("data type declaration");}
#line 3298 "parser.c"
    break;

  case 110: /* btype3: btype2 '!' atype  */
#line 326 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3304 "parser.c"
    break;

  case 111: /* btype3: btype2 bpolyType  */
#line 327 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3310 "parser.c"
    break;

  case 112: /* btype3: btype3 atype  */
#line 328 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3316 "parser.c"
    break;

  case 113: /* btype3: btype3 '!' atype  */
#line 329 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3322 "parser.c"
    break;

  case 114: /* btype3: btype3 bpolyType  */
#line 330 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3328 "parser.c"
    break;

  case 115: /* btype3: '(' CONOP ')'  */
#line 331 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3334 "parser.c"
    break;

  case 116: /* bbtype: '!' btype  */
#line 333 "parser.y"
                                        {yyval = gc2(bang(yyvsp[0]));}
#line 3340 "parser.c"
    break;

  case 117: /* bbtype: btype  */
#line 334 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3346 "parser.c"
    break;

  case 118: /* bbtype: bpolyType  */
#line 335 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3352 "parser.c"
    break;

  case 119: /* nconstr: pconstr  */
#line 337 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3358 "parser.c"
    break;

  case 120: /* fieldspecs: fieldspecs ',' fieldspec  */
#line 339 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3364 "parser.c"
    break;

  case 121: /* fieldspecs: fieldspec  */
#line 340 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3370 "parser.c"
    break;

  case 122: /* fieldspec: vars COCO polyType  */
#line 342 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3376 "parser.c"
    break;

  case 123: /* fieldspec: vars COCO type  */
#line 343 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3382 "parser.c"
    break;

  case 124: /* fieldspec: vars COCO '!' type  */
#line 344 "parser.y"
                                        {yyval = gc4(pair(rev(yyvsp[-3]),bang(yyvsp[0])));}
#line 3388 "parser.c"
    break;

  case 125: /* deriving: %empty  */
#line 346 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3394 "parser.c"
    break;

  case 126: /* deriving: DERIVING qconid  */
#line 347 "parser.y"
                                        {yyval = gc2(singleton(yyvsp[0]));}
#line 3400 "parser.c"
    break;

  case 127: /* deriving: DERIVING '(' derivs0 ')'  */
#line 348 "parser.y"
                                        {yyval = gc4(yyvsp[-1]);}
#line 3406 "parser.c"
    break;

  case 128: /* derivs0: %empty  */
#line 350 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3412 "parser.c"
    break;

  case 129: /* derivs0: derivs  */
#line 351 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3418 "parser.c"
    break;

  case 130: /* derivs: derivs ',' qconid  */
#line 353 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3424 "parser.c"
    break;

  case 131: /* derivs: qconid  */
#line 354 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3430 "parser.c"
    break;

  case 132: /* topDecl: PRIMITIVE prims COCO topType  */
#line 359 "parser.y"
                                        {primDefn(yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=4;}
#line 3436 "parser.c"
    break;

  case 133: /* prims: prims ',' prim  */
#line 361 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3442 "parser.c"
    break;

  case 134: /* prims: prim  */
#line 362 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3448 "parser.c"
    break;

  case 135: /* prims: error  */
#line 363 "parser.y"
                                        {syntaxError("primitive defn");}
#line 3454 "parser.c"
    break;

  case 136: /* prim: var STRINGLIT  */
#line 365 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 3460 "parser.c"
    break;

  case 137: /* prim: var  */
#line 366 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3466 "parser.c"
    break;

  case 138: /* topDecl: FOREIGN IMPORT var STRINGLIT var COCO topType  */
#line 372 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],NIL,yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3472 "parser.c"
    break;

  case 139: /* topDecl: FOREIGN IMPORT var var COCO topType  */
#line 374 "parser.y"
               {foreignImport(yyvsp[-5],yyvsp[-3],NIL,yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=6;}
#line 3478 "parser.c"
    break;

  case 140: /* topDecl: FOREIGN IMPORT var var STRINGLIT var COCO topType  */
#line 376 "parser.y"
               {foreignImport(yyvsp[-7],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=8;}
#line 3484 "parser.c"
    break;

  case 141: /* topDecl: FOREIGN IMPORT var var var COCO topType  */
#line 378 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3490 "parser.c"
    break;

  case 142: /* topDecl: FOREIGN var var STRINGLIT var COCO topType  */
#line 380 "parser.y"
               {foreignExport(yyvsp[-6],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3496 "parser.c"
    break;

  case 143: /* topDecl: TCLASS crule fds wherePart  */
#line 385 "parser.y"
                                        {classDefn(intOf(yyvsp[-3]),yyvsp[-2],yyvsp[0],yyvsp[-1]); sp-=4;}
#line 3502 "parser.c"
    break;

  case 144: /* topDecl: TINSTANCE irule wherePart  */
#line 386 "parser.y"
                                        {instDefn(intOf(yyvsp[-2]),yyvsp[-1],yyvsp[0]);  sp-=3;}
#line 3508 "parser.c"
    break;

  case 145: /* topDecl: DEFAULT '(' dtypes ')'  */
#line 387 "parser.y"
                                        {defaultDefn(intOf(yyvsp[-3]),yyvsp[-1]);  sp-=4;}
#line 3514 "parser.c"
    break;

  case 146: /* topDecl: TCLASS error  */
#line 388 "parser.y"
                                        {syntaxError("class declaration");}
#line 3520 "parser.c"
    break;

  case 147: /* topDecl: TINSTANCE error  */
#line 389 "parser.y"
                                        {syntaxError("instance declaration");}
#line 3526 "parser.c"
    break;

  case 148: /* topDecl: DEFAULT error  */
#line 390 "parser.y"
                                        {syntaxError("default declaration");}
#line 3532 "parser.c"
    break;

  case 149: /* crule: context IMPLIES btype2  */
#line 392 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3538 "parser.c"
    break;

  case 150: /* crule: btype2  */
#line 393 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3544 "parser.c"
    break;

  case 151: /* irule: context IMPLIES btype2  */
#line 395 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3550 "parser.c"
    break;

  case 152: /* irule: btype2  */
#line 396 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3556 "parser.c"
    break;

  case 153: /* dtypes: %empty  */
#line 398 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3562 "parser.c"
    break;

  case 154: /* dtypes: dtypes1  */
#line 399 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3568 "parser.c"
    break;

  case 155: /* dtypes1: dtypes1 ',' type  */
#line 401 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3574 "parser.c"
    break;

  case 156: /* dtypes1: type  */
#line 402 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3580 "parser.c"
    break;

  case 157: /* fds: %empty  */
#line 404 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3586 "parser.c"
    break;

  case 158: /* fds: '|' fds1  */
#line 405 "parser.y"
                                        {yyval = gc2(rev(yyvsp[0]));}
#line 3592 "parser.c"
    break;

  case 159: /* fds1: fds1 ',' fd  */
#line 407 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3598 "parser.c"
    break;

  case 160: /* fds1: fd  */
#line 408 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3604 "parser.c"
    break;

  case 161: /* fd: varids0 ARROW varids0  */
#line 410 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),rev(yyvsp[0])));}
#line 3610 "parser.c"
    break;

  case 162: /* fd: error  */
#line 411 "parser.y"
                                        {syntaxError("functional dependency");}
#line 3616 "parser.c"
    break;

  case 163: /* varids0: %empty  */
#line 413 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3622 "parser.c"
    break;

  case 164: /* varids0: varids0 varid  */
#line 414 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3628 "parser.c"
    break;

  case 165: /* topType: ALL varids '.' topType0  */
#line 419 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3635 "parser.c"
    break;

  case 166: /* topType: topType0  */
#line 421 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3641 "parser.c"
    break;

  case 167: /* topType0: context IMPLIES topType1  */
#line 423 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3647 "parser.c"
    break;

  case 168: /* topType0: topType1  */
#line 424 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3653 "parser.c"
    break;

  case 169: /* topType1: bpolyType ARROW topType1  */
#line 426 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3659 "parser.c"
    break;

  case 170: /* topType1: btype1 ARROW topType1  */
#line 427 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3665 "parser.c"
    break;

  case 171: /* topType1: btype2 ARROW topType1  */
#line 428 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3671 "parser.c"
    break;

  case 172: /* topType1: btype  */
#line 429 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3677 "parser.c"
    break;

  case 173: /* polyType: ALL varids '.' sigType  */
#line 431 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3684 "parser.c"
    break;

  case 174: /* polyType: bpolyType  */
#line 433 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3690 "parser.c"
    break;

  case 175: /* bpolyType: '(' polyType ')'  */
#line 435 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3696 "parser.c"
    break;

  case 176: /* varids: varids varid  */
#line 437 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3702 "parser.c"
    break;

  case 177: /* varids: varid  */
#line 438 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3708 "parser.c"
    break;

  case 178: /* sigType: context IMPLIES type  */
#line 440 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3714 "parser.c"
    break;

  case 179: /* sigType: type  */
#line 441 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3720 "parser.c"
    break;

  case 180: /* context: '(' ')'  */
#line 443 "parser.y"
                                        {yyval = gc2(NIL);}
#line 3726 "parser.c"
    break;

  case 181: /* context: btype2  */
#line 444 "parser.y"
                                        {yyval = gc1(singleton(checkPred(yyvsp[0])));}
#line 3732 "parser.c"
    break;

  case 182: /* context: '(' btype2 ')'  */
#line 445 "parser.y"
                                        {yyval = gc3(singleton(checkPred(yyvsp[-1])));}
#line 3738 "parser.c"
    break;

  case 183: /* context: '(' btypes2 ')'  */
#line 446 "parser.y"
                                        {yyval = gc3(checkCtxt(rev(yyvsp[-1])));}
#line 3744 "parser.c"
    break;

  case 184: /* type: type1  */
#line 448 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3750 "parser.c"
    break;

  case 185: /* type: btype2  */
#line 449 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3756 "parser.c"
    break;

  case 186: /* type1: btype1  */
#line 451 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3762 "parser.c"
    break;

  case 187: /* type1: bpolyType ARROW type  */
#line 452 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3768 "parser.c"
    break;

  case 188: /* type1: btype1 ARROW type  */
#line 453 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3774 "parser.c"
    break;

  case 189: /* type1: btype2 ARROW type  */
#line 454 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3780 "parser.c"
    break;

  case 190: /* type1: error  */
#line 455 "parser.y"
                                        {syntaxError("type expression");}
#line 3786 "parser.c"
    break;

  case 191: /* btype: btype1  */
#line 457 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3792 "parser.c"
    break;

  case 192: /* btype: btype2  */
#line 458 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3798 "parser.c"
    break;

  case 193: /* btype1: btype1 atype  */
#line 460 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3804 "parser.c"
    break;

  case 194: /* btype1: atype1  */
#line 461 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3810 "parser.c"
    break;

  case 195: /* btype2: btype2 atype  */
#line 463 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3816 "parser.c"
    break;

  case 196: /* btype2: qconid  */
#line 464 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3822 "parser.c"
    break;

  case 197: /* atype: atype1  */
#line 466 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3828 "parser.c"
    break;

  case 198: /* atype: qconid  */
#line 467 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3834 "parser.c"
    break;

  case 199: /* atype1: varid  */
#line 469 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3840 "parser.c"
    break;

  case 200: /* atype1: '(' ')'  */
#line 470 "parser.y"
                                        {yyval = gc2(typeUnit);}
#line 3846 "parser.c"
    break;

  case 201: /* atype1: '(' ARROW ')'  */
#line 471 "parser.y"
                                        {yyval = gc3(typeArrow);}
#line 3852 "parser.c"
    break;

  case 202: /* atype1: '(' type1 ')'  */
#line 472 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3858 "parser.c"
    break;

  case 203: /* atype1: '(' btype2 ')'  */
#line 473 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3864 "parser.c"
    break;

  case 204: /* atype1: '(' tupCommas ')'  */
#line 474 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3870 "parser.c"
    break;

  case 205: /* atype1: '(' btypes2 ')'  */
#line 475 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3876 "parser.c"
    break;

  case 206: /* atype1: '(' typeTuple ')'  */
#line 476 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3882 "parser.c"
    break;

  case 207: /* atype1: '[' type ']'  */
#line 477 "parser.y"
                                        {yyval = gc3(ap(typeList,yyvsp[-1]));}
#line 3888 "parser.c"
    break;

  case 208: /* atype1: '[' ']'  */
#line 478 "parser.y"
                                        {yyval = gc2(typeList);}
#line 3894 "parser.c"
    break;

  case 209: /* atype1: '_'  */
#line 479 "parser.y"
                                        {yyval = gc1(inventVar());}
#line 3900 "parser.c"
    break;

  case 210: /* btypes2: btypes2 ',' btype2  */
#line 481 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3906 "parser.c"
    break;

  case 211: /* btypes2: btype2 ',' btype2  */
#line 482 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3912 "parser.c"
    break;

  case 212: /* typeTuple: type1 ',' type  */
#line 484 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3918 "parser.c"
    break;

  case 213: /* typeTuple: btype2 ',' type1  */
#line 485 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3924 "parser.c"
    break;

  case 214: /* typeTuple: btypes2 ',' type1  */
#line 486 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3930 "parser.c"
    break;

  case 215: /* typeTuple: typeTuple ',' type  */
#line 487 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3936 "parser.c"
    break;

  case 216: /* gendecl: INFIXN optDigit ops  */
#line 492 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],NON_ASS,yyvsp[-1]));}
#line 3942 "parser.c"
    break;

  case 217: /* gendecl: INFIXN error  */
#line 493 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3948 "parser.c"
    break;

  case 218: /* gendecl: INFIXL optDigit ops  */
#line 494 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],LEFT_ASS,yyvsp[-1]));}
#line 3954 "parser.c"
    break;

  case 219: /* gendecl: INFIXL error  */
#line 495 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3960 "parser.c"
    break;

  case 220: /* gendecl: INFIXR optDigit ops  */
#line 496 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],RIGHT_ASS,yyvsp[-1]));}
#line 3966 "parser.c"
    break;

  case 221: /* gendecl: INFIXR error  */
#line 497 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3972 "parser.c"
    break;

  case 222: /* gendecl: vars COCO topType  */
#line 498 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 3978 "parser.c"
    break;

  case 223: /* gendecl: vars COCO error  */
#line 499 "parser.y"
                                        {syntaxError("type signature");}
#line 3984 "parser.c"
    break;

  case 224: /* optDigit: NUMLIT  */
#line 501 "parser.y"
                                        {yyval = gc1(checkPrec(yyvsp[0]));}
#line 3990 "parser.c"
    break;

  case 225: /* optDigit: %empty  */
#line 502 "parser.y"
                                        {yyval = gc0(mkInt(DEF_PREC));}
#line 3996 "parser.c"
    break;

  case 226: /* ops: ops ',' op  */
#line 504 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4002 "parser.c"
    break;

  case 227: /* ops: op  */
#line 505 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4008 "parser.c"
    break;

  case 228: /* vars: vars ',' var  */
#line 507 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4014 "parser.c"
    break;

  case 229: /* vars: var  */
#line 508 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4020 "parser.c"
    break;

  case 230: /* decls: '{' decls0 end  */
#line 510 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4026 "parser.c"
    break;

  case 231: /* decls: '{' decls1 end  */
#line 511 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4032 "parser.c"
    break;

  case 232: /* decls0: %empty  */
#line 513 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4038 "parser.c"
    break;

  case 233: /* decls0: decls0 ';'  */
#line 514 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4044 "parser.c"
    break;

  case 234: /* decls0: decls1 ';'  */
#line 515 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4050 "parser.c"
    break;

  case 235: /* decls1: decls0 decl  */
#line 517 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4056 "parser.c"
    break;

  case 236: /* decl: gendecl  */
#line 519 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4062 "parser.c"
    break;

  case 237: /* decl: funlhs rhs  */
#line 520 "parser.y"
                                        {yyval = gc2(ap(FUNBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4068 "parser.c"
    break;

  case 238: /* decl: funlhs COCO type rhs  */
#line 521 "parser.y"
                                        {yyval = gc4(ap(FUNBIND,
						     pair(yyvsp[-3],ap(RSIGN,
								ap(yyvsp[0],yyvsp[-1])))));}
#line 4076 "parser.c"
    break;

  case 239: /* decl: pat0 rhs  */
#line 524 "parser.y"
                                        {yyval = gc2(ap(PATBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4082 "parser.c"
    break;

  case 240: /* funlhs: funlhs0  */
#line 526 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4088 "parser.c"
    break;

  case 241: /* funlhs: funlhs1  */
#line 527 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4094 "parser.c"
    break;

  case 242: /* funlhs0: pat10_vI varop pat0  */
#line 529 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4100 "parser.c"
    break;

  case 243: /* funlhs0: var '+' pat0  */
#line 530 "parser.y"
                                        {yyval = gc3(ap2(varPlus,yyvsp[-2],yyvsp[0]));}
#line 4106 "parser.c"
    break;

  case 244: /* funlhs0: infixPat varop pat0  */
#line 531 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4112 "parser.c"
    break;

  case 245: /* funlhs0: NUMLIT varop pat0  */
#line 532 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4118 "parser.c"
    break;

  case 246: /* funlhs0: var varop_pl pat0  */
#line 533 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4124 "parser.c"
    break;

  case 247: /* funlhs1: '(' funlhs0 ')' apat  */
#line 535 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4130 "parser.c"
    break;

  case 248: /* funlhs1: '(' funlhs1 ')' apat  */
#line 536 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4136 "parser.c"
    break;

  case 249: /* funlhs1: var apat  */
#line 537 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4142 "parser.c"
    break;

  case 250: /* funlhs1: funlhs1 apat  */
#line 538 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4148 "parser.c"
    break;

  case 251: /* rhs: rhs1 wherePart  */
#line 540 "parser.y"
                                        {yyval = gc2(letrec(yyvsp[0],yyvsp[-1]));}
#line 4154 "parser.c"
    break;

  case 252: /* rhs: error  */
#line 541 "parser.y"
                                        {syntaxError("declaration");}
#line 4160 "parser.c"
    break;

  case 253: /* rhs1: '=' exp  */
#line 543 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 4166 "parser.c"
    break;

  case 254: /* rhs1: gdrhs  */
#line 544 "parser.y"
                                        {yyval = gc1(desugarGuards(rev(yyvsp[0])));}
#line 4172 "parser.c"
    break;

  case 255: /* gdrhs: gdrhs gddef  */
#line 546 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4178 "parser.c"
    break;

  case 256: /* gdrhs: gddef  */
#line 547 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4184 "parser.c"
    break;

  case 257: /* gddef: '|' quals '=' exp  */
#line 549 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 4190 "parser.c"
    break;

  case 258: /* wherePart: %empty  */
#line 551 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4196 "parser.c"
    break;

  case 259: /* wherePart: WHERE decls  */
#line 552 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4202 "parser.c"
    break;

  case 260: /* lwherePart: %empty  */
#line 557 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4208 "parser.c"
    break;

  case 261: /* lwherePart: WHERE ldecls  */
#line 558 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4214 "parser.c"
    break;

  case 262: /* ldecls: '{' ldecls0 end  */
#line 561 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4220 "parser.c"
    break;

  case 263: /* ldecls: '{' ldecls1 end  */
#line 562 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4226 "parser.c"
    break;

  case 264: /* ldecls0: %empty  */
#line 565 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4232 "parser.c"
    break;

  case 265: /* ldecls0: ldecls0 ';'  */
#line 566 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4238 "parser.c"
    break;

  case 266: /* ldecls0: ldecls1 ';'  */
#line 567 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4244 "parser.c"
    break;

  case 267: /* ldecls1: ldecls0 ldecl  */
#line 570 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4250 "parser.c"
    break;

  case 268: /* ldecl: decl  */
#line 572 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4256 "parser.c"
    break;

  case 269: /* pat: pat_npk  */
#line 577 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4262 "parser.c"
    break;

  case 270: /* pat_npk: pat0 COCO type  */
#line 579 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4268 "parser.c"
    break;

  case 271: /* pat_npk: pat0  */
#line 580 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4274 "parser.c"
    break;

  case 272: /* pat0: var  */
#line 582 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4280 "parser.c"
    break;

  case 273: /* pat0: NUMLIT  */
#line 583 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4286 "parser.c"
    break;

  case 274: /* pat0: pat0_vI  */
#line 584 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4292 "parser.c"
    break;

  case 275: /* pat0_vI: pat10_vI  */
#line 586 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4298 "parser.c"
    break;

  case 276: /* pat0_vI: infixPat  */
#line 587 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4304 "parser.c"
    break;

  case 277: /* infixPat: '-' pat10  */
#line 589 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4310 "parser.c"
    break;

  case 278: /* infixPat: '-' error  */
#line 590 "parser.y"
                                        {syntaxError("pattern");}
#line 4316 "parser.c"
    break;

  case 279: /* infixPat: var qconop pat10  */
#line 591 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4322 "parser.c"
    break;

  case 280: /* infixPat: var qconop '-' pat10  */
#line 592 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4328 "parser.c"
    break;

  case 281: /* infixPat: NUMLIT qconop pat10  */
#line 593 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4334 "parser.c"
    break;

  case 282: /* infixPat: NUMLIT qconop '-' pat10  */
#line 594 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4340 "parser.c"
    break;

  case 283: /* infixPat: pat10_vI qconop pat10  */
#line 595 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4346 "parser.c"
    break;

  case 284: /* infixPat: pat10_vI qconop '-' pat10  */
#line 596 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4352 "parser.c"
    break;

  case 285: /* infixPat: infixPat qconop pat10  */
#line 597 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4358 "parser.c"
    break;

  case 286: /* infixPat: infixPat qconop '-' pat10  */
#line 598 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4364 "parser.c"
    break;

  case 287: /* pat10: fpat  */
#line 600 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4370 "parser.c"
    break;

  case 288: /* pat10: apat  */
#line 601 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4376 "parser.c"
    break;

  case 289: /* pat10_vI: fpat  */
#line 603 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4382 "parser.c"
    break;

  case 290: /* pat10_vI: apat_vI  */
#line 604 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4388 "parser.c"
    break;

  case 291: /* fpat: fpat apat  */
#line 606 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4394 "parser.c"
    break;

  case 292: /* fpat: gcon apat  */
#line 607 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4400 "parser.c"
    break;

  case 293: /* apat: NUMLIT  */
#line 609 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4406 "parser.c"
    break;

  case 294: /* apat: var  */
#line 610 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4412 "parser.c"
    break;

  case 295: /* apat: apat_vI  */
#line 611 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4418 "parser.c"
    break;

  case 296: /* apat_vI: var '@' apat  */
#line 613 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4424 "parser.c"
    break;

  case 297: /* apat_vI: gcon  */
#line 614 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4430 "parser.c"
    break;

  case 298: /* apat_vI: qcon '{' patbinds '}'  */
#line 615 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4436 "parser.c"
    break;

  case 299: /* apat_vI: CHARLIT  */
#line 616 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4442 "parser.c"
    break;

  case 300: /* apat_vI: STRINGLIT  */
#line 617 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4448 "parser.c"
    break;

  case 301: /* apat_vI: '_'  */
#line 618 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4454 "parser.c"
    break;

  case 302: /* apat_vI: '(' pat_npk ')'  */
#line 619 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4460 "parser.c"
    break;

  case 303: /* apat_vI: '(' pats2 ')'  */
#line 620 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4466 "parser.c"
    break;

  case 304: /* apat_vI: '[' pats1 ']'  */
#line 621 "parser.y"
                                        {yyval = gc3(ap(FINLIST,rev(yyvsp[-1])));}
#line 4472 "parser.c"
    break;

  case 305: /* apat_vI: '~' apat  */
#line 622 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4478 "parser.c"
    break;

  case 306: /* pats2: pats2 ',' pat  */
#line 624 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4484 "parser.c"
    break;

  case 307: /* pats2: pat ',' pat  */
#line 625 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],singleton(yyvsp[-2])));}
#line 4490 "parser.c"
    break;

  case 308: /* pats1: pats1 ',' pat  */
#line 627 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4496 "parser.c"
    break;

  case 309: /* pats1: pat  */
#line 628 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4502 "parser.c"
    break;

  case 310: /* patbinds: %empty  */
#line 630 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4508 "parser.c"
    break;

  case 311: /* patbinds: patbinds1  */
#line 631 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 4514 "parser.c"
    break;

  case 312: /* patbinds1: patbinds1 ',' patbind  */
#line 633 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4520 "parser.c"
    break;

  case 313: /* patbinds1: patbind  */
#line 634 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4526 "parser.c"
    break;

  case 314: /* patbind: qvar '=' pat  */
#line 636 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 4532 "parser.c"
    break;

  case 315: /* patbind: var  */
#line 637 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4538 "parser.c"
    break;

  case 316: /* exp: exp_err  */
#line 642 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4544 "parser.c"
    break;

  case 317: /* exp: error  */
#line 643 "parser.y"
                                        {syntaxError("expression");}
#line 4550 "parser.c"
    break;

  case 318: /* exp_err: exp0a COCO sigType  */
#line 645 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4556 "parser.c"
    break;

  case 319: /* exp_err: exp0  */
#line 646 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4562 "parser.c"
    break;

  case 320: /* exp0: exp0a  */
#line 648 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4568 "parser.c"
    break;

  case 321: /* exp0: exp0b  */
#line 649 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4574 "parser.c"
    break;

  case 322: /* exp0a: infixExpa  */
#line 651 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4580 "parser.c"
    break;

  case 323: /* exp0a: exp10a  */
#line 652 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4586 "parser.c"
    break;

  case 324: /* exp0b: infixExpb  */
#line 654 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4592 "parser.c"
    break;

  case 325: /* exp0b: exp10b  */
#line 655 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4598 "parser.c"
    break;

  case 326: /* infixExpa: infixExpa qop '-' exp10a  */
#line 657 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4604 "parser.c"
    break;

  case 327: /* infixExpa: infixExpa qop exp10a  */
#line 658 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4610 "parser.c"
    break;

  case 328: /* infixExpa: '-' exp10a  */
#line 659 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4616 "parser.c"
    break;

  case 329: /* infixExpa: exp10a qop '-' exp10a  */
#line 660 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4623 "parser.c"
    break;

  case 330: /* infixExpa: exp10a qop exp10a  */
#line 662 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4629 "parser.c"
    break;

  case 331: /* infixExpb: infixExpa qop '-' exp10b  */
#line 664 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4635 "parser.c"
    break;

  case 332: /* infixExpb: infixExpa qop exp10b  */
#line 665 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4641 "parser.c"
    break;

  case 333: /* infixExpb: '-' exp10b  */
#line 666 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4647 "parser.c"
    break;

  case 334: /* infixExpb: exp10a qop '-' exp10b  */
#line 667 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4654 "parser.c"
    break;

  case 335: /* infixExpb: exp10a qop exp10b  */
#line 669 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4660 "parser.c"
    break;

  case 336: /* exp10a: CASEXP exp OF '{' alts end  */
#line 671 "parser.y"
                                        {yyval = gc6(ap(CASE,pair(yyvsp[-4],rev(yyvsp[-1]))));}
#line 4666 "parser.c"
    break;

  case 337: /* exp10a: DO '{' stmts end  */
#line 672 "parser.y"
                                        {yyval = gc4(ap(DOCOMP,checkDo(yyvsp[-1])));}
#line 4672 "parser.c"
    break;

  case 338: /* exp10a: appExp  */
#line 673 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4678 "parser.c"
    break;

  case 339: /* exp10b: '\\' pats ARROW exp  */
#line 675 "parser.y"
                                        {yyval = gc4(ap(LAMBDA,      
						     pair(rev(yyvsp[-2]),
							  pair(yyvsp[-1],yyvsp[0]))));}
#line 4686 "parser.c"
    break;

  case 340: /* exp10b: LET ldecls IN exp  */
#line 678 "parser.y"
                                        {yyval = gc4(letrec(yyvsp[-2],yyvsp[0]));}
#line 4692 "parser.c"
    break;

  case 341: /* exp10b: IF exp then_exp else_exp  */
#line 679 "parser.y"
                                        {yyval = gc4(ap(COND,triple(yyvsp[-2],yyvsp[-1],yyvsp[0])));}
#line 4698 "parser.c"
    break;

  case 342: /* then_exp: ';' THEN exp  */
#line 684 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4704 "parser.c"
    break;

  case 343: /* then_exp: THEN exp  */
#line 685 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4710 "parser.c"
    break;

  case 344: /* else_exp: ';' ELSE exp  */
#line 687 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4716 "parser.c"
    break;

  case 345: /* else_exp: ELSE exp  */
#line 688 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4722 "parser.c"
    break;

  case 346: /* pats: pats apat  */
#line 691 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4728 "parser.c"
    break;

  case 347: /* pats: apat  */
#line 692 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4734 "parser.c"
    break;

  case 348: /* appExp: appExp aexp  */
#line 694 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4740 "parser.c"
    break;

  case 349: /* appExp: aexp  */
#line 695 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4746 "parser.c"
    break;

  case 350: /* aexp: qvar  */
#line 697 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4752 "parser.c"
    break;

  case 351: /* aexp: qvar '@' aexp  */
#line 698 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4758 "parser.c"
    break;

  case 352: /* aexp: '~' aexp  */
#line 699 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4764 "parser.c"
    break;

  case 353: /* aexp: '_'  */
#line 700 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4770 "parser.c"
    break;

  case 354: /* aexp: gcon  */
#line 701 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4776 "parser.c"
    break;

  case 355: /* aexp: qcon '{' fbinds '}'  */
#line 702 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4782 "parser.c"
    break;

  case 356: /* aexp: aexp '{' fbinds '}'  */
#line 703 "parser.y"
                                        {yyval = gc4(ap(UPDFLDS,
						     triple(yyvsp[-3],NIL,yyvsp[-1])));}
#line 4789 "parser.c"
    break;

  case 357: /* aexp: NUMLIT  */
#line 705 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4795 "parser.c"
    break;

  case 358: /* aexp: CHARLIT  */
#line 706 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4801 "parser.c"
    break;

  case 359: /* aexp: STRINGLIT  */
#line 707 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4807 "parser.c"
    break;

  case 360: /* aexp: REPEAT  */
#line 708 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4813 "parser.c"
    break;

  case 361: /* aexp: '(' exp ')'  */
#line 709 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4819 "parser.c"
    break;

  case 362: /* aexp: '(' exps2 ')'  */
#line 710 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4825 "parser.c"
    break;

  case 363: /* aexp: '[' list ']'  */
#line 711 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4831 "parser.c"
    break;

  case 364: /* aexp: '(' exp10a qop ')'  */
#line 712 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-1],yyvsp[-2]));}
#line 4837 "parser.c"
    break;

  case 365: /* aexp: '(' qvarop_mi exp0 ')'  */
#line 713 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4843 "parser.c"
    break;

  case 366: /* aexp: '(' qconop exp0 ')'  */
#line 714 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4849 "parser.c"
    break;

  case 367: /* exps2: exps2 ',' exp  */
#line 716 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4855 "parser.c"
    break;

  case 368: /* exps2: exp ',' exp  */
#line 717 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 4861 "parser.c"
    break;

  case 369: /* alts: alts1  */
#line 719 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4867 "parser.c"
    break;

  case 370: /* alts: ';' alts  */
#line 720 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4873 "parser.c"
    break;

  case 371: /* alts1: alts1 ';' alt  */
#line 722 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4879 "parser.c"
    break;

  case 372: /* alts1: alts1 ';'  */
#line 723 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4885 "parser.c"
    break;

  case 373: /* alts1: alt  */
#line 724 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4891 "parser.c"
    break;

  case 374: /* alt: pat altRhs wherePart  */
#line 726 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],letrec(yyvsp[0],yyvsp[-1])));}
#line 4897 "parser.c"
    break;

  case 375: /* altRhs: guardAlts  */
#line 728 "parser.y"
                                        {yyval = gc1(desugarGuards(rev(yyvsp[0])));}
#line 4903 "parser.c"
    break;

  case 376: /* altRhs: ARROW exp  */
#line 729 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 4909 "parser.c"
    break;

  case 377: /* altRhs: error  */
#line 730 "parser.y"
                                        {syntaxError("case expression");}
#line 4915 "parser.c"
    break;

  case 378: /* guardAlts: guardAlts guardAlt  */
#line 732 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4921 "parser.c"
    break;

  case 379: /* guardAlts: guardAlt  */
#line 733 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4927 "parser.c"
    break;

  case 380: /* guardAlt: '|' quals ARROW exp  */
#line 735 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 4933 "parser.c"
    break;

  case 381: /* stmts: stmts1  */
#line 738 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4939 "parser.c"
    break;

  case 382: /* stmts: ';' stmts  */
#line 739 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4945 "parser.c"
    break;

  case 383: /* stmts1: stmts1 ';' stmt  */
#line 741 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4951 "parser.c"
    break;

  case 384: /* stmts1: stmts1 ';'  */
#line 742 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4957 "parser.c"
    break;

  case 385: /* stmts1: stmt  */
#line 743 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4963 "parser.c"
    break;

  case 386: /* stmt: exp_err FROM exp  */
#line 746 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 4969 "parser.c"
    break;

  case 387: /* stmt: LET ldecls  */
#line 747 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 4975 "parser.c"
    break;

  case 388: /* stmt: exp_err  */
#line 749 "parser.y"
                                        {yyval = gc1(ap(DOQUAL,yyvsp[0]));}
#line 4981 "parser.c"
    break;

  case 389: /* fbinds: %empty  */
#line 751 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4987 "parser.c"
    break;

  case 390: /* fbinds: fbinds1  */
#line 752 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 4993 "parser.c"
    break;

  case 391: /* fbinds1: fbinds1 ',' fbind  */
#line 754 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4999 "parser.c"
    break;

  case 392: /* fbinds1: fbind  */
#line 755 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 5005 "parser.c"
    break;

  case 393: /* fbind: var  */
#line 757 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5011 "parser.c"
    break;

  case 394: /* fbind: qvar '=' exp  */
#line 758 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 5017 "parser.c"
    break;

  case 395: /* list: exp  */
#line 763 "parser.y"
                                        {yyval = gc1(ap(FINLIST,cons(yyvsp[0],NIL)));}
#line 5023 "parser.c"
    break;

  case 396: /* list: exps2  */
#line 764 "parser.y"
                                        {yyval = gc1(ap(FINLIST,rev(yyvsp[0])));}
#line 5029 "parser.c"
    break;

  case 397: /* list: exp zipquals  */
#line 765 "parser.y"
                                        {
					 if (length(yyvsp[0])!=1) {
					     syntaxError("list comprehension");
					 }
					 yyval = gc2(ap(COMP,pair(yyvsp[-1],hd(yyvsp[0]))));
					}
#line 5040 "parser.c"
    break;

  case 398: /* list: exp UPTO exp  */
#line 771 "parser.y"
                                        {yyval = gc3(ap(ap(nameFromTo,yyvsp[-2]),yyvsp[0]));}
#line 5046 "parser.c"
    break;

  case 399: /* list: exp ',' exp UPTO  */
#line 772 "parser.y"
                                        {yyval = gc4(ap(ap(nameFromThen,yyvsp[-3]),yyvsp[-1]));}
#line 5052 "parser.c"
    break;

  case 400: /* list: exp UPTO  */
#line 773 "parser.y"
                                        {yyval = gc2(ap(nameFrom,yyvsp[-1]));}
#line 5058 "parser.c"
    break;

  case 401: /* list: exp ',' exp UPTO exp  */
#line 774 "parser.y"
                                        {yyval = gc5(ap(ap(ap(nameFromThenTo,
								yyvsp[-4]),yyvsp[-2]),yyvsp[0]));}
#line 5065 "parser.c"
    break;

  case 402: /* zipquals: zipquals '|' quals  */
#line 777 "parser.y"
                                        {yyval = gc3(cons(rev(yyvsp[0]),yyvsp[-2]));}
#line 5071 "parser.c"
    break;

  case 403: /* zipquals: '|' quals  */
#line 778 "parser.y"
                                        {yyval = gc2(cons(rev(yyvsp[0]),NIL));}
#line 5077 "parser.c"
    break;

  case 404: /* quals: quals ',' qual  */
#line 780 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 5083 "parser.c"
    break;

  case 405: /* quals: qual  */
#line 781 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 5089 "parser.c"
    break;

  case 406: /* qual: exp FROM exp  */
#line 783 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 5095 "parser.c"
    break;

  case 407: /* qual: exp  */
#line 784 "parser.y"
                                        {yyval = gc1(ap(BOOLQUAL,yyvsp[0]));}
#line 5101 "parser.c"
    break;

  case 408: /* qual: LET ldecls  */
#line 785 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 5107 "parser.c"
    break;

  case 409: /* gcon: qcon  */
#line 790 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5113 "parser.c"
    break;

  case 410: /* gcon: '(' ')'  */
#line 791 "parser.y"
                                        {yyval = gc2(nameUnit);}
#line 5119 "parser.c"
    break;

  case 411: /* gcon: '[' ']'  */
#line 792 "parser.y"
                                        {yyval = gc2(nameNil);}
#line 5125 "parser.c"
    break;

  case 412: /* gcon: '(' tupCommas ')'  */
#line 793 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5131 "parser.c"
    break;

  case 413: /* tupCommas: tupCommas ','  */
#line 795 "parser.y"
                                        {yyval = gc2(mkTuple(tupleOf(yyvsp[-1])+1));}
#line 5137 "parser.c"
    break;

  case 414: /* tupCommas: ','  */
#line 796 "parser.y"
                                        {yyval = gc1(mkTuple(2));}
#line 5143 "parser.c"
    break;

  case 415: /* varid: VARID  */
#line 798 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5149 "parser.c"
    break;

  case 416: /* varid: HIDING  */
#line 799 "parser.y"
                                        {yyval = gc1(varHiding);}
#line 5155 "parser.c"
    break;

  case 417: /* varid: QUALIFIED  */
#line 800 "parser.y"
                                        {yyval = gc1(varQualified);}
#line 5161 "parser.c"
    break;

  case 418: /* varid: ASMOD  */
#line 801 "parser.y"
                                        {yyval = gc1(varAsMod);}
#line 5167 "parser.c"
    break;

  case 419: /* qconid: QCONID  */
#line 803 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5173 "parser.c"
    break;

  case 420: /* qconid: CONID  */
#line 804 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5179 "parser.c"
    break;

  case 421: /* var: varid  */
#line 806 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5185 "parser.c"
    break;

  case 422: /* var: '(' VAROP ')'  */
#line 807 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5191 "parser.c"
    break;

  case 423: /* var: '(' '+' ')'  */
#line 808 "parser.y"
                                        {yyval = gc3(varPlus);}
#line 5197 "parser.c"
    break;

  case 424: /* var: '(' '-' ')'  */
#line 809 "parser.y"
                                        {yyval = gc3(varMinus);}
#line 5203 "parser.c"
    break;

  case 425: /* var: '(' '!' ')'  */
#line 810 "parser.y"
                                        {yyval = gc3(varBang);}
#line 5209 "parser.c"
    break;

  case 426: /* var: '(' '.' ')'  */
#line 811 "parser.y"
                                        {yyval = gc3(varDot);}
#line 5215 "parser.c"
    break;

  case 427: /* qvar: QVARID  */
#line 813 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5221 "parser.c"
    break;

  case 428: /* qvar: '(' QVAROP ')'  */
#line 814 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5227 "parser.c"
    break;

  case 429: /* qvar: var  */
#line 815 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5233 "parser.c"
    break;

  case 430: /* con: CONID  */
#line 817 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5239 "parser.c"
    break;

  case 431: /* con: '(' CONOP ')'  */
#line 818 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5245 "parser.c"
    break;

  case 432: /* qcon: QCONID  */
#line 820 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5251 "parser.c"
    break;

  case 433: /* qcon: '(' QCONOP ')'  */
#line 821 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5257 "parser.c"
    break;

  case 434: /* qcon: con  */
#line 822 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5263 "parser.c"
    break;

  case 435: /* varop: '+'  */
#line 824 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5269 "parser.c"
    break;

  case 436: /* varop: '-'  */
#line 825 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5275 "parser.c"
    break;

  case 437: /* varop: varop_mipl  */
#line 826 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5281 "parser.c"
    break;

  case 438: /* varop_mi: '+'  */
#line 828 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5287 "parser.c"
    break;

  case 439: /* varop_mi: varop_mipl  */
#line 829 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5293 "parser.c"
    break;

  case 440: /* varop_pl: '-'  */
#line 831 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5299 "parser.c"
    break;

  case 441: /* varop_pl: varop_mipl  */
#line 832 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5305 "parser.c"
    break;

  case 442: /* varop_mipl: VAROP  */
#line 834 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5311 "parser.c"
    break;

  case 443: /* varop_mipl: '`' varid '`'  */
#line 835 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5317 "parser.c"
    break;

  case 444: /* varop_mipl: '!'  */
#line 836 "parser.y"
                                        {yyval = gc1(varBang);}
#line 5323 "parser.c"
    break;

  case 445: /* varop_mipl: '.'  */
#line 837 "parser.y"
                                        {yyval = gc1(varDot);}
#line 5329 "parser.c"
    break;

  case 446: /* qvarop: '-'  */
#line 839 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5335 "parser.c"
    break;

  case 447: /* qvarop: qvarop_mi  */
#line 840 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5341 "parser.c"
    break;

  case 448: /* qvarop_mi: QVAROP  */
#line 842 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5347 "parser.c"
    break;

  case 449: /* qvarop_mi: '`' QVARID '`'  */
#line 843 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5353 "parser.c"
    break;

  case 450: /* qvarop_mi: varop_mi  */
#line 844 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5359 "parser.c"
    break;

  case 451: /* conop: CONOP  */
#line 847 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5365 "parser.c"
    break;

  case 452: /* conop: '`' CONID '`'  */
#line 848 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5371 "parser.c"
    break;

  case 453: /* qconop: QCONOP  */
#line 850 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5377 "parser.c"
    break;

  case 454: /* qconop: '`' QCONID '`'  */
#line 851 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5383 "parser.c"
    break;

  case 455: /* qconop: conop  */
#line 852 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5389 "parser.c"
    break;

  case 456: /* op: varop  */
#line 854 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5395 "parser.c"
    break;

  case 457: /* op: conop  */
#line 855 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5401 "parser.c"
    break;

  case 458: /* qop: qvarop  */
#line 857 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5407 "parser.c"
    break;

  case 459: /* qop: qconop  */
#line 858 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5413 "parser.c"
    break;

  case 460: /* begin: %empty  */
#line 863 "parser.y"
                                        {goOffside(startColumn);}
#line 5419 "parser.c"
    break;

  case 461: /* end: '}'  */
#line 866 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5425 "parser.c"
    break;

  case 462: /* end: error  */
#line 867 "parser.y"
                                        {yyerrok; 
					 if (canUnOffside()) {
					     unOffside();
					     /* insert extra token on stack*/
					     push(NIL);
					     pushed(0) = pushed(1);
					     pushed(1) = mkInt(column);
					 }
					 else
					     syntaxError("declaration");
					}
#line 5441 "parser.c"
    break;


#line 5445 "parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 882 "parser.y"


static Cell local gcShadow(Int n, Cell e)		/* keep parsed fragments on stack  */
{
    /* If a look ahead token is held then the required stack transformation
     * is:
     *   pushed: n               1     0          1     0
     *           x1  |  ...  |  xn  |  la   ===>  e  |  la
     *                                top()            top()
     *
     * Othwerwise, the transformation is:
     *   pushed: n-1             0        0
     *           x1  |  ...  |  xn  ===>  e
     *                         top()     top()
     */
    if (yychar>=0) {
	pushed(n-1) = top();
	pushed(n)   = e;
    }
    else
	pushed(n-1) = e;
    sp -= (n-1);
    return e;
}

static Void local syntaxError(String s)	/* report on syntax error	   */
{
    ERRMSG(row) "Syntax error in %s (unexpected %s)", s, unexpected()
    EEND;
}

static String local unexpected() {     /* find name for unexpected token   */
    static char buffer[100];
    static char *fmt = "%s \"%s\"";
    static char *kwd = "keyword";

    switch (yychar) {
	case 0         : return "end of input";

#define keyword(kw) sprintf(buffer,fmt,kwd,kw); return buffer;
	case INFIXL    : keyword("infixl");
	case INFIXR    : keyword("infixr");
	case INFIXN    : keyword("infix");
	case TINSTANCE : keyword("instance");
	case TCLASS    : keyword("class");
	case PRIMITIVE : keyword("primitive");
	case CASEXP    : keyword("case");
	case OF        : keyword("of");
	case IF        : keyword("if");
	case THEN      : keyword("then");
	case ELSE      : keyword("else");
	case WHERE     : keyword("where");
	case TYPE      : keyword("type");
	case DATA      : keyword("data");
	case TNEWTYPE  : keyword("newtype");
	case LET       : keyword("let");
	case IN        : keyword("in");
	case DERIVING  : keyword("deriving");
	case DEFAULT   : keyword("default");
	case IMPORT    : keyword("import");
	case TMODULE   : keyword("module");
	case ALL       : keyword("forall");
#undef keyword

	case ARROW     : return "`->'";
	case '='       : return "`='";
	case COCO      : return "`::'";
	case '-'       : return "`-'";
	case '!'       : return "`!'";
	case ','       : return "comma";
	case '@'       : return "`@'";
	case '('       : return "`('";
	case ')'       : return "`)'";
	case '{'       : return "`{', possibly due to bad layout";
	case '}'       : return "`}', possibly due to bad layout";
	case '_'       : return "`_'";
	case '|'       : return "`|'";
	case '.'       : return "`.'";
	case ';'       : return "`;', possibly due to bad layout";
	case UPTO      : return "`..'";
	case '['       : return "`['";
	case ']'       : return "`]'";
	case FROM      : return "`<-'";
	case '\\'      : return "backslash (lambda)";
	case '~'       : return "tilde";
	case '`'       : return "backquote";
	case VAROP     :
	case VARID     :
	case CONOP     :
	case CONID     : sprintf(buffer,"symbol \"%s\"",
				 textToStr(textOf(yylval)));
			 return buffer;
	case QVAROP    :
	case QVARID    :
	case QCONOP    : 
	case QCONID    : sprintf(buffer,"symbol \"%s\"",
				 identToStr(yylval));
			 return buffer;
	case HIDING    : return "symbol \"hiding\"";
	case QUALIFIED : return "symbol \"qualified\"";
	case ASMOD     : return "symbol \"as\"";
	case NUMLIT    : return "numeric literal";
	case CHARLIT   : return "character literal";
	case STRINGLIT : return "string literal";
	case IMPLIES   : return "`=>'";
	default        : return "token";
    }
}

static Cell local checkPrec(Cell p)		/* Check for valid precedence value*/
{
    if (!isInt(p) || intOf(p)<MIN_PREC || intOf(p)>MAX_PREC) {
	ERRMSG(row) "Precedence value must be an integer in the range [%d..%d]",
		    MIN_PREC, MAX_PREC
	EEND;
    }
    return p;
}

/* --------------------------------------------------------------------------
 * Pattern guards (Haskell 2010).  A guard is parsed as a list of qualifiers
 * (line,(quals,rhs)); a plain boolean guard is left as (line,(cond,rhs)).
 * A guard with anything else is desugared here, so that no later stage of
 * the system has to know about pattern guards:
 *
 *      | q1, ..., qn = e
 *
 * becomes the boolean guard
 *
 *      | case m of { Just _ -> True; Nothing -> False }
 *            = case m of { Just r -> r }
 *
 * together with a hidden binding, placed in a let around the guarded rhs,
 *
 *      m = Q(q1, ..., qn)   where   Q()           = Just e
 *                                   Q(p <- x, qs) = case x of { p -> Q(qs); _ -> Nothing }
 *                                   Q(b, qs)      = if b then Q(qs) else Nothing
 *                                   Q(let ds, qs) = let ds in Q(qs)
 *
 * Evaluating the guard performs the matches and tests in order; the rhs
 * stays a thunk inside the Just, with every variable the qualifiers bind
 * in scope.  If all guards fail the guarded rhs fails as before, so the
 * next equation is tried.
 * ------------------------------------------------------------------------*/

static Cell local guardMatch(Cell l, List qs, Cell res)	/* build Q above */
{
    Cell q;
    if (isNull(qs))
	return res;
    q   = hd(qs);
    res = guardMatch(l,tl(qs),res);
    switch (whatIs(q)) {
	case BOOLQUAL : return ap(COND,triple(snd(q),res,nameNothing));
	case QWHERE   : return ap(LETREC,pair(snd(q),res));
	case FROMQUAL : return ap(CASE,pair(snd(snd(q)),
			       cons(pair(fst(snd(q)),pair(l,res)),
			       cons(pair(WILDCARD,pair(l,nameNothing)),NIL))));
	default       : internal("guardMatch");
    }
    return NIL;/*NOTREACHED*/
}

static Cell local desugarGuards(List gs)	/* [(line,(quals,rhs))] -> rhs */
{
    List binds = NIL;
    List g;
    for (g=gs; nonNull(g); g=tl(g)) {
	Cell l   = fst(hd(g));
	List qs  = fst(snd(hd(g)));
	Cell e   = snd(snd(hd(g)));
	if (nonNull(qs) && isNull(tl(qs)) && whatIs(hd(qs))==BOOLQUAL) {
	    fst(snd(hd(g))) = snd(hd(qs));	/* ordinary boolean guard */
	} else {
	    Cell m  = inventVar();
	    Cell r  = inventVar();
	    Cell mv = mkVar(textOf(m));
	    Cell rv = mkVar(textOf(r));
	    binds   = cons(ap(PATBIND,pair(m,pair(l,guardMatch(l,qs,ap(nameJust,e))))),
			   binds);
	    fst(snd(hd(g)))
		    = ap(CASE,pair(mv,
			 cons(pair(ap(nameJust,WILDCARD),pair(l,nameTrue)),
			 cons(pair(nameNothing,pair(l,nameFalse)),NIL))));
	    snd(snd(hd(g)))
		    = ap(CASE,pair(mkVar(textOf(m)),
			 cons(pair(ap(nameJust,r),pair(l,rv)),NIL)));
	}
    }
    return letrec(binds,grded(gs));
}

static Cell local buildTuple(List tup)	/* build tuple (x1,...,xn) from	   */
{				/* list [xn,...,x1]		   */
    Int  n = 0;
    Cell t = tup;
    Cell x;

    do {				/*    .                    .	   */
	x      = fst(t);		/*   / \                  / \	   */
	fst(t) = snd(t);		/*  xn  .                .   xn	   */
	snd(t) = x;			/*       .    ===>      .	   */
	x      = t;			/*        .            .	   */
	t      = fun(x);		/*         .          .		   */
	n++;				/*        / \        / \	   */
    } while (nonNull(t));		/*       x1  NIL   (n)  x1	   */
    fst(x) = mkTuple(n);
    return tup;
}

static List local checkCtxt(Type con)	/* validate context		   */
{
    mapOver(checkPred, con);
    return con;
}

static Cell local checkPred(Cell c)		/* check that type expr is a valid */
{				/* constraint			   */
    Cell cn = getHead(c);
    if (!isQCon(cn) /*|| argCount==0*/)
	syntaxError("class expression");
    return c;
}

static Pair local checkDo(List dqs)		/* convert reversed list of dquals */
{				/* to an (expr,quals) pair         */
    if (isNull(dqs) || whatIs(hd(dqs))!=DOQUAL) {
	ERRMSG(row) "Last generator in do {...} must be an expression"
	EEND;
    }
    fst(dqs) = snd(fst(dqs));		/* put expression in fst of pair   */
    snd(dqs) = rev(snd(dqs));		/* & reversed list of quals in snd */
    return dqs;
}


static Cell local checkTyLhs(Cell c)		/* check that lhs is of the form   */
{				/* T a1 ... a			   */
    Cell tlhs = c;
    while (isAp(tlhs) && whatIs(arg(tlhs))==VARIDCELL) {
	tlhs = fun(tlhs);
    }
    if (whatIs(tlhs)!=CONIDCELL) {
	ERRMSG(row) "Illegal left hand side in data type declaration"
	EEND;
    }
    return c;
}

static Cell local checkConstr(Cell c)	/* check that data constructor has */
{				/* an unqualified conid as head    */
    Cell chd = c;
    while (isAp(chd)) {
	chd = fun(chd);
    }
    if (whatIs(chd)==QUALIDENT) {
	ERRMSG(row) "Qualified constructor in data type declaration"
	EEND;
    }
    return c;
}



/*-------------------------------------------------------------------------*/
