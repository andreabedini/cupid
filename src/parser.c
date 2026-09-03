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

#define lint
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


#line 134 "parser.c"

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
  YYSYMBOL_pat = 153,                      /* pat  */
  YYSYMBOL_pat_npk = 154,                  /* pat_npk  */
  YYSYMBOL_pat0 = 155,                     /* pat0  */
  YYSYMBOL_pat0_vI = 156,                  /* pat0_vI  */
  YYSYMBOL_infixPat = 157,                 /* infixPat  */
  YYSYMBOL_pat10 = 158,                    /* pat10  */
  YYSYMBOL_pat10_vI = 159,                 /* pat10_vI  */
  YYSYMBOL_fpat = 160,                     /* fpat  */
  YYSYMBOL_apat = 161,                     /* apat  */
  YYSYMBOL_apat_vI = 162,                  /* apat_vI  */
  YYSYMBOL_pats2 = 163,                    /* pats2  */
  YYSYMBOL_pats1 = 164,                    /* pats1  */
  YYSYMBOL_patbinds = 165,                 /* patbinds  */
  YYSYMBOL_patbinds1 = 166,                /* patbinds1  */
  YYSYMBOL_patbind = 167,                  /* patbind  */
  YYSYMBOL_exp = 168,                      /* exp  */
  YYSYMBOL_exp_err = 169,                  /* exp_err  */
  YYSYMBOL_exp0 = 170,                     /* exp0  */
  YYSYMBOL_exp0a = 171,                    /* exp0a  */
  YYSYMBOL_exp0b = 172,                    /* exp0b  */
  YYSYMBOL_infixExpa = 173,                /* infixExpa  */
  YYSYMBOL_infixExpb = 174,                /* infixExpb  */
  YYSYMBOL_exp10a = 175,                   /* exp10a  */
  YYSYMBOL_exp10b = 176,                   /* exp10b  */
  YYSYMBOL_then_exp = 177,                 /* then_exp  */
  YYSYMBOL_else_exp = 178,                 /* else_exp  */
  YYSYMBOL_pats = 179,                     /* pats  */
  YYSYMBOL_appExp = 180,                   /* appExp  */
  YYSYMBOL_aexp = 181,                     /* aexp  */
  YYSYMBOL_exps2 = 182,                    /* exps2  */
  YYSYMBOL_alts = 183,                     /* alts  */
  YYSYMBOL_alts1 = 184,                    /* alts1  */
  YYSYMBOL_alt = 185,                      /* alt  */
  YYSYMBOL_altRhs = 186,                   /* altRhs  */
  YYSYMBOL_guardAlts = 187,                /* guardAlts  */
  YYSYMBOL_guardAlt = 188,                 /* guardAlt  */
  YYSYMBOL_stmts = 189,                    /* stmts  */
  YYSYMBOL_stmts1 = 190,                   /* stmts1  */
  YYSYMBOL_stmt = 191,                     /* stmt  */
  YYSYMBOL_fbinds = 192,                   /* fbinds  */
  YYSYMBOL_fbinds1 = 193,                  /* fbinds1  */
  YYSYMBOL_fbind = 194,                    /* fbind  */
  YYSYMBOL_list = 195,                     /* list  */
  YYSYMBOL_quals = 196,                    /* quals  */
  YYSYMBOL_qual = 197,                     /* qual  */
  YYSYMBOL_gcon = 198,                     /* gcon  */
  YYSYMBOL_tupCommas = 199,                /* tupCommas  */
  YYSYMBOL_varid = 200,                    /* varid  */
  YYSYMBOL_qconid = 201,                   /* qconid  */
  YYSYMBOL_var = 202,                      /* var  */
  YYSYMBOL_qvar = 203,                     /* qvar  */
  YYSYMBOL_con = 204,                      /* con  */
  YYSYMBOL_qcon = 205,                     /* qcon  */
  YYSYMBOL_varop = 206,                    /* varop  */
  YYSYMBOL_varop_mi = 207,                 /* varop_mi  */
  YYSYMBOL_varop_pl = 208,                 /* varop_pl  */
  YYSYMBOL_varop_mipl = 209,               /* varop_mipl  */
  YYSYMBOL_qvarop = 210,                   /* qvarop  */
  YYSYMBOL_qvarop_mi = 211,                /* qvarop_mi  */
  YYSYMBOL_conop = 212,                    /* conop  */
  YYSYMBOL_qconop = 213,                   /* qconop  */
  YYSYMBOL_op = 214,                       /* op  */
  YYSYMBOL_qop = 215,                      /* qop  */
  YYSYMBOL_begin = 216,                    /* begin  */
  YYSYMBOL_end = 217                       /* end  */
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
#define YYLAST   3672

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  148
/* YYNRULES -- Number of rules.  */
#define YYNRULES  451
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  788

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
       0,    99,    99,   100,   101,   102,   115,   119,   123,   125,
     127,   133,   136,   138,   139,   147,   148,   149,   150,   151,
     156,   157,   158,   159,   160,   162,   163,   168,   169,   170,
     171,   172,   174,   175,   176,   177,   179,   180,   182,   183,
     188,   189,   190,   192,   203,   205,   208,   211,   214,   216,
     217,   218,   220,   221,   222,   223,   225,   226,   228,   229,
     230,   231,   233,   234,   235,   236,   238,   239,   241,   242,
     247,   248,   249,   250,   251,   256,   257,   260,   261,   264,
     268,   270,   273,   274,   277,   281,   282,   288,   290,   291,
     292,   294,   295,   297,   299,   301,   302,   304,   306,   308,
     309,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     321,   322,   323,   324,   325,   326,   328,   329,   330,   332,
     334,   335,   337,   338,   339,   341,   342,   343,   345,   346,
     348,   349,   354,   356,   357,   358,   360,   361,   366,   368,
     370,   372,   374,   380,   381,   382,   383,   384,   385,   387,
     388,   390,   391,   393,   394,   396,   397,   399,   400,   402,
     403,   405,   406,   408,   409,   414,   416,   418,   419,   421,
     422,   423,   424,   426,   428,   430,   432,   433,   435,   436,
     438,   439,   440,   441,   443,   444,   446,   447,   448,   449,
     450,   452,   453,   455,   456,   458,   459,   461,   462,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     476,   477,   479,   480,   481,   482,   487,   488,   489,   490,
     491,   492,   493,   494,   496,   497,   499,   500,   502,   503,
     505,   506,   508,   509,   510,   512,   514,   515,   516,   519,
     521,   522,   524,   525,   526,   527,   528,   530,   531,   532,
     533,   535,   536,   538,   539,   541,   542,   544,   546,   547,
     551,   553,   554,   556,   557,   558,   560,   561,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   574,   575,
     577,   578,   580,   581,   583,   584,   585,   587,   588,   589,
     590,   591,   592,   593,   594,   595,   596,   598,   599,   601,
     602,   604,   605,   607,   608,   610,   611,   616,   617,   619,
     620,   622,   623,   625,   626,   628,   629,   631,   632,   633,
     634,   636,   638,   639,   640,   641,   643,   645,   646,   647,
     649,   652,   653,   658,   659,   661,   662,   665,   666,   668,
     669,   671,   672,   673,   674,   675,   676,   677,   679,   680,
     681,   682,   683,   684,   685,   686,   687,   688,   690,   691,
     693,   694,   696,   697,   698,   700,   702,   703,   704,   706,
     707,   709,   712,   713,   715,   716,   717,   720,   721,   723,
     725,   726,   728,   729,   731,   732,   737,   738,   739,   740,
     741,   742,   743,   746,   747,   749,   750,   751,   756,   757,
     758,   759,   761,   762,   764,   765,   766,   767,   769,   770,
     772,   773,   774,   775,   776,   777,   779,   780,   781,   783,
     784,   786,   787,   788,   790,   791,   792,   794,   795,   797,
     798,   800,   801,   802,   803,   805,   806,   808,   809,   810,
     813,   814,   816,   817,   818,   820,   821,   823,   824,   829,
     832,   833
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
  "rhs1", "gdrhs", "gddef", "wherePart", "pat", "pat_npk", "pat0",
  "pat0_vI", "infixPat", "pat10", "pat10_vI", "fpat", "apat", "apat_vI",
  "pats2", "pats1", "patbinds", "patbinds1", "patbind", "exp", "exp_err",
  "exp0", "exp0a", "exp0b", "infixExpa", "infixExpb", "exp10a", "exp10b",
  "then_exp", "else_exp", "pats", "appExp", "aexp", "exps2", "alts",
  "alts1", "alt", "altRhs", "guardAlts", "guardAlt", "stmts", "stmts1",
  "stmt", "fbinds", "fbinds1", "fbind", "list", "quals", "qual", "gcon",
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

#define YYPACT_NINF (-683)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-421)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     469,  -683,  1859,    29,    10,   110,  -683,  1859,  1859,    22,
      52,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  1492,
    2504,  2659,  1676,  1234,  -683,  -683,  -683,  -683,   154,  -683,
    -683,   157,  -683,   994,  -683,   994,  -683,  2659,   176,  -683,
    -683,  -683,   181,  -683,   201,  -683,  -683,    28,  -683,  3571,
    -683,   309,  -683,   225,  -683,   246,    38,  -683,   310,  2197,
    -683,  -683,  -683,  1492,  2742,  2868,  -683,  -683,  -683,  2942,
    -683,   285,   287,  -683,  -683,   176,   342,   347,   379,   384,
    2286,   386,  -683,  -683,   760,   391,   396,   318,   994,   323,
     346,  -683,  -683,  2332,  -683,  2332,  -683,   267,   399,   407,
      22,  -683,  2133,  -683,  -683,  -683,  -683,  -683,  -683,  -683,
    -683,  -683,  -683,  -683,  2375,  2418,   176,   583,  2659,   583,
    -683,  3286,   374,  2010,   953,  -683,  -683,  -683,  -683,  -683,
    -683,   425,  -683,  2560,  2560,   420,  1859,   482,    31,  1946,
      70,  1859,    22,  2197,   464,    44,   445,  -683,  -683,   173,
     342,   347,   384,   875,   386,   391,   396,   462,   467,   500,
    -683,   173,   173,  1492,  -683,   394,  1492,   511,  1293,  -683,
    -683,   114,  1859,  -683,  1492,   583,  -683,  -683,  -683,  -683,
    -683,  -683,   485,   492,   502,   507,  -683,  -683,  1859,  -683,
    2240,  1859,  -683,  -683,  -683,   517,  -683,   522,  1556,  1905,
    1859,  -683,  -683,  -683,  1112,   530,  -683,   534,  -683,  -683,
    3408,  3312,  -683,  2504,  -683,  -683,  2504,  -683,  -683,   711,
     524,   542,  -683,   565,   582,   176,   569,   187,  -683,   187,
    -683,   585,  1978,  -683,   449,  3216,   499,   505,   561,  -683,
     599,  3434,  3148,   648,   120,    71,   498,   775,   863,   152,
     184,    36,   235,   275,   543,  2784,  2560,   371,    56,   778,
     595,   609,  -683,   613,  -683,  -683,   351,  -683,   231,  -683,
    1492,   106,   543,   543,  2700,    44,  2905,  -683,  1859,  1859,
     660,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,
     310,  -683,  1859,  -683,  2461,   299,  2979,  -683,  -683,  1492,
    -683,  3016,  -683,  2154,  3053,  3090,  -683,  3016,  -683,  -683,
    3127,  3016,  -683,  -683,  -683,   608,   629,  -683,   565,   644,
    -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683,
      22,   639,   635,  -683,   649,   472,   645,   647,   530,  3242,
     578,  2154,  2154,  2154,  -683,  2154,  -683,  -683,  -683,  -683,
     379,   655,  -683,   583,  1859,  -683,  3571,  3571,  -683,  2154,
    -683,  2154,  -683,  2154,  -683,  2154,  -683,  -683,  -683,   389,
     646,   602,   659,  -683,   454,   597,  -683,   651,  -683,  -683,
      23,  -683,   668,  3338,    81,  -683,   428,  -683,  -683,  1026,
    -683,  1026,  -683,  1026,  -683,   361,   363,  -683,   689,  -683,
     671,  3452,  -683,  1337,  -683,   678,   674,  3467,  -683,   154,
     675,  3467,  -683,   475,  -683,  3016,  -683,   680,  3164,  2826,
    -683,  -683,  -683,   454,   126,  -683,  -683,  -683,   826,   826,
    -683,    35,  -683,  2618,  2074,   826,  -683,  2154,  1859,  1905,
    -683,   154,   683,  -683,  -683,  -683,  3016,  3016,  -683,  3016,
    -683,  3016,  -683,  -683,  2905,   196,    44,   673,  -683,  -683,
    -683,  1859,  -683,  -683,  1492,  -683,  -683,  -683,  1492,  -683,
    1492,  -683,  -683,  1492,  -683,  -683,  -683,   583,  3016,   310,
    1859,  1905,  1750,   541,  -683,  -683,   685,   691,  -683,  -683,
    -683,  -683,  -683,  -683,  -683,  -683,  3434,  -683,  3434,  -683,
    -683,  -683,  1805,  -683,  2512,  -683,  2560,    96,   818,  2154,
    -683,   654,   687,  -683,  -683,  -683,   687,   687,   669,   826,
    -683,    96,   818,   692,   695,  -683,   350,   154,   187,  -683,
     187,  -683,  1492,  1492,   211,  1176,   701,   454,  -683,   418,
     727,  2618,  -683,  -683,  -683,  -683,   472,  -683,  -683,  -683,
     712,   716,  -683,  3493,  3364,  -683,   106,  -683,   164,  -683,
    -683,  -683,  -683,  -683,  -683,  -683,  -683,  1905,  1859,   154,
     717,  -683,  -683,  3016,  -683,  -683,  -683,  -683,  -683,  -683,
    -683,  -683,  -683,  -683,  2133,  -683,  -683,  -683,   714,  -683,
     719,   718,  -683,  -683,  -683,    44,  -683,   901,  -683,   472,
    3571,  1431,    39,  -683,  -683,  3511,    13,  3390,  3268,   707,
     759,  1026,  -683,  -683,   921,  -683,   753,  -683,  2154,  -683,
     725,  -683,   702,  -683,  3571,  3571,  -683,  -683,   454,  -683,
     730,  -683,   729,   726,  -683,  -683,  1176,    88,   826,   455,
     826,   613,   910,  3589,  3589,  3589,  3589,  -683,  1859,   276,
    -683,  -683,  -683,  -683,  -683,  -683,  -683,  1758,  -683,   818,
    1006,    13,  3571,  3571,   731,   127,   818,  -683,  3571,  -683,
    -683,   754,  3532,  3532,  3571,  -683,  3532,   316,   826,  -683,
     818,  -683,  -683,   350,  -683,  -683,    88,   555,  -683,   918,
     742,  -683,   758,   826,   669,   761,   763,  3604,  -683,  3550,
    -683,  -683,  -683,  -683,  1859,  -683,    39,  2089,  3532,   733,
     187,  -683,  -683,  -683,  3571,  -683,  -683,  -683,  -683,  -683,
    -683,  -683,   131,  -683,   382,  -683,   766,  -683,   773,   753,
    -683,   472,  -683,   772,   866,  -683,   774,   776,  -683,  -683,
    -683,  -683,  -683,   669,   787,  -683,   669,   669,  -683,  -683,
    -683,  1610,  -683,  -683,   786,  3190,  -683,   784,   790,  -683,
    -683,   826,  -683,  2042,   826,   669,  -683,  -683,  -683,   961,
    -683,   669,  -683,  -683,  2115,  -683,   187,  -683,  2154,  -683,
    -683,  -683,  -683,  -683,  -683,  -683,  -683,  -683
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     5,     0,     0,    11,     0,   308,     0,     0,     0,
       0,   351,   348,   349,   350,   404,   419,   416,   421,     0,
       0,     0,     0,     0,   405,   406,   407,   344,   258,   307,
     310,   311,   312,   313,   315,   314,   316,   329,   340,   345,
     410,   418,   341,   423,   398,   409,   408,     0,     3,   181,
     196,     0,     4,   449,     1,     0,     0,   232,     0,     0,
     284,   290,   291,     0,     0,     0,   292,   338,   286,     0,
     288,   285,   398,   319,   324,   343,   431,   440,   437,   442,
       0,   433,   403,   399,     0,   434,   427,     0,   314,     0,
       0,   439,   428,     0,   444,     0,   400,   386,   387,     0,
       0,     2,     0,   431,   440,   437,   442,   435,   433,   434,
     427,   447,   436,   448,     0,     0,   339,   380,     0,   380,
     180,     0,     0,     0,     0,   209,   195,   197,   199,   198,
      10,    20,    12,    15,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,   372,   376,   296,   264,
       0,     0,     0,     0,     0,     0,     0,     0,   260,   262,
     265,   267,   266,   280,   281,     0,   288,   263,     0,   300,
     260,     0,     0,   337,     0,   301,   411,   420,   417,   422,
     413,   414,     0,     0,     0,     0,   415,   412,     0,   352,
       0,     0,   353,   402,   401,     0,   311,     0,     0,     0,
       0,   354,   259,   190,     0,     0,   309,     0,   179,   184,
     186,   185,   194,     0,   318,   323,     0,   321,   326,     0,
       0,   381,   383,   384,     0,   342,     0,     0,   182,     0,
     183,     0,     0,   200,     0,     0,     0,     0,     0,   208,
       0,   185,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   264,     0,    15,     0,     0,     0,
       0,    43,    42,    17,    73,   236,     0,    74,     0,   240,
     241,     0,   267,   266,   263,     0,     0,   334,     0,     0,
       0,   332,   451,   233,   450,   235,   230,   234,   231,   331,
     378,   373,     0,   328,   375,     0,     0,   269,   268,   278,
     279,     0,   293,     0,     0,     0,   282,     0,   294,   283,
       0,     0,   295,   330,   287,     0,   302,   304,   306,     0,
     441,   438,   443,   432,   359,   355,   358,   356,   357,   389,
       0,   396,   388,   394,   359,     0,   200,     0,   174,     0,
       0,     0,     0,     0,   193,     0,   317,   322,   320,   325,
       0,     0,   347,     0,     0,   346,   211,   210,   201,     0,
     202,     0,   203,     0,   205,     0,   206,   204,   207,   409,
     421,     0,     0,    21,     0,     0,    26,     0,    27,    28,
       0,    82,     0,    80,    90,    89,     0,   217,   224,     0,
     219,     0,   221,     0,   135,     0,     0,   134,   137,    85,
       0,   181,   148,     0,   146,   157,     0,   150,   147,   258,
       0,   152,   425,     0,   424,     0,   426,     0,     0,   263,
      16,    48,    14,     0,    49,    13,    87,    86,     0,     0,
       7,    43,    18,    70,     0,     0,   252,     0,     0,     0,
     237,   258,   254,   256,   250,   239,     0,     0,   429,     0,
     249,     0,   430,     6,     0,     0,     0,   360,   364,   333,
     336,     0,   377,   374,     0,   272,   298,   261,     0,   276,
       0,   274,   297,     0,   270,   299,   289,     0,     0,   397,
       0,     0,     0,     0,   177,   175,   203,   205,   187,   178,
     188,   189,   382,   385,   212,   213,   211,   214,   210,   215,
      22,    31,     0,    23,    32,     9,    15,     0,     0,     0,
      88,     0,   216,   445,   446,   227,   218,   220,     0,     0,
     136,     0,     0,     0,   154,   156,     0,   258,     0,   144,
       0,   245,     0,     0,    49,    52,     0,     0,    44,     0,
       0,     0,    40,    71,    72,   223,     0,   222,   166,   168,
       0,     0,   172,   191,   192,   228,     0,   253,     0,   251,
     255,   244,   242,   243,   246,   361,   368,     0,     0,   258,
     366,   370,   327,   363,   335,   273,   277,   275,   271,   303,
     305,   395,   393,   392,     0,   176,    24,    25,     0,    33,
       0,    34,    37,    38,    39,     0,    90,    81,   109,     0,
       0,     0,   125,    96,    98,   106,     0,     0,   105,     0,
      75,     0,   132,   133,     0,   119,   125,   145,     0,   162,
     158,   160,     0,   143,   149,   151,   247,   248,     0,    47,
      59,    53,     0,    54,    57,    58,    52,    49,     0,     0,
       0,    19,     0,     0,     0,     0,     0,   238,     0,     0,
     367,   365,   369,   362,   173,    29,    30,    35,     8,     0,
       0,     0,   191,   192,     0,     0,     0,    78,     0,   114,
     112,     0,     0,     0,     0,   111,     0,     0,     0,   226,
       0,    83,   155,     0,   163,   164,    49,    62,    51,    55,
       0,    45,     0,     0,     0,     0,     0,     0,   169,   192,
     167,   170,   171,   257,     0,    36,   125,     0,     0,   115,
     128,   126,    95,   113,     0,   104,   118,   117,   102,   110,
     103,   108,     0,   121,     0,   229,    76,    92,    94,   125,
     159,   161,    46,     0,     0,    63,     0,    64,    67,    68,
      69,    56,    50,     0,     0,   139,     0,     0,   165,   371,
      79,     0,    97,   100,     0,   105,   101,     0,   129,   131,
     116,     0,   107,     0,     0,     0,    84,    60,    61,    65,
     138,     0,   141,   142,     0,   127,     0,   120,     0,   122,
     123,    91,    93,    66,   140,    99,   130,   124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -683,  -683,  -683,  -683,  -683,  -345,  -125,  -683,  -683,   341,
    -683,  -683,   190,  -683,   415,   417,  -524,   217,  -683,   165,
    -683,  -683,    93,   322,   424,  -203,  -683,   101,   209,  -492,
    -683,  -682,  -683,  -319,   191,  -683,   112,  -582,  -683,  -683,
    -683,   356,  -683,  -683,  -683,  -683,  -683,  -683,   195,   200,
    -391,   193,   -35,   130,   670,  -514,   311,     2,   -64,  -103,
    -497,   831,    -3,  -191,   -42,   -45,  -683,  -683,   406,  -316,
    -642,   -74,  -683,  -683,  -135,  -683,   643,   661,  -254,  -683,
    -683,   468,  -367,   -50,   -48,   -83,  -683,  -121,  -124,  -112,
     -77,    46,   559,  -683,  -683,  -683,  -683,   429,   538,   -26,
      45,   189,  -683,  -683,  -683,    18,    76,  -683,  -683,  -683,
    -683,    27,   891,   463,  -683,   349,  -683,  -683,   355,   791,
    -683,   625,   811,  -683,   586,  -683,  -416,   459,  1384,     9,
     448,   544,   -11,  -116,  -184,  1194,    63,  -683,  -683,     6,
    -683,   925,  -277,    33,   337,    12,  -683,  -134
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     5,    52,    53,   131,   424,   260,   243,   375,   376,
     590,   591,   592,   261,   432,   262,   538,   632,   633,   634,
     736,   737,   738,   263,   264,   386,   726,   727,   602,   603,
     752,   604,   605,   715,   616,   722,   723,   667,   757,   758,
     396,   397,   405,   409,   523,   524,   527,   620,   621,   622,
     547,   548,   549,   337,   205,   483,   206,   551,   208,   209,
     552,   210,   241,   126,   212,   236,   237,   265,   389,   512,
     266,    58,   139,   140,   267,   268,   269,   270,   440,   441,
     442,   443,   101,   455,   170,   159,   160,   161,   298,   162,
     163,   300,    68,   165,   171,   315,   316,   317,   331,    29,
      30,    31,    32,    33,    34,    35,    36,   138,   281,    69,
      37,    38,    89,   456,   457,   458,   569,   570,   571,   145,
     146,   147,   220,   221,   222,    99,   332,   333,    39,   238,
      40,    50,    41,    42,    43,    72,   513,    91,   451,   416,
     111,   112,    94,   113,   515,   114,   134,   286
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,   224,   122,   224,   285,    48,   288,   127,    71,   275,
     629,   293,   272,   272,   157,   169,   158,   445,   272,   344,
     234,   273,   273,   558,   505,   753,   202,   273,    92,   501,
     615,    90,   642,   144,   681,   724,   -41,   402,    73,    92,
      88,    92,   529,   279,   121,   282,   104,   115,    75,   136,
     271,   271,    71,   167,   167,    95,   271,   426,    71,   319,
     240,   665,    45,    45,   116,    67,    46,    46,   290,    51,
     671,   282,   384,    90,   559,   516,   299,   517,   534,   127,
      47,   120,   -77,   666,   427,   660,   280,   403,    57,   506,
     -41,   299,   785,   137,    92,   257,    74,   596,    73,   211,
     190,   234,   -41,   661,   207,   385,   223,   436,   223,   148,
      54,   284,   514,   691,   514,   173,   514,   144,    59,   724,
     235,   381,   274,   274,   750,   287,   378,   612,   274,   234,
     385,   420,   214,   217,   272,   272,   -77,   284,   195,   535,
     197,   453,    71,   273,   273,   225,   438,   766,   -77,   536,
     439,   649,    71,   394,    45,    71,    74,    71,    46,   340,
     623,    45,   732,    71,   318,    46,   311,   100,   127,   127,
     312,    47,   465,   271,   712,   717,   717,   535,   710,   717,
     469,   471,   296,   761,    15,   399,   474,   536,   615,   537,
     215,   218,   637,   127,   304,   305,   102,   566,   762,   127,
     310,   339,   651,   395,   648,   157,   104,   158,   217,   306,
     106,   717,   309,    24,    25,    26,   481,   760,    45,   299,
     314,    45,    46,   118,   356,    46,   357,   299,   299,   235,
     295,   346,   436,   299,   348,    47,   404,   224,   398,   467,
     567,   383,   117,   568,   419,   274,   382,   401,   429,   407,
     411,   466,   400,   135,   406,   410,   479,   472,   495,    71,
     497,   475,   535,    71,    90,   167,   218,   119,   144,    45,
     437,   438,   536,    46,   628,   439,   408,   488,   489,   490,
     452,   491,   196,   686,   196,    71,    47,   296,    71,   347,
     167,   133,   349,    71,    71,   494,   167,   127,   544,    71,
     167,   499,   647,   745,   597,   304,   305,   310,   198,    45,
     130,   199,   272,    46,   127,   127,   444,   415,   614,   200,
     450,   273,   572,   704,   609,   141,    47,   174,   481,   672,
     673,   676,   531,   182,   514,   446,   447,   184,   609,   525,
     575,   127,   223,    45,   576,   306,   577,    46,    15,   578,
     271,   619,   770,   175,   718,   772,   773,   720,   496,   127,
     498,   319,   344,   561,   562,   127,   563,   395,   564,   127,
     188,   189,   421,   556,   782,   191,   192,    24,    25,    26,
     784,   595,  -163,   721,   708,   272,   378,   299,   593,   756,
     434,   299,   150,   299,   273,   176,   299,  -163,   193,   194,
     177,   422,   518,   435,   167,    45,   351,    71,    71,    46,
     154,  -163,  -163,  -163,   670,   519,   344,   539,   540,   155,
     272,   763,   274,   271,   555,   452,   229,   230,   580,   273,
     156,   554,   178,   423,   435,   167,   167,   179,   167,   181,
     167,  -419,  -419,   167,   186,   610,   307,   308,   638,   187,
      15,   191,   310,    71,   127,  -419,   127,    71,   271,    71,
      15,   658,    71,   201,   444,   450,   318,   167,   509,   395,
       1,   344,     2,     3,     4,   609,   242,   713,   676,    24,
      25,    26,   609,   719,   422,   693,   276,    15,    45,    24,
      25,    26,    46,   278,   694,   274,   609,   128,   234,   387,
     294,   359,   360,   740,    15,   608,   395,    15,   398,   182,
     292,   127,   127,   184,   301,   554,    24,    25,    26,   608,
     302,    71,    71,   609,   635,   624,   388,   625,   639,  -225,
     274,  -225,   185,    24,    25,    26,    24,    25,    26,   303,
      28,   593,   320,  -225,   104,    55,    56,  -225,   106,   321,
     128,   363,   364,   174,   682,  -225,  -225,   365,   366,   322,
      87,    97,   167,   127,   323,   127,   127,  -225,   295,   128,
     327,   128,   128,    15,   103,   328,   104,   341,   626,   627,
     106,   211,   127,   127,   342,   740,   207,    15,   412,    16,
     609,   352,   108,   129,   353,   132,   733,   663,   235,   584,
     413,   109,    24,    25,    26,  -418,   734,   735,   698,   700,
     701,   702,   414,   193,   367,    15,    24,    25,    26,    17,
     127,   127,   354,   164,   164,   635,   127,   692,   695,   696,
     363,   487,   127,   150,   219,   151,   355,   350,   358,   152,
     699,   699,   699,   699,    24,    25,    26,   351,   234,   502,
     503,   154,   128,   391,   393,   368,   608,   127,   128,   128,
     155,   380,   430,   608,   431,   129,   725,   728,   433,   663,
     663,   156,   461,   663,   277,   476,   739,   608,   635,   289,
     128,   477,   744,   128,   478,   480,    15,   481,   182,   128,
     482,   554,   164,   164,   554,  -180,   546,  -408,   164,   780,
     485,    15,   504,    45,   755,   663,   340,    46,   180,   754,
     313,   663,   500,   127,   787,    24,    25,    26,   507,   520,
     204,   521,   526,   124,   528,   530,   324,   439,   573,   326,
      24,    25,    26,   532,    15,  -182,   329,   125,   334,   611,
     554,  -183,   150,   554,   554,   617,   350,   618,   339,   684,
     725,   128,   636,   728,   129,   129,   351,   640,   739,   643,
     154,   567,   554,    24,    25,    26,   644,   655,   554,   155,
     657,   608,   656,   677,   678,   665,   390,   683,   689,   129,
     156,   687,   688,   484,   709,   129,   377,   128,   182,   128,
     128,   128,    15,   128,   182,   742,   183,   743,   184,  -420,
     746,   425,   747,   388,   128,   128,  -225,   128,  -225,   128,
      15,   128,   765,   128,   164,   164,   459,   460,   764,   598,
    -225,    24,    25,    26,  -225,   767,   771,   768,   769,   395,
     462,   128,  -225,  -225,   510,   164,   774,   775,   428,    24,
      25,    26,   776,   587,  -225,   599,   541,   705,   542,   128,
      15,   128,   369,   690,   741,   128,    46,   543,    15,   128,
     164,   185,   783,   641,   392,   781,   164,   600,   706,   601,
     164,   729,   124,   777,   338,   613,   297,   395,   730,    24,
      25,    26,   128,   129,   731,   128,   125,    24,    25,    26,
     748,   388,   493,   779,  -225,   654,  -225,   150,   417,   151,
     129,   129,   338,    60,    61,    62,   579,    15,  -225,    16,
     560,   351,  -225,    18,    98,   154,   418,   565,   425,   463,
    -225,  -225,   653,    63,   155,   652,    64,   129,   180,    65,
     226,   585,  -225,    15,   291,   156,    24,    25,    26,   492,
     582,   659,    15,    66,   128,   129,   128,    93,   679,     0,
      15,   129,   630,    15,   203,   129,   128,   128,     0,   185,
       0,   680,    24,    25,    26,     0,   128,   425,   697,   395,
     128,    24,    25,    26,   164,     0,   557,     0,     0,    24,
      25,    26,    24,    25,    26,    15,     0,    45,     0,     0,
       0,    46,   164,    15,   484,    16,     0,     0,     0,   574,
       0,   128,   128,     0,   232,   164,   164,   124,   164,   239,
     164,     0,   734,   164,    24,    25,    26,     0,   581,     0,
     583,   125,    24,    25,    26,   103,     0,   104,     0,   105,
       0,   106,   128,     0,     0,     0,     0,   164,    15,   107,
     129,     0,   129,   108,     0,   510,   377,   484,   128,   128,
       0,    84,   109,   128,     0,   128,   128,   103,     0,   104,
       0,     0,   510,   110,   707,   164,   128,    24,    25,    26,
     685,   412,   128,   128,     0,   108,     0,     0,     0,     0,
       0,   425,     0,   511,   109,     0,     0,     0,     0,     0,
     585,   128,   128,   128,   128,   414,     0,   129,   129,     0,
     164,     0,     0,     0,   550,     0,   650,   128,   585,     0,
     128,   128,     0,   203,   128,     0,   128,     0,     0,     0,
     128,   128,   128,     0,   128,     0,     0,     0,   128,     0,
       0,     0,   164,     0,     0,     0,     0,     0,     0,   335,
       0,     0,   128,     0,    15,   128,    45,   128,     0,   129,
      46,   129,   129,     0,     0,   128,   128,     0,     0,   231,
       0,     0,   128,   232,    82,   336,   124,     0,   129,   129,
       0,     0,   425,    24,    25,    26,     0,     0,   606,   685,
     125,     0,     0,     0,     0,     0,   703,     0,   550,     0,
       0,   128,   606,     0,   128,   128,    44,     0,     0,   128,
       0,    44,    44,   128,     0,     0,   129,   129,    15,   711,
     630,   128,   129,   128,    44,    44,    44,    44,   129,   128,
       0,     0,   128,     0,     0,     0,   128,   395,   631,     0,
       0,    44,     0,     0,     0,     6,     0,    24,    25,    26,
       7,     0,   749,   129,     8,     0,     0,     0,     9,     0,
       0,     0,     0,    44,   759,     0,     0,    10,     0,     0,
      11,     0,    12,    13,    14,   553,    15,     0,    16,     0,
      17,   338,    18,     0,    44,   669,     0,    19,   675,    20,
       0,     0,    21,     0,     0,    22,     0,    44,    23,    44,
      96,     0,     0,     0,   297,    24,    25,    26,     0,   129,
       0,     0,    27,     0,     0,     0,     0,     0,    44,    44,
       0,     0,    44,   550,   550,   550,   550,     0,     0,     0,
     786,    60,    61,    62,     0,    15,     0,    16,     0,   606,
      44,    18,     0,     0,     0,    44,   606,    44,   203,   607,
       0,    63,   716,   716,    64,     0,   716,    65,     0,   553,
     606,     0,     0,   607,    24,    25,    26,     0,     0,     0,
       0,    66,     0,     0,   550,     0,    44,   550,     0,    15,
       0,    45,     0,     0,     0,    46,     0,   606,   716,     0,
       0,     0,    44,     0,    44,    44,     0,     0,   232,     0,
    -153,   124,    44,    44,    44,     0,     0,     0,    24,    25,
      26,     0,     0,    70,     0,   125,     0,    44,     0,     0,
      44,     0,     0,   550,     0,     0,   550,   550,     0,     0,
       0,   338,     0,     0,     0,   675,     0,     0,     0,     0,
       0,   662,   203,   338,     0,   550,   379,     0,     0,     0,
       0,   550,     0,     0,   606,     0,     0,    70,   166,   166,
       0,     0,     0,    70,     0,     0,     0,     0,   335,     0,
       0,     0,     0,    15,   664,    45,     0,     0,     0,    46,
       0,     0,    44,    44,   553,   553,   553,   553,   231,     0,
       0,     0,   232,    82,   233,   124,    44,     0,    44,     0,
     607,     0,    24,    25,    26,     0,     0,   607,     0,   125,
       0,     0,     0,   662,   662,     0,     0,   662,     0,     0,
       0,   607,     0,     0,     0,     0,     0,   166,   166,     0,
      60,    61,    62,   166,    15,   553,    16,     0,   553,     0,
      18,     0,     0,     0,     0,     0,     0,   166,   607,   662,
      63,     0,     0,    64,     0,   662,    65,    70,    44,     0,
      70,     0,   166,    24,    25,    26,     0,     6,    70,     0,
      66,     0,     7,     0,     0,     0,     8,     0,     0,     0,
       9,     0,     0,     0,   553,     0,     0,   553,   553,    10,
       0,     0,    11,     0,    12,    13,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,   553,     0,     0,    19,
       0,    20,   553,     0,    21,   607,     0,    22,     0,     0,
      23,   203,  -391,     0,     0,     0,     0,    24,    25,    26,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    44,    44,     0,     0,     0,   335,     0,   166,
     166,     0,    15,   664,    45,     0,     0,     0,    46,     0,
       0,     0,     0,     0,    70,    44,     0,   231,    70,     0,
     166,   232,    82,   336,   124,     0,     0,     0,     0,     0,
       0,    24,    25,    26,    44,    44,    44,     6,   125,     0,
     166,     0,     7,    70,     0,   166,     8,     0,   166,   166,
       9,   166,     0,     0,   166,   166,   379,     0,   594,    10,
       0,     0,    11,     0,    12,    13,    14,    76,    15,    77,
      16,    78,    17,    79,    18,     0,     0,     0,     0,    19,
       0,    80,     0,     0,    21,    81,     0,    22,    82,    83,
      23,     0,     0,    84,    85,     0,     0,    24,    25,    26,
       0,     0,     0,     0,    27,    86,     0,     0,     0,     0,
       0,     6,     0,     0,     0,     0,     7,     0,     0,     0,
       8,    44,    44,     0,     9,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,    11,     0,    12,    13,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
      15,     0,    16,    19,    17,    20,    18,     0,    21,   166,
       0,    22,    70,    70,    23,     0,  -390,     0,     0,   371,
       0,    24,    25,    26,     0,     0,     0,   166,    27,    24,
      25,    26,     0,     0,     0,     0,     0,     0,     0,     0,
     166,   166,     0,   166,     0,   166,     0,    15,   166,   369,
       0,    17,    44,   370,     0,     0,     0,     0,   166,     0,
       0,   594,   166,     0,   166,     0,   371,   166,   586,     0,
       6,     0,   166,     0,   374,     7,    24,    25,    26,     8,
       0,     0,     0,     9,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     0,     0,    11,     0,    12,    13,    14,
     166,    15,     0,    16,     0,    17,     0,    18,    44,     0,
       0,     0,    19,     0,    20,     0,     6,    21,     0,     0,
      22,     7,     0,    23,     0,     8,    70,    70,     0,   330,
      24,    25,    26,     0,     0,   166,     0,    27,    10,     0,
       0,    11,     0,    12,    13,    14,     0,    15,     0,    16,
       0,    17,     0,    18,     0,     0,     0,   282,    19,     0,
      20,     0,     0,    21,     0,     0,    22,   166,     0,    23,
       0,     0,   246,   247,   248,     0,    24,    25,    26,     0,
       0,     0,     0,    27,   254,    61,    62,     0,    15,   203,
      16,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,   168,     0,     0,    63,     0,     0,   255,     0,     0,
      65,   283,     0,     0,     0,   335,     0,    24,    25,    26,
      15,   203,    45,   284,    66,     0,    46,     0,     0,     0,
       0,     0,     0,     0,     0,   231,     0,     0,     0,   232,
      82,   233,   124,     0,     0,     0,     0,     0,     0,    24,
      25,    26,    15,   203,    45,     0,   125,     0,    46,     0,
       0,     0,     0,     0,     0,     0,     0,   231,     0,     0,
       0,   232,    82,   233,   124,     0,     0,     0,     0,   335,
       0,    24,    25,    26,    15,   545,    45,     0,   125,     0,
      46,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     598,   778,     0,   232,     0,     0,   124,     0,     0,     0,
       0,   546,     0,    24,    25,    26,    15,     0,    45,     0,
     125,     0,    46,     0,     0,     0,   598,     0,     0,     0,
       0,    15,     0,   369,     0,   204,     0,    46,   124,     0,
       0,     0,     0,     0,   203,    24,    25,    26,   600,     0,
     751,     0,   125,   124,     0,     0,     0,    15,     0,   369,
      24,    25,    26,    46,     0,   203,     0,   125,     0,     0,
       0,     0,     0,     0,   600,    15,   601,    45,     0,   124,
       0,    46,     0,     0,     0,     0,    24,    25,    26,     0,
       0,     0,     0,   125,   204,     0,    15,   124,    45,     0,
       0,     0,    46,     0,    24,    25,    26,     0,     0,     0,
       0,   125,     0,     7,     0,   232,     0,     8,   124,     0,
       0,   142,     0,     0,     0,    24,    25,    26,     0,     0,
      10,     0,   125,    11,     0,    12,    13,    14,     0,    15,
       0,    16,     0,    17,     0,    18,     0,     0,     0,     0,
      19,     0,    20,     0,     0,    21,     7,     0,    22,     0,
       8,    23,   143,     0,     9,     0,     0,     0,    24,    25,
      26,     0,     0,    10,     0,    27,    11,     0,    12,    13,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
       0,     0,     0,    19,     0,   216,     0,     0,    21,     0,
       0,    22,     7,   325,    23,     0,     8,     0,     0,     0,
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
     213,     0,     0,    21,     7,     0,    22,     0,     8,    23,
       0,     0,     9,     0,     0,     0,    24,    25,    26,     0,
       0,    10,     0,    27,    11,     0,    12,    13,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,    19,     0,   216,     0,     0,    21,     7,     0,    22,
       0,     8,    23,     0,     0,   142,     0,     0,     0,    24,
      25,    26,     0,     0,    10,     0,    27,    11,     0,    12,
      13,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,    19,     0,    20,     0,     0,    21,
       7,     0,    22,     0,     8,    23,     0,     0,     9,     0,
       0,     0,    24,    25,    26,     0,     0,    10,     0,    27,
      11,     0,    12,    13,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,    15,     0,    16,    19,    17,     0,
      18,     0,    21,   588,     0,    22,     0,     0,    23,     0,
       0,     0,     0,   371,   589,    24,    25,    26,   244,   245,
       0,     0,    27,    24,    25,    26,   246,   247,   248,   249,
     250,   251,     0,     0,   252,   253,     0,     0,   254,    61,
      62,     0,    15,     0,    16,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,   168,     0,     0,    63,     0,
       0,   255,     0,     0,    65,   256,     0,     0,     0,     0,
     257,    24,    25,    26,   258,   259,   244,   245,    66,     0,
       0,     0,     0,     0,   246,   247,   248,   249,   250,   251,
       0,     0,   252,   253,     0,     0,   254,    61,    62,     0,
      15,     0,    16,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,   168,     0,     0,    63,     0,     0,   255,
       0,     0,    65,     0,     0,     0,     0,     0,     0,    24,
      25,    26,   258,   259,     0,    11,    66,    12,    13,    14,
       0,    15,     0,    16,     0,    17,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
      22,     0,     0,    23,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,     0,     0,     0,    27,    60,    61,
      62,   103,    15,   104,    16,     0,     0,   106,    18,  -229,
       0,     0,   174,     0,     0,   448,     0,     0,    63,   108,
       0,    64,  -229,     0,    65,     0,     0,   413,   109,     0,
       0,    24,    25,    26,     0,     0,     0,     0,    66,   449,
     149,    61,    62,   150,    15,   151,    16,     0,     0,   152,
      18,     0,     0,     0,     0,     0,     0,   153,     0,     0,
      63,   154,     0,    64,    82,    83,    65,     0,     0,     0,
     155,     0,     0,    24,    25,    26,     0,     0,     0,     0,
      66,   156,   254,    61,    62,   150,    15,   151,    16,     0,
       0,   152,    18,     0,     0,     0,     0,     0,     0,   153,
       0,     0,    63,   154,     0,   255,    82,    83,    65,     0,
       0,     0,   155,     0,     0,    24,    25,    26,     0,     0,
       0,     0,    66,   156,    60,    61,    62,   103,    15,   104,
      16,     0,     0,   106,    18,     0,     0,     0,   174,     0,
       0,   448,     0,     0,    63,   108,     0,    64,     0,     0,
      65,     0,     0,   413,   109,     0,     0,    24,    25,    26,
       0,     0,     0,     0,    66,   449,   149,    61,    62,     0,
      15,     0,    16,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,   168,     0,     0,    63,     0,     0,    64,
       0,     0,    65,     0,    96,     0,     0,     0,     0,    24,
      25,    26,     0,   149,    61,    62,    66,    15,     0,    16,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
     168,     0,     0,    63,     0,     0,    64,     0,     0,    65,
     454,     0,     0,     0,     0,     0,    24,    25,    26,     0,
      60,    61,    62,    66,    15,     0,    16,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,   172,
      63,     0,     0,    64,     0,     0,    65,     0,     0,     0,
       0,     0,     0,    24,    25,    26,     0,    60,    61,    62,
      66,    15,     0,    16,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,   464,     0,     0,    63,     0,     0,
      64,     0,     0,    65,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,   149,    61,    62,    66,    15,     0,
      16,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,   168,     0,     0,    63,     0,     0,    64,     0,     0,
      65,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,    60,    61,    62,    66,    15,     0,    16,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,   468,     0,
       0,    63,     0,     0,    64,     0,     0,    65,     0,     0,
       0,     0,     0,     0,    24,    25,    26,     0,    60,    61,
      62,    66,    15,     0,    16,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,   470,     0,     0,    63,     0,
       0,    64,     0,     0,    65,     0,     0,     0,     0,     0,
       0,    24,    25,    26,     0,    60,    61,    62,    66,    15,
       0,    16,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,   473,     0,     0,    63,     0,     0,    64,     0,
      15,    65,   369,     0,    17,     0,   370,     0,    24,    25,
      26,     0,    60,    61,    62,    66,    15,     0,    16,   371,
     372,   373,    18,     0,     0,     0,     0,   374,     0,    24,
      25,    26,    63,     0,     0,    64,     0,   533,    65,     0,
       0,     0,    15,   104,    45,    24,    25,    26,    46,     0,
       0,     0,    66,     0,     0,     0,     0,     0,     0,   674,
    -181,   232,     0,     0,   124,     0,     0,   671,    15,     0,
      45,    24,    25,    26,    46,     0,     0,     0,   125,     0,
       0,     0,     0,   345,     0,     0,     0,   123,   361,   362,
     124,     0,     0,     0,    15,     0,    45,    24,    25,    26,
      46,     0,     0,     0,   125,     0,     0,     0,     0,   345,
       0,     0,     0,   123,   361,   486,   124,     0,     0,     0,
      15,   104,    45,    24,    25,    26,    46,     0,     0,     0,
     125,     0,     0,     0,     0,     0,     0,   674,    15,   232,
      45,     0,   124,     0,    46,   671,     0,     0,     0,    24,
      25,    26,     0,     0,     0,     0,   125,   123,   227,   228,
     124,     0,     0,     0,    15,     0,    45,    24,    25,    26,
      46,     0,     0,     0,   125,     0,     0,     0,     0,   345,
       0,     0,  -181,   123,     0,     0,   124,     0,     0,     0,
      15,     0,    45,    24,    25,    26,    46,     0,   508,     0,
     125,     0,     0,     0,     0,     0,     0,     0,  -181,   123,
       0,     0,   124,     0,     0,     0,    15,     0,    45,    24,
      25,    26,    46,     0,     0,     0,   125,     0,     0,     0,
       0,   646,     0,     0,  -181,   123,     0,     0,   124,     0,
       0,     0,    15,   104,    45,    24,    25,    26,    46,     0,
       0,     0,   125,     0,     0,     0,     0,     0,     0,     0,
      15,   123,    45,     0,   124,     0,    46,   671,     0,     0,
       0,    24,    25,    26,     0,   343,     0,     0,   125,   123,
       0,     0,   124,     0,     0,     0,    15,     0,    45,    24,
      25,    26,    46,     0,     0,     0,   125,     0,     0,     0,
       0,   345,     0,     0,    15,   123,    45,     0,   124,     0,
      46,     0,   522,     0,     0,    24,    25,    26,     0,    15,
       0,    45,   125,   123,     0,    46,   124,     0,     0,     0,
       0,     0,     0,    24,    25,    26,     0,  -181,   123,     0,
     125,   124,     0,     0,     0,    15,     0,    45,    24,    25,
      26,    46,     0,     0,     0,   125,     0,     0,     0,     0,
     645,     0,     0,    15,   123,    45,     0,   124,     0,    46,
       0,     0,     0,     0,    24,    25,    26,     0,     0,     0,
     668,   125,   232,     0,    15,   124,    45,     0,     0,     0,
      46,     0,    24,    25,    26,     0,     0,     0,     0,   125,
       0,   714,    15,   232,    45,     0,   124,     0,    46,     0,
       0,     0,     0,    24,    25,    26,     0,   646,     0,     0,
     125,   123,     0,    15,   124,    45,     0,     0,     0,    46,
       0,    24,    25,    26,     0,     0,     0,     0,   125,     0,
       0,    15,   123,    45,     0,   124,     0,    46,     0,     0,
       0,     0,    24,    25,    26,     0,    15,     0,    45,   125,
     232,     0,    46,   124,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,     0,   204,     0,   125,   124,     0,
       0,     0,     0,     0,     0,    24,    25,    26,     0,     0,
       0,     0,   125
};

static const yytype_int16 yycheck[] =
{
       3,   117,    47,   119,   139,     3,   140,    49,    19,   134,
     534,   145,   133,   134,    64,    65,    64,   271,   139,   210,
     123,   133,   134,   439,     1,   707,   100,   139,    22,   374,
     522,    22,   546,    59,   616,   677,     1,     1,    20,    33,
      22,    35,   409,    12,    47,     1,    33,    35,    21,    11,
     133,   134,    63,    64,    65,    22,   139,     1,    69,   175,
     124,    22,    34,    34,    37,    19,    38,    38,   142,    59,
      57,     1,     1,    64,   441,   391,   153,   393,   423,   121,
      51,    53,     1,    44,    28,   599,    55,    51,    66,    66,
      55,   168,   774,    55,    88,    60,    20,     1,    80,   102,
      88,   204,    67,   600,   102,    34,   117,     1,   119,    63,
       0,    67,   389,   637,   391,    69,   393,   143,    66,   761,
     123,     1,   133,   134,   706,    55,   242,   518,   139,   232,
      34,   256,   114,   115,   255,   256,    55,    67,    93,    51,
      95,   275,   153,   255,   256,   118,    40,   729,    67,    61,
      44,   567,   163,     1,    34,   166,    80,   168,    38,   204,
     527,    34,   686,   174,   175,    38,    52,    13,   210,   211,
      56,    51,   296,   256,   666,   672,   673,    51,    51,   676,
     304,   305,   149,    52,    32,     1,   310,    61,   680,    63,
     114,   115,   537,   235,   161,   162,    39,     1,    67,   241,
     167,   204,   569,    51,    40,   255,    33,   255,   190,   163,
      37,   708,   166,    61,    62,    63,    52,   714,    34,   296,
     174,    34,    38,    42,   227,    38,   229,   304,   305,   232,
      57,   213,     1,   310,   216,    51,     1,   353,   249,   303,
      44,   244,    66,    47,   255,   256,   244,   250,   259,   252,
     253,   301,   250,     7,   252,   253,   330,   307,   361,   270,
     363,   311,    51,   274,   255,   276,   190,    66,   294,    34,
      39,    40,    61,    38,    63,    44,     1,   341,   342,   343,
     274,   345,    93,   628,    95,   296,    51,   254,   299,   213,
     301,    66,   216,   304,   305,   359,   307,   339,   433,   310,
     311,   365,   556,   694,   507,   272,   273,   274,    41,    34,
       1,    44,   433,    38,   356,   357,   270,   254,   521,    52,
     274,   433,   456,    47,   508,    15,    51,    42,    52,   606,
     607,   608,   415,    34,   611,   272,   273,    38,   522,   403,
     464,   383,   353,    34,   468,   299,   470,    38,    32,   473,
     433,     1,   743,    66,   673,   746,   747,   676,   361,   401,
     363,   477,   553,   446,   447,   407,   449,    51,   451,   411,
      52,    53,     1,   437,   765,    52,    53,    61,    62,    63,
     771,   506,    32,    67,   661,   506,   502,   464,   504,   708,
      39,   468,    31,   470,   506,    53,   473,    47,    52,    53,
      53,    30,    39,    52,   415,    34,    45,   418,   419,    38,
      49,    61,    62,    63,   605,    52,   607,   428,   429,    58,
     541,    39,   433,   506,   435,   419,    52,    53,   478,   541,
      69,   434,    53,    62,    52,   446,   447,    53,   449,    53,
     451,    52,    53,   454,    53,   509,    52,    53,    30,    53,
      32,    52,   419,   464,   496,    66,   498,   468,   541,   470,
      32,   595,   473,    56,   418,   419,   477,   478,    40,    51,
       1,   662,     3,     4,     5,   659,    51,   668,   755,    61,
      62,    63,   666,   674,    30,    30,    66,    32,    34,    61,
      62,    63,    38,    11,    39,   506,   680,    49,   601,     1,
      55,    52,    53,   687,    32,   508,    51,    32,   519,    34,
      46,   553,   554,    38,    52,   518,    61,    62,    63,   522,
      53,   532,   533,   707,   535,   528,    28,   530,   539,    31,
     541,    33,    84,    61,    62,    63,    61,    62,    63,    39,
       2,   657,    57,    45,    33,     7,     8,    49,    37,    57,
     102,    52,    53,    42,   618,    57,    58,    52,    53,    57,
      22,    23,   573,   605,    57,   607,   608,    69,    57,   121,
      53,   123,   124,    32,    31,    53,    33,    47,   532,   533,
      37,   584,   624,   625,    50,   769,   584,    32,    45,    34,
     774,    67,    49,    49,    52,    51,    41,   600,   601,    58,
      57,    58,    61,    62,    63,    40,    51,    52,   643,   644,
     645,   646,    69,    52,    53,    32,    61,    62,    63,    36,
     662,   663,    40,    64,    65,   636,   668,   638,   639,   640,
      52,    53,   674,    31,    51,    33,    67,    35,    53,    37,
     643,   644,   645,   646,    61,    62,    63,    45,   751,    52,
      53,    49,   204,   247,   248,    56,   659,   699,   210,   211,
      58,    13,    67,   666,    55,   121,   677,   678,    55,   672,
     673,    69,    12,   676,   136,    67,   687,   680,   689,   141,
     232,    52,   693,   235,    40,    46,    32,    52,    34,   241,
      41,   694,   133,   134,   697,    50,    27,    51,   139,   763,
      53,    32,    51,    34,   707,   708,   751,    38,    53,   707,
     172,   714,    53,   755,   778,    61,    62,    63,    50,    30,
      51,    50,    44,    54,    50,    50,   188,    44,    55,   191,
      61,    62,    63,    53,    32,    50,   198,    68,   200,    52,
     743,    50,    31,   746,   747,    53,    35,    52,   751,    47,
     761,   303,    51,   764,   210,   211,    45,    30,   769,    47,
      49,    44,   765,    61,    62,    63,    50,    53,   771,    58,
      52,   774,    53,    66,    15,    22,     1,    52,    52,   235,
      69,    51,    53,   335,    53,   241,   242,   339,    34,   341,
     342,   343,    32,   345,    34,    53,    36,    39,    38,    66,
      39,   257,    39,    28,   356,   357,    31,   359,    33,   361,
      32,   363,    39,   365,   255,   256,   278,   279,    52,     1,
      45,    61,    62,    63,    49,    53,    39,    53,    52,    51,
     292,   383,    57,    58,   386,   276,    50,    53,    60,    61,
      62,    63,    52,   502,    69,    27,   431,   657,   431,   401,
      32,   403,    34,   636,   689,   407,    38,   433,    32,   411,
     301,   413,   769,   541,     1,   764,   307,    49,   659,    51,
     311,   680,    54,   761,   204,   519,     1,    51,   683,    61,
      62,    63,   434,   339,   684,   437,    68,    61,    62,    63,
     697,    28,   354,   763,    31,   584,    33,    31,   255,    33,
     356,   357,   232,    28,    29,    30,   477,    32,    45,    34,
     442,    45,    49,    38,    23,    49,   255,   454,   374,   294,
      57,    58,   573,    48,    58,   570,    51,   383,    53,    54,
     119,   483,    69,    32,   143,    69,    61,    62,    63,   353,
     481,    40,    32,    68,   496,   401,   498,    22,   611,    -1,
      32,   407,    34,    32,     1,   411,   508,   509,    -1,   511,
      -1,    40,    61,    62,    63,    -1,   518,   423,    58,    51,
     522,    61,    62,    63,   415,    -1,   438,    -1,    -1,    61,
      62,    63,    61,    62,    63,    32,    -1,    34,    -1,    -1,
      -1,    38,   433,    32,   546,    34,    -1,    -1,    -1,   461,
      -1,   553,   554,    -1,    51,   446,   447,    54,   449,    56,
     451,    -1,    51,   454,    61,    62,    63,    -1,   480,    -1,
     482,    68,    61,    62,    63,    31,    -1,    33,    -1,    35,
      -1,    37,   584,    -1,    -1,    -1,    -1,   478,    32,    45,
     496,    -1,   498,    49,    -1,   597,   502,   599,   600,   601,
      -1,    57,    58,   605,    -1,   607,   608,    31,    -1,    33,
      -1,    -1,   614,    69,    58,   506,   618,    61,    62,    63,
     622,    45,   624,   625,    -1,    49,    -1,    -1,    -1,    -1,
      -1,   537,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,
     642,   643,   644,   645,   646,    69,    -1,   553,   554,    -1,
     541,    -1,    -1,    -1,   434,    -1,   568,   659,   660,    -1,
     662,   663,    -1,     1,   666,    -1,   668,    -1,    -1,    -1,
     672,   673,   674,    -1,   676,    -1,    -1,    -1,   680,    -1,
      -1,    -1,   573,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,   694,    -1,    32,   697,    34,   699,    -1,   605,
      38,   607,   608,    -1,    -1,   707,   708,    -1,    -1,    47,
      -1,    -1,   714,    51,    52,    53,    54,    -1,   624,   625,
      -1,    -1,   628,    61,    62,    63,    -1,    -1,   508,   731,
      68,    -1,    -1,    -1,    -1,    -1,   648,    -1,   518,    -1,
      -1,   743,   522,    -1,   746,   747,     2,    -1,    -1,   751,
      -1,     7,     8,   755,    -1,    -1,   662,   663,    32,   665,
      34,   763,   668,   765,    20,    21,    22,    23,   674,   771,
      -1,    -1,   774,    -1,    -1,    -1,   778,    51,    52,    -1,
      -1,    37,    -1,    -1,    -1,     1,    -1,    61,    62,    63,
       6,    -1,   704,   699,    10,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    59,   710,    -1,    -1,    23,    -1,    -1,
      26,    -1,    28,    29,    30,   434,    32,    -1,    34,    -1,
      36,   601,    38,    -1,    80,   605,    -1,    43,   608,    45,
      -1,    -1,    48,    -1,    -1,    51,    -1,    93,    54,    95,
      56,    -1,    -1,    -1,     1,    61,    62,    63,    -1,   755,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,   114,   115,
      -1,    -1,   118,   643,   644,   645,   646,    -1,    -1,    -1,
     776,    28,    29,    30,    -1,    32,    -1,    34,    -1,   659,
     136,    38,    -1,    -1,    -1,   141,   666,   143,     1,   508,
      -1,    48,   672,   673,    51,    -1,   676,    54,    -1,   518,
     680,    -1,    -1,   522,    61,    62,    63,    -1,    -1,    -1,
      -1,    68,    -1,    -1,   694,    -1,   172,   697,    -1,    32,
      -1,    34,    -1,    -1,    -1,    38,    -1,   707,   708,    -1,
      -1,    -1,   188,    -1,   190,   191,    -1,    -1,    51,    -1,
      53,    54,   198,   199,   200,    -1,    -1,    -1,    61,    62,
      63,    -1,    -1,    19,    -1,    68,    -1,   213,    -1,    -1,
     216,    -1,    -1,   743,    -1,    -1,   746,   747,    -1,    -1,
      -1,   751,    -1,    -1,    -1,   755,    -1,    -1,    -1,    -1,
      -1,   600,     1,   763,    -1,   765,   242,    -1,    -1,    -1,
      -1,   771,    -1,    -1,   774,    -1,    -1,    63,    64,    65,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    32,    33,    34,    -1,    -1,    -1,    38,
      -1,    -1,   278,   279,   643,   644,   645,   646,    47,    -1,
      -1,    -1,    51,    52,    53,    54,   292,    -1,   294,    -1,
     659,    -1,    61,    62,    63,    -1,    -1,   666,    -1,    68,
      -1,    -1,    -1,   672,   673,    -1,    -1,   676,    -1,    -1,
      -1,   680,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
      28,    29,    30,   139,    32,   694,    34,    -1,   697,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,   153,   707,   708,
      48,    -1,    -1,    51,    -1,   714,    54,   163,   354,    -1,
     166,    -1,   168,    61,    62,    63,    -1,     1,   174,    -1,
      68,    -1,     6,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      14,    -1,    -1,    -1,   743,    -1,    -1,   746,   747,    23,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    -1,
      34,    -1,    36,    -1,    38,    -1,   765,    -1,    -1,    43,
      -1,    45,   771,    -1,    48,   774,    -1,    51,    -1,    -1,
      54,     1,    56,    -1,    -1,    -1,    -1,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   438,   439,    -1,    -1,    -1,    27,    -1,   255,
     256,    -1,    32,    33,    34,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,   270,   461,    -1,    47,   274,    -1,
     276,    51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    63,   480,   481,   482,     1,    68,    -1,
     296,    -1,     6,   299,    -1,   301,    10,    -1,   304,   305,
      14,   307,    -1,    -1,   310,   311,   502,    -1,   504,    23,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    43,
      -1,    45,    -1,    -1,    48,    49,    -1,    51,    52,    53,
      54,    -1,    -1,    57,    58,    -1,    -1,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    69,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,    -1,    -1,     6,    -1,    -1,    -1,
      10,   567,   568,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    26,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,
      32,    -1,    34,    43,    36,    45,    38,    -1,    48,   415,
      -1,    51,   418,   419,    54,    -1,    56,    -1,    -1,    51,
      -1,    61,    62,    63,    -1,    -1,    -1,   433,    68,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     446,   447,    -1,   449,    -1,   451,    -1,    32,   454,    34,
      -1,    36,   648,    38,    -1,    -1,    -1,    -1,   464,    -1,
      -1,   657,   468,    -1,   470,    -1,    51,   473,    53,    -1,
       1,    -1,   478,    -1,    59,     6,    61,    62,    63,    10,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    -1,    26,    -1,    28,    29,    30,
     506,    32,    -1,    34,    -1,    36,    -1,    38,   704,    -1,
      -1,    -1,    43,    -1,    45,    -1,     1,    48,    -1,    -1,
      51,     6,    -1,    54,    -1,    10,   532,   533,    -1,    14,
      61,    62,    63,    -1,    -1,   541,    -1,    68,    23,    -1,
      -1,    26,    -1,    28,    29,    30,    -1,    32,    -1,    34,
      -1,    36,    -1,    38,    -1,    -1,    -1,     1,    43,    -1,
      45,    -1,    -1,    48,    -1,    -1,    51,   573,    -1,    54,
      -1,    -1,    16,    17,    18,    -1,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    28,    29,    30,    -1,    32,     1,
      34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,
      54,    55,    -1,    -1,    -1,    27,    -1,    61,    62,    63,
      32,     1,    34,    67,    68,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    32,     1,    34,    -1,    68,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    51,    52,    53,    54,    -1,    -1,    -1,    -1,    27,
      -1,    61,    62,    63,    32,     1,    34,    -1,    68,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    49,    -1,    51,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    27,    -1,    61,    62,    63,    32,    -1,    34,    -1,
      68,    -1,    38,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    32,    -1,    34,    -1,    51,    -1,    38,    54,    -1,
      -1,    -1,    -1,    -1,     1,    61,    62,    63,    49,    -1,
      51,    -1,    68,    54,    -1,    -1,    -1,    32,    -1,    34,
      61,    62,    63,    38,    -1,     1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    32,    51,    34,    -1,    54,
      -1,    38,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    51,    -1,    32,    54,    34,    -1,
      -1,    -1,    38,    -1,    61,    62,    63,    -1,    -1,    -1,
      -1,    68,    -1,     6,    -1,    51,    -1,    10,    54,    -1,
      -1,    14,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,
      23,    -1,    68,    26,    -1,    28,    29,    30,    -1,    32,
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
      -1,    -1,    32,    33,    34,    61,    62,    63,    38,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    54,    -1,    -1,    57,    32,    -1,
      34,    61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    51,    52,    53,
      54,    -1,    -1,    -1,    32,    -1,    34,    61,    62,    63,
      38,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,    -1,
      32,    33,    34,    61,    62,    63,    38,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    49,    32,    51,
      34,    -1,    54,    -1,    38,    57,    -1,    -1,    -1,    61,
      62,    63,    -1,    -1,    -1,    -1,    68,    51,    52,    53,
      54,    -1,    -1,    -1,    32,    -1,    34,    61,    62,    63,
      38,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,
      32,    -1,    34,    61,    62,    63,    38,    -1,    40,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    54,    -1,    -1,    -1,    32,    -1,    34,    61,
      62,    63,    38,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    32,    33,    34,    61,    62,    63,    38,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    51,    34,    -1,    54,    -1,    38,    57,    -1,    -1,
      -1,    61,    62,    63,    -1,    47,    -1,    -1,    68,    51,
      -1,    -1,    54,    -1,    -1,    -1,    32,    -1,    34,    61,
      62,    63,    38,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    32,    51,    34,    -1,    54,    -1,
      38,    -1,    40,    -1,    -1,    61,    62,    63,    -1,    32,
      -1,    34,    68,    51,    -1,    38,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    62,    63,    -1,    50,    51,    -1,
      68,    54,    -1,    -1,    -1,    32,    -1,    34,    61,    62,
      63,    38,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    32,    51,    34,    -1,    54,    -1,    38,
      -1,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,    -1,
      49,    68,    51,    -1,    32,    54,    34,    -1,    -1,    -1,
      38,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,
      -1,    49,    32,    51,    34,    -1,    54,    -1,    38,    -1,
      -1,    -1,    -1,    61,    62,    63,    -1,    47,    -1,    -1,
      68,    51,    -1,    32,    54,    34,    -1,    -1,    -1,    38,
      -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    32,    51,    34,    -1,    54,    -1,    38,    -1,    -1,
      -1,    -1,    61,    62,    63,    -1,    32,    -1,    34,    68,
      51,    -1,    38,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    -1,    51,    -1,    68,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,
      -1,    -1,    68
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,    71,     1,     6,    10,    14,
      23,    26,    28,    29,    30,    32,    34,    36,    38,    43,
      45,    48,    51,    54,    61,    62,    63,    68,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   180,   181,   198,
     200,   202,   203,   204,   205,    34,    38,    51,   127,   132,
     201,    59,    72,    73,     0,   168,   168,    66,   141,    66,
      28,    29,    30,    48,    51,    54,    68,   161,   162,   179,
     198,   202,   205,   175,   176,   181,    31,    33,    35,    37,
      45,    49,    52,    53,    57,    58,    69,   168,   175,   182,
     199,   207,   209,   211,   212,   213,    56,   168,   182,   195,
      13,   152,    39,    31,    33,    35,    37,    45,    49,    58,
      69,   210,   211,   213,   215,   215,   181,    66,    42,    66,
      53,   132,   135,    51,    54,    68,   133,   134,   200,   201,
       1,    74,   201,    66,   216,     7,    11,    55,   177,   142,
     143,    15,    14,    55,   169,   189,   190,   191,   161,    28,
      31,    33,    37,    45,    49,    58,    69,   153,   154,   155,
     156,   157,   159,   160,   162,   163,   198,   202,    45,   153,
     154,   164,    47,   161,    42,    66,    53,    53,    53,    53,
      53,    53,    34,    36,    38,   200,    53,    53,    52,    53,
     215,    52,    53,    52,    53,   170,   171,   170,    41,    44,
      52,    56,   141,     1,    51,   124,   126,   127,   128,   129,
     131,   132,   134,    45,   175,   176,    45,   175,   176,    51,
     192,   193,   194,   202,   203,   181,   192,    52,    53,    52,
      53,    47,    51,    53,   129,   132,   135,   136,   199,    56,
     128,   132,    51,    77,     8,     9,    16,    17,    18,    19,
      20,    21,    24,    25,    28,    51,    55,    60,    64,    65,
      76,    83,    85,    93,    94,   137,   140,   144,   145,   146,
     147,   155,   157,   159,   202,    76,    66,   168,    11,    12,
      55,   178,     1,    55,    67,   144,   217,    55,   217,   168,
     141,   189,    46,   217,    55,    57,   213,     1,   158,   160,
     161,    52,    53,    39,   213,   213,   161,    52,    53,   161,
     213,    52,    56,   168,   161,   165,   166,   167,   202,   203,
      57,    57,    57,    57,   168,    53,   168,    53,    53,   168,
      14,   168,   196,   197,   168,    27,    53,   123,   124,   132,
     135,    47,    50,    47,   133,    47,   175,   176,   175,   176,
      35,    45,    67,    52,    40,    67,   132,   132,    53,    52,
      53,    52,    53,    52,    53,    52,    53,    53,    56,    34,
      38,    51,    52,    53,    59,    78,    79,   201,   203,   205,
      13,     1,   127,   132,     1,    34,    95,     1,    28,   138,
       1,   138,     1,   138,     1,    51,   110,   111,   202,     1,
     127,   132,     1,    51,     1,   112,   127,   132,     1,   113,
     127,   132,    45,    57,    69,   206,   209,   146,   147,   202,
      76,     1,    30,    62,    75,   201,     1,    28,    60,   202,
      67,    55,    84,    55,    39,    52,     1,    39,    40,    44,
     148,   149,   150,   151,   161,   148,   206,   206,    45,    69,
     161,   208,   209,   217,    55,   153,   183,   184,   185,   168,
     168,    12,   168,   191,    45,   158,   153,   128,    45,   158,
      45,   158,   153,    45,   158,   153,    67,    52,    40,   141,
      46,    52,    41,   125,   200,    53,    53,    53,   128,   128,
     128,   128,   194,   168,   128,   129,   132,   129,   132,   128,
      53,    75,    52,    53,    51,     1,    66,    50,    40,    40,
     200,    57,   139,   206,   212,   214,   139,   139,    39,    52,
      30,    50,    40,   114,   115,   128,    44,   116,    50,   152,
      50,   155,    53,    53,    75,    51,    61,    63,    86,   202,
     202,    84,    85,    94,   144,     1,    27,   120,   121,   122,
     124,   127,   130,   131,   132,   202,   128,   168,   196,   152,
     151,   155,   155,   155,   155,   183,     1,    44,    47,   186,
     187,   188,   217,    55,   168,   158,   158,   158,   158,   167,
     153,   168,   197,   168,    58,   200,    53,    79,    41,    52,
      80,    81,    82,   203,   205,    76,     1,    95,     1,    27,
      49,    51,    98,    99,   101,   102,   124,   131,   132,   204,
     128,    52,   120,   111,    95,    99,   104,    53,    52,     1,
     117,   118,   119,   152,   132,   132,   161,   161,    63,    86,
      34,    52,    87,    88,    89,   202,    51,    75,    30,   202,
      30,    93,   125,    47,    50,    47,    47,   148,    40,   196,
     168,   152,   188,   185,   126,    53,    53,    52,   217,    40,
     125,   130,   131,   132,    33,    22,    44,   107,    49,   124,
     133,    57,   212,   212,    49,   124,   212,    66,    15,   214,
      40,   107,   128,    52,    47,   200,    75,    51,    53,    52,
      87,    86,   202,    30,    39,   202,   202,    58,   122,   132,
     122,   122,   122,   168,    47,    82,    98,    58,   212,    53,
      51,   201,    99,   133,    49,   103,   124,   130,   103,   133,
     103,    67,   105,   106,   140,   202,    96,    97,   202,   104,
     118,   119,    86,    41,    51,    52,    90,    91,    92,   202,
     204,    89,    53,    39,   202,   120,    39,    39,   121,   168,
     107,    51,   100,   101,   127,   132,   103,   108,   109,   201,
     130,    52,    67,    39,    52,    39,   107,    53,    53,    52,
     120,    39,   120,   120,    50,    53,    52,   106,    49,   123,
     128,    97,   120,    92,   120,   101,   201,   128
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
     153,   154,   154,   155,   155,   155,   156,   156,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   158,   158,
     159,   159,   160,   160,   161,   161,   161,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   163,   163,   164,
     164,   165,   165,   166,   166,   167,   167,   168,   168,   169,
     169,   170,   170,   171,   171,   172,   172,   173,   173,   173,
     173,   173,   174,   174,   174,   174,   174,   175,   175,   175,
     176,   176,   176,   177,   177,   178,   178,   179,   179,   180,
     180,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   182,   182,
     183,   183,   184,   184,   184,   185,   186,   186,   186,   187,
     187,   188,   189,   189,   190,   190,   190,   191,   191,   191,
     192,   192,   193,   193,   194,   194,   195,   195,   195,   195,
     195,   195,   195,   196,   196,   197,   197,   197,   198,   198,
     198,   198,   199,   199,   200,   200,   200,   200,   201,   201,
     202,   202,   202,   202,   202,   202,   203,   203,   203,   204,
     204,   205,   205,   205,   206,   206,   206,   207,   207,   208,
     208,   209,   209,   209,   209,   210,   210,   211,   211,   211,
     212,   212,   213,   213,   213,   214,   214,   215,   215,   216,
     217,   217
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
       1,     3,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     4,     3,     4,     3,     4,     3,     4,     1,     1,
       1,     1,     2,     2,     1,     1,     1,     3,     1,     4,
       1,     1,     1,     3,     3,     3,     2,     3,     3,     3,
       1,     0,     1,     3,     1,     3,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     4,     3,     2,
       4,     3,     4,     3,     2,     4,     3,     6,     4,     1,
       4,     4,     4,     3,     2,     3,     2,     2,     1,     2,
       1,     1,     3,     2,     1,     1,     4,     4,     1,     1,
       1,     1,     3,     3,     3,     4,     4,     4,     3,     3,
       1,     2,     3,     2,     1,     3,     1,     2,     1,     2,
       1,     4,     1,     2,     3,     2,     1,     3,     2,     1,
       0,     1,     3,     1,     1,     3,     1,     1,     3,     3,
       4,     2,     5,     3,     1,     3,     1,     2,     1,     2,
       2,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     1,     3,     1,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     3,     1,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     0,
       1,     1
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
  case 2: /* start: EXPR exp wherePart  */
#line 99 "parser.y"
                                        {inputExpr = letrec(yyvsp[0],yyvsp[-1]); sp-=2;}
#line 2573 "parser.c"
    break;

  case 3: /* start: CTXT context  */
#line 100 "parser.y"
                                        {inputContext = yyvsp[0];	    sp-=1;}
#line 2579 "parser.c"
    break;

  case 4: /* start: SCRIPT topModule  */
#line 101 "parser.y"
                                        {valDefns  = yyvsp[0];	    sp-=1;}
#line 2585 "parser.c"
    break;

  case 5: /* start: error  */
#line 102 "parser.y"
                                        {syntaxError("input");}
#line 2591 "parser.c"
    break;

  case 6: /* topModule: startMain begin modBody end  */
#line 115 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc3(yyvsp[-1]);
					}
#line 2600 "parser.c"
    break;

  case 7: /* topModule: startMain '{' modBody '}'  */
#line 119 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc4(yyvsp[-1]);
					}
#line 2609 "parser.c"
    break;

  case 8: /* topModule: TMODULE modname expspec WHERE '{' modBody end  */
#line 124 "parser.y"
                                        {setExportList(yyvsp[-4]);   yyval = gc7(yyvsp[-1]);}
#line 2615 "parser.c"
    break;

  case 9: /* topModule: TMODULE modname expspec WHERE error  */
#line 126 "parser.y"
                                        {syntaxError("declaration");}
#line 2621 "parser.c"
    break;

  case 10: /* topModule: TMODULE error  */
#line 127 "parser.y"
                                        {syntaxError("module definition");}
#line 2627 "parser.c"
    break;

  case 11: /* startMain: %empty  */
#line 133 "parser.y"
                                        {startModule(conMain); 
					 yyval = gc0(NIL);}
#line 2634 "parser.c"
    break;

  case 12: /* modname: qconid  */
#line 136 "parser.y"
                                        {startModule(mkCon(mkNestedQual(yyvsp[0]))); yyval = gc1(NIL);}
#line 2640 "parser.c"
    break;

  case 13: /* modid: qconid  */
#line 138 "parser.y"
                                        {yyval = mkCon(mkNestedQual(yyvsp[0]));}
#line 2646 "parser.c"
    break;

  case 14: /* modid: STRINGLIT  */
#line 139 "parser.y"
                                        { String modName = findPathname(textToStr(textOf(yyvsp[0])));
					  if (modName) { /* fillin pathname if known */
					      yyval = mkStr(findText(modName));
					  } else {
					      yyval = yyvsp[0];
					  }
					}
#line 2658 "parser.c"
    break;

  case 15: /* modBody: %empty  */
#line 147 "parser.y"
                                        {yyval = gc0(NIL); }
#line 2664 "parser.c"
    break;

  case 16: /* modBody: ';' modBody  */
#line 148 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 2670 "parser.c"
    break;

  case 17: /* modBody: topDecls  */
#line 149 "parser.y"
                                        {yyval = gc1(yyvsp[0]);}
#line 2676 "parser.c"
    break;

  case 18: /* modBody: impDecls chase  */
#line 150 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2682 "parser.c"
    break;

  case 19: /* modBody: impDecls ';' chase topDecls  */
#line 151 "parser.y"
                                        {yyval = gc4(yyvsp[0]);}
#line 2688 "parser.c"
    break;

  case 20: /* expspec: %empty  */
#line 156 "parser.y"
                                        {yyval = gc0(exportSelf());}
#line 2694 "parser.c"
    break;

  case 21: /* expspec: '(' ')'  */
#line 157 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2700 "parser.c"
    break;

  case 22: /* expspec: '(' ',' ')'  */
#line 158 "parser.y"
                                        {yyval = gc3(NIL);}
#line 2706 "parser.c"
    break;

  case 23: /* expspec: '(' exports ')'  */
#line 159 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2712 "parser.c"
    break;

  case 24: /* expspec: '(' exports ',' ')'  */
#line 160 "parser.y"
                                        {yyval = gc4(yyvsp[-2]);}
#line 2718 "parser.c"
    break;

  case 25: /* exports: exports ',' export  */
#line 162 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2724 "parser.c"
    break;

  case 26: /* exports: export  */
#line 163 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2730 "parser.c"
    break;

  case 27: /* export: qvar  */
#line 168 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2736 "parser.c"
    break;

  case 28: /* export: qcon  */
#line 169 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2742 "parser.c"
    break;

  case 29: /* export: qconid '(' UPTO ')'  */
#line 170 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 2748 "parser.c"
    break;

  case 30: /* export: qconid '(' qnames ')'  */
#line 171 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 2754 "parser.c"
    break;

  case 31: /* export: TMODULE modid  */
#line 172 "parser.y"
                                        {yyval = gc2(ap(MODULEENT,yyvsp[0]));}
#line 2760 "parser.c"
    break;

  case 32: /* qnames: %empty  */
#line 174 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2766 "parser.c"
    break;

  case 33: /* qnames: ','  */
#line 175 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2772 "parser.c"
    break;

  case 34: /* qnames: qnames1  */
#line 176 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2778 "parser.c"
    break;

  case 35: /* qnames: qnames1 ','  */
#line 177 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2784 "parser.c"
    break;

  case 36: /* qnames1: qnames1 ',' qname  */
#line 179 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2790 "parser.c"
    break;

  case 37: /* qnames1: qname  */
#line 180 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2796 "parser.c"
    break;

  case 38: /* qname: qvar  */
#line 182 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2802 "parser.c"
    break;

  case 39: /* qname: qcon  */
#line 183 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2808 "parser.c"
    break;

  case 40: /* impDecls: impDecls ';' impDecl  */
#line 188 "parser.y"
                                        {imps = cons(yyvsp[0],imps); yyval=gc3(NIL);}
#line 2814 "parser.c"
    break;

  case 41: /* impDecls: impDecls ';'  */
#line 189 "parser.y"
                                        {yyval   = gc2(NIL); }
#line 2820 "parser.c"
    break;

  case 42: /* impDecls: impDecl  */
#line 190 "parser.y"
                                        {imps = singleton(yyvsp[0]); yyval=gc1(NIL);}
#line 2826 "parser.c"
    break;

  case 43: /* chase: %empty  */
#line 192 "parser.y"
                                        {if (chase(imps)) {
					     clearStack();
					     onto(imps);
					     done();
					     closeAnyInput();
					     return 0;
					 }
					 yyval = gc0(NIL);
					}
#line 2840 "parser.c"
    break;

  case 44: /* impDecl: IMPORT modid impspec  */
#line 203 "parser.y"
                                        {addUnqualImport(yyvsp[-1],NIL,yyvsp[0]);
					 yyval = gc3(yyvsp[-1]);}
#line 2847 "parser.c"
    break;

  case 45: /* impDecl: IMPORT modid ASMOD modid impspec  */
#line 206 "parser.y"
                                        {addUnqualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc5(yyvsp[-3]);}
#line 2854 "parser.c"
    break;

  case 46: /* impDecl: IMPORT QUALIFIED modid ASMOD modid impspec  */
#line 209 "parser.y"
                                        {addQualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc6(yyvsp[-3]);}
#line 2861 "parser.c"
    break;

  case 47: /* impDecl: IMPORT QUALIFIED modid impspec  */
#line 212 "parser.y"
                                        {addQualImport(yyvsp[-1],yyvsp[-1],yyvsp[0]);
					 yyval = gc4(yyvsp[-1]);}
#line 2868 "parser.c"
    break;

  case 48: /* impDecl: IMPORT error  */
#line 214 "parser.y"
                                        {syntaxError("import declaration");}
#line 2874 "parser.c"
    break;

  case 49: /* impspec: %empty  */
#line 216 "parser.y"
                                        {yyval = gc0(DOTDOT);}
#line 2880 "parser.c"
    break;

  case 50: /* impspec: HIDING '(' imports ')'  */
#line 217 "parser.y"
                                        {yyval = gc4(ap(HIDDEN,yyvsp[-1]));}
#line 2886 "parser.c"
    break;

  case 51: /* impspec: '(' imports ')'  */
#line 218 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2892 "parser.c"
    break;

  case 52: /* imports: %empty  */
#line 220 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2898 "parser.c"
    break;

  case 53: /* imports: ','  */
#line 221 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2904 "parser.c"
    break;

  case 54: /* imports: imports1  */
#line 222 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2910 "parser.c"
    break;

  case 55: /* imports: imports1 ','  */
#line 223 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2916 "parser.c"
    break;

  case 56: /* imports1: imports1 ',' import  */
#line 225 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2922 "parser.c"
    break;

  case 57: /* imports1: import  */
#line 226 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2928 "parser.c"
    break;

  case 58: /* import: var  */
#line 228 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2934 "parser.c"
    break;

  case 59: /* import: CONID  */
#line 229 "parser.y"
                                        {yyval = gc1(pair(yyvsp[0],NONE));}
#line 2940 "parser.c"
    break;

  case 60: /* import: CONID '(' UPTO ')'  */
#line 230 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 2946 "parser.c"
    break;

  case 61: /* import: CONID '(' names ')'  */
#line 231 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 2952 "parser.c"
    break;

  case 62: /* names: %empty  */
#line 233 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2958 "parser.c"
    break;

  case 63: /* names: ','  */
#line 234 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2964 "parser.c"
    break;

  case 64: /* names: names1  */
#line 235 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2970 "parser.c"
    break;

  case 65: /* names: names1 ','  */
#line 236 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2976 "parser.c"
    break;

  case 66: /* names1: names1 ',' name  */
#line 238 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2982 "parser.c"
    break;

  case 67: /* names1: name  */
#line 239 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2988 "parser.c"
    break;

  case 68: /* name: var  */
#line 241 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2994 "parser.c"
    break;

  case 69: /* name: con  */
#line 242 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3000 "parser.c"
    break;

  case 70: /* topDecls: topDecls ';'  */
#line 247 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3006 "parser.c"
    break;

  case 71: /* topDecls: topDecls ';' topDecl  */
#line 248 "parser.y"
                                        {yyval = gc2(yyvsp[-2]);}
#line 3012 "parser.c"
    break;

  case 72: /* topDecls: topDecls ';' decl  */
#line 249 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3018 "parser.c"
    break;

  case 73: /* topDecls: topDecl  */
#line 250 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3024 "parser.c"
    break;

  case 74: /* topDecls: decl  */
#line 251 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3030 "parser.c"
    break;

  case 75: /* topDecl: TYPE tyLhs '=' type  */
#line 256 "parser.y"
                                        {defTycon(4,yyvsp[-1],yyvsp[-2],yyvsp[0],SYNONYM);}
#line 3036 "parser.c"
    break;

  case 76: /* topDecl: TYPE tyLhs '=' type IN invars  */
#line 258 "parser.y"
                                        {defTycon(6,yyvsp[-3],yyvsp[-4],
						    ap(yyvsp[-2],yyvsp[0]),RESTRICTSYN);}
#line 3043 "parser.c"
    break;

  case 77: /* topDecl: TYPE error  */
#line 260 "parser.y"
                                        {syntaxError("type declaration");}
#line 3049 "parser.c"
    break;

  case 78: /* topDecl: DATA btype2 '=' constrs deriving  */
#line 262 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(rev(yyvsp[-1]),yyvsp[0]),DATATYPE);}
#line 3056 "parser.c"
    break;

  case 79: /* topDecl: DATA context IMPLIES tyLhs '=' constrs deriving  */
#line 265 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],rev(yyvsp[-1])),
						     yyvsp[0]),DATATYPE);}
#line 3064 "parser.c"
    break;

  case 80: /* topDecl: DATA btype2  */
#line 268 "parser.y"
                                        {defTycon(2,yyvsp[-1],checkTyLhs(yyvsp[0]),
						    ap(NIL,NIL),DATATYPE);}
#line 3071 "parser.c"
    break;

  case 81: /* topDecl: DATA context IMPLIES tyLhs  */
#line 270 "parser.y"
                                        {defTycon(4,yyvsp[-3],yyvsp[0],
						  ap(qualify(yyvsp[-2],NIL),
						     NIL),DATATYPE);}
#line 3079 "parser.c"
    break;

  case 82: /* topDecl: DATA error  */
#line 273 "parser.y"
                                        {syntaxError("data declaration");}
#line 3085 "parser.c"
    break;

  case 83: /* topDecl: TNEWTYPE btype2 '=' nconstr deriving  */
#line 275 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(yyvsp[-1],yyvsp[0]),NEWTYPE);}
#line 3092 "parser.c"
    break;

  case 84: /* topDecl: TNEWTYPE context IMPLIES tyLhs '=' nconstr deriving  */
#line 278 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],yyvsp[-1]),
						     yyvsp[0]),NEWTYPE);}
#line 3100 "parser.c"
    break;

  case 85: /* topDecl: TNEWTYPE error  */
#line 281 "parser.y"
                                        {syntaxError("newtype declaration");}
#line 3106 "parser.c"
    break;

  case 86: /* topDecl: NEEDPRIMS NUMLIT  */
#line 282 "parser.y"
                                        {if (isInt(yyvsp[0])) {
					     needPrims(intOf(yyvsp[0]), NULL);
					 } else {
					     syntaxError("needprims decl");
					 }
					 sp-=2;}
#line 3117 "parser.c"
    break;

  case 87: /* topDecl: NEEDPRIMS error  */
#line 288 "parser.y"
                                        {syntaxError("needprims decl");}
#line 3123 "parser.c"
    break;

  case 88: /* tyLhs: tyLhs varid  */
#line 290 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3129 "parser.c"
    break;

  case 89: /* tyLhs: CONID  */
#line 291 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3135 "parser.c"
    break;

  case 90: /* tyLhs: error  */
#line 292 "parser.y"
                                        {syntaxError("type defn lhs");}
#line 3141 "parser.c"
    break;

  case 91: /* invars: invars ',' invar  */
#line 294 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3147 "parser.c"
    break;

  case 92: /* invars: invar  */
#line 295 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3153 "parser.c"
    break;

  case 93: /* invar: var COCO topType  */
#line 297 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],singleton(yyvsp[-2]),
									yyvsp[0]));}
#line 3160 "parser.c"
    break;

  case 94: /* invar: var  */
#line 299 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3166 "parser.c"
    break;

  case 95: /* constrs: constrs '|' pconstr  */
#line 301 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3172 "parser.c"
    break;

  case 96: /* constrs: pconstr  */
#line 302 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3178 "parser.c"
    break;

  case 97: /* pconstr: ALL varids '.' qconstr  */
#line 304 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3185 "parser.c"
    break;

  case 98: /* pconstr: constr  */
#line 306 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3191 "parser.c"
    break;

  case 99: /* qconstr: context IMPLIES constr  */
#line 308 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3197 "parser.c"
    break;

  case 100: /* qconstr: constr  */
#line 309 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3203 "parser.c"
    break;

  case 101: /* constr: '!' btype conop bbtype  */
#line 311 "parser.y"
                                        {yyval = gc4(ap(ap(yyvsp[-1],bang(yyvsp[-2])),yyvsp[0]));}
#line 3209 "parser.c"
    break;

  case 102: /* constr: btype1 conop bbtype  */
#line 312 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3215 "parser.c"
    break;

  case 103: /* constr: btype2 conop bbtype  */
#line 313 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3221 "parser.c"
    break;

  case 104: /* constr: bpolyType conop bbtype  */
#line 314 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3227 "parser.c"
    break;

  case 105: /* constr: btype2  */
#line 315 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3233 "parser.c"
    break;

  case 106: /* constr: btype3  */
#line 316 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3239 "parser.c"
    break;

  case 107: /* constr: con '{' fieldspecs '}'  */
#line 317 "parser.y"
                                        {yyval = gc4(ap(LABC,pair(yyvsp[-3],rev(yyvsp[-1]))));}
#line 3245 "parser.c"
    break;

  case 108: /* constr: con '{' '}'  */
#line 318 "parser.y"
                                        {yyval = gc3(ap(LABC,pair(yyvsp[-2],NIL)));}
#line 3251 "parser.c"
    break;

  case 109: /* constr: error  */
#line 319 "parser.y"
                                        {syntaxError("data type declaration");}
#line 3257 "parser.c"
    break;

  case 110: /* btype3: btype2 '!' atype  */
#line 321 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3263 "parser.c"
    break;

  case 111: /* btype3: btype2 bpolyType  */
#line 322 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3269 "parser.c"
    break;

  case 112: /* btype3: btype3 atype  */
#line 323 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3275 "parser.c"
    break;

  case 113: /* btype3: btype3 '!' atype  */
#line 324 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3281 "parser.c"
    break;

  case 114: /* btype3: btype3 bpolyType  */
#line 325 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3287 "parser.c"
    break;

  case 115: /* btype3: '(' CONOP ')'  */
#line 326 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3293 "parser.c"
    break;

  case 116: /* bbtype: '!' btype  */
#line 328 "parser.y"
                                        {yyval = gc2(bang(yyvsp[0]));}
#line 3299 "parser.c"
    break;

  case 117: /* bbtype: btype  */
#line 329 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3305 "parser.c"
    break;

  case 118: /* bbtype: bpolyType  */
#line 330 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3311 "parser.c"
    break;

  case 119: /* nconstr: pconstr  */
#line 332 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3317 "parser.c"
    break;

  case 120: /* fieldspecs: fieldspecs ',' fieldspec  */
#line 334 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3323 "parser.c"
    break;

  case 121: /* fieldspecs: fieldspec  */
#line 335 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3329 "parser.c"
    break;

  case 122: /* fieldspec: vars COCO polyType  */
#line 337 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3335 "parser.c"
    break;

  case 123: /* fieldspec: vars COCO type  */
#line 338 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3341 "parser.c"
    break;

  case 124: /* fieldspec: vars COCO '!' type  */
#line 339 "parser.y"
                                        {yyval = gc4(pair(rev(yyvsp[-3]),bang(yyvsp[0])));}
#line 3347 "parser.c"
    break;

  case 125: /* deriving: %empty  */
#line 341 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3353 "parser.c"
    break;

  case 126: /* deriving: DERIVING qconid  */
#line 342 "parser.y"
                                        {yyval = gc2(singleton(yyvsp[0]));}
#line 3359 "parser.c"
    break;

  case 127: /* deriving: DERIVING '(' derivs0 ')'  */
#line 343 "parser.y"
                                        {yyval = gc4(yyvsp[-1]);}
#line 3365 "parser.c"
    break;

  case 128: /* derivs0: %empty  */
#line 345 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3371 "parser.c"
    break;

  case 129: /* derivs0: derivs  */
#line 346 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3377 "parser.c"
    break;

  case 130: /* derivs: derivs ',' qconid  */
#line 348 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3383 "parser.c"
    break;

  case 131: /* derivs: qconid  */
#line 349 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3389 "parser.c"
    break;

  case 132: /* topDecl: PRIMITIVE prims COCO topType  */
#line 354 "parser.y"
                                        {primDefn(yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=4;}
#line 3395 "parser.c"
    break;

  case 133: /* prims: prims ',' prim  */
#line 356 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3401 "parser.c"
    break;

  case 134: /* prims: prim  */
#line 357 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3407 "parser.c"
    break;

  case 135: /* prims: error  */
#line 358 "parser.y"
                                        {syntaxError("primitive defn");}
#line 3413 "parser.c"
    break;

  case 136: /* prim: var STRINGLIT  */
#line 360 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 3419 "parser.c"
    break;

  case 137: /* prim: var  */
#line 361 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3425 "parser.c"
    break;

  case 138: /* topDecl: FOREIGN IMPORT var STRINGLIT var COCO topType  */
#line 367 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],NIL,yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3431 "parser.c"
    break;

  case 139: /* topDecl: FOREIGN IMPORT var var COCO topType  */
#line 369 "parser.y"
               {foreignImport(yyvsp[-5],yyvsp[-3],NIL,yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=6;}
#line 3437 "parser.c"
    break;

  case 140: /* topDecl: FOREIGN IMPORT var var STRINGLIT var COCO topType  */
#line 371 "parser.y"
               {foreignImport(yyvsp[-7],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=8;}
#line 3443 "parser.c"
    break;

  case 141: /* topDecl: FOREIGN IMPORT var var var COCO topType  */
#line 373 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3449 "parser.c"
    break;

  case 142: /* topDecl: FOREIGN var var STRINGLIT var COCO topType  */
#line 375 "parser.y"
               {foreignExport(yyvsp[-6],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3455 "parser.c"
    break;

  case 143: /* topDecl: TCLASS crule fds wherePart  */
#line 380 "parser.y"
                                        {classDefn(intOf(yyvsp[-3]),yyvsp[-2],yyvsp[0],yyvsp[-1]); sp-=4;}
#line 3461 "parser.c"
    break;

  case 144: /* topDecl: TINSTANCE irule wherePart  */
#line 381 "parser.y"
                                        {instDefn(intOf(yyvsp[-2]),yyvsp[-1],yyvsp[0]);  sp-=3;}
#line 3467 "parser.c"
    break;

  case 145: /* topDecl: DEFAULT '(' dtypes ')'  */
#line 382 "parser.y"
                                        {defaultDefn(intOf(yyvsp[-3]),yyvsp[-1]);  sp-=4;}
#line 3473 "parser.c"
    break;

  case 146: /* topDecl: TCLASS error  */
#line 383 "parser.y"
                                        {syntaxError("class declaration");}
#line 3479 "parser.c"
    break;

  case 147: /* topDecl: TINSTANCE error  */
#line 384 "parser.y"
                                        {syntaxError("instance declaration");}
#line 3485 "parser.c"
    break;

  case 148: /* topDecl: DEFAULT error  */
#line 385 "parser.y"
                                        {syntaxError("default declaration");}
#line 3491 "parser.c"
    break;

  case 149: /* crule: context IMPLIES btype2  */
#line 387 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3497 "parser.c"
    break;

  case 150: /* crule: btype2  */
#line 388 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3503 "parser.c"
    break;

  case 151: /* irule: context IMPLIES btype2  */
#line 390 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3509 "parser.c"
    break;

  case 152: /* irule: btype2  */
#line 391 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3515 "parser.c"
    break;

  case 153: /* dtypes: %empty  */
#line 393 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3521 "parser.c"
    break;

  case 154: /* dtypes: dtypes1  */
#line 394 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3527 "parser.c"
    break;

  case 155: /* dtypes1: dtypes1 ',' type  */
#line 396 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3533 "parser.c"
    break;

  case 156: /* dtypes1: type  */
#line 397 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3539 "parser.c"
    break;

  case 157: /* fds: %empty  */
#line 399 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3545 "parser.c"
    break;

  case 158: /* fds: '|' fds1  */
#line 400 "parser.y"
                                        {yyval = gc2(rev(yyvsp[0]));}
#line 3551 "parser.c"
    break;

  case 159: /* fds1: fds1 ',' fd  */
#line 402 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3557 "parser.c"
    break;

  case 160: /* fds1: fd  */
#line 403 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3563 "parser.c"
    break;

  case 161: /* fd: varids0 ARROW varids0  */
#line 405 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),rev(yyvsp[0])));}
#line 3569 "parser.c"
    break;

  case 162: /* fd: error  */
#line 406 "parser.y"
                                        {syntaxError("functional dependency");}
#line 3575 "parser.c"
    break;

  case 163: /* varids0: %empty  */
#line 408 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3581 "parser.c"
    break;

  case 164: /* varids0: varids0 varid  */
#line 409 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3587 "parser.c"
    break;

  case 165: /* topType: ALL varids '.' topType0  */
#line 414 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3594 "parser.c"
    break;

  case 166: /* topType: topType0  */
#line 416 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3600 "parser.c"
    break;

  case 167: /* topType0: context IMPLIES topType1  */
#line 418 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3606 "parser.c"
    break;

  case 168: /* topType0: topType1  */
#line 419 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3612 "parser.c"
    break;

  case 169: /* topType1: bpolyType ARROW topType1  */
#line 421 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3618 "parser.c"
    break;

  case 170: /* topType1: btype1 ARROW topType1  */
#line 422 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3624 "parser.c"
    break;

  case 171: /* topType1: btype2 ARROW topType1  */
#line 423 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3630 "parser.c"
    break;

  case 172: /* topType1: btype  */
#line 424 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3636 "parser.c"
    break;

  case 173: /* polyType: ALL varids '.' sigType  */
#line 426 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3643 "parser.c"
    break;

  case 174: /* polyType: bpolyType  */
#line 428 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3649 "parser.c"
    break;

  case 175: /* bpolyType: '(' polyType ')'  */
#line 430 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3655 "parser.c"
    break;

  case 176: /* varids: varids varid  */
#line 432 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3661 "parser.c"
    break;

  case 177: /* varids: varid  */
#line 433 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3667 "parser.c"
    break;

  case 178: /* sigType: context IMPLIES type  */
#line 435 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3673 "parser.c"
    break;

  case 179: /* sigType: type  */
#line 436 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3679 "parser.c"
    break;

  case 180: /* context: '(' ')'  */
#line 438 "parser.y"
                                        {yyval = gc2(NIL);}
#line 3685 "parser.c"
    break;

  case 181: /* context: btype2  */
#line 439 "parser.y"
                                        {yyval = gc1(singleton(checkPred(yyvsp[0])));}
#line 3691 "parser.c"
    break;

  case 182: /* context: '(' btype2 ')'  */
#line 440 "parser.y"
                                        {yyval = gc3(singleton(checkPred(yyvsp[-1])));}
#line 3697 "parser.c"
    break;

  case 183: /* context: '(' btypes2 ')'  */
#line 441 "parser.y"
                                        {yyval = gc3(checkCtxt(rev(yyvsp[-1])));}
#line 3703 "parser.c"
    break;

  case 184: /* type: type1  */
#line 443 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3709 "parser.c"
    break;

  case 185: /* type: btype2  */
#line 444 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3715 "parser.c"
    break;

  case 186: /* type1: btype1  */
#line 446 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3721 "parser.c"
    break;

  case 187: /* type1: bpolyType ARROW type  */
#line 447 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3727 "parser.c"
    break;

  case 188: /* type1: btype1 ARROW type  */
#line 448 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3733 "parser.c"
    break;

  case 189: /* type1: btype2 ARROW type  */
#line 449 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3739 "parser.c"
    break;

  case 190: /* type1: error  */
#line 450 "parser.y"
                                        {syntaxError("type expression");}
#line 3745 "parser.c"
    break;

  case 191: /* btype: btype1  */
#line 452 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3751 "parser.c"
    break;

  case 192: /* btype: btype2  */
#line 453 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3757 "parser.c"
    break;

  case 193: /* btype1: btype1 atype  */
#line 455 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3763 "parser.c"
    break;

  case 194: /* btype1: atype1  */
#line 456 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3769 "parser.c"
    break;

  case 195: /* btype2: btype2 atype  */
#line 458 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3775 "parser.c"
    break;

  case 196: /* btype2: qconid  */
#line 459 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3781 "parser.c"
    break;

  case 197: /* atype: atype1  */
#line 461 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3787 "parser.c"
    break;

  case 198: /* atype: qconid  */
#line 462 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3793 "parser.c"
    break;

  case 199: /* atype1: varid  */
#line 464 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3799 "parser.c"
    break;

  case 200: /* atype1: '(' ')'  */
#line 465 "parser.y"
                                        {yyval = gc2(typeUnit);}
#line 3805 "parser.c"
    break;

  case 201: /* atype1: '(' ARROW ')'  */
#line 466 "parser.y"
                                        {yyval = gc3(typeArrow);}
#line 3811 "parser.c"
    break;

  case 202: /* atype1: '(' type1 ')'  */
#line 467 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3817 "parser.c"
    break;

  case 203: /* atype1: '(' btype2 ')'  */
#line 468 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3823 "parser.c"
    break;

  case 204: /* atype1: '(' tupCommas ')'  */
#line 469 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3829 "parser.c"
    break;

  case 205: /* atype1: '(' btypes2 ')'  */
#line 470 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3835 "parser.c"
    break;

  case 206: /* atype1: '(' typeTuple ')'  */
#line 471 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3841 "parser.c"
    break;

  case 207: /* atype1: '[' type ']'  */
#line 472 "parser.y"
                                        {yyval = gc3(ap(typeList,yyvsp[-1]));}
#line 3847 "parser.c"
    break;

  case 208: /* atype1: '[' ']'  */
#line 473 "parser.y"
                                        {yyval = gc2(typeList);}
#line 3853 "parser.c"
    break;

  case 209: /* atype1: '_'  */
#line 474 "parser.y"
                                        {yyval = gc1(inventVar());}
#line 3859 "parser.c"
    break;

  case 210: /* btypes2: btypes2 ',' btype2  */
#line 476 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3865 "parser.c"
    break;

  case 211: /* btypes2: btype2 ',' btype2  */
#line 477 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3871 "parser.c"
    break;

  case 212: /* typeTuple: type1 ',' type  */
#line 479 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3877 "parser.c"
    break;

  case 213: /* typeTuple: btype2 ',' type1  */
#line 480 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3883 "parser.c"
    break;

  case 214: /* typeTuple: btypes2 ',' type1  */
#line 481 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3889 "parser.c"
    break;

  case 215: /* typeTuple: typeTuple ',' type  */
#line 482 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3895 "parser.c"
    break;

  case 216: /* gendecl: INFIXN optDigit ops  */
#line 487 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],NON_ASS,yyvsp[-1]));}
#line 3901 "parser.c"
    break;

  case 217: /* gendecl: INFIXN error  */
#line 488 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3907 "parser.c"
    break;

  case 218: /* gendecl: INFIXL optDigit ops  */
#line 489 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],LEFT_ASS,yyvsp[-1]));}
#line 3913 "parser.c"
    break;

  case 219: /* gendecl: INFIXL error  */
#line 490 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3919 "parser.c"
    break;

  case 220: /* gendecl: INFIXR optDigit ops  */
#line 491 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],RIGHT_ASS,yyvsp[-1]));}
#line 3925 "parser.c"
    break;

  case 221: /* gendecl: INFIXR error  */
#line 492 "parser.y"
                                        {syntaxError("fixity decl");}
#line 3931 "parser.c"
    break;

  case 222: /* gendecl: vars COCO topType  */
#line 493 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 3937 "parser.c"
    break;

  case 223: /* gendecl: vars COCO error  */
#line 494 "parser.y"
                                        {syntaxError("type signature");}
#line 3943 "parser.c"
    break;

  case 224: /* optDigit: NUMLIT  */
#line 496 "parser.y"
                                        {yyval = gc1(checkPrec(yyvsp[0]));}
#line 3949 "parser.c"
    break;

  case 225: /* optDigit: %empty  */
#line 497 "parser.y"
                                        {yyval = gc0(mkInt(DEF_PREC));}
#line 3955 "parser.c"
    break;

  case 226: /* ops: ops ',' op  */
#line 499 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3961 "parser.c"
    break;

  case 227: /* ops: op  */
#line 500 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3967 "parser.c"
    break;

  case 228: /* vars: vars ',' var  */
#line 502 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3973 "parser.c"
    break;

  case 229: /* vars: var  */
#line 503 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3979 "parser.c"
    break;

  case 230: /* decls: '{' decls0 end  */
#line 505 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3985 "parser.c"
    break;

  case 231: /* decls: '{' decls1 end  */
#line 506 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3991 "parser.c"
    break;

  case 232: /* decls0: %empty  */
#line 508 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3997 "parser.c"
    break;

  case 233: /* decls0: decls0 ';'  */
#line 509 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4003 "parser.c"
    break;

  case 234: /* decls0: decls1 ';'  */
#line 510 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4009 "parser.c"
    break;

  case 235: /* decls1: decls0 decl  */
#line 512 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4015 "parser.c"
    break;

  case 236: /* decl: gendecl  */
#line 514 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4021 "parser.c"
    break;

  case 237: /* decl: funlhs rhs  */
#line 515 "parser.y"
                                        {yyval = gc2(ap(FUNBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4027 "parser.c"
    break;

  case 238: /* decl: funlhs COCO type rhs  */
#line 516 "parser.y"
                                        {yyval = gc4(ap(FUNBIND,
						     pair(yyvsp[-3],ap(RSIGN,
								ap(yyvsp[0],yyvsp[-1])))));}
#line 4035 "parser.c"
    break;

  case 239: /* decl: pat0 rhs  */
#line 519 "parser.y"
                                        {yyval = gc2(ap(PATBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4041 "parser.c"
    break;

  case 240: /* funlhs: funlhs0  */
#line 521 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4047 "parser.c"
    break;

  case 241: /* funlhs: funlhs1  */
#line 522 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4053 "parser.c"
    break;

  case 242: /* funlhs0: pat10_vI varop pat0  */
#line 524 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4059 "parser.c"
    break;

  case 243: /* funlhs0: var '+' pat0  */
#line 525 "parser.y"
                                        {yyval = gc3(ap2(varPlus,yyvsp[-2],yyvsp[0]));}
#line 4065 "parser.c"
    break;

  case 244: /* funlhs0: infixPat varop pat0  */
#line 526 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4071 "parser.c"
    break;

  case 245: /* funlhs0: NUMLIT varop pat0  */
#line 527 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4077 "parser.c"
    break;

  case 246: /* funlhs0: var varop_pl pat0  */
#line 528 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4083 "parser.c"
    break;

  case 247: /* funlhs1: '(' funlhs0 ')' apat  */
#line 530 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4089 "parser.c"
    break;

  case 248: /* funlhs1: '(' funlhs1 ')' apat  */
#line 531 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4095 "parser.c"
    break;

  case 249: /* funlhs1: var apat  */
#line 532 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4101 "parser.c"
    break;

  case 250: /* funlhs1: funlhs1 apat  */
#line 533 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4107 "parser.c"
    break;

  case 251: /* rhs: rhs1 wherePart  */
#line 535 "parser.y"
                                        {yyval = gc2(letrec(yyvsp[0],yyvsp[-1]));}
#line 4113 "parser.c"
    break;

  case 252: /* rhs: error  */
#line 536 "parser.y"
                                        {syntaxError("declaration");}
#line 4119 "parser.c"
    break;

  case 253: /* rhs1: '=' exp  */
#line 538 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 4125 "parser.c"
    break;

  case 254: /* rhs1: gdrhs  */
#line 539 "parser.y"
                                        {yyval = gc1(desugarGuards(rev(yyvsp[0])));}
#line 4131 "parser.c"
    break;

  case 255: /* gdrhs: gdrhs gddef  */
#line 541 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4137 "parser.c"
    break;

  case 256: /* gdrhs: gddef  */
#line 542 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4143 "parser.c"
    break;

  case 257: /* gddef: '|' quals '=' exp  */
#line 544 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 4149 "parser.c"
    break;

  case 258: /* wherePart: %empty  */
#line 546 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4155 "parser.c"
    break;

  case 259: /* wherePart: WHERE decls  */
#line 547 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4161 "parser.c"
    break;

  case 260: /* pat: pat_npk  */
#line 551 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4167 "parser.c"
    break;

  case 261: /* pat_npk: pat0 COCO type  */
#line 553 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4173 "parser.c"
    break;

  case 262: /* pat_npk: pat0  */
#line 554 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4179 "parser.c"
    break;

  case 263: /* pat0: var  */
#line 556 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4185 "parser.c"
    break;

  case 264: /* pat0: NUMLIT  */
#line 557 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4191 "parser.c"
    break;

  case 265: /* pat0: pat0_vI  */
#line 558 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4197 "parser.c"
    break;

  case 266: /* pat0_vI: pat10_vI  */
#line 560 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4203 "parser.c"
    break;

  case 267: /* pat0_vI: infixPat  */
#line 561 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4209 "parser.c"
    break;

  case 268: /* infixPat: '-' pat10  */
#line 563 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4215 "parser.c"
    break;

  case 269: /* infixPat: '-' error  */
#line 564 "parser.y"
                                        {syntaxError("pattern");}
#line 4221 "parser.c"
    break;

  case 270: /* infixPat: var qconop pat10  */
#line 565 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4227 "parser.c"
    break;

  case 271: /* infixPat: var qconop '-' pat10  */
#line 566 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4233 "parser.c"
    break;

  case 272: /* infixPat: NUMLIT qconop pat10  */
#line 567 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4239 "parser.c"
    break;

  case 273: /* infixPat: NUMLIT qconop '-' pat10  */
#line 568 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4245 "parser.c"
    break;

  case 274: /* infixPat: pat10_vI qconop pat10  */
#line 569 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4251 "parser.c"
    break;

  case 275: /* infixPat: pat10_vI qconop '-' pat10  */
#line 570 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4257 "parser.c"
    break;

  case 276: /* infixPat: infixPat qconop pat10  */
#line 571 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4263 "parser.c"
    break;

  case 277: /* infixPat: infixPat qconop '-' pat10  */
#line 572 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4269 "parser.c"
    break;

  case 278: /* pat10: fpat  */
#line 574 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4275 "parser.c"
    break;

  case 279: /* pat10: apat  */
#line 575 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4281 "parser.c"
    break;

  case 280: /* pat10_vI: fpat  */
#line 577 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4287 "parser.c"
    break;

  case 281: /* pat10_vI: apat_vI  */
#line 578 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4293 "parser.c"
    break;

  case 282: /* fpat: fpat apat  */
#line 580 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4299 "parser.c"
    break;

  case 283: /* fpat: gcon apat  */
#line 581 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4305 "parser.c"
    break;

  case 284: /* apat: NUMLIT  */
#line 583 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4311 "parser.c"
    break;

  case 285: /* apat: var  */
#line 584 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4317 "parser.c"
    break;

  case 286: /* apat: apat_vI  */
#line 585 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4323 "parser.c"
    break;

  case 287: /* apat_vI: var '@' apat  */
#line 587 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4329 "parser.c"
    break;

  case 288: /* apat_vI: gcon  */
#line 588 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4335 "parser.c"
    break;

  case 289: /* apat_vI: qcon '{' patbinds '}'  */
#line 589 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4341 "parser.c"
    break;

  case 290: /* apat_vI: CHARLIT  */
#line 590 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4347 "parser.c"
    break;

  case 291: /* apat_vI: STRINGLIT  */
#line 591 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4353 "parser.c"
    break;

  case 292: /* apat_vI: '_'  */
#line 592 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4359 "parser.c"
    break;

  case 293: /* apat_vI: '(' pat_npk ')'  */
#line 593 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4365 "parser.c"
    break;

  case 294: /* apat_vI: '(' pats2 ')'  */
#line 594 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4371 "parser.c"
    break;

  case 295: /* apat_vI: '[' pats1 ']'  */
#line 595 "parser.y"
                                        {yyval = gc3(ap(FINLIST,rev(yyvsp[-1])));}
#line 4377 "parser.c"
    break;

  case 296: /* apat_vI: '~' apat  */
#line 596 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4383 "parser.c"
    break;

  case 297: /* pats2: pats2 ',' pat  */
#line 598 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4389 "parser.c"
    break;

  case 298: /* pats2: pat ',' pat  */
#line 599 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],singleton(yyvsp[-2])));}
#line 4395 "parser.c"
    break;

  case 299: /* pats1: pats1 ',' pat  */
#line 601 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4401 "parser.c"
    break;

  case 300: /* pats1: pat  */
#line 602 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4407 "parser.c"
    break;

  case 301: /* patbinds: %empty  */
#line 604 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4413 "parser.c"
    break;

  case 302: /* patbinds: patbinds1  */
#line 605 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 4419 "parser.c"
    break;

  case 303: /* patbinds1: patbinds1 ',' patbind  */
#line 607 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4425 "parser.c"
    break;

  case 304: /* patbinds1: patbind  */
#line 608 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4431 "parser.c"
    break;

  case 305: /* patbind: qvar '=' pat  */
#line 610 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 4437 "parser.c"
    break;

  case 306: /* patbind: var  */
#line 611 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4443 "parser.c"
    break;

  case 307: /* exp: exp_err  */
#line 616 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4449 "parser.c"
    break;

  case 308: /* exp: error  */
#line 617 "parser.y"
                                        {syntaxError("expression");}
#line 4455 "parser.c"
    break;

  case 309: /* exp_err: exp0a COCO sigType  */
#line 619 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4461 "parser.c"
    break;

  case 310: /* exp_err: exp0  */
#line 620 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4467 "parser.c"
    break;

  case 311: /* exp0: exp0a  */
#line 622 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4473 "parser.c"
    break;

  case 312: /* exp0: exp0b  */
#line 623 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4479 "parser.c"
    break;

  case 313: /* exp0a: infixExpa  */
#line 625 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4485 "parser.c"
    break;

  case 314: /* exp0a: exp10a  */
#line 626 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4491 "parser.c"
    break;

  case 315: /* exp0b: infixExpb  */
#line 628 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4497 "parser.c"
    break;

  case 316: /* exp0b: exp10b  */
#line 629 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4503 "parser.c"
    break;

  case 317: /* infixExpa: infixExpa qop '-' exp10a  */
#line 631 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4509 "parser.c"
    break;

  case 318: /* infixExpa: infixExpa qop exp10a  */
#line 632 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4515 "parser.c"
    break;

  case 319: /* infixExpa: '-' exp10a  */
#line 633 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4521 "parser.c"
    break;

  case 320: /* infixExpa: exp10a qop '-' exp10a  */
#line 634 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4528 "parser.c"
    break;

  case 321: /* infixExpa: exp10a qop exp10a  */
#line 636 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4534 "parser.c"
    break;

  case 322: /* infixExpb: infixExpa qop '-' exp10b  */
#line 638 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4540 "parser.c"
    break;

  case 323: /* infixExpb: infixExpa qop exp10b  */
#line 639 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4546 "parser.c"
    break;

  case 324: /* infixExpb: '-' exp10b  */
#line 640 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4552 "parser.c"
    break;

  case 325: /* infixExpb: exp10a qop '-' exp10b  */
#line 641 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4559 "parser.c"
    break;

  case 326: /* infixExpb: exp10a qop exp10b  */
#line 643 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4565 "parser.c"
    break;

  case 327: /* exp10a: CASEXP exp OF '{' alts end  */
#line 645 "parser.y"
                                        {yyval = gc6(ap(CASE,pair(yyvsp[-4],rev(yyvsp[-1]))));}
#line 4571 "parser.c"
    break;

  case 328: /* exp10a: DO '{' stmts end  */
#line 646 "parser.y"
                                        {yyval = gc4(ap(DOCOMP,checkDo(yyvsp[-1])));}
#line 4577 "parser.c"
    break;

  case 329: /* exp10a: appExp  */
#line 647 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4583 "parser.c"
    break;

  case 330: /* exp10b: '\\' pats ARROW exp  */
#line 649 "parser.y"
                                        {yyval = gc4(ap(LAMBDA,      
						     pair(rev(yyvsp[-2]),
							  pair(yyvsp[-1],yyvsp[0]))));}
#line 4591 "parser.c"
    break;

  case 331: /* exp10b: LET decls IN exp  */
#line 652 "parser.y"
                                        {yyval = gc4(letrec(yyvsp[-2],yyvsp[0]));}
#line 4597 "parser.c"
    break;

  case 332: /* exp10b: IF exp then_exp else_exp  */
#line 653 "parser.y"
                                        {yyval = gc4(ap(COND,triple(yyvsp[-2],yyvsp[-1],yyvsp[0])));}
#line 4603 "parser.c"
    break;

  case 333: /* then_exp: ';' THEN exp  */
#line 658 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4609 "parser.c"
    break;

  case 334: /* then_exp: THEN exp  */
#line 659 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4615 "parser.c"
    break;

  case 335: /* else_exp: ';' ELSE exp  */
#line 661 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4621 "parser.c"
    break;

  case 336: /* else_exp: ELSE exp  */
#line 662 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4627 "parser.c"
    break;

  case 337: /* pats: pats apat  */
#line 665 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4633 "parser.c"
    break;

  case 338: /* pats: apat  */
#line 666 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4639 "parser.c"
    break;

  case 339: /* appExp: appExp aexp  */
#line 668 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4645 "parser.c"
    break;

  case 340: /* appExp: aexp  */
#line 669 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4651 "parser.c"
    break;

  case 341: /* aexp: qvar  */
#line 671 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4657 "parser.c"
    break;

  case 342: /* aexp: qvar '@' aexp  */
#line 672 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4663 "parser.c"
    break;

  case 343: /* aexp: '~' aexp  */
#line 673 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4669 "parser.c"
    break;

  case 344: /* aexp: '_'  */
#line 674 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4675 "parser.c"
    break;

  case 345: /* aexp: gcon  */
#line 675 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4681 "parser.c"
    break;

  case 346: /* aexp: qcon '{' fbinds '}'  */
#line 676 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4687 "parser.c"
    break;

  case 347: /* aexp: aexp '{' fbinds '}'  */
#line 677 "parser.y"
                                        {yyval = gc4(ap(UPDFLDS,
						     triple(yyvsp[-3],NIL,yyvsp[-1])));}
#line 4694 "parser.c"
    break;

  case 348: /* aexp: NUMLIT  */
#line 679 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4700 "parser.c"
    break;

  case 349: /* aexp: CHARLIT  */
#line 680 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4706 "parser.c"
    break;

  case 350: /* aexp: STRINGLIT  */
#line 681 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4712 "parser.c"
    break;

  case 351: /* aexp: REPEAT  */
#line 682 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4718 "parser.c"
    break;

  case 352: /* aexp: '(' exp ')'  */
#line 683 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4724 "parser.c"
    break;

  case 353: /* aexp: '(' exps2 ')'  */
#line 684 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4730 "parser.c"
    break;

  case 354: /* aexp: '[' list ']'  */
#line 685 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4736 "parser.c"
    break;

  case 355: /* aexp: '(' exp10a qop ')'  */
#line 686 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-1],yyvsp[-2]));}
#line 4742 "parser.c"
    break;

  case 356: /* aexp: '(' qvarop_mi exp0 ')'  */
#line 687 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4748 "parser.c"
    break;

  case 357: /* aexp: '(' qconop exp0 ')'  */
#line 688 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4754 "parser.c"
    break;

  case 358: /* exps2: exps2 ',' exp  */
#line 690 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4760 "parser.c"
    break;

  case 359: /* exps2: exp ',' exp  */
#line 691 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 4766 "parser.c"
    break;

  case 360: /* alts: alts1  */
#line 693 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4772 "parser.c"
    break;

  case 361: /* alts: ';' alts  */
#line 694 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4778 "parser.c"
    break;

  case 362: /* alts1: alts1 ';' alt  */
#line 696 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4784 "parser.c"
    break;

  case 363: /* alts1: alts1 ';'  */
#line 697 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4790 "parser.c"
    break;

  case 364: /* alts1: alt  */
#line 698 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4796 "parser.c"
    break;

  case 365: /* alt: pat altRhs wherePart  */
#line 700 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],letrec(yyvsp[0],yyvsp[-1])));}
#line 4802 "parser.c"
    break;

  case 366: /* altRhs: guardAlts  */
#line 702 "parser.y"
                                        {yyval = gc1(desugarGuards(rev(yyvsp[0])));}
#line 4808 "parser.c"
    break;

  case 367: /* altRhs: ARROW exp  */
#line 703 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 4814 "parser.c"
    break;

  case 368: /* altRhs: error  */
#line 704 "parser.y"
                                        {syntaxError("case expression");}
#line 4820 "parser.c"
    break;

  case 369: /* guardAlts: guardAlts guardAlt  */
#line 706 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4826 "parser.c"
    break;

  case 370: /* guardAlts: guardAlt  */
#line 707 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4832 "parser.c"
    break;

  case 371: /* guardAlt: '|' quals ARROW exp  */
#line 709 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 4838 "parser.c"
    break;

  case 372: /* stmts: stmts1  */
#line 712 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4844 "parser.c"
    break;

  case 373: /* stmts: ';' stmts  */
#line 713 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4850 "parser.c"
    break;

  case 374: /* stmts1: stmts1 ';' stmt  */
#line 715 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4856 "parser.c"
    break;

  case 375: /* stmts1: stmts1 ';'  */
#line 716 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4862 "parser.c"
    break;

  case 376: /* stmts1: stmt  */
#line 717 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4868 "parser.c"
    break;

  case 377: /* stmt: exp_err FROM exp  */
#line 720 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 4874 "parser.c"
    break;

  case 378: /* stmt: LET decls  */
#line 721 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 4880 "parser.c"
    break;

  case 379: /* stmt: exp_err  */
#line 723 "parser.y"
                                        {yyval = gc1(ap(DOQUAL,yyvsp[0]));}
#line 4886 "parser.c"
    break;

  case 380: /* fbinds: %empty  */
#line 725 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4892 "parser.c"
    break;

  case 381: /* fbinds: fbinds1  */
#line 726 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 4898 "parser.c"
    break;

  case 382: /* fbinds1: fbinds1 ',' fbind  */
#line 728 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4904 "parser.c"
    break;

  case 383: /* fbinds1: fbind  */
#line 729 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4910 "parser.c"
    break;

  case 384: /* fbind: var  */
#line 731 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4916 "parser.c"
    break;

  case 385: /* fbind: qvar '=' exp  */
#line 732 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 4922 "parser.c"
    break;

  case 386: /* list: exp  */
#line 737 "parser.y"
                                        {yyval = gc1(ap(FINLIST,cons(yyvsp[0],NIL)));}
#line 4928 "parser.c"
    break;

  case 387: /* list: exps2  */
#line 738 "parser.y"
                                        {yyval = gc1(ap(FINLIST,rev(yyvsp[0])));}
#line 4934 "parser.c"
    break;

  case 388: /* list: exp '|' quals  */
#line 739 "parser.y"
                                        {yyval = gc3(ap(COMP,pair(yyvsp[-2],rev(yyvsp[0]))));}
#line 4940 "parser.c"
    break;

  case 389: /* list: exp UPTO exp  */
#line 740 "parser.y"
                                        {yyval = gc3(ap(ap(nameFromTo,yyvsp[-2]),yyvsp[0]));}
#line 4946 "parser.c"
    break;

  case 390: /* list: exp ',' exp UPTO  */
#line 741 "parser.y"
                                        {yyval = gc4(ap(ap(nameFromThen,yyvsp[-3]),yyvsp[-1]));}
#line 4952 "parser.c"
    break;

  case 391: /* list: exp UPTO  */
#line 742 "parser.y"
                                        {yyval = gc2(ap(nameFrom,yyvsp[-1]));}
#line 4958 "parser.c"
    break;

  case 392: /* list: exp ',' exp UPTO exp  */
#line 743 "parser.y"
                                        {yyval = gc5(ap(ap(ap(nameFromThenTo,
								yyvsp[-4]),yyvsp[-2]),yyvsp[0]));}
#line 4965 "parser.c"
    break;

  case 393: /* quals: quals ',' qual  */
#line 746 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4971 "parser.c"
    break;

  case 394: /* quals: qual  */
#line 747 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4977 "parser.c"
    break;

  case 395: /* qual: exp FROM exp  */
#line 749 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 4983 "parser.c"
    break;

  case 396: /* qual: exp  */
#line 750 "parser.y"
                                        {yyval = gc1(ap(BOOLQUAL,yyvsp[0]));}
#line 4989 "parser.c"
    break;

  case 397: /* qual: LET decls  */
#line 751 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 4995 "parser.c"
    break;

  case 398: /* gcon: qcon  */
#line 756 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5001 "parser.c"
    break;

  case 399: /* gcon: '(' ')'  */
#line 757 "parser.y"
                                        {yyval = gc2(nameUnit);}
#line 5007 "parser.c"
    break;

  case 400: /* gcon: '[' ']'  */
#line 758 "parser.y"
                                        {yyval = gc2(nameNil);}
#line 5013 "parser.c"
    break;

  case 401: /* gcon: '(' tupCommas ')'  */
#line 759 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5019 "parser.c"
    break;

  case 402: /* tupCommas: tupCommas ','  */
#line 761 "parser.y"
                                        {yyval = gc2(mkTuple(tupleOf(yyvsp[-1])+1));}
#line 5025 "parser.c"
    break;

  case 403: /* tupCommas: ','  */
#line 762 "parser.y"
                                        {yyval = gc1(mkTuple(2));}
#line 5031 "parser.c"
    break;

  case 404: /* varid: VARID  */
#line 764 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5037 "parser.c"
    break;

  case 405: /* varid: HIDING  */
#line 765 "parser.y"
                                        {yyval = gc1(varHiding);}
#line 5043 "parser.c"
    break;

  case 406: /* varid: QUALIFIED  */
#line 766 "parser.y"
                                        {yyval = gc1(varQualified);}
#line 5049 "parser.c"
    break;

  case 407: /* varid: ASMOD  */
#line 767 "parser.y"
                                        {yyval = gc1(varAsMod);}
#line 5055 "parser.c"
    break;

  case 408: /* qconid: QCONID  */
#line 769 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5061 "parser.c"
    break;

  case 409: /* qconid: CONID  */
#line 770 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5067 "parser.c"
    break;

  case 410: /* var: varid  */
#line 772 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5073 "parser.c"
    break;

  case 411: /* var: '(' VAROP ')'  */
#line 773 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5079 "parser.c"
    break;

  case 412: /* var: '(' '+' ')'  */
#line 774 "parser.y"
                                        {yyval = gc3(varPlus);}
#line 5085 "parser.c"
    break;

  case 413: /* var: '(' '-' ')'  */
#line 775 "parser.y"
                                        {yyval = gc3(varMinus);}
#line 5091 "parser.c"
    break;

  case 414: /* var: '(' '!' ')'  */
#line 776 "parser.y"
                                        {yyval = gc3(varBang);}
#line 5097 "parser.c"
    break;

  case 415: /* var: '(' '.' ')'  */
#line 777 "parser.y"
                                        {yyval = gc3(varDot);}
#line 5103 "parser.c"
    break;

  case 416: /* qvar: QVARID  */
#line 779 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5109 "parser.c"
    break;

  case 417: /* qvar: '(' QVAROP ')'  */
#line 780 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5115 "parser.c"
    break;

  case 418: /* qvar: var  */
#line 781 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5121 "parser.c"
    break;

  case 419: /* con: CONID  */
#line 783 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5127 "parser.c"
    break;

  case 420: /* con: '(' CONOP ')'  */
#line 784 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5133 "parser.c"
    break;

  case 421: /* qcon: QCONID  */
#line 786 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5139 "parser.c"
    break;

  case 422: /* qcon: '(' QCONOP ')'  */
#line 787 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5145 "parser.c"
    break;

  case 423: /* qcon: con  */
#line 788 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5151 "parser.c"
    break;

  case 424: /* varop: '+'  */
#line 790 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5157 "parser.c"
    break;

  case 425: /* varop: '-'  */
#line 791 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5163 "parser.c"
    break;

  case 426: /* varop: varop_mipl  */
#line 792 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5169 "parser.c"
    break;

  case 427: /* varop_mi: '+'  */
#line 794 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5175 "parser.c"
    break;

  case 428: /* varop_mi: varop_mipl  */
#line 795 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5181 "parser.c"
    break;

  case 429: /* varop_pl: '-'  */
#line 797 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5187 "parser.c"
    break;

  case 430: /* varop_pl: varop_mipl  */
#line 798 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5193 "parser.c"
    break;

  case 431: /* varop_mipl: VAROP  */
#line 800 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5199 "parser.c"
    break;

  case 432: /* varop_mipl: '`' varid '`'  */
#line 801 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5205 "parser.c"
    break;

  case 433: /* varop_mipl: '!'  */
#line 802 "parser.y"
                                        {yyval = gc1(varBang);}
#line 5211 "parser.c"
    break;

  case 434: /* varop_mipl: '.'  */
#line 803 "parser.y"
                                        {yyval = gc1(varDot);}
#line 5217 "parser.c"
    break;

  case 435: /* qvarop: '-'  */
#line 805 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5223 "parser.c"
    break;

  case 436: /* qvarop: qvarop_mi  */
#line 806 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5229 "parser.c"
    break;

  case 437: /* qvarop_mi: QVAROP  */
#line 808 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5235 "parser.c"
    break;

  case 438: /* qvarop_mi: '`' QVARID '`'  */
#line 809 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5241 "parser.c"
    break;

  case 439: /* qvarop_mi: varop_mi  */
#line 810 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5247 "parser.c"
    break;

  case 440: /* conop: CONOP  */
#line 813 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5253 "parser.c"
    break;

  case 441: /* conop: '`' CONID '`'  */
#line 814 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5259 "parser.c"
    break;

  case 442: /* qconop: QCONOP  */
#line 816 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5265 "parser.c"
    break;

  case 443: /* qconop: '`' QCONID '`'  */
#line 817 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5271 "parser.c"
    break;

  case 444: /* qconop: conop  */
#line 818 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5277 "parser.c"
    break;

  case 445: /* op: varop  */
#line 820 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5283 "parser.c"
    break;

  case 446: /* op: conop  */
#line 821 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5289 "parser.c"
    break;

  case 447: /* qop: qvarop  */
#line 823 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5295 "parser.c"
    break;

  case 448: /* qop: qconop  */
#line 824 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5301 "parser.c"
    break;

  case 449: /* begin: %empty  */
#line 829 "parser.y"
                                        {goOffside(startColumn);}
#line 5307 "parser.c"
    break;

  case 450: /* end: '}'  */
#line 832 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5313 "parser.c"
    break;

  case 451: /* end: error  */
#line 833 "parser.y"
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
#line 5329 "parser.c"
    break;


#line 5333 "parser.c"

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

#line 848 "parser.y"


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

#define keyword(kw) snprintf(buffer,sizeof(buffer),fmt,kwd,kw); return buffer;
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
	case CONID     : snprintf(buffer,sizeof(buffer),"symbol \"%s\"",
				 textToStr(textOf(yylval)));
			 return buffer;
	case QVAROP    :
	case QVARID    :
	case QCONOP    :
	case QCONID    : snprintf(buffer,sizeof(buffer),"symbol \"%s\"",
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
