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


#line 137 "parser.c.new"

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
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define EXPR 258
#define CTXT 259
#define SCRIPT 260
#define CASEXP 261
#define OF 262
#define DATA 263
#define TYPE 264
#define IF 265
#define THEN 266
#define ELSE 267
#define WHERE 268
#define LET 269
#define IN 270
#define INFIXN 271
#define INFIXL 272
#define INFIXR 273
#define PRIMITIVE 274
#define TNEWTYPE 275
#define DEFAULT 276
#define DERIVING 277
#define DO 278
#define TCLASS 279
#define TINSTANCE 280
#define REPEAT 281
#define ALL 282
#define NUMLIT 283
#define CHARLIT 284
#define STRINGLIT 285
#define VAROP 286
#define VARID 287
#define CONOP 288
#define CONID 289
#define QVAROP 290
#define QVARID 291
#define QCONOP 292
#define QCONID 293
#define COCO 294
#define UPTO 295
#define FROM 296
#define ARROW 297
#define IMPLIES 298
#define TMODULE 299
#define IMPORT 300
#define HIDING 301
#define QUALIFIED 302
#define ASMOD 303
#define NEEDPRIMS 304
#define FOREIGN 305

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
  YYSYMBOL_npk = 160,                      /* npk  */
  YYSYMBOL_pat0 = 161,                     /* pat0  */
  YYSYMBOL_pat0_INT = 162,                 /* pat0_INT  */
  YYSYMBOL_pat0_vI = 163,                  /* pat0_vI  */
  YYSYMBOL_infixPat = 164,                 /* infixPat  */
  YYSYMBOL_pat10 = 165,                    /* pat10  */
  YYSYMBOL_pat10_vI = 166,                 /* pat10_vI  */
  YYSYMBOL_fpat = 167,                     /* fpat  */
  YYSYMBOL_apat = 168,                     /* apat  */
  YYSYMBOL_apat_vI = 169,                  /* apat_vI  */
  YYSYMBOL_pats2 = 170,                    /* pats2  */
  YYSYMBOL_pats1 = 171,                    /* pats1  */
  YYSYMBOL_patbinds = 172,                 /* patbinds  */
  YYSYMBOL_patbinds1 = 173,                /* patbinds1  */
  YYSYMBOL_patbind = 174,                  /* patbind  */
  YYSYMBOL_exp = 175,                      /* exp  */
  YYSYMBOL_exp_err = 176,                  /* exp_err  */
  YYSYMBOL_exp0 = 177,                     /* exp0  */
  YYSYMBOL_exp0a = 178,                    /* exp0a  */
  YYSYMBOL_exp0b = 179,                    /* exp0b  */
  YYSYMBOL_infixExpa = 180,                /* infixExpa  */
  YYSYMBOL_infixExpb = 181,                /* infixExpb  */
  YYSYMBOL_exp10a = 182,                   /* exp10a  */
  YYSYMBOL_exp10b = 183,                   /* exp10b  */
  YYSYMBOL_then_exp = 184,                 /* then_exp  */
  YYSYMBOL_else_exp = 185,                 /* else_exp  */
  YYSYMBOL_pats = 186,                     /* pats  */
  YYSYMBOL_appExp = 187,                   /* appExp  */
  YYSYMBOL_aexp = 188,                     /* aexp  */
  YYSYMBOL_exps2 = 189,                    /* exps2  */
  YYSYMBOL_alts = 190,                     /* alts  */
  YYSYMBOL_alts1 = 191,                    /* alts1  */
  YYSYMBOL_alt = 192,                      /* alt  */
  YYSYMBOL_altRhs = 193,                   /* altRhs  */
  YYSYMBOL_guardAlts = 194,                /* guardAlts  */
  YYSYMBOL_guardAlt = 195,                 /* guardAlt  */
  YYSYMBOL_stmts = 196,                    /* stmts  */
  YYSYMBOL_stmts1 = 197,                   /* stmts1  */
  YYSYMBOL_stmt = 198,                     /* stmt  */
  YYSYMBOL_fbinds = 199,                   /* fbinds  */
  YYSYMBOL_fbinds1 = 200,                  /* fbinds1  */
  YYSYMBOL_fbind = 201,                    /* fbind  */
  YYSYMBOL_list = 202,                     /* list  */
  YYSYMBOL_zipquals = 203,                 /* zipquals  */
  YYSYMBOL_quals = 204,                    /* quals  */
  YYSYMBOL_qual = 205,                     /* qual  */
  YYSYMBOL_gcon = 206,                     /* gcon  */
  YYSYMBOL_tupCommas = 207,                /* tupCommas  */
  YYSYMBOL_varid = 208,                    /* varid  */
  YYSYMBOL_qconid = 209,                   /* qconid  */
  YYSYMBOL_var = 210,                      /* var  */
  YYSYMBOL_qvar = 211,                     /* qvar  */
  YYSYMBOL_con = 212,                      /* con  */
  YYSYMBOL_qcon = 213,                     /* qcon  */
  YYSYMBOL_varop = 214,                    /* varop  */
  YYSYMBOL_varop_mi = 215,                 /* varop_mi  */
  YYSYMBOL_varop_pl = 216,                 /* varop_pl  */
  YYSYMBOL_varop_mipl = 217,               /* varop_mipl  */
  YYSYMBOL_qvarop = 218,                   /* qvarop  */
  YYSYMBOL_qvarop_mi = 219,                /* qvarop_mi  */
  YYSYMBOL_conop = 220,                    /* conop  */
  YYSYMBOL_qconop = 221,                   /* qconop  */
  YYSYMBOL_op = 222,                       /* op  */
  YYSYMBOL_qop = 223,                      /* qop  */
  YYSYMBOL_begin = 224,                    /* begin  */
  YYSYMBOL_end = 225                       /* end  */
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
#define YYLAST   3798

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  156
/* YYNRULES -- Number of rules.  */
#define YYNRULES  469
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  815

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
       0,   102,   102,   103,   104,   105,   118,   122,   126,   128,
     130,   136,   139,   141,   142,   150,   151,   152,   153,   154,
     159,   160,   161,   162,   163,   165,   166,   171,   172,   173,
     174,   175,   177,   178,   179,   180,   182,   183,   185,   186,
     191,   192,   193,   195,   206,   208,   211,   214,   217,   219,
     220,   221,   223,   224,   225,   226,   228,   229,   231,   232,
     233,   234,   236,   237,   238,   239,   241,   242,   244,   245,
     250,   251,   252,   253,   254,   259,   260,   263,   264,   267,
     271,   273,   276,   277,   280,   284,   285,   291,   293,   294,
     295,   297,   298,   300,   302,   304,   305,   307,   309,   311,
     312,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     324,   325,   326,   327,   328,   329,   331,   332,   333,   335,
     337,   338,   340,   341,   342,   344,   345,   346,   348,   349,
     351,   352,   357,   359,   360,   361,   363,   364,   369,   371,
     373,   375,   377,   383,   384,   385,   386,   387,   388,   390,
     391,   393,   394,   396,   397,   399,   400,   402,   403,   406,
     407,   409,   410,   412,   413,   418,   420,   422,   423,   425,
     426,   427,   428,   430,   432,   434,   436,   437,   439,   440,
     442,   443,   444,   445,   447,   448,   450,   451,   452,   453,
     454,   456,   457,   459,   460,   462,   463,   465,   466,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     481,   482,   484,   485,   486,   487,   492,   493,   494,   495,
     496,   497,   498,   499,   501,   502,   504,   505,   507,   508,
     510,   511,   513,   514,   515,   517,   519,   520,   521,   524,
     526,   527,   528,   530,   531,   532,   533,   534,   536,   537,
     538,   539,   540,   542,   543,   545,   546,   548,   549,   551,
     553,   554,   559,   560,   563,   564,   567,   568,   569,   572,
     574,   579,   580,   582,   583,   585,   587,   588,   589,   591,
     592,   594,   595,   597,   598,   599,   600,   601,   602,   603,
     604,   605,   606,   608,   609,   611,   612,   614,   615,   617,
     618,   619,   621,   622,   623,   624,   625,   626,   627,   628,
     629,   630,   631,   633,   634,   636,   637,   639,   640,   642,
     643,   645,   646,   651,   652,   654,   655,   657,   658,   660,
     661,   663,   664,   666,   667,   668,   669,   671,   673,   674,
     675,   676,   678,   680,   681,   682,   684,   687,   688,   693,
     694,   696,   697,   700,   701,   703,   704,   706,   707,   708,
     709,   710,   711,   712,   714,   715,   716,   717,   718,   719,
     720,   721,   722,   723,   725,   726,   728,   729,   731,   732,
     733,   735,   737,   738,   739,   741,   742,   744,   747,   748,
     750,   751,   752,   755,   756,   758,   760,   761,   763,   764,
     766,   767,   772,   773,   774,   780,   781,   782,   783,   786,
     787,   789,   790,   792,   793,   794,   799,   800,   801,   802,
     804,   805,   807,   808,   809,   810,   812,   813,   815,   816,
     817,   818,   819,   820,   822,   823,   824,   826,   827,   829,
     830,   831,   833,   834,   835,   837,   838,   840,   841,   843,
     844,   845,   846,   848,   849,   851,   852,   853,   856,   857,
     859,   860,   861,   863,   864,   866,   867,   872,   875,   876
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
  "ldecls1", "ldecl", "pat", "pat_npk", "npk", "pat0", "pat0_INT",
  "pat0_vI", "infixPat", "pat10", "pat10_vI", "fpat", "apat", "apat_vI",
  "pats2", "pats1", "patbinds", "patbinds1", "patbind", "exp", "exp_err",
  "exp0", "exp0a", "exp0b", "infixExpa", "infixExpb", "exp10a", "exp10b",
  "then_exp", "else_exp", "pats", "appExp", "aexp", "exps2", "alts",
  "alts1", "alt", "altRhs", "guardAlts", "guardAlt", "stmts", "stmts1",
  "stmt", "fbinds", "fbinds1", "fbind", "list", "zipquals", "quals",
  "qual", "gcon", "tupCommas", "varid", "qconid", "var", "qvar", "con",
  "qcon", "varop", "varop_mi", "varop_pl", "varop_mipl", "qvarop",
  "qvarop_mi", "conop", "qconop", "op", "qop", "begin", "end", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-676)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-439)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     859,  -676,  1890,   124,    10,   132,  -676,  1890,  1890,    74,
      87,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  3290,
    2593,  2748,   928,  1044,  -676,  -676,  -676,  -676,   163,  -676,
    -676,   125,  -676,  1247,  -676,  1247,  -676,  2748,   148,  -676,
    -676,  -676,   149,  -676,   156,  -676,  -676,   229,  -676,  3697,
    -676,   151,  -676,   159,  -676,   236,    46,  -676,   231,  2286,
    -676,  -676,  -676,  3290,  2831,  2957,  -676,  -676,  -676,  3031,
    -676,   209,   189,  -676,  -676,   148,   205,   211,   230,   245,
    2375,   268,  -676,  -676,   485,   275,   278,   305,  1247,   317,
     383,  -676,  -676,  2421,  -676,  2421,  -676,   365,   237,   259,
      74,  -676,  2225,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  2464,  2507,   148,   659,  2748,   659,
    -676,  3412,   386,  2103,  1090,  -676,  -676,  -676,  -676,  -676,
    -676,   287,  -676,  2649,  2649,   274,  1890,   351,    51,  1358,
      61,  1890,    74,  2286,   306,    38,   329,  -676,  -676,   180,
     205,   211,   245,  1485,   268,   275,   278,   350,   354,   357,
     373,  -676,   180,   180,  3290,  -676,   423,  3290,   455,  2047,
    -676,  -676,  -676,    25,  1890,  -676,  3290,   659,  -676,  -676,
    -676,  -676,  -676,  -676,   359,   368,   371,   396,  -676,  -676,
    1890,  -676,  2329,  1890,  -676,  -676,  -676,   377,  -676,   406,
    1804,  1933,  1890,   418,  -676,  -676,  -676,  1189,   420,  -676,
     429,  -676,  -676,  3534,  3438,  -676,  2593,  -676,  -676,  2593,
    -676,  -676,   560,   433,   438,  -676,   445,   447,   148,   439,
      37,  -676,    37,  -676,   449,  2065,  -676,   481,  3342,   499,
     517,   524,  -676,   462,  3560,  3274,   507,   273,    60,   595,
     778,   800,   177,   341,    59,   399,   440,   866,  2873,  2649,
     311,    64,  1226,   460,   474,  -676,   480,  -676,  -676,    35,
    -676,   315,  -676,  3290,  -676,   128,   866,   866,  2789,    38,
    2994,  -676,  1890,  1890,   530,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,   231,  -676,  1890,  -676,  2550,
     154,  3068,  -676,  -676,  3290,  -676,  3105,  -676,  -676,  2240,
    3142,  3179,  -676,  3105,  -676,  -676,   521,  3216,  3105,  -676,
    -676,  -676,   478,   509,  -676,   445,   519,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,    74,   518,   528,
    -676,   543,  1933,   731,   544,   564,   420,  3368,   548,  2240,
    2240,  2240,  -676,  2240,  -676,  -676,  -676,  -676,   230,   568,
    -676,   659,  1890,  -676,  3697,  3697,  -676,  2240,  -676,  2240,
    -676,  2240,  -676,  2240,  -676,  -676,  -676,   312,   562,  1300,
     579,  -676,   435,   584,  -676,   588,  -676,  -676,    48,  -676,
     592,  3464,    69,  -676,   394,  -676,  -676,   733,  -676,   733,
    -676,   733,  -676,   277,   158,  -676,   613,  -676,   601,  3578,
    -676,  1560,  -676,   610,   608,  3593,  -676,   646,   615,  3593,
    -676,   821,  -676,  3105,  -676,   620,  1411,   622,  2915,  -676,
    -676,  -676,   435,   208,  -676,  -676,  -676,   616,   616,  -676,
      42,  -676,  2707,  1729,   616,  -676,  2240,  1890,  2421,  -676,
     646,   637,  -676,  -676,  -676,  3105,  3105,  -676,  3253,  -676,
    3105,  -676,  -676,  2994,    97,    38,   611,  -676,  -676,  -676,
    1890,  -676,  -676,  3290,  -676,  -676,  -676,  3290,  -676,  3290,
    -676,  -676,  -676,  3290,  -676,  -676,  -676,   659,  3105,   231,
    1890,  1933,  1847,   528,   505,  -676,  -676,   649,   651,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  3560,  -676,  3560,
    -676,  -676,  -676,  1762,  -676,  1493,  -676,  2649,    66,  2135,
    2240,  -676,   572,   635,  -676,  -676,  -676,   635,   635,   975,
     616,  -676,    66,  2135,   653,   650,  -676,   382,   646,    37,
     643,  -676,    37,  -676,   114,  3290,  3290,  3290,   325,  1290,
     660,   435,  -676,   722,   682,  2707,  -676,  -676,  -676,  -676,
     731,  -676,  -676,  -676,   678,   679,  -676,  3619,  3490,  -676,
     128,  -676,   691,  -676,  -676,  -676,  -676,   180,  -676,  -676,
     114,  -676,  -676,  -676,  2421,  1890,   646,   694,  -676,  -676,
    3105,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  2225,  -676,  -676,  -676,   687,  -676,   690,   697,  -676,
    -676,  -676,    38,  -676,   609,  -676,   731,  3697,  1652,    24,
    -676,  -676,  3637,    39,  3516,  3394,   681,   736,   733,  -676,
    -676,   656,  -676,   734,  -676,  2240,  -676,   701,  -676,   985,
    -676,  3697,  -676,  -676,  3697,  -676,  -676,  -676,   435,  -676,
     707,  -676,   716,   720,  -676,  -676,  1290,   262,   616,  1001,
     616,   480,   887,  3715,  3715,  3715,  3715,  -676,  1890,   727,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  2181,  -676,  2135,
    1076,    39,  3697,  3697,   723,   477,  2135,  -676,  3697,  -676,
    -676,   743,  3658,  3658,  3697,  -676,  3658,   824,   616,  -676,
    2135,  -676,  -676,   382,  -676,  -676,  1974,    72,   262,  1598,
    -676,  1137,   728,  -676,   741,   616,   975,   747,   748,  3730,
    -676,  3676,  -676,  -676,  -676,  -676,  1890,  -676,    24,   839,
    3658,   730,    37,  -676,  -676,  -676,  3697,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,    90,  -676,   182,  -676,   745,  -676,
     749,   734,  -676,   731,  -676,  -676,  -676,  -676,  -676,  -676,
     746,   970,  -676,   750,   753,  -676,  -676,  -676,  -676,  -676,
     975,   761,  -676,   975,   975,  -676,  -676,  -676,  2006,  -676,
    -676,   757,  3316,  -676,   759,   763,  -676,  -676,   616,  -676,
    2167,   616,   975,  -676,  -676,  -676,  1620,  -676,   975,  -676,
    -676,  2199,  -676,    37,  -676,  2240,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     5,     0,     0,    11,     0,   324,     0,     0,     0,
       0,   367,   364,   365,   366,   422,   437,   434,   439,     0,
       0,     0,     0,     0,   423,   424,   425,   360,   262,   323,
     326,   327,   328,   329,   331,   330,   332,   345,   356,   361,
     428,   436,   357,   441,   416,   427,   426,     0,     3,   181,
     196,     0,     4,   467,     1,     0,     0,   266,     0,     0,
     299,   305,   306,     0,     0,     0,   307,   354,   301,     0,
     303,   300,   416,   335,   340,   359,   449,   458,   455,   460,
       0,   451,   421,   417,     0,   452,   445,     0,   330,     0,
       0,   457,   446,     0,   462,     0,   418,   402,   403,     0,
       0,     2,     0,   449,   458,   455,   460,   453,   451,   452,
     445,   465,   454,   466,     0,     0,   355,   396,     0,   396,
     180,     0,     0,     0,     0,   209,   195,   197,   199,   198,
      10,    20,    12,    15,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   395,     0,   388,   392,   312,   277,
       0,     0,     0,     0,     0,     0,     0,     0,   272,   271,
     274,   278,   282,   281,   295,   296,     0,   303,   276,     0,
     316,   272,   271,     0,     0,   353,     0,   317,   429,   438,
     435,   440,   431,   432,     0,     0,     0,     0,   433,   430,
       0,   368,     0,     0,   369,   420,   419,     0,   327,     0,
       0,     0,     0,   404,   370,   263,   190,     0,     0,   325,
       0,   179,   184,   186,   185,   194,     0,   334,   339,     0,
     337,   342,     0,     0,   397,   399,   400,     0,   358,     0,
       0,   182,     0,   183,     0,     0,   200,     0,     0,     0,
       0,     0,   208,     0,   185,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   277,     0,    15,
       0,     0,     0,     0,    43,    42,    17,    73,   236,     0,
      74,     0,   240,   241,   242,     0,   282,   281,   276,     0,
       0,   350,     0,     0,     0,   348,   469,   267,   468,   270,
     269,   264,   268,   265,   347,   394,   389,     0,   344,   391,
       0,     0,   284,   283,   293,   294,     0,   308,   309,     0,
       0,     0,   297,     0,   310,   298,     0,     0,     0,   311,
     346,   302,     0,   318,   320,   322,     0,   459,   456,   461,
     450,   375,   371,   374,   372,   373,   405,     0,   414,   410,
     412,   375,     0,     0,   200,     0,   174,     0,     0,     0,
       0,     0,   193,     0,   333,   338,   336,   341,     0,     0,
     363,     0,     0,   362,   211,   210,   201,     0,   202,     0,
     203,     0,   205,     0,   206,   204,   207,   427,   439,     0,
       0,    21,     0,     0,    26,     0,    27,    28,     0,    82,
       0,    80,    90,    89,     0,   217,   224,     0,   219,     0,
     221,     0,   135,     0,     0,   134,   137,    85,     0,   181,
     148,     0,   146,   157,     0,   150,   147,   260,     0,   152,
     443,     0,   442,     0,   444,     0,     0,   271,   276,    16,
      48,    14,     0,    49,    13,    87,    86,     0,     0,     7,
      43,    18,    70,     0,     0,   254,     0,     0,     0,   237,
     260,   256,   258,   252,   239,     0,     0,   447,     0,   251,
       0,   448,     6,     0,     0,     0,   376,   380,   349,   352,
       0,   393,   390,     0,   287,   314,   273,     0,   291,     0,
     289,   313,   275,     0,   285,   315,   304,     0,     0,   415,
       0,     0,     0,   409,     0,   177,   175,   203,   205,   187,
     178,   188,   189,   398,   401,   212,   213,   211,   214,   210,
     215,    22,    31,     0,    23,    32,     9,    15,     0,     0,
       0,    88,     0,   216,   463,   464,   227,   218,   220,     0,
       0,   136,     0,     0,     0,   154,   156,     0,   260,     0,
       0,   144,     0,   245,   276,     0,     0,   309,    49,    52,
       0,     0,    44,     0,     0,     0,    40,    71,    72,   223,
       0,   222,   166,   168,     0,     0,   172,   191,   192,   228,
       0,   255,     0,   253,   257,   244,   243,   275,   247,   280,
     279,   246,   377,   384,     0,     0,   260,   382,   386,   343,
     379,   351,   288,   292,   290,   286,   319,   321,   413,   411,
     408,     0,   176,    24,    25,     0,    33,     0,    34,    37,
      38,    39,     0,    90,    81,   109,     0,     0,     0,   125,
      96,    98,   106,     0,     0,   105,     0,    75,     0,   132,
     133,     0,   119,   125,   145,     0,   162,   158,   160,     0,
     143,   149,   232,   261,   151,   248,   249,   250,     0,    47,
      59,    53,     0,    54,    57,    58,    52,    49,     0,     0,
       0,    19,     0,     0,     0,     0,     0,   238,     0,     0,
     383,   381,   385,   378,   173,    29,    30,    35,     8,     0,
       0,     0,   191,   192,     0,     0,     0,    78,     0,   114,
     112,     0,     0,     0,     0,   111,     0,     0,     0,   226,
       0,    83,   155,     0,   163,   164,     0,     0,    49,    62,
      51,    55,     0,    45,     0,     0,     0,     0,     0,     0,
     169,   192,   167,   170,   171,   259,     0,    36,   125,     0,
       0,   115,   128,   126,    95,   113,     0,   104,   118,   117,
     102,   110,   103,   108,     0,   121,     0,   229,    76,    92,
      94,   125,   159,   161,   233,   235,   230,   234,   231,    46,
       0,     0,    63,     0,    64,    67,    68,    69,    56,    50,
       0,     0,   139,     0,     0,   165,   387,    79,     0,    97,
     100,     0,   105,   101,     0,   129,   131,   116,     0,   107,
       0,     0,     0,    84,    60,    61,    65,   138,     0,   141,
     142,     0,   127,     0,   120,     0,   122,   123,    91,    93,
      66,   140,    99,   130,   124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -676,  -676,  -676,  -676,  -676,  -353,  -113,  -676,  -676,   300,
    -676,  -676,   147,  -676,   385,   389,  -507,   170,  -676,   119,
    -676,  -676,    36,   279,   395,  -338,  -676,    47,   160,  -513,
    -676,  -675,  -676,  -519,   141,  -676,    54,  -603,  -676,  -676,
    -676,   316,  -676,  -676,  -676,  -676,  -676,  -676,   164,   157,
    -498,   175,   155,    86,   792,  -512,   288,     1,   -48,  -122,
    -581,   972,    -3,  -211,   -14,   -44,  -676,  -676,   196,  -109,
    -650,  -676,  -676,  -676,  -132,  -676,   638,   640,  -261,  -676,
    -676,   427,  -416,  -676,   -92,  -676,  -676,  -676,   -27,   -32,
     -51,   -75,  -676,   446,  -128,   282,  -124,   221,   334,   102,
    -676,  -676,  -676,  -676,   419,   258,   -26,   -77,   -76,  -676,
    -676,  -676,   101,     4,  -676,  -676,  -676,  -676,    19,   882,
     450,  -676,   324,  -676,  -676,   330,   775,  -676,   626,   802,
    -676,   566,  -676,  -676,   586,   441,  1299,    29,   501,   695,
      26,   -91,  -467,  1154,   -71,  -676,  -676,   -10,  -676,   908,
      22,    20,   303,    45,  -676,  -118
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     5,    52,    53,   131,   433,   263,   246,   383,   384,
     607,   608,   609,   264,   441,   265,   552,   652,   653,   654,
     763,   764,   765,   266,   267,   394,   748,   749,   619,   620,
     779,   621,   622,   737,   633,   744,   745,   687,   784,   785,
     404,   405,   413,   417,   534,   535,   538,   637,   638,   639,
     561,   562,   563,   345,   208,   494,   209,   565,   211,   212,
     566,   213,   244,   126,   215,   239,   240,   268,   397,   523,
     269,   643,   706,   707,   270,   271,   272,   273,   449,   450,
     451,   452,   541,   101,    58,   139,   140,   290,   464,   171,
     172,   160,   578,   161,   162,   303,   163,   164,   305,    68,
     166,   173,   322,   323,   324,   338,    29,    30,    31,    32,
      33,    34,    35,    36,   138,   285,    69,    37,    38,    89,
     465,   466,   467,   586,   587,   588,   145,   146,   147,   223,
     224,   225,    99,   203,   339,   340,    39,   241,    40,    50,
      41,    42,    43,    72,   524,    91,   460,   424,   111,   112,
      94,   317,   526,   114,   134,   291
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,   237,   352,   122,    48,   276,   276,   289,   205,   277,
     277,   276,    92,   159,   454,   277,   197,   198,   199,   198,
     632,   279,   293,    92,    74,    92,   227,   298,   227,   512,
     701,   629,   158,   144,   573,   127,   681,   157,   170,   286,
      75,   649,    95,   -41,   121,    71,   685,   746,   662,   516,
     295,    90,   626,   113,   780,   113,   116,   136,   275,   275,
     410,   392,   286,   283,   275,   435,   626,   613,   686,    51,
     -77,    45,   104,   286,   443,    46,   243,   318,    92,   548,
     115,   319,   274,   274,    74,   237,   326,   444,   274,    71,
     168,   168,   436,    90,   393,    71,   691,   -41,   583,   214,
     393,   137,   260,   210,   680,   288,   284,   127,   113,   -41,
     411,   739,   739,   237,   517,   739,   292,   144,   218,   221,
     238,    73,   640,    88,   -77,   777,   812,   757,   288,   445,
     276,   276,    54,   192,   277,   277,   -77,   228,   746,   288,
      57,   584,   788,   226,   585,   226,   429,   104,   793,   739,
     713,   106,   130,    59,   386,   787,   176,   789,    45,   278,
     278,   462,    46,   348,   102,   278,   165,   165,   447,   301,
     671,   300,   448,   734,   740,    47,   100,   742,   402,    71,
     614,    73,   310,   311,   275,    45,   423,   632,   184,    46,
      71,   118,   186,    71,   631,    71,   221,   529,   657,   127,
     127,   759,    71,   325,   347,   455,   456,   427,   274,    15,
     530,   783,   626,   104,   117,   217,   220,   106,   772,   626,
     355,   790,   119,   357,   127,   133,   158,   364,   403,   365,
     127,   157,   238,   626,   444,   165,   165,   300,    24,    25,
      26,   165,   767,   135,   391,   489,   141,   506,   390,   508,
     409,   176,   415,   419,   408,   177,   414,   418,   178,   549,
      28,   476,   626,    45,   179,    55,    56,    46,   461,   550,
     227,   551,   797,   144,   389,   799,   800,   301,   406,   475,
      87,    97,   120,   180,   428,   278,   481,    90,   438,   193,
     527,   485,   528,   220,   809,   708,   310,   311,   181,    71,
     811,   499,   500,   501,    71,   502,   168,    45,   150,   667,
     558,    46,   430,   549,   276,   204,   445,   354,   277,   505,
     356,   183,   359,   550,    47,   510,   154,    71,   188,   767,
      71,   189,   168,   127,   626,   155,    71,    71,   245,   168,
     280,   431,   407,    71,   168,    45,   156,   589,   543,    46,
     127,   127,   297,    67,   446,   447,   352,   190,   191,   448,
     165,   165,   282,   536,  -437,  -437,   507,   275,   509,   193,
     194,   572,   198,   432,   304,    45,   549,   127,  -437,    46,
     575,   576,   165,   636,   299,   581,   550,   226,   648,   276,
     304,   274,    47,   277,   281,   127,   326,   148,   570,   294,
     412,   127,   306,   175,   612,   127,   200,   307,   165,   201,
     308,   690,   309,   352,  -163,   165,   327,   202,   461,   525,
     165,   525,   386,   525,   610,   328,    15,   276,   329,  -163,
     334,   277,   320,    45,   520,   195,   196,    46,   232,   233,
     568,   416,   275,  -163,  -163,  -163,   399,   401,   331,   544,
      47,   333,    71,   330,    71,    24,    25,    26,   336,   335,
     341,   597,   342,   553,   554,   431,   274,   349,   278,    45,
     569,   352,   627,    46,    45,   313,   314,   735,    46,   350,
     275,   544,   544,   741,   580,  -436,   544,   362,   104,   168,
     361,    47,   106,   127,   678,   127,   237,   176,   312,    71,
     360,   315,   366,    71,   274,    71,   363,   669,   198,    71,
     321,    45,   300,   325,   168,    46,   625,    15,   376,   184,
     388,   185,   304,   186,   316,   165,   568,   439,   732,   440,
     625,   304,   304,   367,   368,   442,   641,    15,   304,   644,
     468,   469,   470,   278,   165,   486,    24,    25,    26,   482,
     128,   371,   372,   127,   127,   471,   406,   165,   165,   488,
     165,   487,   165,   601,   490,   165,    24,    25,    26,   373,
     374,    71,    71,    71,   755,   655,   195,   375,   276,   659,
     491,   278,   277,   474,   492,   187,   610,   702,   756,   758,
     165,   150,   478,   480,  -180,   358,   395,   301,   214,   484,
     371,   498,   210,   128,    15,   359,   184,   453,   127,   154,
     127,   127,   459,  -426,   683,   238,   168,   496,   155,   165,
     504,   182,   128,   396,   128,   128,  -225,   127,  -225,   156,
     127,   275,   511,    24,    25,    26,   513,   514,   312,   515,
    -225,    15,   518,   531,  -225,   692,   693,   696,    15,   679,
     525,   532,  -225,  -225,   537,   274,   237,   165,   539,   540,
     721,   721,   721,   721,  -225,   542,   590,   403,   127,   127,
      24,    25,    26,   545,   127,   547,   625,    24,    25,    26,
     127,   448,   655,   625,   714,   717,   718,   628,    15,   683,
     683,    15,   165,   683,   304,    17,   700,   625,   304,  -182,
     304,  -183,   635,   730,   304,   571,   634,   127,   128,   642,
     222,   656,   660,   568,   128,   128,   568,    24,    25,    26,
      24,    25,    26,   747,   750,   663,   782,   683,   591,   664,
     781,   668,   278,   683,   348,   766,   128,   655,   584,   128,
     675,   771,   807,   676,   129,   128,   132,   697,   598,   677,
     600,   698,   658,   703,    15,   592,   685,   814,   709,   593,
     453,   594,   459,    15,   103,   595,   104,   568,   127,   710,
     568,   568,   711,   403,   726,   347,   731,   184,   420,   398,
     770,   769,   108,    24,    25,    26,   773,   774,   792,   568,
     522,   109,    24,    25,    26,   568,  -438,   791,   625,   794,
     798,   400,   422,   795,   696,   796,   396,   801,   165,  -225,
     128,  -225,   802,   604,   747,   803,   129,   750,   720,   722,
     723,   724,   766,  -225,   727,   555,   712,  -225,   396,   556,
     768,  -225,   810,  -225,   661,  -225,  -225,   557,   808,   728,
     615,   751,   804,   670,   495,  -225,   630,  -225,   128,  -225,
     128,   128,   128,    15,   128,   184,    15,  -225,  -225,   186,
       1,   753,     2,     3,     4,   128,   128,   752,   128,  -225,
     128,    15,   128,   377,   128,   403,   806,    46,   574,   645,
     646,   647,    24,    25,    26,    24,    25,    26,   617,   674,
     778,   743,   128,   124,   775,   521,   425,   103,   426,   104,
      24,    25,    26,   106,   579,    98,   596,   125,   129,   129,
     128,   420,   128,   582,   673,   108,   128,   672,   296,    15,
     128,   229,   187,   421,   109,   472,   725,   503,   493,     6,
      93,   699,   599,   129,     7,   422,     0,     0,     8,   129,
     385,     0,     9,     0,   128,   719,     0,   128,    24,    25,
      26,    10,     0,     0,    11,   434,    12,    13,    14,    76,
      15,    77,    16,    78,    17,    79,    18,     0,     0,     0,
       0,    19,     0,    80,     0,     0,    21,    81,     0,    22,
      82,    83,    23,     0,   776,    84,    85,     0,     0,    24,
      25,    26,     0,     0,     0,   602,    27,    86,     0,   346,
       0,   150,   560,   151,     0,     0,     0,    15,   128,    45,
     128,     0,     0,    46,     0,   359,     0,    15,     0,   154,
     128,   128,     0,   187,     0,     0,   207,   346,   155,   124,
     128,   715,   704,    15,   128,     0,    24,    25,    26,   156,
     716,     0,   129,   125,     0,     6,    24,    25,    26,     0,
       7,     0,   403,     0,     8,     0,     0,     0,     9,   129,
     129,   495,    24,    25,    26,     0,     0,    10,   128,   128,
      11,     0,    12,    13,    14,     0,    15,   434,    16,     0,
      17,     0,    18,     0,     0,     0,   129,    19,     0,    20,
       0,   206,    21,     0,     0,    22,     0,     0,    23,     0,
      96,     0,   128,     0,   129,    24,    25,    26,    15,     0,
     129,     0,    27,     0,   129,   521,     0,   495,   128,   128,
       0,     0,    15,   128,    45,   128,   128,   434,    46,     0,
       0,     0,   521,     0,   729,     0,   128,    24,    25,    26,
     705,   235,   128,     0,   124,   128,   242,     0,     0,     0,
       0,    24,    25,    26,     0,     0,    44,     0,   125,     0,
       0,    44,    44,   602,   128,   128,   128,   128,     0,    15,
       0,   650,     0,     0,    44,    44,    44,    44,     0,     0,
     128,   602,     0,   128,   128,     0,     0,   128,   403,   128,
     206,    44,     0,   128,   128,   128,     0,   128,    24,    25,
      26,   128,   129,     0,   129,     0,     0,     0,   385,     0,
       0,     0,     0,    44,     0,     0,   343,   128,     0,     0,
     128,    15,   128,    45,     0,     0,     0,    46,     0,     0,
     128,   128,     0,     0,    44,   564,   234,   128,     0,     0,
     235,    82,   344,   124,     0,     0,   434,    44,     0,    44,
      24,    25,    26,     0,   705,     0,     0,   125,    15,     0,
       0,     0,   129,   129,     0,     0,     0,     0,    44,    44,
       0,   128,    44,     0,   128,   128,     0,   403,   103,   128,
     104,     0,   105,   128,   106,     0,   437,    24,    25,    26,
      44,   128,   107,   128,     0,    44,   108,    44,     0,   128,
       0,     0,   128,     0,    84,   109,   128,     0,     0,     0,
       0,   623,     0,     0,     0,     0,   110,   129,    70,   129,
     129,   564,    15,     0,   650,   623,     0,     0,    44,     0,
       0,   150,     0,   151,     0,   358,   129,   152,     0,   129,
       0,   403,   651,   434,    44,   359,    44,    44,     0,   154,
       0,    24,    25,    26,    44,    44,    44,     0,   155,   286,
       0,     0,    70,   167,   167,     0,     0,     0,    70,   156,
      44,     0,     0,    44,   249,   250,   251,   129,   129,     0,
     733,     0,     0,   129,     0,     0,   257,    61,    62,   129,
      15,     0,    16,     0,     0,     0,    18,     0,     0,   387,
       0,     0,     0,   169,     0,     0,    63,     0,     0,   258,
     346,     0,    65,   287,   689,   567,   129,   695,     0,    24,
      25,    26,     0,     0,     0,   288,    66,   786,     0,     0,
       0,     0,   167,   167,     0,     0,    44,    44,   167,    60,
      61,    62,     0,    15,     0,    16,     0,     0,     0,    18,
       0,    44,   167,    44,     0,   564,   564,   564,   564,    63,
       0,     0,    64,    70,   546,    65,    70,     0,   167,     0,
       0,   623,    24,    25,    26,    70,     0,   129,   623,    66,
       0,     0,     0,     0,   738,   738,   302,     0,   738,     0,
       0,   624,   623,     0,     0,     0,    44,     0,   813,     0,
       0,   567,     0,     0,     0,   624,     0,     0,   564,     0,
       0,   564,     0,    60,    61,    62,    44,    15,     0,    16,
       0,   623,   738,    18,     0,    15,     0,    16,     0,    17,
       0,    18,     0,    63,   605,     0,    64,     0,   182,    65,
       0,     0,     0,     0,   379,   606,    24,    25,    26,     0,
       0,     0,     0,    66,    24,    25,    26,   167,   167,     0,
       0,   206,   564,     0,     0,   564,   564,     0,     0,     0,
     346,     0,    70,     0,   695,     0,     0,    70,     0,   167,
       0,     0,   346,     0,   564,     0,     0,     0,     0,   682,
     564,     0,    15,   623,    45,     0,     0,     0,    46,     0,
     167,    44,    44,    70,     0,   167,     0,     0,     0,   167,
     167,   235,   167,  -153,   124,     0,   167,   167,     0,     0,
       0,    24,    25,    26,    44,     0,     0,     0,   125,     0,
      15,     0,    16,     0,     0,   567,   567,   567,   567,   760,
       0,     0,     0,     0,    44,    44,    44,     0,     0,   761,
     762,   624,    15,   206,    16,     0,     0,     0,   624,    24,
      25,    26,     0,     0,   682,   682,     0,   387,   682,   611,
       0,   761,   624,     0,     0,     0,     0,     0,     0,   343,
       0,    24,    25,    26,    15,   684,    45,     0,   567,     0,
      46,   567,     0,     0,     0,     0,     0,     0,     0,   234,
       0,   624,   682,   235,    82,   236,   124,     0,   682,     0,
       0,     0,     0,    24,    25,    26,     0,     0,     0,     0,
     125,     0,   167,     0,     0,    70,     0,    70,     0,     0,
     559,     0,     0,     0,     0,     0,     0,     0,    44,    44,
       0,   167,   567,     0,     0,   567,   567,     0,     0,     0,
       0,     0,     0,     0,   167,   167,   560,   167,     0,   167,
       0,    15,   167,    45,   567,     0,     0,    46,     0,     0,
     567,     0,   167,   624,     0,     0,   167,     0,   167,     0,
     207,     0,   167,   124,     0,     0,     0,   167,     0,     0,
      24,    25,    26,     0,    15,     0,   377,   125,    17,     0,
     378,     0,     0,     0,     0,     6,     0,     0,     0,     0,
       7,     0,     0,   379,     8,   603,   167,     0,     9,     0,
       0,   382,    44,    24,    25,    26,     0,    10,     0,     0,
      11,   611,    12,    13,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,    70,    70,    70,    19,     6,    20,
       0,     0,    21,     7,   167,    22,     0,     8,    23,     0,
    -407,     9,     0,     0,     0,    24,    25,    26,     0,     0,
      10,     0,    27,    11,     0,    12,    13,    14,     0,    15,
      44,    16,     0,    17,     0,    18,     0,     0,     0,   167,
      19,     6,    20,     0,     0,    21,     7,     0,    22,     0,
       8,    23,     0,  -406,     9,     0,     0,     0,    24,    25,
      26,     0,     0,    10,     0,    27,    11,     0,    12,    13,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
       0,     0,     0,    19,     6,    20,     0,     0,    21,     7,
       0,    22,     0,     8,    23,     0,     0,   337,     0,     0,
       0,    24,    25,    26,     0,     0,    10,     0,    27,    11,
       0,    12,    13,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,     0,     0,   286,    19,     0,    20,     0,
       0,    21,     0,     0,    22,     0,     0,    23,     0,     0,
     249,   250,   251,     0,    24,    25,    26,     0,     0,     0,
       0,    27,   257,    61,    62,   167,    15,   206,    16,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,   169,
       0,     0,    63,     0,     0,   258,     0,     0,    65,   754,
       0,     0,     0,   343,     0,    24,    25,    26,    15,   684,
      45,   288,    66,     0,    46,     0,     0,     0,   302,     0,
       0,     0,     0,   234,     0,     0,     0,   235,    82,   344,
     124,     0,     0,     0,     0,     0,   206,    24,    25,    26,
       0,     0,     0,     0,   125,    60,    61,    62,     0,    15,
       0,    16,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,   343,     0,     0,    63,     0,    15,    64,    45,
       0,    65,     0,    46,   206,     0,     0,     0,    24,    25,
      26,     0,   234,     0,     0,    66,   235,    82,   236,   124,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,     0,     0,   125,     0,    15,   615,    45,     0,     0,
       0,    46,     0,     0,     0,     0,     0,     0,     0,     0,
     234,     0,     0,     0,   235,    82,   236,   124,     0,     0,
       0,     0,   616,     0,    24,    25,    26,    15,   206,   377,
       0,   125,     0,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   617,     0,   618,     0,     0,   124,
       0,     0,     0,     0,   343,     0,    24,    25,    26,    15,
     615,    45,     0,   125,     0,    46,     0,     0,     0,     0,
       0,     0,     0,    15,     0,    16,   805,    17,   235,    18,
       0,   124,     0,     0,     0,     0,   206,     0,    24,    25,
      26,    15,   379,   377,     0,   125,     0,    46,     0,     0,
       0,   206,    24,    25,    26,     0,     0,     0,   617,     0,
     618,     0,     0,   124,     0,     0,     0,    15,     0,    45,
      24,    25,    26,    46,     0,     0,     0,   125,     0,     0,
       0,     0,    15,     0,    45,     0,   207,     0,    46,   124,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,   235,     7,   125,   124,     0,     8,     0,     0,     0,
     142,    24,    25,    26,     0,     0,     0,     0,   125,    10,
       0,     0,    11,     0,    12,    13,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,     0,     0,     0,    19,
       0,    20,     0,     0,    21,     7,     0,    22,     0,     8,
      23,   143,     0,     9,     0,     0,     0,    24,    25,    26,
       0,     0,    10,     0,    27,    11,     0,    12,    13,    14,
       0,    15,     0,    16,     0,    17,     0,    18,     0,     0,
       0,     0,    19,     0,   219,     0,     0,    21,     0,     0,
      22,     7,   332,    23,     0,     8,     0,     0,     0,     9,
      24,    25,    26,     0,     0,     0,     0,    27,    10,     0,
       0,    11,     0,    12,    13,    14,     0,    15,     0,    16,
       0,    17,     0,    18,     0,     0,     0,     0,    19,     0,
       0,     0,     0,    21,     0,     0,    22,     7,   182,    23,
       0,     8,     0,     0,     0,     9,    24,    25,    26,     0,
       0,     0,     0,    27,    10,     0,     0,    11,     0,    12,
      13,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,    19,     0,    20,     0,     0,    21,
       7,     0,    22,     0,     8,    23,     0,     0,     9,     0,
       0,     0,    24,    25,    26,     0,     0,    10,     0,    27,
      11,     0,    12,    13,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,     0,     0,     0,    19,     0,   216,
       0,     0,    21,     7,     0,    22,     0,     8,    23,     0,
       0,     9,     0,     0,     0,    24,    25,    26,     0,     0,
      10,     0,    27,    11,     0,    12,    13,    14,     0,    15,
       0,    16,     0,    17,     0,    18,     0,     0,     0,     0,
      19,     0,   219,     0,     0,    21,     7,     0,    22,     0,
       8,    23,     0,     0,   142,     0,     0,     0,    24,    25,
      26,     0,     0,    10,     0,    27,    11,     0,    12,    13,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
       0,     0,     0,    19,     0,    20,     0,     0,    21,     7,
       0,    22,     0,     8,    23,     0,     0,     9,     0,     0,
       0,    24,    25,    26,     0,     0,    10,     0,    27,    11,
       0,    12,    13,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,     0,     0,     0,    19,     0,     0,     0,
       0,    21,     0,     0,    22,     0,     0,    23,     0,     0,
       0,     0,     0,     0,    24,    25,    26,   247,   248,     0,
       0,    27,     0,     0,     0,   249,   250,   251,   252,   253,
     254,     0,     0,   255,   256,     0,     0,   257,    61,    62,
       0,    15,     0,    16,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,   169,     0,     0,    63,     0,     0,
     258,     0,     0,    65,   259,     0,     0,     0,     0,   260,
      24,    25,    26,   261,   262,   247,   248,    66,     0,     0,
       0,     0,     0,   249,   250,   251,   252,   253,   254,     0,
       0,   255,   256,     0,     0,   257,    61,    62,     0,    15,
       0,    16,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,   169,     0,     0,    63,     0,     0,   258,     0,
       0,    65,     0,     0,     0,     0,     0,     0,    24,    25,
      26,   261,   262,     0,    11,    66,    12,    13,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,    22,
       0,     0,    23,     0,     0,     0,     0,     0,     0,    24,
      25,    26,     0,     0,     0,     0,    27,    60,    61,    62,
     103,    15,   104,    16,     0,     0,   106,    18,  -229,     0,
       0,   176,     0,     0,   457,     0,     0,    63,   108,     0,
      64,  -229,     0,    65,     0,     0,   421,   109,     0,     0,
      24,    25,    26,     0,     0,     0,     0,    66,   458,   149,
      61,    62,   150,    15,   151,    16,     0,     0,   152,    18,
       0,     0,     0,     0,     0,     0,   153,     0,     0,    63,
     154,     0,    64,    82,    83,    65,     0,     0,     0,   155,
       0,     0,    24,    25,    26,     0,     0,     0,     0,    66,
     156,   257,    61,    62,   150,    15,   151,    16,     0,     0,
     152,    18,     0,     0,     0,     0,     0,     0,   153,     0,
       0,    63,   154,     0,   258,    82,    83,    65,     0,     0,
       0,   155,     0,     0,    24,    25,    26,     0,     0,     0,
       0,    66,   156,    60,    61,    62,   103,    15,   104,    16,
       0,     0,   106,    18,     0,     0,     0,   176,     0,     0,
     457,     0,     0,    63,   108,     0,    64,     0,     0,    65,
       0,     0,   421,   109,     0,     0,    24,    25,    26,     0,
       0,     0,     0,    66,   458,   149,    61,    62,     0,    15,
       0,    16,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,   169,     0,     0,    63,     0,     0,    64,     0,
       0,    65,     0,    96,     0,     0,     0,     0,    24,    25,
      26,     0,   149,    61,    62,    66,    15,     0,    16,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,   169,
       0,     0,    63,     0,     0,    64,     0,     0,    65,   463,
       0,     0,     0,     0,     0,    24,    25,    26,     0,    60,
      61,    62,    66,    15,     0,    16,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,   174,    63,
       0,     0,    64,     0,     0,    65,     0,     0,     0,     0,
       0,     0,    24,    25,    26,     0,    60,    61,    62,    66,
      15,     0,    16,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,   473,     0,     0,    63,     0,     0,    64,
       0,     0,    65,     0,     0,     0,     0,     0,     0,    24,
      25,    26,     0,   149,    61,    62,    66,    15,     0,    16,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
     169,     0,     0,    63,     0,     0,    64,     0,     0,    65,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
      60,    61,    62,    66,    15,     0,    16,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,   477,     0,     0,
      63,     0,     0,    64,     0,     0,    65,     0,     0,     0,
       0,     0,     0,    24,    25,    26,     0,    60,    61,    62,
      66,    15,     0,    16,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,   479,     0,     0,    63,     0,     0,
      64,     0,     0,    65,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,    60,    61,    62,    66,    15,     0,
      16,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,   483,     0,     0,    63,     0,     0,    64,     0,     0,
      65,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,   577,    61,    62,    66,    15,     0,    16,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,   169,     0,
       0,    63,     0,     0,    64,     0,    15,    65,   377,     0,
      17,     0,   378,     0,    24,    25,    26,     0,    60,    61,
      62,    66,    15,     0,    16,   379,   380,   381,    18,     0,
       0,     0,     0,   382,     0,    24,    25,    26,    63,     0,
       0,    64,     0,     0,    65,     0,     0,     0,    15,   104,
      45,    24,    25,    26,    46,     0,     0,     0,    66,     0,
       0,     0,     0,     0,     0,   694,  -181,   235,     0,     0,
     124,     0,     0,   691,    15,     0,    45,    24,    25,    26,
      46,     0,     0,     0,   125,     0,     0,     0,     0,   353,
       0,     0,     0,   123,   369,   370,   124,     0,     0,     0,
      15,     0,    45,    24,    25,    26,    46,     0,     0,     0,
     125,     0,     0,     0,     0,   353,     0,     0,     0,   123,
     369,   497,   124,     0,     0,     0,    15,   104,    45,    24,
      25,    26,    46,     0,     0,     0,   125,     0,     0,     0,
       0,     0,     0,   694,    15,   235,    45,     0,   124,     0,
      46,   691,     0,     0,     0,    24,    25,    26,     0,     0,
       0,     0,   125,   123,   230,   231,   124,     0,     0,     0,
      15,     0,    45,    24,    25,    26,    46,     0,     0,     0,
     125,     0,     0,     0,     0,   353,     0,     0,  -181,   123,
       0,     0,   124,     0,     0,     0,    15,     0,    45,    24,
      25,    26,    46,     0,   519,     0,   125,     0,     0,     0,
       0,     0,     0,     0,  -181,   123,     0,     0,   124,     0,
       0,     0,    15,     0,    45,    24,    25,    26,    46,     0,
       0,     0,   125,     0,     0,     0,     0,   666,     0,     0,
    -181,   123,     0,     0,   124,     0,     0,     0,    15,   104,
      45,    24,    25,    26,    46,     0,     0,     0,   125,     0,
       0,     0,     0,     0,     0,     0,    15,   123,    45,     0,
     124,     0,    46,   691,     0,     0,     0,    24,    25,    26,
       0,   351,     0,     0,   125,   123,     0,     0,   124,     0,
       0,     0,    15,     0,    45,    24,    25,    26,    46,     0,
       0,     0,   125,     0,     0,     0,     0,   353,     0,     0,
      15,   123,    45,     0,   124,     0,    46,     0,   533,     0,
       0,    24,    25,    26,     0,    15,     0,    45,   125,   123,
       0,    46,   124,     0,     0,     0,     0,     0,     0,    24,
      25,    26,     0,  -181,   123,     0,   125,   124,     0,     0,
       0,    15,     0,    45,    24,    25,    26,    46,     0,     0,
       0,   125,     0,     0,     0,     0,   665,     0,     0,    15,
     123,    45,     0,   124,     0,    46,     0,     0,     0,     0,
      24,    25,    26,     0,     0,     0,   688,   125,   235,     0,
      15,   124,    45,     0,     0,     0,    46,     0,    24,    25,
      26,     0,     0,     0,     0,   125,     0,   736,    15,   235,
      45,     0,   124,     0,    46,     0,     0,     0,     0,    24,
      25,    26,     0,   666,     0,     0,   125,   123,     0,    15,
     124,    45,     0,     0,     0,    46,     0,    24,    25,    26,
       0,     0,     0,     0,   125,     0,     0,    15,   123,    45,
       0,   124,     0,    46,     0,     0,     0,     0,    24,    25,
      26,     0,    15,     0,    45,   125,   235,     0,    46,   124,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,   207,     0,   125,   124,     0,     0,     0,     0,     0,
       0,    24,    25,    26,     0,     0,     0,     0,   125
};

static const yytype_int16 yycheck[] =
{
       3,   123,   213,    47,     3,   133,   134,   139,   100,   133,
     134,   139,    22,    64,   275,   139,    93,    93,    95,    95,
     533,   134,   140,    33,    20,    35,   117,   145,   119,   382,
     633,   529,    64,    59,   450,    49,   617,    64,    65,     1,
      21,   548,    22,     1,    47,    19,    22,   697,   560,     1,
     142,    22,   519,    33,   729,    35,    37,    11,   133,   134,
       1,     1,     1,    12,   139,     1,   533,     1,    44,    59,
       1,    34,    33,     1,    39,    38,   124,    52,    88,   432,
      35,    56,   133,   134,    80,   207,   177,    52,   139,    63,
      64,    65,    28,    64,    34,    69,    57,    55,     1,   102,
      34,    55,    60,   102,   616,    67,    55,   121,    88,    67,
      51,   692,   693,   235,    66,   696,    55,   143,   114,   115,
     123,    20,   538,    22,    55,   728,   801,    55,    67,     1,
     258,   259,     0,    88,   258,   259,    67,   118,   788,    67,
      66,    44,    52,   117,    47,   119,   259,    33,   751,   730,
     657,    37,     1,    66,   245,   736,    42,    67,    34,   133,
     134,   279,    38,   207,    39,   139,    64,    65,    40,   149,
     586,    57,    44,   686,   693,    51,    13,   696,     1,   153,
     518,    80,   162,   163,   259,    34,   257,   700,    34,    38,
     164,    42,    38,   167,   532,   169,   192,    39,   551,   213,
     214,   708,   176,   177,   207,   276,   277,   258,   259,    32,
      52,   730,   679,    33,    66,   114,   115,    37,   716,   686,
     216,    39,    66,   219,   238,    66,   258,   230,    51,   232,
     244,   258,   235,   700,    52,   133,   134,    57,    61,    62,
      63,   139,   709,     7,   247,   337,    15,   369,   247,   371,
     253,    42,   255,   256,   253,    66,   255,   256,    53,    51,
       2,   309,   729,    34,    53,     7,     8,    38,   278,    61,
     361,    63,   770,   299,     1,   773,   774,   257,   252,   306,
      22,    23,    53,    53,   258,   259,   313,   258,   262,    52,
     399,   318,   401,   192,   792,   648,   276,   277,    53,   273,
     798,   349,   350,   351,   278,   353,   280,    34,    31,   570,
     442,    38,     1,    51,   442,    56,     1,   216,   442,   367,
     219,    53,    45,    61,    51,   373,    49,   301,    53,   796,
     304,    53,   306,   347,   801,    58,   310,   311,    51,   313,
      66,    30,     1,   317,   318,    34,    69,   465,   423,    38,
     364,   365,    46,    19,    39,    40,   567,    52,    53,    44,
     258,   259,    11,   411,    52,    53,   369,   442,   371,    52,
      53,   448,   448,    62,   153,    34,    51,   391,    66,    38,
     455,   456,   280,     1,    55,   460,    61,   361,    63,   517,
     169,   442,    51,   517,   136,   409,   487,    63,   446,   141,
       1,   415,    52,    69,   517,   419,    41,    53,   306,    44,
      53,   622,    39,   624,    32,   313,    57,    52,   428,   397,
     318,   399,   513,   401,   515,    57,    32,   555,    57,    47,
      53,   555,   174,    34,    40,    52,    53,    38,    52,    53,
     443,     1,   517,    61,    62,    63,   250,   251,   190,   423,
      51,   193,   426,    57,   428,    61,    62,    63,   200,    53,
     202,   488,    44,   437,   438,    30,   517,    47,   442,    34,
     444,   682,   520,    38,    34,    52,    53,   688,    38,    50,
     555,   455,   456,   694,   458,    40,   460,    40,    33,   463,
      52,    51,    37,   507,   612,   509,   618,    42,   164,   473,
      67,   167,    53,   477,   555,   479,    67,   584,   584,   483,
     176,    34,    57,   487,   488,    38,   519,    32,    56,    34,
      13,    36,   301,    38,    69,   423,   529,    67,    51,    55,
     533,   310,   311,    52,    53,    55,   539,    32,   317,   542,
     282,   283,    12,   517,   442,    67,    61,    62,    63,    28,
      49,    52,    53,   567,   568,   297,   530,   455,   456,    40,
     458,    52,   460,    58,    46,   463,    61,    62,    63,    52,
      53,   545,   546,   547,   706,   549,    52,    53,   706,   553,
      52,   555,   706,   301,    41,    84,   677,   635,   706,   707,
     488,    31,   310,   311,    50,    35,     1,   577,   601,   317,
      52,    53,   601,   102,    32,    45,    34,   273,   622,    49,
     624,   625,   278,    51,   617,   618,   590,    53,    58,   517,
     362,    53,   121,    28,   123,   124,    31,   641,    33,    69,
     644,   706,    53,    61,    62,    63,    52,    53,   304,    51,
      45,    32,    50,    30,    49,   623,   624,   625,    32,    40,
     628,    50,    57,    58,    44,   706,   778,   555,    50,    13,
     663,   664,   665,   666,    69,    50,    55,    51,   682,   683,
      61,    62,    63,    53,   688,    53,   679,    61,    62,    63,
     694,    44,   656,   686,   658,   659,   660,    52,    32,   692,
     693,    32,   590,   696,   473,    36,    40,   700,   477,    50,
     479,    50,    52,   681,   483,   447,    53,   721,   207,    66,
      51,    51,    30,   716,   213,   214,   719,    61,    62,    63,
      61,    62,    63,   697,   698,    47,   729,   730,   470,    50,
     729,    40,   706,   736,   778,   709,   235,   711,    44,   238,
      53,   715,   790,    53,    49,   244,    51,    66,   490,    52,
     492,    15,    30,    52,    32,   473,    22,   805,    51,   477,
     426,   479,   428,    32,    31,   483,    33,   770,   782,    53,
     773,   774,    52,    51,    47,   778,    53,    34,    45,     1,
      39,    53,    49,    61,    62,    63,    39,    39,    39,   792,
      57,    58,    61,    62,    63,   798,    66,    52,   801,    53,
      39,     1,    69,    53,   782,    52,    28,    50,   706,    31,
     309,    33,    53,   513,   788,    52,   121,   791,   663,   664,
     665,   666,   796,    45,   677,   440,   656,    49,    28,   440,
     711,    31,   796,    33,   555,    57,    58,   442,   791,   679,
       1,   700,   788,   585,   343,    45,   530,    69,   347,    49,
     349,   350,   351,    32,   353,    34,    32,    57,    58,    38,
       1,   704,     3,     4,     5,   364,   365,   703,   367,    69,
     369,    32,   371,    34,   373,    51,   790,    38,   451,   545,
     546,   547,    61,    62,    63,    61,    62,    63,    49,   601,
      51,    67,   391,    54,   719,   394,   258,    31,   258,    33,
      61,    62,    63,    37,   458,    23,   487,    68,   213,   214,
     409,    45,   411,   463,   590,    49,   415,   587,   143,    32,
     419,   119,   421,    57,    58,   299,   668,   361,   342,     1,
      22,   628,   491,   238,     6,    69,    -1,    -1,    10,   244,
     245,    -1,    14,    -1,   443,    58,    -1,   446,    61,    62,
      63,    23,    -1,    -1,    26,   260,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    43,    -1,    45,    -1,    -1,    48,    49,    -1,    51,
      52,    53,    54,    -1,   726,    57,    58,    -1,    -1,    61,
      62,    63,    -1,    -1,    -1,   494,    68,    69,    -1,   207,
      -1,    31,    27,    33,    -1,    -1,    -1,    32,   507,    34,
     509,    -1,    -1,    38,    -1,    45,    -1,    32,    -1,    49,
     519,   520,    -1,   522,    -1,    -1,    51,   235,    58,    54,
     529,    30,    47,    32,   533,    -1,    61,    62,    63,    69,
      39,    -1,   347,    68,    -1,     1,    61,    62,    63,    -1,
       6,    -1,    51,    -1,    10,    -1,    -1,    -1,    14,   364,
     365,   560,    61,    62,    63,    -1,    -1,    23,   567,   568,
      26,    -1,    28,    29,    30,    -1,    32,   382,    34,    -1,
      36,    -1,    38,    -1,    -1,    -1,   391,    43,    -1,    45,
      -1,     1,    48,    -1,    -1,    51,    -1,    -1,    54,    -1,
      56,    -1,   601,    -1,   409,    61,    62,    63,    32,    -1,
     415,    -1,    68,    -1,   419,   614,    -1,   616,   617,   618,
      -1,    -1,    32,   622,    34,   624,   625,   432,    38,    -1,
      -1,    -1,   631,    -1,    58,    -1,   635,    61,    62,    63,
     639,    51,   641,    -1,    54,   644,    56,    -1,    -1,    -1,
      -1,    61,    62,    63,    -1,    -1,     2,    -1,    68,    -1,
      -1,     7,     8,   662,   663,   664,   665,   666,    -1,    32,
      -1,    34,    -1,    -1,    20,    21,    22,    23,    -1,    -1,
     679,   680,    -1,   682,   683,    -1,    -1,   686,    51,   688,
       1,    37,    -1,   692,   693,   694,    -1,   696,    61,    62,
      63,   700,   507,    -1,   509,    -1,    -1,    -1,   513,    -1,
      -1,    -1,    -1,    59,    -1,    -1,    27,   716,    -1,    -1,
     719,    32,   721,    34,    -1,    -1,    -1,    38,    -1,    -1,
     729,   730,    -1,    -1,    80,   443,    47,   736,    -1,    -1,
      51,    52,    53,    54,    -1,    -1,   551,    93,    -1,    95,
      61,    62,    63,    -1,   753,    -1,    -1,    68,    32,    -1,
      -1,    -1,   567,   568,    -1,    -1,    -1,    -1,   114,   115,
      -1,   770,   118,    -1,   773,   774,    -1,    51,    31,   778,
      33,    -1,    35,   782,    37,    -1,    60,    61,    62,    63,
     136,   790,    45,   792,    -1,   141,    49,   143,    -1,   798,
      -1,    -1,   801,    -1,    57,    58,   805,    -1,    -1,    -1,
      -1,   519,    -1,    -1,    -1,    -1,    69,   622,    19,   624,
     625,   529,    32,    -1,    34,   533,    -1,    -1,   174,    -1,
      -1,    31,    -1,    33,    -1,    35,   641,    37,    -1,   644,
      -1,    51,    52,   648,   190,    45,   192,   193,    -1,    49,
      -1,    61,    62,    63,   200,   201,   202,    -1,    58,     1,
      -1,    -1,    63,    64,    65,    -1,    -1,    -1,    69,    69,
     216,    -1,    -1,   219,    16,    17,    18,   682,   683,    -1,
     685,    -1,    -1,   688,    -1,    -1,    28,    29,    30,   694,
      32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,   245,
      -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
     618,    -1,    54,    55,   622,   443,   721,   625,    -1,    61,
      62,    63,    -1,    -1,    -1,    67,    68,   732,    -1,    -1,
      -1,    -1,   133,   134,    -1,    -1,   282,   283,   139,    28,
      29,    30,    -1,    32,    -1,    34,    -1,    -1,    -1,    38,
      -1,   297,   153,   299,    -1,   663,   664,   665,   666,    48,
      -1,    -1,    51,   164,    53,    54,   167,    -1,   169,    -1,
      -1,   679,    61,    62,    63,   176,    -1,   782,   686,    68,
      -1,    -1,    -1,    -1,   692,   693,     1,    -1,   696,    -1,
      -1,   519,   700,    -1,    -1,    -1,   342,    -1,   803,    -1,
      -1,   529,    -1,    -1,    -1,   533,    -1,    -1,   716,    -1,
      -1,   719,    -1,    28,    29,    30,   362,    32,    -1,    34,
      -1,   729,   730,    38,    -1,    32,    -1,    34,    -1,    36,
      -1,    38,    -1,    48,    41,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    -1,    51,    52,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    61,    62,    63,   258,   259,    -1,
      -1,     1,   770,    -1,    -1,   773,   774,    -1,    -1,    -1,
     778,    -1,   273,    -1,   782,    -1,    -1,   278,    -1,   280,
      -1,    -1,   790,    -1,   792,    -1,    -1,    -1,    -1,   617,
     798,    -1,    32,   801,    34,    -1,    -1,    -1,    38,    -1,
     301,   447,   448,   304,    -1,   306,    -1,    -1,    -1,   310,
     311,    51,   313,    53,    54,    -1,   317,   318,    -1,    -1,
      -1,    61,    62,    63,   470,    -1,    -1,    -1,    68,    -1,
      32,    -1,    34,    -1,    -1,   663,   664,   665,   666,    41,
      -1,    -1,    -1,    -1,   490,   491,   492,    -1,    -1,    51,
      52,   679,    32,     1,    34,    -1,    -1,    -1,   686,    61,
      62,    63,    -1,    -1,   692,   693,    -1,   513,   696,   515,
      -1,    51,   700,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    61,    62,    63,    32,    33,    34,    -1,   716,    -1,
      38,   719,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,   729,   730,    51,    52,    53,    54,    -1,   736,    -1,
      -1,    -1,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,
      68,    -1,   423,    -1,    -1,   426,    -1,   428,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   584,   585,
      -1,   442,   770,    -1,    -1,   773,   774,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   455,   456,    27,   458,    -1,   460,
      -1,    32,   463,    34,   792,    -1,    -1,    38,    -1,    -1,
     798,    -1,   473,   801,    -1,    -1,   477,    -1,   479,    -1,
      51,    -1,   483,    54,    -1,    -1,    -1,   488,    -1,    -1,
      61,    62,    63,    -1,    32,    -1,    34,    68,    36,    -1,
      38,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,
       6,    -1,    -1,    51,    10,    53,   517,    -1,    14,    -1,
      -1,    59,   668,    61,    62,    63,    -1,    23,    -1,    -1,
      26,   677,    28,    29,    30,    -1,    32,    -1,    34,    -1,
      36,    -1,    38,    -1,   545,   546,   547,    43,     1,    45,
      -1,    -1,    48,     6,   555,    51,    -1,    10,    54,    -1,
      56,    14,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,
      23,    -1,    68,    26,    -1,    28,    29,    30,    -1,    32,
     726,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,   590,
      43,     1,    45,    -1,    -1,    48,     6,    -1,    51,    -1,
      10,    54,    -1,    56,    14,    -1,    -1,    -1,    61,    62,
      63,    -1,    -1,    23,    -1,    68,    26,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,
      -1,    -1,    -1,    43,     1,    45,    -1,    -1,    48,     6,
      -1,    51,    -1,    10,    54,    -1,    -1,    14,    -1,    -1,
      -1,    61,    62,    63,    -1,    -1,    23,    -1,    68,    26,
      -1,    28,    29,    30,    -1,    32,    -1,    34,    -1,    36,
      -1,    38,    -1,    -1,    -1,     1,    43,    -1,    45,    -1,
      -1,    48,    -1,    -1,    51,    -1,    -1,    54,    -1,    -1,
      16,    17,    18,    -1,    61,    62,    63,    -1,    -1,    -1,
      -1,    68,    28,    29,    30,   706,    32,     1,    34,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,    55,
      -1,    -1,    -1,    27,    -1,    61,    62,    63,    32,    33,
      34,    67,    68,    -1,    38,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,     1,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    28,    29,    30,    -1,    32,
      -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    48,    -1,    32,    51,    34,
      -1,    54,    -1,    38,     1,    -1,    -1,    -1,    61,    62,
      63,    -1,    47,    -1,    -1,    68,    51,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    -1,    32,     1,    34,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,
      -1,    -1,    27,    -1,    61,    62,    63,    32,     1,    34,
      -1,    68,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    51,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    27,    -1,    61,    62,    63,    32,
       1,    34,    -1,    68,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    34,    49,    36,    51,    38,
      -1,    54,    -1,    -1,    -1,    -1,     1,    -1,    61,    62,
      63,    32,    51,    34,    -1,    68,    -1,    38,    -1,    -1,
      -1,     1,    61,    62,    63,    -1,    -1,    -1,    49,    -1,
      51,    -1,    -1,    54,    -1,    -1,    -1,    32,    -1,    34,
      61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    32,    -1,    34,    -1,    51,    -1,    38,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    51,     6,    68,    54,    -1,    10,    -1,    -1,    -1,
      14,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    23,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    -1,
      34,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    43,
      -1,    45,    -1,    -1,    48,     6,    -1,    51,    -1,    10,
      54,    55,    -1,    14,    -1,    -1,    -1,    61,    62,    63,
      -1,    -1,    23,    -1,    68,    26,    -1,    28,    29,    30,
      -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,    -1,
      -1,    -1,    43,    -1,    45,    -1,    -1,    48,    -1,    -1,
      51,     6,    53,    54,    -1,    10,    -1,    -1,    -1,    14,
      61,    62,    63,    -1,    -1,    -1,    -1,    68,    23,    -1,
      -1,    26,    -1,    28,    29,    30,    -1,    32,    -1,    34,
      -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,     6,    53,    54,
      -1,    10,    -1,    -1,    -1,    14,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    23,    -1,    -1,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,
      -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,
       6,    -1,    51,    -1,    10,    54,    -1,    -1,    14,    -1,
      -1,    -1,    61,    62,    63,    -1,    -1,    23,    -1,    68,
      26,    -1,    28,    29,    30,    -1,    32,    -1,    34,    -1,
      36,    -1,    38,    -1,    -1,    -1,    -1,    43,    -1,    45,
      -1,    -1,    48,     6,    -1,    51,    -1,    10,    54,    -1,
      -1,    14,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,
      23,    -1,    68,    26,    -1,    28,    29,    30,    -1,    32,
      -1,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,     6,    -1,    51,    -1,
      10,    54,    -1,    -1,    14,    -1,    -1,    -1,    61,    62,
      63,    -1,    -1,    23,    -1,    68,    26,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    36,    -1,    38,    -1,
      -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,     6,
      -1,    51,    -1,    10,    54,    -1,    -1,    14,    -1,    -1,
      -1,    61,    62,    63,    -1,    -1,    23,    -1,    68,    26,
      -1,    28,    29,    30,    -1,    32,    -1,    34,    -1,    36,
      -1,    38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    62,    63,     8,     9,    -1,
      -1,    68,    -1,    -1,    -1,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    30,
      -1,    32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,
      51,    -1,    -1,    54,    55,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,     8,     9,    68,    -1,    -1,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    21,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    30,    -1,    32,
      -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,
      63,    64,    65,    -1,    26,    68,    28,    29,    30,    -1,
      32,    -1,    34,    -1,    36,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    -1,    -1,    -1,    68,    28,    29,    30,
      31,    32,    33,    34,    -1,    -1,    37,    38,    39,    -1,
      -1,    42,    -1,    -1,    45,    -1,    -1,    48,    49,    -1,
      51,    52,    -1,    54,    -1,    -1,    57,    58,    -1,    -1,
      61,    62,    63,    -1,    -1,    -1,    -1,    68,    69,    28,
      29,    30,    31,    32,    33,    34,    -1,    -1,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    -1,    51,    52,    53,    54,    -1,    -1,    -1,    58,
      -1,    -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68,
      69,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    -1,    51,    52,    53,    54,    -1,    -1,
      -1,    58,    -1,    -1,    61,    62,    63,    -1,    -1,    -1,
      -1,    68,    69,    28,    29,    30,    31,    32,    33,    34,
      -1,    -1,    37,    38,    -1,    -1,    -1,    42,    -1,    -1,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    -1,    54,
      -1,    -1,    57,    58,    -1,    -1,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    69,    28,    29,    30,    -1,    32,
      -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,    -1,
      -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    61,    62,
      63,    -1,    28,    29,    30,    68,    32,    -1,    34,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,    55,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,    28,
      29,    30,    68,    32,    -1,    34,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    51,    -1,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    62,    63,    -1,    28,    29,    30,    68,
      32,    -1,    34,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    28,    29,    30,    68,    32,    -1,    34,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    -1,    -1,    51,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      28,    29,    30,    68,    32,    -1,    34,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
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
      -1,    48,    -1,    -1,    51,    -1,    32,    54,    34,    -1,
      36,    -1,    38,    -1,    61,    62,    63,    -1,    28,    29,
      30,    68,    32,    -1,    34,    51,    52,    53,    38,    -1,
      -1,    -1,    -1,    59,    -1,    61,    62,    63,    48,    -1,
      -1,    51,    -1,    -1,    54,    -1,    -1,    -1,    32,    33,
      34,    61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      54,    -1,    -1,    57,    32,    -1,    34,    61,    62,    63,
      38,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,    -1,
      32,    -1,    34,    61,    62,    63,    38,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    51,
      52,    53,    54,    -1,    -1,    -1,    32,    33,    34,    61,
      62,    63,    38,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    32,    51,    34,    -1,    54,    -1,
      38,    57,    -1,    -1,    -1,    61,    62,    63,    -1,    -1,
      -1,    -1,    68,    51,    52,    53,    54,    -1,    -1,    -1,
      32,    -1,    34,    61,    62,    63,    38,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      -1,    -1,    54,    -1,    -1,    -1,    32,    -1,    34,    61,
      62,    63,    38,    -1,    40,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    32,    -1,    34,    61,    62,    63,    38,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    -1,    -1,    54,    -1,    -1,    -1,    32,    33,
      34,    61,    62,    63,    38,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    51,    34,    -1,
      54,    -1,    38,    57,    -1,    -1,    -1,    61,    62,    63,
      -1,    47,    -1,    -1,    68,    51,    -1,    -1,    54,    -1,
      -1,    -1,    32,    -1,    34,    61,    62,    63,    38,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      32,    51,    34,    -1,    54,    -1,    38,    -1,    40,    -1,
      -1,    61,    62,    63,    -1,    32,    -1,    34,    68,    51,
      -1,    38,    54,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    50,    51,    -1,    68,    54,    -1,    -1,
      -1,    32,    -1,    34,    61,    62,    63,    38,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    47,    -1,    -1,    32,
      51,    34,    -1,    54,    -1,    38,    -1,    -1,    -1,    -1,
      61,    62,    63,    -1,    -1,    -1,    49,    68,    51,    -1,
      32,    54,    34,    -1,    -1,    -1,    38,    -1,    61,    62,
      63,    -1,    -1,    -1,    -1,    68,    -1,    49,    32,    51,
      34,    -1,    54,    -1,    38,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    47,    -1,    -1,    68,    51,    -1,    32,
      54,    34,    -1,    -1,    -1,    38,    -1,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    32,    51,    34,
      -1,    54,    -1,    38,    -1,    -1,    -1,    -1,    61,    62,
      63,    -1,    32,    -1,    34,    68,    51,    -1,    38,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    -1,
      -1,    51,    -1,    68,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    -1,    68
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,    71,     1,     6,    10,    14,
      23,    26,    28,    29,    30,    32,    34,    36,    38,    43,
      45,    48,    51,    54,    61,    62,    63,    68,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   187,   188,   206,
     208,   210,   211,   212,   213,    34,    38,    51,   127,   132,
     209,    59,    72,    73,     0,   175,   175,    66,   154,    66,
      28,    29,    30,    48,    51,    54,    68,   168,   169,   186,
     206,   210,   213,   182,   183,   188,    31,    33,    35,    37,
      45,    49,    52,    53,    57,    58,    69,   175,   182,   189,
     207,   215,   217,   219,   220,   221,    56,   175,   189,   202,
      13,   153,    39,    31,    33,    35,    37,    45,    49,    58,
      69,   218,   219,   221,   223,   223,   188,    66,    42,    66,
      53,   132,   135,    51,    54,    68,   133,   134,   208,   209,
       1,    74,   209,    66,   224,     7,    11,    55,   184,   155,
     156,    15,    14,    55,   176,   196,   197,   198,   168,    28,
      31,    33,    37,    45,    49,    58,    69,   158,   159,   160,
     161,   163,   164,   166,   167,   169,   170,   206,   210,    45,
     158,   159,   160,   171,    47,   168,    42,    66,    53,    53,
      53,    53,    53,    53,    34,    36,    38,   208,    53,    53,
      52,    53,   223,    52,    53,    52,    53,   177,   178,   177,
      41,    44,    52,   203,    56,   154,     1,    51,   124,   126,
     127,   128,   129,   131,   132,   134,    45,   182,   183,    45,
     182,   183,    51,   199,   200,   201,   210,   211,   188,   199,
      52,    53,    52,    53,    47,    51,    53,   129,   132,   135,
     136,   207,    56,   128,   132,    51,    77,     8,     9,    16,
      17,    18,    19,    20,    21,    24,    25,    28,    51,    55,
      60,    64,    65,    76,    83,    85,    93,    94,   137,   140,
     144,   145,   146,   147,   160,   161,   164,   166,   210,    76,
      66,   175,    11,    12,    55,   185,     1,    55,    67,   144,
     157,   225,    55,   225,   175,   154,   196,    46,   225,    55,
      57,   221,     1,   165,   167,   168,    52,    53,    53,    39,
     221,   221,   168,    52,    53,   168,    69,   221,    52,    56,
     175,   168,   172,   173,   174,   210,   211,    57,    57,    57,
      57,   175,    53,   175,    53,    53,   175,    14,   175,   204,
     205,   175,    44,    27,    53,   123,   124,   132,   135,    47,
      50,    47,   133,    47,   182,   183,   182,   183,    35,    45,
      67,    52,    40,    67,   132,   132,    53,    52,    53,    52,
      53,    52,    53,    52,    53,    53,    56,    34,    38,    51,
      52,    53,    59,    78,    79,   209,   211,   213,    13,     1,
     127,   132,     1,    34,    95,     1,    28,   138,     1,   138,
       1,   138,     1,    51,   110,   111,   210,     1,   127,   132,
       1,    51,     1,   112,   127,   132,     1,   113,   127,   132,
      45,    57,    69,   214,   217,   146,   147,   160,   210,    76,
       1,    30,    62,    75,   209,     1,    28,    60,   210,    67,
      55,    84,    55,    39,    52,     1,    39,    40,    44,   148,
     149,   150,   151,   168,   148,   214,   214,    45,    69,   168,
     216,   217,   225,    55,   158,   190,   191,   192,   175,   175,
      12,   175,   198,    45,   165,   158,   128,    45,   165,    45,
     165,   158,    28,    45,   165,   158,    67,    52,    40,   154,
      46,    52,    41,   204,   125,   208,    53,    53,    53,   128,
     128,   128,   128,   201,   175,   128,   129,   132,   129,   132,
     128,    53,    75,    52,    53,    51,     1,    66,    50,    40,
      40,   208,    57,   139,   214,   220,   222,   139,   139,    39,
      52,    30,    50,    40,   114,   115,   128,    44,   116,    50,
      13,   152,    50,   161,   210,    53,    53,    53,    75,    51,
      61,    63,    86,   210,   210,    84,    85,    94,   144,     1,
      27,   120,   121,   122,   124,   127,   130,   131,   132,   210,
     128,   175,   177,   152,   151,   161,   161,    28,   162,   163,
     210,   161,   190,     1,    44,    47,   193,   194,   195,   225,
      55,   175,   165,   165,   165,   165,   174,   158,   175,   205,
     175,    58,   208,    53,    79,    41,    52,    80,    81,    82,
     211,   213,    76,     1,    95,     1,    27,    49,    51,    98,
      99,   101,   102,   124,   131,   132,   212,   128,    52,   120,
     111,    95,    99,   104,    53,    52,     1,   117,   118,   119,
     152,   132,    66,   141,   132,   168,   168,   168,    63,    86,
      34,    52,    87,    88,    89,   210,    51,    75,    30,   210,
      30,    93,   125,    47,    50,    47,    47,   148,    40,   177,
     175,   152,   195,   192,   126,    53,    53,    52,   225,    40,
     125,   130,   131,   132,    33,    22,    44,   107,    49,   124,
     133,    57,   220,   220,    49,   124,   220,    66,    15,   222,
      40,   107,   128,    52,    47,   208,   142,   143,    75,    51,
      53,    52,    87,    86,   210,    30,    39,   210,   210,    58,
     122,   132,   122,   122,   122,   175,    47,    82,    98,    58,
     220,    53,    51,   209,    99,   133,    49,   103,   124,   130,
     103,   133,   103,    67,   105,   106,   140,   210,    96,    97,
     210,   104,   118,   119,    55,   144,   225,    55,   225,    86,
      41,    51,    52,    90,    91,    92,   210,   212,    89,    53,
      39,   210,   120,    39,    39,   121,   175,   107,    51,   100,
     101,   127,   132,   103,   108,   109,   209,   130,    52,    67,
      39,    52,    39,   107,    53,    53,    52,   120,    39,   120,
     120,    50,    53,    52,   106,    49,   123,   128,    97,   120,
      92,   120,   101,   209,   128
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
     145,   145,   145,   146,   146,   146,   146,   146,   147,   147,
     147,   147,   147,   148,   148,   149,   149,   150,   150,   151,
     152,   152,   153,   153,   154,   154,   155,   155,   155,   156,
     157,   158,   158,   159,   159,   160,   161,   161,   161,   162,
     162,   163,   163,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   165,   165,   166,   166,   167,   167,   168,
     168,   168,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   170,   170,   171,   171,   172,   172,   173,
     173,   174,   174,   175,   175,   176,   176,   177,   177,   178,
     178,   179,   179,   180,   180,   180,   180,   180,   181,   181,
     181,   181,   181,   182,   182,   182,   183,   183,   183,   184,
     184,   185,   185,   186,   186,   187,   187,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   189,   189,   190,   190,   191,   191,
     191,   192,   193,   193,   193,   194,   194,   195,   196,   196,
     197,   197,   197,   198,   198,   198,   199,   199,   200,   200,
     201,   201,   202,   202,   202,   202,   202,   202,   202,   203,
     203,   204,   204,   205,   205,   205,   206,   206,   206,   206,
     207,   207,   208,   208,   208,   208,   209,   209,   210,   210,
     210,   210,   210,   210,   211,   211,   211,   212,   212,   213,
     213,   213,   214,   214,   214,   215,   215,   216,   216,   217,
     217,   217,   217,   218,   218,   219,   219,   219,   220,   220,
     221,   221,   221,   222,   222,   223,   223,   224,   225,   225
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
       1,     1,     1,     3,     3,     3,     3,     3,     4,     4,
       4,     2,     2,     2,     1,     2,     1,     2,     1,     4,
       0,     2,     0,     2,     3,     3,     0,     2,     2,     2,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     4,     3,     4,     3,
       4,     3,     4,     1,     1,     1,     1,     2,     2,     1,
       1,     1,     3,     1,     4,     1,     1,     1,     3,     3,
       3,     3,     2,     3,     3,     3,     1,     0,     1,     3,
       1,     3,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     4,     3,     2,     4,     3,     4,     3,
       2,     4,     3,     6,     4,     1,     4,     4,     4,     3,
       2,     3,     2,     2,     1,     2,     1,     1,     3,     2,
       1,     1,     4,     4,     1,     1,     1,     1,     3,     3,
       3,     4,     4,     4,     3,     3,     1,     2,     3,     2,
       1,     3,     1,     2,     1,     2,     1,     4,     1,     2,
       3,     2,     1,     3,     2,     1,     0,     1,     3,     1,
       1,     3,     1,     1,     2,     3,     4,     2,     5,     3,
       2,     3,     1,     3,     1,     2,     1,     2,     2,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     1,     3,     1,     1,     3,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     3,     1,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     0,     1,     1
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
#line 102 "parser.y"
                                        {inputExpr = letrec(yyvsp[0],yyvsp[-1]); sp-=2;}
#line 2676 "parser.c.new"
    break;

  case 3: /* start: CTXT context  */
#line 103 "parser.y"
                                        {inputContext = yyvsp[0];	    sp-=1;}
#line 2682 "parser.c.new"
    break;

  case 4: /* start: SCRIPT topModule  */
#line 104 "parser.y"
                                        {valDefns  = yyvsp[0];	    sp-=1;}
#line 2688 "parser.c.new"
    break;

  case 5: /* start: error  */
#line 105 "parser.y"
                                        {syntaxError("input");}
#line 2694 "parser.c.new"
    break;

  case 6: /* topModule: startMain begin modBody end  */
#line 118 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc3(yyvsp[-1]);
					}
#line 2703 "parser.c.new"
    break;

  case 7: /* topModule: startMain '{' modBody '}'  */
#line 122 "parser.y"
                                        {
					 setExportList(singleton(ap(MODULEENT,mkCon(module(currentModule).text))));
					 yyval = gc4(yyvsp[-1]);
					}
#line 2712 "parser.c.new"
    break;

  case 8: /* topModule: TMODULE modname expspec WHERE '{' modBody end  */
#line 127 "parser.y"
                                        {setExportList(yyvsp[-4]);   yyval = gc7(yyvsp[-1]);}
#line 2718 "parser.c.new"
    break;

  case 9: /* topModule: TMODULE modname expspec WHERE error  */
#line 129 "parser.y"
                                        {syntaxError("declaration");}
#line 2724 "parser.c.new"
    break;

  case 10: /* topModule: TMODULE error  */
#line 130 "parser.y"
                                        {syntaxError("module definition");}
#line 2730 "parser.c.new"
    break;

  case 11: /* startMain: %empty  */
#line 136 "parser.y"
                                        {startModule(conMain); 
					 yyval = gc0(NIL);}
#line 2737 "parser.c.new"
    break;

  case 12: /* modname: qconid  */
#line 139 "parser.y"
                                        {startModule(mkCon(mkNestedQual(yyvsp[0]))); yyval = gc1(NIL);}
#line 2743 "parser.c.new"
    break;

  case 13: /* modid: qconid  */
#line 141 "parser.y"
                                        {yyval = mkCon(mkNestedQual(yyvsp[0]));}
#line 2749 "parser.c.new"
    break;

  case 14: /* modid: STRINGLIT  */
#line 142 "parser.y"
                                        { String modName = findPathname(textToStr(textOf(yyvsp[0])));
					  if (modName) { /* fillin pathname if known */
					      yyval = mkStr(findText(modName));
					  } else {
					      yyval = yyvsp[0];
					  }
					}
#line 2761 "parser.c.new"
    break;

  case 15: /* modBody: %empty  */
#line 150 "parser.y"
                                        {yyval = gc0(NIL); }
#line 2767 "parser.c.new"
    break;

  case 16: /* modBody: ';' modBody  */
#line 151 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 2773 "parser.c.new"
    break;

  case 17: /* modBody: topDecls  */
#line 152 "parser.y"
                                        {yyval = gc1(yyvsp[0]);}
#line 2779 "parser.c.new"
    break;

  case 18: /* modBody: impDecls chase  */
#line 153 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2785 "parser.c.new"
    break;

  case 19: /* modBody: impDecls ';' chase topDecls  */
#line 154 "parser.y"
                                        {yyval = gc4(yyvsp[0]);}
#line 2791 "parser.c.new"
    break;

  case 20: /* expspec: %empty  */
#line 159 "parser.y"
                                        {yyval = gc0(exportSelf());}
#line 2797 "parser.c.new"
    break;

  case 21: /* expspec: '(' ')'  */
#line 160 "parser.y"
                                        {yyval = gc2(NIL);}
#line 2803 "parser.c.new"
    break;

  case 22: /* expspec: '(' ',' ')'  */
#line 161 "parser.y"
                                        {yyval = gc3(NIL);}
#line 2809 "parser.c.new"
    break;

  case 23: /* expspec: '(' exports ')'  */
#line 162 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2815 "parser.c.new"
    break;

  case 24: /* expspec: '(' exports ',' ')'  */
#line 163 "parser.y"
                                        {yyval = gc4(yyvsp[-2]);}
#line 2821 "parser.c.new"
    break;

  case 25: /* exports: exports ',' export  */
#line 165 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2827 "parser.c.new"
    break;

  case 26: /* exports: export  */
#line 166 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2833 "parser.c.new"
    break;

  case 27: /* export: qvar  */
#line 171 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2839 "parser.c.new"
    break;

  case 28: /* export: qcon  */
#line 172 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2845 "parser.c.new"
    break;

  case 29: /* export: qconid '(' UPTO ')'  */
#line 173 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 2851 "parser.c.new"
    break;

  case 30: /* export: qconid '(' qnames ')'  */
#line 174 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 2857 "parser.c.new"
    break;

  case 31: /* export: TMODULE modid  */
#line 175 "parser.y"
                                        {yyval = gc2(ap(MODULEENT,yyvsp[0]));}
#line 2863 "parser.c.new"
    break;

  case 32: /* qnames: %empty  */
#line 177 "parser.y"
                                        {yyval = gc0(NIL);}
#line 2869 "parser.c.new"
    break;

  case 33: /* qnames: ','  */
#line 178 "parser.y"
                                        {yyval = gc1(NIL);}
#line 2875 "parser.c.new"
    break;

  case 34: /* qnames: qnames1  */
#line 179 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2881 "parser.c.new"
    break;

  case 35: /* qnames: qnames1 ','  */
#line 180 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 2887 "parser.c.new"
    break;

  case 36: /* qnames1: qnames1 ',' qname  */
#line 182 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 2893 "parser.c.new"
    break;

  case 37: /* qnames1: qname  */
#line 183 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 2899 "parser.c.new"
    break;

  case 38: /* qname: qvar  */
#line 185 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2905 "parser.c.new"
    break;

  case 39: /* qname: qcon  */
#line 186 "parser.y"
                                        {yyval = yyvsp[0];}
#line 2911 "parser.c.new"
    break;

  case 40: /* impDecls: impDecls ';' impDecl  */
#line 191 "parser.y"
                                        {imps = cons(yyvsp[0],imps); yyval=gc3(NIL);}
#line 2917 "parser.c.new"
    break;

  case 41: /* impDecls: impDecls ';'  */
#line 192 "parser.y"
                                        {yyval   = gc2(NIL); }
#line 2923 "parser.c.new"
    break;

  case 42: /* impDecls: impDecl  */
#line 193 "parser.y"
                                        {imps = singleton(yyvsp[0]); yyval=gc1(NIL);}
#line 2929 "parser.c.new"
    break;

  case 43: /* chase: %empty  */
#line 195 "parser.y"
                                        {if (chase(imps)) {
					     clearStack();
					     onto(imps);
					     done();
					     closeAnyInput();
					     return 0;
					 }
					 yyval = gc0(NIL);
					}
#line 2943 "parser.c.new"
    break;

  case 44: /* impDecl: IMPORT modid impspec  */
#line 206 "parser.y"
                                        {addUnqualImport(yyvsp[-1],NIL,yyvsp[0]);
					 yyval = gc3(yyvsp[-1]);}
#line 2950 "parser.c.new"
    break;

  case 45: /* impDecl: IMPORT modid ASMOD modid impspec  */
#line 209 "parser.y"
                                        {addUnqualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc5(yyvsp[-3]);}
#line 2957 "parser.c.new"
    break;

  case 46: /* impDecl: IMPORT QUALIFIED modid ASMOD modid impspec  */
#line 212 "parser.y"
                                        {addQualImport(yyvsp[-3],yyvsp[-1],yyvsp[0]);
					 yyval = gc6(yyvsp[-3]);}
#line 2964 "parser.c.new"
    break;

  case 47: /* impDecl: IMPORT QUALIFIED modid impspec  */
#line 215 "parser.y"
                                        {addQualImport(yyvsp[-1],yyvsp[-1],yyvsp[0]);
					 yyval = gc4(yyvsp[-1]);}
#line 2971 "parser.c.new"
    break;

  case 48: /* impDecl: IMPORT error  */
#line 217 "parser.y"
                                        {syntaxError("import declaration");}
#line 2977 "parser.c.new"
    break;

  case 49: /* impspec: %empty  */
#line 219 "parser.y"
                                        {yyval = gc0(DOTDOT);}
#line 2983 "parser.c.new"
    break;

  case 50: /* impspec: HIDING '(' imports ')'  */
#line 220 "parser.y"
                                        {yyval = gc4(ap(HIDDEN,yyvsp[-1]));}
#line 2989 "parser.c.new"
    break;

  case 51: /* impspec: '(' imports ')'  */
#line 221 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 2995 "parser.c.new"
    break;

  case 52: /* imports: %empty  */
#line 223 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3001 "parser.c.new"
    break;

  case 53: /* imports: ','  */
#line 224 "parser.y"
                                        {yyval = gc1(NIL);}
#line 3007 "parser.c.new"
    break;

  case 54: /* imports: imports1  */
#line 225 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3013 "parser.c.new"
    break;

  case 55: /* imports: imports1 ','  */
#line 226 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3019 "parser.c.new"
    break;

  case 56: /* imports1: imports1 ',' import  */
#line 228 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3025 "parser.c.new"
    break;

  case 57: /* imports1: import  */
#line 229 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3031 "parser.c.new"
    break;

  case 58: /* import: var  */
#line 231 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3037 "parser.c.new"
    break;

  case 59: /* import: CONID  */
#line 232 "parser.y"
                                        {yyval = gc1(pair(yyvsp[0],NONE));}
#line 3043 "parser.c.new"
    break;

  case 60: /* import: CONID '(' UPTO ')'  */
#line 233 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],DOTDOT));}
#line 3049 "parser.c.new"
    break;

  case 61: /* import: CONID '(' names ')'  */
#line 234 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-3],yyvsp[-1]));}
#line 3055 "parser.c.new"
    break;

  case 62: /* names: %empty  */
#line 236 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3061 "parser.c.new"
    break;

  case 63: /* names: ','  */
#line 237 "parser.y"
                                        {yyval = gc1(NIL);}
#line 3067 "parser.c.new"
    break;

  case 64: /* names: names1  */
#line 238 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3073 "parser.c.new"
    break;

  case 65: /* names: names1 ','  */
#line 239 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3079 "parser.c.new"
    break;

  case 66: /* names1: names1 ',' name  */
#line 241 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3085 "parser.c.new"
    break;

  case 67: /* names1: name  */
#line 242 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3091 "parser.c.new"
    break;

  case 68: /* name: var  */
#line 244 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3097 "parser.c.new"
    break;

  case 69: /* name: con  */
#line 245 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3103 "parser.c.new"
    break;

  case 70: /* topDecls: topDecls ';'  */
#line 250 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 3109 "parser.c.new"
    break;

  case 71: /* topDecls: topDecls ';' topDecl  */
#line 251 "parser.y"
                                        {yyval = gc2(yyvsp[-2]);}
#line 3115 "parser.c.new"
    break;

  case 72: /* topDecls: topDecls ';' decl  */
#line 252 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3121 "parser.c.new"
    break;

  case 73: /* topDecls: topDecl  */
#line 253 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3127 "parser.c.new"
    break;

  case 74: /* topDecls: decl  */
#line 254 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3133 "parser.c.new"
    break;

  case 75: /* topDecl: TYPE tyLhs '=' type  */
#line 259 "parser.y"
                                        {defTycon(4,yyvsp[-1],yyvsp[-2],yyvsp[0],SYNONYM);}
#line 3139 "parser.c.new"
    break;

  case 76: /* topDecl: TYPE tyLhs '=' type IN invars  */
#line 261 "parser.y"
                                        {defTycon(6,yyvsp[-3],yyvsp[-4],
						    ap(yyvsp[-2],yyvsp[0]),RESTRICTSYN);}
#line 3146 "parser.c.new"
    break;

  case 77: /* topDecl: TYPE error  */
#line 263 "parser.y"
                                        {syntaxError("type declaration");}
#line 3152 "parser.c.new"
    break;

  case 78: /* topDecl: DATA btype2 '=' constrs deriving  */
#line 265 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(rev(yyvsp[-1]),yyvsp[0]),DATATYPE);}
#line 3159 "parser.c.new"
    break;

  case 79: /* topDecl: DATA context IMPLIES tyLhs '=' constrs deriving  */
#line 268 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],rev(yyvsp[-1])),
						     yyvsp[0]),DATATYPE);}
#line 3167 "parser.c.new"
    break;

  case 80: /* topDecl: DATA btype2  */
#line 271 "parser.y"
                                        {defTycon(2,yyvsp[-1],checkTyLhs(yyvsp[0]),
						    ap(NIL,NIL),DATATYPE);}
#line 3174 "parser.c.new"
    break;

  case 81: /* topDecl: DATA context IMPLIES tyLhs  */
#line 273 "parser.y"
                                        {defTycon(4,yyvsp[-3],yyvsp[0],
						  ap(qualify(yyvsp[-2],NIL),
						     NIL),DATATYPE);}
#line 3182 "parser.c.new"
    break;

  case 82: /* topDecl: DATA error  */
#line 276 "parser.y"
                                        {syntaxError("data declaration");}
#line 3188 "parser.c.new"
    break;

  case 83: /* topDecl: TNEWTYPE btype2 '=' nconstr deriving  */
#line 278 "parser.y"
                                        {defTycon(5,yyvsp[-2],checkTyLhs(yyvsp[-3]),
						    ap(yyvsp[-1],yyvsp[0]),NEWTYPE);}
#line 3195 "parser.c.new"
    break;

  case 84: /* topDecl: TNEWTYPE context IMPLIES tyLhs '=' nconstr deriving  */
#line 281 "parser.y"
                                        {defTycon(7,yyvsp[-2],yyvsp[-3],
						  ap(qualify(yyvsp[-5],yyvsp[-1]),
						     yyvsp[0]),NEWTYPE);}
#line 3203 "parser.c.new"
    break;

  case 85: /* topDecl: TNEWTYPE error  */
#line 284 "parser.y"
                                        {syntaxError("newtype declaration");}
#line 3209 "parser.c.new"
    break;

  case 86: /* topDecl: NEEDPRIMS NUMLIT  */
#line 285 "parser.y"
                                        {if (isInt(yyvsp[0])) {
					     needPrims(intOf(yyvsp[0]), NULL);
					 } else {
					     syntaxError("needprims decl");
					 }
					 sp-=2;}
#line 3220 "parser.c.new"
    break;

  case 87: /* topDecl: NEEDPRIMS error  */
#line 291 "parser.y"
                                        {syntaxError("needprims decl");}
#line 3226 "parser.c.new"
    break;

  case 88: /* tyLhs: tyLhs varid  */
#line 293 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3232 "parser.c.new"
    break;

  case 89: /* tyLhs: CONID  */
#line 294 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3238 "parser.c.new"
    break;

  case 90: /* tyLhs: error  */
#line 295 "parser.y"
                                        {syntaxError("type defn lhs");}
#line 3244 "parser.c.new"
    break;

  case 91: /* invars: invars ',' invar  */
#line 297 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3250 "parser.c.new"
    break;

  case 92: /* invars: invar  */
#line 298 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3256 "parser.c.new"
    break;

  case 93: /* invar: var COCO topType  */
#line 300 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],singleton(yyvsp[-2]),
									yyvsp[0]));}
#line 3263 "parser.c.new"
    break;

  case 94: /* invar: var  */
#line 302 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3269 "parser.c.new"
    break;

  case 95: /* constrs: constrs '|' pconstr  */
#line 304 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3275 "parser.c.new"
    break;

  case 96: /* constrs: pconstr  */
#line 305 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3281 "parser.c.new"
    break;

  case 97: /* pconstr: ALL varids '.' qconstr  */
#line 307 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3288 "parser.c.new"
    break;

  case 98: /* pconstr: constr  */
#line 309 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3294 "parser.c.new"
    break;

  case 99: /* qconstr: context IMPLIES constr  */
#line 311 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3300 "parser.c.new"
    break;

  case 100: /* qconstr: constr  */
#line 312 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3306 "parser.c.new"
    break;

  case 101: /* constr: '!' btype conop bbtype  */
#line 314 "parser.y"
                                        {yyval = gc4(ap(ap(yyvsp[-1],bang(yyvsp[-2])),yyvsp[0]));}
#line 3312 "parser.c.new"
    break;

  case 102: /* constr: btype1 conop bbtype  */
#line 315 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3318 "parser.c.new"
    break;

  case 103: /* constr: btype2 conop bbtype  */
#line 316 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3324 "parser.c.new"
    break;

  case 104: /* constr: bpolyType conop bbtype  */
#line 317 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 3330 "parser.c.new"
    break;

  case 105: /* constr: btype2  */
#line 318 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3336 "parser.c.new"
    break;

  case 106: /* constr: btype3  */
#line 319 "parser.y"
                                        {yyval = checkConstr(yyvsp[0]);}
#line 3342 "parser.c.new"
    break;

  case 107: /* constr: con '{' fieldspecs '}'  */
#line 320 "parser.y"
                                        {yyval = gc4(ap(LABC,pair(yyvsp[-3],rev(yyvsp[-1]))));}
#line 3348 "parser.c.new"
    break;

  case 108: /* constr: con '{' '}'  */
#line 321 "parser.y"
                                        {yyval = gc3(ap(LABC,pair(yyvsp[-2],NIL)));}
#line 3354 "parser.c.new"
    break;

  case 109: /* constr: error  */
#line 322 "parser.y"
                                        {syntaxError("data type declaration");}
#line 3360 "parser.c.new"
    break;

  case 110: /* btype3: btype2 '!' atype  */
#line 324 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3366 "parser.c.new"
    break;

  case 111: /* btype3: btype2 bpolyType  */
#line 325 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3372 "parser.c.new"
    break;

  case 112: /* btype3: btype3 atype  */
#line 326 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3378 "parser.c.new"
    break;

  case 113: /* btype3: btype3 '!' atype  */
#line 327 "parser.y"
                                        {yyval = gc3(ap(yyvsp[-2],bang(yyvsp[0])));}
#line 3384 "parser.c.new"
    break;

  case 114: /* btype3: btype3 bpolyType  */
#line 328 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3390 "parser.c.new"
    break;

  case 115: /* btype3: '(' CONOP ')'  */
#line 329 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3396 "parser.c.new"
    break;

  case 116: /* bbtype: '!' btype  */
#line 331 "parser.y"
                                        {yyval = gc2(bang(yyvsp[0]));}
#line 3402 "parser.c.new"
    break;

  case 117: /* bbtype: btype  */
#line 332 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3408 "parser.c.new"
    break;

  case 118: /* bbtype: bpolyType  */
#line 333 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3414 "parser.c.new"
    break;

  case 119: /* nconstr: pconstr  */
#line 335 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3420 "parser.c.new"
    break;

  case 120: /* fieldspecs: fieldspecs ',' fieldspec  */
#line 337 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3426 "parser.c.new"
    break;

  case 121: /* fieldspecs: fieldspec  */
#line 338 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3432 "parser.c.new"
    break;

  case 122: /* fieldspec: vars COCO polyType  */
#line 340 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3438 "parser.c.new"
    break;

  case 123: /* fieldspec: vars COCO type  */
#line 341 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),yyvsp[0]));}
#line 3444 "parser.c.new"
    break;

  case 124: /* fieldspec: vars COCO '!' type  */
#line 342 "parser.y"
                                        {yyval = gc4(pair(rev(yyvsp[-3]),bang(yyvsp[0])));}
#line 3450 "parser.c.new"
    break;

  case 125: /* deriving: %empty  */
#line 344 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3456 "parser.c.new"
    break;

  case 126: /* deriving: DERIVING qconid  */
#line 345 "parser.y"
                                        {yyval = gc2(singleton(yyvsp[0]));}
#line 3462 "parser.c.new"
    break;

  case 127: /* deriving: DERIVING '(' derivs0 ')'  */
#line 346 "parser.y"
                                        {yyval = gc4(yyvsp[-1]);}
#line 3468 "parser.c.new"
    break;

  case 128: /* derivs0: %empty  */
#line 348 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3474 "parser.c.new"
    break;

  case 129: /* derivs0: derivs  */
#line 349 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3480 "parser.c.new"
    break;

  case 130: /* derivs: derivs ',' qconid  */
#line 351 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3486 "parser.c.new"
    break;

  case 131: /* derivs: qconid  */
#line 352 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3492 "parser.c.new"
    break;

  case 132: /* topDecl: PRIMITIVE prims COCO topType  */
#line 357 "parser.y"
                                        {primDefn(yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=4;}
#line 3498 "parser.c.new"
    break;

  case 133: /* prims: prims ',' prim  */
#line 359 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3504 "parser.c.new"
    break;

  case 134: /* prims: prim  */
#line 360 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3510 "parser.c.new"
    break;

  case 135: /* prims: error  */
#line 361 "parser.y"
                                        {syntaxError("primitive defn");}
#line 3516 "parser.c.new"
    break;

  case 136: /* prim: var STRINGLIT  */
#line 363 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 3522 "parser.c.new"
    break;

  case 137: /* prim: var  */
#line 364 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3528 "parser.c.new"
    break;

  case 138: /* topDecl: FOREIGN IMPORT var STRINGLIT var COCO topType  */
#line 370 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],NIL,yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3534 "parser.c.new"
    break;

  case 139: /* topDecl: FOREIGN IMPORT var var COCO topType  */
#line 372 "parser.y"
               {foreignImport(yyvsp[-5],yyvsp[-3],NIL,yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=6;}
#line 3540 "parser.c.new"
    break;

  case 140: /* topDecl: FOREIGN IMPORT var var STRINGLIT var COCO topType  */
#line 374 "parser.y"
               {foreignImport(yyvsp[-7],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=8;}
#line 3546 "parser.c.new"
    break;

  case 141: /* topDecl: FOREIGN IMPORT var var var COCO topType  */
#line 376 "parser.y"
               {foreignImport(yyvsp[-6],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3552 "parser.c.new"
    break;

  case 142: /* topDecl: FOREIGN var var STRINGLIT var COCO topType  */
#line 378 "parser.y"
               {foreignExport(yyvsp[-6],yyvsp[-5],yyvsp[-4],yyvsp[-3],yyvsp[-2],yyvsp[0]); sp-=7;}
#line 3558 "parser.c.new"
    break;

  case 143: /* topDecl: TCLASS crule fds wherePart  */
#line 383 "parser.y"
                                        {classDefn(intOf(yyvsp[-3]),yyvsp[-2],yyvsp[0],yyvsp[-1]); sp-=4;}
#line 3564 "parser.c.new"
    break;

  case 144: /* topDecl: TINSTANCE irule wherePart  */
#line 384 "parser.y"
                                        {instDefn(intOf(yyvsp[-2]),yyvsp[-1],yyvsp[0]);  sp-=3;}
#line 3570 "parser.c.new"
    break;

  case 145: /* topDecl: DEFAULT '(' dtypes ')'  */
#line 385 "parser.y"
                                        {defaultDefn(intOf(yyvsp[-3]),yyvsp[-1]);  sp-=4;}
#line 3576 "parser.c.new"
    break;

  case 146: /* topDecl: TCLASS error  */
#line 386 "parser.y"
                                        {syntaxError("class declaration");}
#line 3582 "parser.c.new"
    break;

  case 147: /* topDecl: TINSTANCE error  */
#line 387 "parser.y"
                                        {syntaxError("instance declaration");}
#line 3588 "parser.c.new"
    break;

  case 148: /* topDecl: DEFAULT error  */
#line 388 "parser.y"
                                        {syntaxError("default declaration");}
#line 3594 "parser.c.new"
    break;

  case 149: /* crule: context IMPLIES btype2  */
#line 390 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3600 "parser.c.new"
    break;

  case 150: /* crule: btype2  */
#line 391 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3606 "parser.c.new"
    break;

  case 151: /* irule: context IMPLIES btype2  */
#line 393 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],checkPred(yyvsp[0])));}
#line 3612 "parser.c.new"
    break;

  case 152: /* irule: btype2  */
#line 394 "parser.y"
                                        {yyval = gc1(pair(NIL,checkPred(yyvsp[0])));}
#line 3618 "parser.c.new"
    break;

  case 153: /* dtypes: %empty  */
#line 396 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3624 "parser.c.new"
    break;

  case 154: /* dtypes: dtypes1  */
#line 397 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 3630 "parser.c.new"
    break;

  case 155: /* dtypes1: dtypes1 ',' type  */
#line 399 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3636 "parser.c.new"
    break;

  case 156: /* dtypes1: type  */
#line 400 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3642 "parser.c.new"
    break;

  case 157: /* fds: %empty  */
#line 402 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3648 "parser.c.new"
    break;

  case 158: /* fds: '|' fds1  */
#line 403 "parser.y"
                                        {h98DoesntSupport(row,"dependent parameters");
					 yyval = gc2(rev(yyvsp[0]));}
#line 3655 "parser.c.new"
    break;

  case 159: /* fds1: fds1 ',' fd  */
#line 406 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3661 "parser.c.new"
    break;

  case 160: /* fds1: fd  */
#line 407 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 3667 "parser.c.new"
    break;

  case 161: /* fd: varids0 ARROW varids0  */
#line 409 "parser.y"
                                        {yyval = gc3(pair(rev(yyvsp[-2]),rev(yyvsp[0])));}
#line 3673 "parser.c.new"
    break;

  case 162: /* fd: error  */
#line 410 "parser.y"
                                        {syntaxError("functional dependency");}
#line 3679 "parser.c.new"
    break;

  case 163: /* varids0: %empty  */
#line 412 "parser.y"
                                        {yyval = gc0(NIL);}
#line 3685 "parser.c.new"
    break;

  case 164: /* varids0: varids0 varid  */
#line 413 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3691 "parser.c.new"
    break;

  case 165: /* topType: ALL varids '.' topType0  */
#line 418 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3698 "parser.c.new"
    break;

  case 166: /* topType: topType0  */
#line 420 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3704 "parser.c.new"
    break;

  case 167: /* topType0: context IMPLIES topType1  */
#line 422 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3710 "parser.c.new"
    break;

  case 168: /* topType0: topType1  */
#line 423 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3716 "parser.c.new"
    break;

  case 169: /* topType1: bpolyType ARROW topType1  */
#line 425 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3722 "parser.c.new"
    break;

  case 170: /* topType1: btype1 ARROW topType1  */
#line 426 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3728 "parser.c.new"
    break;

  case 171: /* topType1: btype2 ARROW topType1  */
#line 427 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3734 "parser.c.new"
    break;

  case 172: /* topType1: btype  */
#line 428 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3740 "parser.c.new"
    break;

  case 173: /* polyType: ALL varids '.' sigType  */
#line 430 "parser.y"
                                        {yyval = gc4(ap(POLYTYPE,
						     pair(rev(yyvsp[-2]),yyvsp[0])));}
#line 3747 "parser.c.new"
    break;

  case 174: /* polyType: bpolyType  */
#line 432 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3753 "parser.c.new"
    break;

  case 175: /* bpolyType: '(' polyType ')'  */
#line 434 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3759 "parser.c.new"
    break;

  case 176: /* varids: varids varid  */
#line 436 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 3765 "parser.c.new"
    break;

  case 177: /* varids: varid  */
#line 437 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 3771 "parser.c.new"
    break;

  case 178: /* sigType: context IMPLIES type  */
#line 439 "parser.y"
                                        {yyval = gc3(qualify(yyvsp[-2],yyvsp[0]));}
#line 3777 "parser.c.new"
    break;

  case 179: /* sigType: type  */
#line 440 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3783 "parser.c.new"
    break;

  case 180: /* context: '(' ')'  */
#line 442 "parser.y"
                                        {yyval = gc2(NIL);}
#line 3789 "parser.c.new"
    break;

  case 181: /* context: btype2  */
#line 443 "parser.y"
                                        {yyval = gc1(singleton(checkPred(yyvsp[0])));}
#line 3795 "parser.c.new"
    break;

  case 182: /* context: '(' btype2 ')'  */
#line 444 "parser.y"
                                        {yyval = gc3(singleton(checkPred(yyvsp[-1])));}
#line 3801 "parser.c.new"
    break;

  case 183: /* context: '(' btypes2 ')'  */
#line 445 "parser.y"
                                        {yyval = gc3(checkCtxt(rev(yyvsp[-1])));}
#line 3807 "parser.c.new"
    break;

  case 184: /* type: type1  */
#line 447 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3813 "parser.c.new"
    break;

  case 185: /* type: btype2  */
#line 448 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3819 "parser.c.new"
    break;

  case 186: /* type1: btype1  */
#line 450 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3825 "parser.c.new"
    break;

  case 187: /* type1: bpolyType ARROW type  */
#line 451 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3831 "parser.c.new"
    break;

  case 188: /* type1: btype1 ARROW type  */
#line 452 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3837 "parser.c.new"
    break;

  case 189: /* type1: btype2 ARROW type  */
#line 453 "parser.y"
                                        {yyval = gc3(fn(yyvsp[-2],yyvsp[0]));}
#line 3843 "parser.c.new"
    break;

  case 190: /* type1: error  */
#line 454 "parser.y"
                                        {syntaxError("type expression");}
#line 3849 "parser.c.new"
    break;

  case 191: /* btype: btype1  */
#line 456 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3855 "parser.c.new"
    break;

  case 192: /* btype: btype2  */
#line 457 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3861 "parser.c.new"
    break;

  case 193: /* btype1: btype1 atype  */
#line 459 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3867 "parser.c.new"
    break;

  case 194: /* btype1: atype1  */
#line 460 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3873 "parser.c.new"
    break;

  case 195: /* btype2: btype2 atype  */
#line 462 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 3879 "parser.c.new"
    break;

  case 196: /* btype2: qconid  */
#line 463 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3885 "parser.c.new"
    break;

  case 197: /* atype: atype1  */
#line 465 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3891 "parser.c.new"
    break;

  case 198: /* atype: qconid  */
#line 466 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3897 "parser.c.new"
    break;

  case 199: /* atype1: varid  */
#line 468 "parser.y"
                                        {yyval = yyvsp[0];}
#line 3903 "parser.c.new"
    break;

  case 200: /* atype1: '(' ')'  */
#line 469 "parser.y"
                                        {yyval = gc2(typeUnit);}
#line 3909 "parser.c.new"
    break;

  case 201: /* atype1: '(' ARROW ')'  */
#line 470 "parser.y"
                                        {yyval = gc3(typeArrow);}
#line 3915 "parser.c.new"
    break;

  case 202: /* atype1: '(' type1 ')'  */
#line 471 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3921 "parser.c.new"
    break;

  case 203: /* atype1: '(' btype2 ')'  */
#line 472 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3927 "parser.c.new"
    break;

  case 204: /* atype1: '(' tupCommas ')'  */
#line 473 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 3933 "parser.c.new"
    break;

  case 205: /* atype1: '(' btypes2 ')'  */
#line 474 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3939 "parser.c.new"
    break;

  case 206: /* atype1: '(' typeTuple ')'  */
#line 475 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 3945 "parser.c.new"
    break;

  case 207: /* atype1: '[' type ']'  */
#line 476 "parser.y"
                                        {yyval = gc3(ap(typeList,yyvsp[-1]));}
#line 3951 "parser.c.new"
    break;

  case 208: /* atype1: '[' ']'  */
#line 477 "parser.y"
                                        {yyval = gc2(typeList);}
#line 3957 "parser.c.new"
    break;

  case 209: /* atype1: '_'  */
#line 478 "parser.y"
                                        {h98DoesntSupport(row,"anonymous type variables");
					 yyval = gc1(inventVar());}
#line 3964 "parser.c.new"
    break;

  case 210: /* btypes2: btypes2 ',' btype2  */
#line 481 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3970 "parser.c.new"
    break;

  case 211: /* btypes2: btype2 ',' btype2  */
#line 482 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3976 "parser.c.new"
    break;

  case 212: /* typeTuple: type1 ',' type  */
#line 484 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3982 "parser.c.new"
    break;

  case 213: /* typeTuple: btype2 ',' type1  */
#line 485 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 3988 "parser.c.new"
    break;

  case 214: /* typeTuple: btypes2 ',' type1  */
#line 486 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 3994 "parser.c.new"
    break;

  case 215: /* typeTuple: typeTuple ',' type  */
#line 487 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4000 "parser.c.new"
    break;

  case 216: /* gendecl: INFIXN optDigit ops  */
#line 492 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],NON_ASS,yyvsp[-1]));}
#line 4006 "parser.c.new"
    break;

  case 217: /* gendecl: INFIXN error  */
#line 493 "parser.y"
                                        {syntaxError("fixity decl");}
#line 4012 "parser.c.new"
    break;

  case 218: /* gendecl: INFIXL optDigit ops  */
#line 494 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],LEFT_ASS,yyvsp[-1]));}
#line 4018 "parser.c.new"
    break;

  case 219: /* gendecl: INFIXL error  */
#line 495 "parser.y"
                                        {syntaxError("fixity decl");}
#line 4024 "parser.c.new"
    break;

  case 220: /* gendecl: INFIXR optDigit ops  */
#line 496 "parser.y"
                                        {yyval = gc3(fixdecl(yyvsp[-2],yyvsp[0],RIGHT_ASS,yyvsp[-1]));}
#line 4030 "parser.c.new"
    break;

  case 221: /* gendecl: INFIXR error  */
#line 497 "parser.y"
                                        {syntaxError("fixity decl");}
#line 4036 "parser.c.new"
    break;

  case 222: /* gendecl: vars COCO topType  */
#line 498 "parser.y"
                                        {yyval = gc3(sigdecl(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4042 "parser.c.new"
    break;

  case 223: /* gendecl: vars COCO error  */
#line 499 "parser.y"
                                        {syntaxError("type signature");}
#line 4048 "parser.c.new"
    break;

  case 224: /* optDigit: NUMLIT  */
#line 501 "parser.y"
                                        {yyval = gc1(checkPrec(yyvsp[0]));}
#line 4054 "parser.c.new"
    break;

  case 225: /* optDigit: %empty  */
#line 502 "parser.y"
                                        {yyval = gc0(mkInt(DEF_PREC));}
#line 4060 "parser.c.new"
    break;

  case 226: /* ops: ops ',' op  */
#line 504 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4066 "parser.c.new"
    break;

  case 227: /* ops: op  */
#line 505 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4072 "parser.c.new"
    break;

  case 228: /* vars: vars ',' var  */
#line 507 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4078 "parser.c.new"
    break;

  case 229: /* vars: var  */
#line 508 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4084 "parser.c.new"
    break;

  case 230: /* decls: '{' decls0 end  */
#line 510 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4090 "parser.c.new"
    break;

  case 231: /* decls: '{' decls1 end  */
#line 511 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4096 "parser.c.new"
    break;

  case 232: /* decls0: %empty  */
#line 513 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4102 "parser.c.new"
    break;

  case 233: /* decls0: decls0 ';'  */
#line 514 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4108 "parser.c.new"
    break;

  case 234: /* decls0: decls1 ';'  */
#line 515 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4114 "parser.c.new"
    break;

  case 235: /* decls1: decls0 decl  */
#line 517 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4120 "parser.c.new"
    break;

  case 236: /* decl: gendecl  */
#line 519 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4126 "parser.c.new"
    break;

  case 237: /* decl: funlhs rhs  */
#line 520 "parser.y"
                                        {yyval = gc2(ap(FUNBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4132 "parser.c.new"
    break;

  case 238: /* decl: funlhs COCO type rhs  */
#line 521 "parser.y"
                                        {yyval = gc4(ap(FUNBIND,
						     pair(yyvsp[-3],ap(RSIGN,
								ap(yyvsp[0],yyvsp[-1])))));}
#line 4140 "parser.c.new"
    break;

  case 239: /* decl: pat0 rhs  */
#line 524 "parser.y"
                                        {yyval = gc2(ap(PATBIND,pair(yyvsp[-1],yyvsp[0])));}
#line 4146 "parser.c.new"
    break;

  case 240: /* funlhs: funlhs0  */
#line 526 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4152 "parser.c.new"
    break;

  case 241: /* funlhs: funlhs1  */
#line 527 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4158 "parser.c.new"
    break;

  case 242: /* funlhs: npk  */
#line 528 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4164 "parser.c.new"
    break;

  case 243: /* funlhs0: pat10_vI varop pat0  */
#line 530 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4170 "parser.c.new"
    break;

  case 244: /* funlhs0: infixPat varop pat0  */
#line 531 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4176 "parser.c.new"
    break;

  case 245: /* funlhs0: NUMLIT varop pat0  */
#line 532 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4182 "parser.c.new"
    break;

  case 246: /* funlhs0: var varop_pl pat0  */
#line 533 "parser.y"
                                        {yyval = gc3(ap2(yyvsp[-1],yyvsp[-2],yyvsp[0]));}
#line 4188 "parser.c.new"
    break;

  case 247: /* funlhs0: var '+' pat0_INT  */
#line 534 "parser.y"
                                        {yyval = gc3(ap2(varPlus,yyvsp[-2],yyvsp[0]));}
#line 4194 "parser.c.new"
    break;

  case 248: /* funlhs1: '(' funlhs0 ')' apat  */
#line 536 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4200 "parser.c.new"
    break;

  case 249: /* funlhs1: '(' funlhs1 ')' apat  */
#line 537 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4206 "parser.c.new"
    break;

  case 250: /* funlhs1: '(' npk ')' apat  */
#line 538 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-2],yyvsp[0]));}
#line 4212 "parser.c.new"
    break;

  case 251: /* funlhs1: var apat  */
#line 539 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4218 "parser.c.new"
    break;

  case 252: /* funlhs1: funlhs1 apat  */
#line 540 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4224 "parser.c.new"
    break;

  case 253: /* rhs: rhs1 wherePart  */
#line 542 "parser.y"
                                        {yyval = gc2(letrec(yyvsp[0],yyvsp[-1]));}
#line 4230 "parser.c.new"
    break;

  case 254: /* rhs: error  */
#line 543 "parser.y"
                                        {syntaxError("declaration");}
#line 4236 "parser.c.new"
    break;

  case 255: /* rhs1: '=' exp  */
#line 545 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 4242 "parser.c.new"
    break;

  case 256: /* rhs1: gdrhs  */
#line 546 "parser.y"
                                        {yyval = gc1(grded(rev(yyvsp[0])));}
#line 4248 "parser.c.new"
    break;

  case 257: /* gdrhs: gdrhs gddef  */
#line 548 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4254 "parser.c.new"
    break;

  case 258: /* gdrhs: gddef  */
#line 549 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4260 "parser.c.new"
    break;

  case 259: /* gddef: '|' exp0 '=' exp  */
#line 551 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(yyvsp[-2],yyvsp[0])));}
#line 4266 "parser.c.new"
    break;

  case 260: /* wherePart: %empty  */
#line 553 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4272 "parser.c.new"
    break;

  case 261: /* wherePart: WHERE decls  */
#line 554 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4278 "parser.c.new"
    break;

  case 262: /* lwherePart: %empty  */
#line 559 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4284 "parser.c.new"
    break;

  case 263: /* lwherePart: WHERE ldecls  */
#line 560 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4290 "parser.c.new"
    break;

  case 264: /* ldecls: '{' ldecls0 end  */
#line 563 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4296 "parser.c.new"
    break;

  case 265: /* ldecls: '{' ldecls1 end  */
#line 564 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4302 "parser.c.new"
    break;

  case 266: /* ldecls0: %empty  */
#line 567 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4308 "parser.c.new"
    break;

  case 267: /* ldecls0: ldecls0 ';'  */
#line 568 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4314 "parser.c.new"
    break;

  case 268: /* ldecls0: ldecls1 ';'  */
#line 569 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4320 "parser.c.new"
    break;

  case 269: /* ldecls1: ldecls0 ldecl  */
#line 572 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4326 "parser.c.new"
    break;

  case 270: /* ldecl: decl  */
#line 574 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4332 "parser.c.new"
    break;

  case 271: /* pat: npk  */
#line 579 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4338 "parser.c.new"
    break;

  case 272: /* pat: pat_npk  */
#line 580 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4344 "parser.c.new"
    break;

  case 273: /* pat_npk: pat0 COCO type  */
#line 582 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4350 "parser.c.new"
    break;

  case 274: /* pat_npk: pat0  */
#line 583 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4356 "parser.c.new"
    break;

  case 275: /* npk: var '+' NUMLIT  */
#line 585 "parser.y"
                                        {yyval = gc3(ap2(varPlus,yyvsp[-2],yyvsp[0]));}
#line 4362 "parser.c.new"
    break;

  case 276: /* pat0: var  */
#line 587 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4368 "parser.c.new"
    break;

  case 277: /* pat0: NUMLIT  */
#line 588 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4374 "parser.c.new"
    break;

  case 278: /* pat0: pat0_vI  */
#line 589 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4380 "parser.c.new"
    break;

  case 279: /* pat0_INT: var  */
#line 591 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4386 "parser.c.new"
    break;

  case 280: /* pat0_INT: pat0_vI  */
#line 592 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4392 "parser.c.new"
    break;

  case 281: /* pat0_vI: pat10_vI  */
#line 594 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4398 "parser.c.new"
    break;

  case 282: /* pat0_vI: infixPat  */
#line 595 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4404 "parser.c.new"
    break;

  case 283: /* infixPat: '-' pat10  */
#line 597 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4410 "parser.c.new"
    break;

  case 284: /* infixPat: '-' error  */
#line 598 "parser.y"
                                        {syntaxError("pattern");}
#line 4416 "parser.c.new"
    break;

  case 285: /* infixPat: var qconop pat10  */
#line 599 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4422 "parser.c.new"
    break;

  case 286: /* infixPat: var qconop '-' pat10  */
#line 600 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4428 "parser.c.new"
    break;

  case 287: /* infixPat: NUMLIT qconop pat10  */
#line 601 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4434 "parser.c.new"
    break;

  case 288: /* infixPat: NUMLIT qconop '-' pat10  */
#line 602 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4440 "parser.c.new"
    break;

  case 289: /* infixPat: pat10_vI qconop pat10  */
#line 603 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4446 "parser.c.new"
    break;

  case 290: /* infixPat: pat10_vI qconop '-' pat10  */
#line 604 "parser.y"
                                        {yyval = gc4(ap(NEG,ap2(yyvsp[-2],only(yyvsp[-3]),yyvsp[0])));}
#line 4452 "parser.c.new"
    break;

  case 291: /* infixPat: infixPat qconop pat10  */
#line 605 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4458 "parser.c.new"
    break;

  case 292: /* infixPat: infixPat qconop '-' pat10  */
#line 606 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4464 "parser.c.new"
    break;

  case 293: /* pat10: fpat  */
#line 608 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4470 "parser.c.new"
    break;

  case 294: /* pat10: apat  */
#line 609 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4476 "parser.c.new"
    break;

  case 295: /* pat10_vI: fpat  */
#line 611 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4482 "parser.c.new"
    break;

  case 296: /* pat10_vI: apat_vI  */
#line 612 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4488 "parser.c.new"
    break;

  case 297: /* fpat: fpat apat  */
#line 614 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4494 "parser.c.new"
    break;

  case 298: /* fpat: gcon apat  */
#line 615 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4500 "parser.c.new"
    break;

  case 299: /* apat: NUMLIT  */
#line 617 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4506 "parser.c.new"
    break;

  case 300: /* apat: var  */
#line 618 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4512 "parser.c.new"
    break;

  case 301: /* apat: apat_vI  */
#line 619 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4518 "parser.c.new"
    break;

  case 302: /* apat_vI: var '@' apat  */
#line 621 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4524 "parser.c.new"
    break;

  case 303: /* apat_vI: gcon  */
#line 622 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4530 "parser.c.new"
    break;

  case 304: /* apat_vI: qcon '{' patbinds '}'  */
#line 623 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4536 "parser.c.new"
    break;

  case 305: /* apat_vI: CHARLIT  */
#line 624 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4542 "parser.c.new"
    break;

  case 306: /* apat_vI: STRINGLIT  */
#line 625 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4548 "parser.c.new"
    break;

  case 307: /* apat_vI: '_'  */
#line 626 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4554 "parser.c.new"
    break;

  case 308: /* apat_vI: '(' pat_npk ')'  */
#line 627 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4560 "parser.c.new"
    break;

  case 309: /* apat_vI: '(' npk ')'  */
#line 628 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4566 "parser.c.new"
    break;

  case 310: /* apat_vI: '(' pats2 ')'  */
#line 629 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4572 "parser.c.new"
    break;

  case 311: /* apat_vI: '[' pats1 ']'  */
#line 630 "parser.y"
                                        {yyval = gc3(ap(FINLIST,rev(yyvsp[-1])));}
#line 4578 "parser.c.new"
    break;

  case 312: /* apat_vI: '~' apat  */
#line 631 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4584 "parser.c.new"
    break;

  case 313: /* pats2: pats2 ',' pat  */
#line 633 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4590 "parser.c.new"
    break;

  case 314: /* pats2: pat ',' pat  */
#line 634 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],singleton(yyvsp[-2])));}
#line 4596 "parser.c.new"
    break;

  case 315: /* pats1: pats1 ',' pat  */
#line 636 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4602 "parser.c.new"
    break;

  case 316: /* pats1: pat  */
#line 637 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4608 "parser.c.new"
    break;

  case 317: /* patbinds: %empty  */
#line 639 "parser.y"
                                        {yyval = gc0(NIL);}
#line 4614 "parser.c.new"
    break;

  case 318: /* patbinds: patbinds1  */
#line 640 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 4620 "parser.c.new"
    break;

  case 319: /* patbinds1: patbinds1 ',' patbind  */
#line 642 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4626 "parser.c.new"
    break;

  case 320: /* patbinds1: patbind  */
#line 643 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 4632 "parser.c.new"
    break;

  case 321: /* patbind: qvar '=' pat  */
#line 645 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 4638 "parser.c.new"
    break;

  case 322: /* patbind: var  */
#line 646 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4644 "parser.c.new"
    break;

  case 323: /* exp: exp_err  */
#line 651 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4650 "parser.c.new"
    break;

  case 324: /* exp: error  */
#line 652 "parser.y"
                                        {syntaxError("expression");}
#line 4656 "parser.c.new"
    break;

  case 325: /* exp_err: exp0a COCO sigType  */
#line 654 "parser.y"
                                        {yyval = gc3(ap(ESIGN,pair(yyvsp[-2],yyvsp[0])));}
#line 4662 "parser.c.new"
    break;

  case 326: /* exp_err: exp0  */
#line 655 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4668 "parser.c.new"
    break;

  case 327: /* exp0: exp0a  */
#line 657 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4674 "parser.c.new"
    break;

  case 328: /* exp0: exp0b  */
#line 658 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4680 "parser.c.new"
    break;

  case 329: /* exp0a: infixExpa  */
#line 660 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4686 "parser.c.new"
    break;

  case 330: /* exp0a: exp10a  */
#line 661 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4692 "parser.c.new"
    break;

  case 331: /* exp0b: infixExpb  */
#line 663 "parser.y"
                                        {yyval = gc1(ap(INFIX,yyvsp[0]));}
#line 4698 "parser.c.new"
    break;

  case 332: /* exp0b: exp10b  */
#line 664 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4704 "parser.c.new"
    break;

  case 333: /* infixExpa: infixExpa qop '-' exp10a  */
#line 666 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4710 "parser.c.new"
    break;

  case 334: /* infixExpa: infixExpa qop exp10a  */
#line 667 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4716 "parser.c.new"
    break;

  case 335: /* infixExpa: '-' exp10a  */
#line 668 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4722 "parser.c.new"
    break;

  case 336: /* infixExpa: exp10a qop '-' exp10a  */
#line 669 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4729 "parser.c.new"
    break;

  case 337: /* infixExpa: exp10a qop exp10a  */
#line 671 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4735 "parser.c.new"
    break;

  case 338: /* infixExpb: infixExpa qop '-' exp10b  */
#line 673 "parser.y"
                                        {yyval = gc4(ap(NEG,ap(ap(yyvsp[-2],yyvsp[-3]),yyvsp[0])));}
#line 4741 "parser.c.new"
    break;

  case 339: /* infixExpb: infixExpa qop exp10b  */
#line 674 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],yyvsp[-2]),yyvsp[0]));}
#line 4747 "parser.c.new"
    break;

  case 340: /* infixExpb: '-' exp10b  */
#line 675 "parser.y"
                                        {yyval = gc2(ap(NEG,only(yyvsp[0])));}
#line 4753 "parser.c.new"
    break;

  case 341: /* infixExpb: exp10a qop '-' exp10b  */
#line 676 "parser.y"
                                        {yyval = gc4(ap(NEG,
						     ap(ap(yyvsp[-2],only(yyvsp[-3])),yyvsp[0])));}
#line 4760 "parser.c.new"
    break;

  case 342: /* infixExpb: exp10a qop exp10b  */
#line 678 "parser.y"
                                        {yyval = gc3(ap(ap(yyvsp[-1],only(yyvsp[-2])),yyvsp[0]));}
#line 4766 "parser.c.new"
    break;

  case 343: /* exp10a: CASEXP exp OF '{' alts end  */
#line 680 "parser.y"
                                        {yyval = gc6(ap(CASE,pair(yyvsp[-4],rev(yyvsp[-1]))));}
#line 4772 "parser.c.new"
    break;

  case 344: /* exp10a: DO '{' stmts end  */
#line 681 "parser.y"
                                        {yyval = gc4(ap(DOCOMP,checkDo(yyvsp[-1])));}
#line 4778 "parser.c.new"
    break;

  case 345: /* exp10a: appExp  */
#line 682 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4784 "parser.c.new"
    break;

  case 346: /* exp10b: '\\' pats ARROW exp  */
#line 684 "parser.y"
                                        {yyval = gc4(ap(LAMBDA,      
						     pair(rev(yyvsp[-2]),
							  pair(yyvsp[-1],yyvsp[0]))));}
#line 4792 "parser.c.new"
    break;

  case 347: /* exp10b: LET ldecls IN exp  */
#line 687 "parser.y"
                                        {yyval = gc4(letrec(yyvsp[-2],yyvsp[0]));}
#line 4798 "parser.c.new"
    break;

  case 348: /* exp10b: IF exp then_exp else_exp  */
#line 688 "parser.y"
                                        {yyval = gc4(ap(COND,triple(yyvsp[-2],yyvsp[-1],yyvsp[0])));}
#line 4804 "parser.c.new"
    break;

  case 349: /* then_exp: ';' THEN exp  */
#line 693 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4810 "parser.c.new"
    break;

  case 350: /* then_exp: THEN exp  */
#line 694 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4816 "parser.c.new"
    break;

  case 351: /* else_exp: ';' ELSE exp  */
#line 696 "parser.y"
                                        {yyval = gc3(yyvsp[0]);}
#line 4822 "parser.c.new"
    break;

  case 352: /* else_exp: ELSE exp  */
#line 697 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4828 "parser.c.new"
    break;

  case 353: /* pats: pats apat  */
#line 700 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 4834 "parser.c.new"
    break;

  case 354: /* pats: apat  */
#line 701 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4840 "parser.c.new"
    break;

  case 355: /* appExp: appExp aexp  */
#line 703 "parser.y"
                                        {yyval = gc2(ap(yyvsp[-1],yyvsp[0]));}
#line 4846 "parser.c.new"
    break;

  case 356: /* appExp: aexp  */
#line 704 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4852 "parser.c.new"
    break;

  case 357: /* aexp: qvar  */
#line 706 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4858 "parser.c.new"
    break;

  case 358: /* aexp: qvar '@' aexp  */
#line 707 "parser.y"
                                        {yyval = gc3(ap(ASPAT,pair(yyvsp[-2],yyvsp[0])));}
#line 4864 "parser.c.new"
    break;

  case 359: /* aexp: '~' aexp  */
#line 708 "parser.y"
                                        {yyval = gc2(ap(LAZYPAT,yyvsp[0]));}
#line 4870 "parser.c.new"
    break;

  case 360: /* aexp: '_'  */
#line 709 "parser.y"
                                        {yyval = gc1(WILDCARD);}
#line 4876 "parser.c.new"
    break;

  case 361: /* aexp: gcon  */
#line 710 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4882 "parser.c.new"
    break;

  case 362: /* aexp: qcon '{' fbinds '}'  */
#line 711 "parser.y"
                                        {yyval = gc4(ap(CONFLDS,pair(yyvsp[-3],yyvsp[-1])));}
#line 4888 "parser.c.new"
    break;

  case 363: /* aexp: aexp '{' fbinds '}'  */
#line 712 "parser.y"
                                        {yyval = gc4(ap(UPDFLDS,
						     triple(yyvsp[-3],NIL,yyvsp[-1])));}
#line 4895 "parser.c.new"
    break;

  case 364: /* aexp: NUMLIT  */
#line 714 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4901 "parser.c.new"
    break;

  case 365: /* aexp: CHARLIT  */
#line 715 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4907 "parser.c.new"
    break;

  case 366: /* aexp: STRINGLIT  */
#line 716 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4913 "parser.c.new"
    break;

  case 367: /* aexp: REPEAT  */
#line 717 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4919 "parser.c.new"
    break;

  case 368: /* aexp: '(' exp ')'  */
#line 718 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4925 "parser.c.new"
    break;

  case 369: /* aexp: '(' exps2 ')'  */
#line 719 "parser.y"
                                        {yyval = gc3(buildTuple(yyvsp[-1]));}
#line 4931 "parser.c.new"
    break;

  case 370: /* aexp: '[' list ']'  */
#line 720 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 4937 "parser.c.new"
    break;

  case 371: /* aexp: '(' exp10a qop ')'  */
#line 721 "parser.y"
                                        {yyval = gc4(ap(yyvsp[-1],yyvsp[-2]));}
#line 4943 "parser.c.new"
    break;

  case 372: /* aexp: '(' qvarop_mi exp0 ')'  */
#line 722 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4949 "parser.c.new"
    break;

  case 373: /* aexp: '(' qconop exp0 ')'  */
#line 723 "parser.y"
                                        {yyval = gc4(ap(ap(nameFlip,yyvsp[-2]),yyvsp[-1]));}
#line 4955 "parser.c.new"
    break;

  case 374: /* exps2: exps2 ',' exp  */
#line 725 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4961 "parser.c.new"
    break;

  case 375: /* exps2: exp ',' exp  */
#line 726 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],cons(yyvsp[-2],NIL)));}
#line 4967 "parser.c.new"
    break;

  case 376: /* alts: alts1  */
#line 728 "parser.y"
                                        {yyval = yyvsp[0];}
#line 4973 "parser.c.new"
    break;

  case 377: /* alts: ';' alts  */
#line 729 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 4979 "parser.c.new"
    break;

  case 378: /* alts1: alts1 ';' alt  */
#line 731 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 4985 "parser.c.new"
    break;

  case 379: /* alts1: alts1 ';'  */
#line 732 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 4991 "parser.c.new"
    break;

  case 380: /* alts1: alt  */
#line 733 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 4997 "parser.c.new"
    break;

  case 381: /* alt: pat altRhs wherePart  */
#line 735 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],letrec(yyvsp[0],yyvsp[-1])));}
#line 5003 "parser.c.new"
    break;

  case 382: /* altRhs: guardAlts  */
#line 737 "parser.y"
                                        {yyval = gc1(grded(rev(yyvsp[0])));}
#line 5009 "parser.c.new"
    break;

  case 383: /* altRhs: ARROW exp  */
#line 738 "parser.y"
                                        {yyval = gc2(pair(yyvsp[-1],yyvsp[0]));}
#line 5015 "parser.c.new"
    break;

  case 384: /* altRhs: error  */
#line 739 "parser.y"
                                        {syntaxError("case expression");}
#line 5021 "parser.c.new"
    break;

  case 385: /* guardAlts: guardAlts guardAlt  */
#line 741 "parser.y"
                                        {yyval = gc2(cons(yyvsp[0],yyvsp[-1]));}
#line 5027 "parser.c.new"
    break;

  case 386: /* guardAlts: guardAlt  */
#line 742 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 5033 "parser.c.new"
    break;

  case 387: /* guardAlt: '|' exp0 ARROW exp  */
#line 744 "parser.y"
                                        {yyval = gc4(pair(yyvsp[-1],pair(yyvsp[-2],yyvsp[0])));}
#line 5039 "parser.c.new"
    break;

  case 388: /* stmts: stmts1  */
#line 747 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5045 "parser.c.new"
    break;

  case 389: /* stmts: ';' stmts  */
#line 748 "parser.y"
                                        {yyval = gc2(yyvsp[0]);}
#line 5051 "parser.c.new"
    break;

  case 390: /* stmts1: stmts1 ';' stmt  */
#line 750 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 5057 "parser.c.new"
    break;

  case 391: /* stmts1: stmts1 ';'  */
#line 751 "parser.y"
                                        {yyval = gc2(yyvsp[-1]);}
#line 5063 "parser.c.new"
    break;

  case 392: /* stmts1: stmt  */
#line 752 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 5069 "parser.c.new"
    break;

  case 393: /* stmt: exp_err FROM exp  */
#line 755 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 5075 "parser.c.new"
    break;

  case 394: /* stmt: LET ldecls  */
#line 756 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 5081 "parser.c.new"
    break;

  case 395: /* stmt: exp_err  */
#line 758 "parser.y"
                                        {yyval = gc1(ap(DOQUAL,yyvsp[0]));}
#line 5087 "parser.c.new"
    break;

  case 396: /* fbinds: %empty  */
#line 760 "parser.y"
                                        {yyval = gc0(NIL);}
#line 5093 "parser.c.new"
    break;

  case 397: /* fbinds: fbinds1  */
#line 761 "parser.y"
                                        {yyval = gc1(rev(yyvsp[0]));}
#line 5099 "parser.c.new"
    break;

  case 398: /* fbinds1: fbinds1 ',' fbind  */
#line 763 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 5105 "parser.c.new"
    break;

  case 399: /* fbinds1: fbind  */
#line 764 "parser.y"
                                        {yyval = gc1(singleton(yyvsp[0]));}
#line 5111 "parser.c.new"
    break;

  case 400: /* fbind: var  */
#line 766 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5117 "parser.c.new"
    break;

  case 401: /* fbind: qvar '=' exp  */
#line 767 "parser.y"
                                        {yyval = gc3(pair(yyvsp[-2],yyvsp[0]));}
#line 5123 "parser.c.new"
    break;

  case 402: /* list: exp  */
#line 772 "parser.y"
                                        {yyval = gc1(ap(FINLIST,cons(yyvsp[0],NIL)));}
#line 5129 "parser.c.new"
    break;

  case 403: /* list: exps2  */
#line 773 "parser.y"
                                        {yyval = gc1(ap(FINLIST,rev(yyvsp[0])));}
#line 5135 "parser.c.new"
    break;

  case 404: /* list: exp zipquals  */
#line 774 "parser.y"
                                        {
					 if (length(yyvsp[0])!=1) {
					     syntaxError("list comprehension");
					 }
					 yyval = gc2(ap(COMP,pair(yyvsp[-1],hd(yyvsp[0]))));
					}
#line 5146 "parser.c.new"
    break;

  case 405: /* list: exp UPTO exp  */
#line 780 "parser.y"
                                        {yyval = gc3(ap(ap(nameFromTo,yyvsp[-2]),yyvsp[0]));}
#line 5152 "parser.c.new"
    break;

  case 406: /* list: exp ',' exp UPTO  */
#line 781 "parser.y"
                                        {yyval = gc4(ap(ap(nameFromThen,yyvsp[-3]),yyvsp[-1]));}
#line 5158 "parser.c.new"
    break;

  case 407: /* list: exp UPTO  */
#line 782 "parser.y"
                                        {yyval = gc2(ap(nameFrom,yyvsp[-1]));}
#line 5164 "parser.c.new"
    break;

  case 408: /* list: exp ',' exp UPTO exp  */
#line 783 "parser.y"
                                        {yyval = gc5(ap(ap(ap(nameFromThenTo,
								yyvsp[-4]),yyvsp[-2]),yyvsp[0]));}
#line 5171 "parser.c.new"
    break;

  case 409: /* zipquals: zipquals '|' quals  */
#line 786 "parser.y"
                                        {yyval = gc3(cons(rev(yyvsp[0]),yyvsp[-2]));}
#line 5177 "parser.c.new"
    break;

  case 410: /* zipquals: '|' quals  */
#line 787 "parser.y"
                                        {yyval = gc2(cons(rev(yyvsp[0]),NIL));}
#line 5183 "parser.c.new"
    break;

  case 411: /* quals: quals ',' qual  */
#line 789 "parser.y"
                                        {yyval = gc3(cons(yyvsp[0],yyvsp[-2]));}
#line 5189 "parser.c.new"
    break;

  case 412: /* quals: qual  */
#line 790 "parser.y"
                                        {yyval = gc1(cons(yyvsp[0],NIL));}
#line 5195 "parser.c.new"
    break;

  case 413: /* qual: exp FROM exp  */
#line 792 "parser.y"
                                        {yyval = gc3(ap(FROMQUAL,pair(yyvsp[-2],yyvsp[0])));}
#line 5201 "parser.c.new"
    break;

  case 414: /* qual: exp  */
#line 793 "parser.y"
                                        {yyval = gc1(ap(BOOLQUAL,yyvsp[0]));}
#line 5207 "parser.c.new"
    break;

  case 415: /* qual: LET ldecls  */
#line 794 "parser.y"
                                        {yyval = gc2(ap(QWHERE,yyvsp[0]));}
#line 5213 "parser.c.new"
    break;

  case 416: /* gcon: qcon  */
#line 799 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5219 "parser.c.new"
    break;

  case 417: /* gcon: '(' ')'  */
#line 800 "parser.y"
                                        {yyval = gc2(nameUnit);}
#line 5225 "parser.c.new"
    break;

  case 418: /* gcon: '[' ']'  */
#line 801 "parser.y"
                                        {yyval = gc2(nameNil);}
#line 5231 "parser.c.new"
    break;

  case 419: /* gcon: '(' tupCommas ')'  */
#line 802 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5237 "parser.c.new"
    break;

  case 420: /* tupCommas: tupCommas ','  */
#line 804 "parser.y"
                                        {yyval = gc2(mkTuple(tupleOf(yyvsp[-1])+1));}
#line 5243 "parser.c.new"
    break;

  case 421: /* tupCommas: ','  */
#line 805 "parser.y"
                                        {yyval = gc1(mkTuple(2));}
#line 5249 "parser.c.new"
    break;

  case 422: /* varid: VARID  */
#line 807 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5255 "parser.c.new"
    break;

  case 423: /* varid: HIDING  */
#line 808 "parser.y"
                                        {yyval = gc1(varHiding);}
#line 5261 "parser.c.new"
    break;

  case 424: /* varid: QUALIFIED  */
#line 809 "parser.y"
                                        {yyval = gc1(varQualified);}
#line 5267 "parser.c.new"
    break;

  case 425: /* varid: ASMOD  */
#line 810 "parser.y"
                                        {yyval = gc1(varAsMod);}
#line 5273 "parser.c.new"
    break;

  case 426: /* qconid: QCONID  */
#line 812 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5279 "parser.c.new"
    break;

  case 427: /* qconid: CONID  */
#line 813 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5285 "parser.c.new"
    break;

  case 428: /* var: varid  */
#line 815 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5291 "parser.c.new"
    break;

  case 429: /* var: '(' VAROP ')'  */
#line 816 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5297 "parser.c.new"
    break;

  case 430: /* var: '(' '+' ')'  */
#line 817 "parser.y"
                                        {yyval = gc3(varPlus);}
#line 5303 "parser.c.new"
    break;

  case 431: /* var: '(' '-' ')'  */
#line 818 "parser.y"
                                        {yyval = gc3(varMinus);}
#line 5309 "parser.c.new"
    break;

  case 432: /* var: '(' '!' ')'  */
#line 819 "parser.y"
                                        {yyval = gc3(varBang);}
#line 5315 "parser.c.new"
    break;

  case 433: /* var: '(' '.' ')'  */
#line 820 "parser.y"
                                        {yyval = gc3(varDot);}
#line 5321 "parser.c.new"
    break;

  case 434: /* qvar: QVARID  */
#line 822 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5327 "parser.c.new"
    break;

  case 435: /* qvar: '(' QVAROP ')'  */
#line 823 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5333 "parser.c.new"
    break;

  case 436: /* qvar: var  */
#line 824 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5339 "parser.c.new"
    break;

  case 437: /* con: CONID  */
#line 826 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5345 "parser.c.new"
    break;

  case 438: /* con: '(' CONOP ')'  */
#line 827 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5351 "parser.c.new"
    break;

  case 439: /* qcon: QCONID  */
#line 829 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5357 "parser.c.new"
    break;

  case 440: /* qcon: '(' QCONOP ')'  */
#line 830 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5363 "parser.c.new"
    break;

  case 441: /* qcon: con  */
#line 831 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5369 "parser.c.new"
    break;

  case 442: /* varop: '+'  */
#line 833 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5375 "parser.c.new"
    break;

  case 443: /* varop: '-'  */
#line 834 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5381 "parser.c.new"
    break;

  case 444: /* varop: varop_mipl  */
#line 835 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5387 "parser.c.new"
    break;

  case 445: /* varop_mi: '+'  */
#line 837 "parser.y"
                                        {yyval = gc1(varPlus);}
#line 5393 "parser.c.new"
    break;

  case 446: /* varop_mi: varop_mipl  */
#line 838 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5399 "parser.c.new"
    break;

  case 447: /* varop_pl: '-'  */
#line 840 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5405 "parser.c.new"
    break;

  case 448: /* varop_pl: varop_mipl  */
#line 841 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5411 "parser.c.new"
    break;

  case 449: /* varop_mipl: VAROP  */
#line 843 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5417 "parser.c.new"
    break;

  case 450: /* varop_mipl: '`' varid '`'  */
#line 844 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5423 "parser.c.new"
    break;

  case 451: /* varop_mipl: '!'  */
#line 845 "parser.y"
                                        {yyval = gc1(varBang);}
#line 5429 "parser.c.new"
    break;

  case 452: /* varop_mipl: '.'  */
#line 846 "parser.y"
                                        {yyval = gc1(varDot);}
#line 5435 "parser.c.new"
    break;

  case 453: /* qvarop: '-'  */
#line 848 "parser.y"
                                        {yyval = gc1(varMinus);}
#line 5441 "parser.c.new"
    break;

  case 454: /* qvarop: qvarop_mi  */
#line 849 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5447 "parser.c.new"
    break;

  case 455: /* qvarop_mi: QVAROP  */
#line 851 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5453 "parser.c.new"
    break;

  case 456: /* qvarop_mi: '`' QVARID '`'  */
#line 852 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5459 "parser.c.new"
    break;

  case 457: /* qvarop_mi: varop_mi  */
#line 853 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5465 "parser.c.new"
    break;

  case 458: /* conop: CONOP  */
#line 856 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5471 "parser.c.new"
    break;

  case 459: /* conop: '`' CONID '`'  */
#line 857 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5477 "parser.c.new"
    break;

  case 460: /* qconop: QCONOP  */
#line 859 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5483 "parser.c.new"
    break;

  case 461: /* qconop: '`' QCONID '`'  */
#line 860 "parser.y"
                                        {yyval = gc3(yyvsp[-1]);}
#line 5489 "parser.c.new"
    break;

  case 462: /* qconop: conop  */
#line 861 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5495 "parser.c.new"
    break;

  case 463: /* op: varop  */
#line 863 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5501 "parser.c.new"
    break;

  case 464: /* op: conop  */
#line 864 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5507 "parser.c.new"
    break;

  case 465: /* qop: qvarop  */
#line 866 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5513 "parser.c.new"
    break;

  case 466: /* qop: qconop  */
#line 867 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5519 "parser.c.new"
    break;

  case 467: /* begin: %empty  */
#line 872 "parser.y"
                                        {goOffside(startColumn);}
#line 5525 "parser.c.new"
    break;

  case 468: /* end: '}'  */
#line 875 "parser.y"
                                        {yyval = yyvsp[0];}
#line 5531 "parser.c.new"
    break;

  case 469: /* end: error  */
#line 876 "parser.y"
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
#line 5547 "parser.c.new"
    break;


#line 5551 "parser.c.new"

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

#line 891 "parser.y"


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
