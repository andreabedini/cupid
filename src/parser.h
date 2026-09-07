/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_H_INCLUDED
#define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 19 "parser.y"

#define YYSTYPE Cell

#line 53 "parser.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
enum yytokentype {
    YYEMPTY = -2,
    YYEOF = 0,       /* "end of file"  */
    YYerror = 256,   /* error  */
    YYUNDEF = 257,   /* "invalid token"  */
    EXPR = 258,      /* EXPR  */
    CTXT = 259,      /* CTXT  */
    SCRIPT = 260,    /* SCRIPT  */
    CASEXP = 261,    /* CASEXP  */
    OF = 262,        /* OF  */
    DATA = 263,      /* DATA  */
    TYPE = 264,      /* TYPE  */
    IF = 265,        /* IF  */
    THEN = 266,      /* THEN  */
    ELSE = 267,      /* ELSE  */
    WHERE = 268,     /* WHERE  */
    LET = 269,       /* LET  */
    IN = 270,        /* IN  */
    INFIXN = 271,    /* INFIXN  */
    INFIXL = 272,    /* INFIXL  */
    INFIXR = 273,    /* INFIXR  */
    PRIMITIVE = 274, /* PRIMITIVE  */
    TNEWTYPE = 275,  /* TNEWTYPE  */
    DEFAULT = 276,   /* DEFAULT  */
    DERIVING = 277,  /* DERIVING  */
    DO = 278,        /* DO  */
    TCLASS = 279,    /* TCLASS  */
    TINSTANCE = 280, /* TINSTANCE  */
    REPEAT = 281,    /* REPEAT  */
    ALL = 282,       /* ALL  */
    NUMLIT = 283,    /* NUMLIT  */
    CHARLIT = 284,   /* CHARLIT  */
    STRINGLIT = 285, /* STRINGLIT  */
    VAROP = 286,     /* VAROP  */
    VARID = 287,     /* VARID  */
    CONOP = 288,     /* CONOP  */
    CONID = 289,     /* CONID  */
    QVAROP = 290,    /* QVAROP  */
    QVARID = 291,    /* QVARID  */
    QCONOP = 292,    /* QCONOP  */
    QCONID = 293,    /* QCONID  */
    COCO = 294,      /* COCO  */
    UPTO = 295,      /* UPTO  */
    FROM = 296,      /* FROM  */
    ARROW = 297,     /* ARROW  */
    IMPLIES = 298,   /* IMPLIES  */
    TMODULE = 299,   /* TMODULE  */
    IMPORT = 300,    /* IMPORT  */
    HIDING = 301,    /* HIDING  */
    QUALIFIED = 302, /* QUALIFIED  */
    ASMOD = 303,     /* ASMOD  */
    NEEDPRIMS = 304, /* NEEDPRIMS  */
    FOREIGN = 305    /* FOREIGN  */
};
typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

int yyparse(void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
