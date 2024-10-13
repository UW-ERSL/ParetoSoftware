// This recursive descent parser is used to evaluate
// mathematical expressions. It has provisions for creating
// variables, assigning values to them and using them in
// subsequent expressions. It also provides for predefined
// constants. The code is based on similar codes by Herbert
// Schildt and Mark Morley, but with a number of corrections,
// additions and modifications.

// (C)1995, 1999, C. Bond. All rights reserved.

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "evald.h"

/* prototypes for external math functions */


/* prototypes for functions in this file. */

void GetToken(void);
int Level1(double *r);
int Level2(double *r);
int Level3(double *r);
int Level4(double *r);
int Level5(double *r);
int Level6(double *r);

/* variable declarations */
int   ERRNUM;                /* The error number */
char  ERTOK[TOKLEN + 1];     /* The token that generated the error */
int   ERPOS;                 /* The offset from the start of the expression */
char* ERANC;                 /* Used to calculate ERPOS */

/*
   Add any "constants" here...  These are "read-only" values that are
   provided as a convenience to the user.  Their values can not be
   permanently changed, although they can be temporarily overriden by
   assigning a new value.  The first field is the variable name, the
   second is its value.
*/
VARIABLE Consts[] =
{
   /* name,             value */
   { "ans",             0.0 },
   { "pi",              3.141592654 },
   { "e",               2.718281828 },
   { "BoltzmannConst",	1.38066e-23 },
   { "ElectronCharge",  1.60219e-19 },
   { "NA",              6.022e23 },
   { "e0",              8.8542e-12 },
   { "mu0",             3.141592654*4e-7 },
   { "VelocityOfLight",	2.9979e8 },
   { "PlankConst",      6.6262e-34 },
   { "ElectronMass",	9.1095e-31 },
   { "ProtonMass",      1.6726e-27 },
   { "Infinity",        1.23e+308 }, 
   { "",0.0 },
};

/*
   Add any math functions that you wish to recognise here...  The first
   field is the name of the function as it would appear in an expression.
   The second field tells how many arguments to expect.  The third is
   a pointer to the actual function to use. Number of arguments currently
   limited to 4. When more than one argument is indicated, pad the space
   between the number and the function pointer with comma separated NULLs.
*/
FUNCTION Funcs[] =
{
   /* name, function to call */
   { "sin",         1,      sin },
   { "cos",         1,      cos },
   { "tan",         1,      tan },
   { "asin",        1,      asin },
   { "acos",        1,      acos },
   { "atan",        1,      atan },
   { "atan2",       2,      NULL,atan2 },
   { "sinh",        1,      sinh },
   { "cosh",        1,      cosh },
   { "tanh",        1,      tanh },
   { "exp",         1,      exp },
   { "log",         1,      log },
   { "log10",       1,      log10 },
   { "sqrt",        1,      sqrt },
   { "pow",         2,      NULL,pow },
   { "fabs",         1,      fabs },
   { "",0,NULL }
};

VARIABLE        Vars[MAXVARS];       /* Array for user-defined variables */
char*           expression;          /* Pointer to the user's expression */
char   token[TOKLEN + 1];   /* Holds the current token */
int             type;                /* Type of the current token */

char* GetConst(int index)
{
	return Consts[index].name;
}

char* GetFuncs(int index)
{
	return Funcs[index].name;
}
int GetNumArgs(int index)
{
    return Funcs[index].args;
}

/*
 * ClearAllVars()
 * Erases all user-defined variables from memory. Note that constants
 * can not be erased or modified in any way by the user.
 * Returns nothing.
 */
void ClearAllVars()
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
   {
      *Vars[i].name = 0;
      Vars[i].value = 0;
   }
}

/*
 * ClearVar( char* name )
 *
 * Erases the user-defined variable that is called NAME from memory.
 * Note that constants are not affected.
 *
 * Returns 1 if the variable was found and erased, or 0 if it didn't
 * exist.
 */
int ClearVar( char* name )
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
      if( *Vars[i].name && ! strcmp( name, Vars[i].name ) )
      {
         *Vars[i].name = 0;
         Vars[i].value = 0;
         return( 1 );
      }
   return( 0 );
}


/*
 * GetValue( char* name, double* value )
 *
 * Looks up the specified variable (or constant) known as NAME and
 * returns its contents in VALUE.
 *
 * First the user-defined variables are searched, then the constants are
 * searched.
 * Returns 1 if the value was found, or 0 if it wasn't.
 */
int GetValue( char* name, double* value )
{
   int i;

   /* Now check the user-defined variables. */
   for( i = 0; i < MAXVARS; i++ )
      if( *Vars[i].name && ! strcmp( name, Vars[i].name ) )
      {
         *value = Vars[i].value;
         return( 1 );
      }

   /* Now check the programmer-defined constants. */
   for( i = 0; *Consts[i].name; i++ )
      if( *Consts[i].name && ! strcmp( name, Consts[i].name ) )
      {
         *value = Consts[i].value;
         return( 1 );
      }
   return( 0 );
}


/*
 * SetValue( char* name, double* value )
 *
 * First, it erases any user-defined variable that is called NAME.  Then
 * it creates a new variable called NAME and gives it the value VALUE.
 *
 * Returns 1 if the value was added, or 0 if there was no more room.
 */
int SetValue( char* name, double* value )
{
   int  i;

   ClearVar( name );
   for( i = 0; i < MAXVARS; i++ )
      if( ! *Vars[i].name )
      {
         strcpy( Vars[i].name, name );
         Vars[i].name[VARLEN] = 0;
         Vars[i].value = *value;
         return( 1 );
      }
   return( 0 );
}

void ERR(int n) {
	ERRNUM = n;
	ERPOS = (int) (expression-ERANC-1);
	strcpy(ERTOK,token);
}

/*
 * GetToken()   Internal use only
 *
 * This function is used to grab the next token from the expression that
 * is being evaluated.
 */
void GetToken()
{
   char* t, *endptr;
   double value;
   
   type = 0;
   t = token;
   while( iswhite( *expression ) )
      expression++;
   if( isdelim( *expression ) )
   {
      type = DEL;
      *t++ = *expression++;
   }
   else if( isnumer( *expression ) )
   {
      type = NUM;
      value = strtod(expression,&endptr);
      while( expression < endptr)
	      *t++ = *expression++;
   }
   else if( isalnum( *expression ) )
   {
      type = VAR;
      while( isalnum( *expression ) )
        *t++ = *expression++;
      token[VARLEN] = 0;
   }
   else if( *expression )
   {
      *t++ = *expression++;
      *t = 0;
      ERR( E_SYNTAX );
   }
   *t = 0;
   while( iswhite( *expression ) )
      expression++;
}


/*
 * Level1( double* r )   Internal use only
 * This function handles any variable assignment operations.
 */
int Level1( double* r )
{
    char t[VARLEN + 1];

    if( type == VAR ) {
        if( *expression == '=' ) {
            strcpy( t, token );
            GetToken();
            GetToken();
            if( !*token ) {
                ClearVar( t );
                return (E_OK);
            }
            ERRNUM = Level2( r );
            if (ERRNUM != E_OK) return(ERRNUM); 
            if ( ! SetValue( t, r ) )
                return (E_MAXVARS);
            else return(E_OK);
        }
    }
    ERRNUM = Level2(r);
    return ERRNUM;
}

/*
 * Level2( double* r )   Internal use only
 * This function handles any addition and subtraction operations.
 */
int Level2( double* r )
{
    double t = 0;
    char o;

    ERRNUM = Level3( r );
    if (ERRNUM != E_OK) return(ERRNUM);
    while( (o = *token) == '+' || o == '-' ) {
       GetToken();
       ERRNUM=Level3( &t );
       if (ERRNUM != E_OK) return(ERRNUM);
       if( o == '+' ) {
          *r = *r + t;
/*          printf("r+t\n");    */
       }
       else if ( o == '-' ) {
          *r = *r - t;
/*          printf("r-t\n");    */
       }
    }
    return(E_OK);
}

/*
 * Level3( double* r )   Internal use only
 * This function handles any multiplication, division, or modulo.
 */
int Level3( double* r )
{
    double t;
    char o;

    ERRNUM = Level4( r );
    if (ERRNUM != E_OK) return(ERRNUM);
    while( (o = *token) == '*' || o == '/' || o == '%' ) {
       GetToken();
       ERRNUM = Level4( &t );
       if (ERRNUM != E_OK) return(ERRNUM);
       if( o == '*' ) {
          *r = *r * t;
/*          printf("r * t\n");  */
       }
       else if( o == '/' ) {
          if( t == 0 )
             return( E_DIVZERO );
          *r = *r / t;
/*          printf("r / t\n");  */
       }
       else if( o == '%' ) {
          if( t == 0 )
             return( E_DIVZERO );
          *r = fmod( *r, t );
/*          printf("r mod t\n");    */
       }
    }
    return(E_OK);
}

/*
 * Level4( double* r )   Internal use only
 * This function handles any "to the power of" operations.
 */
int Level4( double* r )
{
    double t;

    ERRNUM = Level5( r );
    if (ERRNUM != E_OK) {
        return(ERRNUM);
    }
    while ( *token == '^' ) {
       GetToken();
       ERRNUM = Level5( &t );   /* This was to Level6 !!! */
       if (ERRNUM != E_OK) return(ERRNUM);
       *r = pow( *r, t );
/*       printf("r ^ t\n"); */
    }
    return(E_OK);
}

/*
 * Level5( double* r )   Internal use only
 * This function handles any unary + or - signs.
 */
int Level5( double* r )
{
    char o = 0;

    if ( *token == '+' || *token == '-' ) {
       o = *token;
       GetToken();
    }
    ERRNUM = Level6( r );
    if (ERRNUM != E_OK) return(ERRNUM);
    if( o == '-' ) {
       *r = -*r;
/*       printf("-r \n");   */  
    }
    return(E_OK);
}


/*
 * Level6( double* r )   Internal use only
 * This function handles any literal numbers, variables, or functions.
 */
int Level6( double* r )
{
    int  i;
    int  n;
    double a[MAXARGS];

    if( *token == '(' ) {
        GetToken();
        if( *token == ')' )
            return( E_NOARG );
        ERRNUM = Level1( r );
        if (ERRNUM != E_OK) return (ERRNUM);
        if( *token != ')' )
            return( E_UNBALAN );
        GetToken();
    }
    else {
        if( type == NUM ) {
            *r = atof( token );
            GetToken();
        }
        else if ( type == VAR ) {
            if ( *expression == '(' ) {
                for( i = 0; Funcs[i].args; i++ ) {
                    if ( ! strcmp( token, Funcs[i].name ) ) {
                        GetToken();
                        n = 0;
                        do {
                            GetToken();
                            if ( *token == ')' || *token == ',' )
                                return( E_NOARG );
                            a[n] = 0;
                            ERRNUM = Level1( &a[n] );
                            if(ERRNUM != E_OK) return(ERRNUM);
                            n++;
                        } while ( (n < MAXARGS) && (*token == ','));
                        if ( *token != ')' )
                            return( E_UNBALAN );
                        GetToken();
                        if ( n != Funcs[i].args ) {
                            strcpy( token, Funcs[i].name );
                            return( E_NUMARGS );
                        }
                        switch (n) {
                            case 1:
                                *r = Funcs[i].func1(a[0]);
                                break;
                            case 2:
                                *r = Funcs[i].func2(a[0],a[1]);
                                break;
                            case 3:
                                *r = Funcs[i].func3(a[0],a[1],a[2]);
                                break;
                            case 4:
                                *r = Funcs[i].func4(a[0],a[1],a[2],a[3]);
                                break;
                            case 5:
                                *r = Funcs[i].func5(a[0],a[1],a[2],a[3],a[4]);
                                break;
                        }
                        return( E_OK );
                    }
                }
                if ( ! Funcs[i].name )
                    return( E_BADFUNC );
            }
            else if ( ! GetValue( token, r ) )
                return( E_UNKNOWN );
            GetToken();
        }
        else
            return( E_SYNTAX );
    }
    return( E_OK );
}


/*
 * EvaluateD( char* e, double* result, int* epos )
 *
 * This function is called to evaluate the expression E and return the
 * answer in RESULT.
 *
 * Returns E_OK if the expression is valid, or an error code.
 */
int EvaluateD( char* e, double* result, int* epos )
{
/* initialize  values */
    ERRNUM = E_OK;
    *result = 0.0;
    expression = e;
    ERANC = e;
 
    GetToken();
    if ( ! *token ) {
        if ( *expression == '.' ) {
            *epos = (int) (expression-ERANC);
            *result = 0.0; 
            return( E_SYNTAX );
        }
        else {
            *epos = (int)(expression - ERANC - 1);
            *result = 0.0;
            return( E_EMPTY );
        }
    }
    ERRNUM = Level1( result );
    if (ERRNUM != E_OK) {
        *epos = (int)(expression - ERANC - 1);
        return(ERRNUM);
    }
    if (*expression || *token) {
        *epos = (int)(expression - ERANC - 1);
        *result = 0.0;
        if (*token == ')' || *token == '(')
            return( E_UNBALAN);
        return( E_SYNTAX );
    }
    Consts[0].value = *result;
    return( E_OK );
}

