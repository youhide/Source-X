
#include <cmath>
#include "../game/CObjBase.h"
#include "../game/CResource.h"
#include "../game/CWorld.h"
#include "CException.h"
#include "CExpression.h"
#include "common.h"
#include "CResourceBase.h"

tchar CExpression::sm_szMessages[DEFMSG_QTY][128] =
{
	#define MSG(a,b) b,
	#include "../tables/defmessages.tbl"
};

lpctstr const CExpression::sm_szMsgNames[DEFMSG_QTY] =
{
	#define MSG(a,b) #a,
	#include "../tables/defmessages.tbl"
};

dword ahextoi( lpctstr pszStr ) // Convert hex string to integer
{
	// Unfortunatly the library func cant handle the number FFFFFFFF
	// tchar * sstop; return( strtol( s, &sstop, 16 ));

	if ( pszStr == NULL )
		return 0;
	bool bHex = false;

	GETNONWHITESPACE(pszStr);

	if ( *pszStr == '0' )
	{
		if (*++pszStr != '.')
			bHex = true;
		pszStr--;
	}

	dword val = 0;
	for (;;)
	{
		tchar ch = static_cast<tchar>(toupper(*pszStr));
		if ( IsDigit(ch) )
			ch -= '0';
		else if ( bHex && ( ch >= 'A' ) && ( ch <= 'F' ))
		{
			ch -= 'A' - 10;
		}
		else if ( !bHex && ( ch == '.' ) )
		{
			pszStr++;
			continue;
		}
		else
			break;

		val *= ( bHex ? 0x10 : 10 );
		val += ch;
		pszStr ++;
	}
	return val;
}

int64 ahextoi64( lpctstr pszStr ) // Convert hex string to int64
{
	if ( pszStr == NULL )
		return 0;
	bool bHex = false;

	GETNONWHITESPACE(pszStr);

	if ( *pszStr == '0' )
	{
		if (*++pszStr != '.')
			bHex = true;
		pszStr--;
	}

	int64 val = 0;
	for (;;)
	{
		tchar ch = static_cast<tchar>(toupper(*pszStr));
		if ( IsDigit(ch) )
			ch -= '0';
		else if ( bHex && ( ch >= 'A' ) && ( ch <= 'F' ))
		{
			ch -= 'A' - 10;
		}
		else if ( !bHex && ( ch == '.' ) )
		{
			pszStr++;
			continue;
		}
		else
			break;

		val *= ( bHex ? 0x10 : 10 );
		val += ch;
		pszStr ++;
	}
	return val;
}

llong power(llong base, llong level)
{
	double rc = pow((double)base, (double)level);
	return (llong)rc;
}

bool IsCharNumeric( char & Test )
{
	if ( !Test )
		return false;

	if ( IsDigit( Test ) )
		return true;
	if ( tolower(Test) >= 'a' && tolower(Test) <= 'f' )
		return true;

	return false;
}

bool IsStrEmpty( lpctstr pszTest )
{
	if ( !pszTest || !*pszTest ) return true;

	do
	{
		if ( !IsSpace(*pszTest) ) return false;
	}
	while ( *(++pszTest) );
	return true;
}

bool IsStrNumericDec( lpctstr pszTest )
{
	if ( !pszTest || !*pszTest ) return false;

	do
	{
		if ( !IsDigit(*pszTest) ) return false;
	}
	while ( *(++pszTest) );

	return true;
}


bool IsStrNumeric( lpctstr pszTest )
{
	if ( !pszTest || !*pszTest )
		return false;
	bool	fHex	= false;
	if ( pszTest[0] == '0' )
		fHex	= true;

	do
	{
		if ( IsDigit( *pszTest ) )
			continue;
		if ( fHex && tolower(*pszTest) >= 'a' && tolower(*pszTest) <= 'f' )
			continue;
		return false;
	}
	while ( *(++pszTest) );
	return true;
}

bool IsSimpleNumberString( lpctstr pszTest )
{
	// is this a string or a simple numeric expression ?
	// string = 1 2 3, sdf, sdf sdf sdf, 123d, 123 d,
	// number = 1.0+-\*~|&!%^()2, 0aed, 123

	bool fMathSep			= true;	// last non whitespace was a math sep.
	bool fHextDigitStart	= false;
	bool fWhiteSpace		= false;

	for ( ; ; pszTest++ )
	{
		tchar ch = *pszTest;
		if ( ! ch )
		{
			return true;
		}
		if (( ch >= 'A' && ch <= 'F') || ( ch >= 'a' && ch <= 'f' ))	// isxdigit
		{
			if ( ! fHextDigitStart )
				return false;
			fWhiteSpace = false;
			fMathSep = false;
			continue;
		}
		if ( IsSpace( ch ) )
		{
			fHextDigitStart = false;
			fWhiteSpace = true;
			continue;
		}
		if ( IsDigit( ch ) )
		{
			if ( fWhiteSpace && ! fMathSep )
				return false;
			if ( ch == '0' )
			{
				fHextDigitStart = true;
			}
			fWhiteSpace = false;
			fMathSep = false;
			continue;
		}
		if ( ch == '/' && pszTest[1] != '/' )
			fMathSep	= true;
		else
			fMathSep = strchr("+-\\*~|&!%^()", ch ) ? true : false ;

		if ( ! fMathSep )
		{
			return false;
		}
		fHextDigitStart = false;
		fWhiteSpace = false;
	}
}

static size_t GetIdentifierString( tchar * szTag, lpctstr pszArgs )
{
	// Copy the identifier (valid char set) out to this buffer.
	size_t i = 0;
	for ( ; pszArgs[i]; i++ )
	{
		if ( ! _ISCSYM(pszArgs[i]))
			break;
		if ( i >= EXPRESSION_MAX_KEY_LEN )
			return 0;
		szTag[i] = pszArgs[i];
	}

	szTag[i] = '\0';
	return i;
}

bool IsValidDef( lpctstr pszTest )
{
	CVarDefCont * pVarBase = g_Exp.m_VarDefs.CheckParseKey( pszTest );
	if ( pVarBase == NULL )
	{
		//check VAR.X also
		pVarBase = g_Exp.m_VarGlobals.CheckParseKey( pszTest );
		if ( pVarBase == NULL )
			return false;
	}
	return true;
}

bool IsValidGameObjDef( lpctstr pszTest )
{
	if (!IsSimpleNumberString(pszTest))
	{
		CVarDefCont * pVarBase = g_Exp.m_VarDefs.CheckParseKey( pszTest );
		if ( pVarBase == NULL )
			return false;
		tchar ch = *pVarBase->GetValStr();
		if (( ! ch ) || ( ch == '<'))
			return false;

		RESOURCE_ID rid = g_Cfg.ResourceGetID( RES_QTY, pszTest);
		if (( rid.GetResType() != RES_CHARDEF ) && ( rid.GetResType() != RES_ITEMDEF ) && ( rid.GetResType() != RES_SPAWN ) && ( rid.GetResType() != RES_TEMPLATE ))
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
// -Calculus

int Calc_GetLog2( uint iVal )
{
	// This is really log2 + 1
	int i = 0;
	for ( ; iVal ; i++ )
	{
		ASSERT( i < 32 );
		iVal >>= 1 ;
	}
	return i;
}

int32 Calc_GetRandVal( int32 iQty )
{
	if ( iQty <= 0 )
		return 0;
	if ( iQty >= INT32_MAX )
		return ( MulDivLL(CSRand::genRandInt32(0, iQty), iQty, INT32_MAX ) );
	return CSRand::genRandInt32(0, iQty);
}

int32 Calc_GetRandVal2( int32 iMin, int32 iMax )
{
	if ( iMin > iMax )
	{
		int tmp = iMin;
		iMin = iMax;
		iMax = tmp;
	}
	return CSRand::genRandInt32(iMin, iMax);
}

int64 Calc_GetRandLLVal( int64 iQty )
{
	if ( iQty <= 0 )
		return 0;
	if ( iQty >= INT64_MAX )
		return ( MulDivLL(CSRand::genRandInt64(0, iQty), (uint64) iQty, INT64_MAX ) );
	return CSRand::genRandInt64(0, iQty);
}

int64 Calc_GetRandLLVal2( int64 iMin, int64 iMax )
{
	if ( iMin > iMax )
	{
		int64 tmp = iMin;
		iMin = iMax;
		iMax = tmp;
	}
	return CSRand::genRandInt64(iMin, iMax);
}

int Calc_GetBellCurve( int iValDiff, int iVariance )
{
	// Produce a log curve.
	//
	// 50+
	//	 |
	//	 |
	//	 |
	// 25|  +
	//	 |
	//	 |	   +
	//	 |		  +
	//	0 --+--+--+--+------
	//    iVar				iValDiff
	//
	// ARGS:
	//  iValDiff = Given a value relative to 0
	//		0 = 50.0% chance.
	//  iVariance = the 25.0% point of the bell curve
	// RETURN:
	//  (0-100.0) % chance at this iValDiff.
	//  Chance gets smaller as Diff gets bigger.
	// EXAMPLE:
	//  if ( iValDiff == iVariance ) return( 250 )
	//  if ( iValDiff == 0 ) return( 500 );
	//

	if ( iVariance <= 0 )	// this really should not happen but just in case.
		return 500;
	if ( iValDiff < 0 ) iValDiff = -iValDiff;

	int iChance = 500;
	while ( iValDiff > iVariance && iChance )
	{
		iValDiff -= iVariance;
		iChance /= 2;	// chance is halved for each Variance period.
	}

	return ( iChance - MulDivLL( iChance/2, iValDiff, iVariance ) );
}

int Calc_GetSCurve( int iValDiff, int iVariance )
{
	// ARGS:
	//   iValDiff = Difference between our skill level and difficulty.
	//		positive = high chance, negative = lower chance
	//		0 = 50.0% chance.
	//   iVariance = the 25.0% difference point of the bell curve
	// RETURN:
	//	 what is the (0-100.0)% chance of success = 0-1000
	// NOTE:
	//   Chance of skill gain is inverse to chance of success.
	//
	int iChance = Calc_GetBellCurve( iValDiff, iVariance );
	if ( iValDiff > 0 )
		return ( 1000 - iChance );
	return iChance;
}

CExpression::CExpression()
{
}

CExpression::~CExpression()
{
}

llong CExpression::GetSingle( lpctstr & pszArgs )
{
	ADDTOCALLSTACK("CExpression::GetSingle");
	// Parse just a single expression without any operators or ranges.
	ASSERT(pszArgs);
	GETNONWHITESPACE( pszArgs );

	lpctstr orig = pszArgs;
	if (pszArgs[0]=='.')
		pszArgs++;

	if ( pszArgs[0] == '0' )	// leading '0' = hex value.
	{
		// A hex value.
		if ( pszArgs[1] == '.' )	// leading 0. means it really is decimal.
		{
			pszArgs += 2;
			goto try_dec;
		}

		lpctstr pStart = pszArgs;
		ullong val = 0;
		for (;;)
		{
			tchar ch = *pszArgs;
			if ( IsDigit(ch) )
				ch -= '0';
			else
			{
				ch = static_cast<tchar>(tolower(ch));
				if ( ch > 'f' || ch <'a' )
				{
					if ( ch == '.' && pStart[0] != '0' )	// ok i'm confused. it must be decimal.
					{
						pszArgs = pStart;
						goto try_dec;
					}
					break;
				}
				ch -= 'a' - 10;
			}
			val *= 0x10;
			val += ch;
			pszArgs ++;
		}
		return (llong)val;
	}
	else if ( pszArgs[0] == '.' || IsDigit(pszArgs[0]) )
	{
		// A decminal number
try_dec:
		llong iVal = 0;
		for ( ; ; pszArgs++ )
		{
			if ( *pszArgs == '.' )
				continue;	// just skip this.
			if ( ! IsDigit(*pszArgs) )
				break;
			iVal *= 10;
			iVal += *pszArgs - '0';
		}
		return iVal;
	}
	else if ( ! _ISCSYMF(pszArgs[0]) )
	{
	#pragma region maths
		// some sort of math op ?

		switch ( pszArgs[0] )
		{
		case '{':
			pszArgs ++;
			return GetRange( pszArgs );
		case '[':
		case '(': // Parse out a sub expression.
			pszArgs ++;
			return GetVal( pszArgs );
		case '+':
			pszArgs++;
			break;
		case '-':
			pszArgs++;
			return -GetSingle( pszArgs );
		case '~':	// Bitwise not.
			pszArgs++;
			return ~GetSingle( pszArgs );
		case '!':	// boolean not.
			pszArgs++;
			if ( pszArgs[0] == '=' )  // odd condition such as (!=x) which is always true of course.
			{
				pszArgs++;		// so just skip it. and compare it to 0
				return GetSingle( pszArgs );
			}
			return !GetSingle( pszArgs );
		case ';':	// seperate field.
		case ',':	// seperate field.
		case '\0':
			return 0;
		}
#pragma endregion maths
	}
	else
	#pragma region intrinsics
	{
		// Symbol or intrinsinc function ?

		INTRINSIC_TYPE iIntrinsic = (INTRINSIC_TYPE) FindTableHeadSorted( pszArgs, sm_IntrinsicFunctions, CountOf(sm_IntrinsicFunctions)-1 );
		if ( iIntrinsic >= 0 )
		{
			size_t iLen = strlen(sm_IntrinsicFunctions[iIntrinsic]);
			if ( pszArgs[iLen] == '(' )
			{
				pszArgs += (iLen + 1);
				tchar * pszArgsNext;
				Str_Parse( const_cast<tchar*>(pszArgs), &(pszArgsNext), ")" );

				tchar * ppCmd[5];
				llong iResult;
				size_t iCount = 0;

				switch ( iIntrinsic )
				{
					case INTRINSIC_ID:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = RES_GET_INDEX( GetVal(pszArgs) ); // RES_GET_INDEX
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_LOGARITHM:
					{
						iCount = 0;
						iResult = 0;

						if ( pszArgs && *pszArgs )
						{
							llong iArgument = GetVal(pszArgs);
							if ( iArgument <= 0 )
							{
								DEBUG_ERR(( "Exp_GetVal: (x)Log(%" PRId64 ") is %s\n", iArgument, (!iArgument) ? "infinite" : "undefined" ));
							}
							else
							{
								iCount = 1;

								if ( strchr(pszArgs, ',') )
								{
									iCount++; SKIP_ARGSEP(pszArgs);
									if ( !strcmpi(pszArgs, "e") )
									{
										iResult = (llong)log( (double)iArgument );
									}
									else if ( !strcmpi(pszArgs, "pi") )
									{
										iResult = (llong)(log( (double)iArgument ) / log( M_PI ) );
									}
									else
									{
										llong iBase = GetVal(pszArgs);
										if ( iBase <= 0 )
										{
											DEBUG_ERR(( "Exp_GetVal: (%" PRId64 ")Log(%" PRId64 ") is %s\n", iBase, iArgument, (!iBase ? "infinite" : "undefined") ));
											iCount = 0;
										}
										else
											iResult = (llong)(log( (double)iArgument ) / log( (double)iBase ));
									}
								}
								else
									iResult = (llong)log10( (double)iArgument );
							}
						}

					} break;

					case INTRINSIC_NAPIERPOW:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)exp( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_SQRT:
					{
						iCount = 0;
						iResult = 0;

						if ( pszArgs && *pszArgs )
						{
							llong iTosquare = GetVal(pszArgs);

							if (iTosquare >= 0)
							{
								iCount++;
								iResult = (llong)sqrt( (double)iTosquare );
							}
							else
								DEBUG_ERR(( "Exp_GetVal: Sqrt of negative number (%" PRId64 ") is impossible\n", iTosquare ));
						}

					} break;

					case INTRINSIC_SIN:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)sin( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_ARCSIN:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)asin( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_COS:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)cos( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_ARCCOS:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)acos( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_TAN:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)tan( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_ARCTAN:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = (llong)atan( (double)GetVal( pszArgs ) );
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}

					} break;

					case INTRINSIC_StrIndexOf:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 3, "," );
						if ( iCount < 2 )
							iResult = -1;
						else
							iResult = Str_IndexOf( ppCmd[0] , ppCmd[1] , (iCount==3)?(int)GetVal(ppCmd[2]):0 );
					} break;

					case INTRINSIC_STRMATCH:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount < 2 )
							iResult = 0;
						else
							iResult = (Str_Match( ppCmd[0], ppCmd[1] ) == MATCH_VALID ) ? 1 : 0;
					} break;

					case INTRINSIC_STRREGEX:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount < 2 )
							iResult = 0;
						else
						{
							tchar * tLastError = Str_GetTemp();
							iResult = Str_RegExMatch( ppCmd[0], ppCmd[1], tLastError );
							if ( iResult == -1 )
							{
								DEBUG_ERR(( "STRREGEX bad function usage. Error: %s\n", tLastError ));
							}
						}
					} break;

					case INTRINSIC_RANDBELL:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount < 2 )
							iResult = 0;
						else
							iResult = Calc_GetBellCurve( (int)GetVal( ppCmd[0] ), (int)GetVal( ppCmd[1] ) );
					} break;

					case INTRINSIC_STRASCII:
					{
						if ( pszArgs && *pszArgs )
						{
							iCount = 1;
							iResult = pszArgs[0];
						}
						else
						{
							iCount = 0;
							iResult = 0;
						}
					} break;

					case INTRINSIC_RAND:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount <= 0 )
							iResult = 0;
						else
						{
							int64 val1 = GetVal( ppCmd[0] );
							if ( iCount == 2 )
							{
								int64 val2 = GetVal( ppCmd[1] );
								iResult = Calc_GetRandLLVal2( val1, val2 );
							}
							else
								iResult = Calc_GetRandLLVal(val1);
						}
					} break;

					case INTRINSIC_STRCMP:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount < 2 )
							iResult = 1;
						else
							iResult = strcmp(ppCmd[0], ppCmd[1]);
					} break;

					case INTRINSIC_STRCMPI:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 2, "," );
						if ( iCount < 2 )
							iResult = 1;
						else
							iResult = strcmpi(ppCmd[0], ppCmd[1]);
					} break;

					case INTRINSIC_STRLEN:
					{
						iCount = 1;
						iResult = strlen(pszArgs);
					} break;

					case INTRINSIC_ISOBSCENE:
					{
						iCount = 1;
						iResult = g_Cfg.IsObscene( pszArgs );
					} break;
					case INTRINSIC_ISNUMBER:
					{
						iCount = 1;
						{
							char z[64];
							LTOA(atol(pszArgs), z, 10);
							iResult = strcmp(pszArgs, z) ? 0 : 1;
						}
					} break;

					case INTRINSIC_QVAL:
					{
						iCount = Str_ParseCmds( const_cast<tchar*>(pszArgs), ppCmd, 5, "," );
						if ( iCount < 3 )
							iResult = 0;
						else
						{
							llong a1 = GetSingle(ppCmd[0]);
							llong a2 = GetSingle(ppCmd[1]);
							if ( a1 < a2 )			iResult = GetSingle(ppCmd[2]);
							else if ( a1 == a2 )	iResult = ( iCount < 4 ) ? 0 : GetSingle(ppCmd[3]);
							else					iResult = ( iCount < 5 ) ? 0 : GetSingle(ppCmd[4]);
						}
					} break;

					case INTRINSIC_ABS:
					{
						iCount = 1;
						iResult = llabs(GetVal(pszArgs));
					} break;

					default:
						iCount = 0;
						iResult = 0;
						break;
				}

				pszArgs = pszArgsNext;

				if ( !iCount )
				{
					DEBUG_ERR(( "Bad intrinsic function usage: Missing arguments\n" ));
					return 0;
				}
				else
					return iResult;
			}
		}

		// Must be a symbol of some sort ?
		llong lVal;
		if ( m_VarGlobals.GetParseVal( pszArgs, &lVal ) )
			return lVal;
		if ( m_VarDefs.GetParseVal( pszArgs, &lVal ) )
			return lVal;
	}
#pragma endregion intrinsics

	// hard end ! Error of some sort.
	tchar szTag[ EXPRESSION_MAX_KEY_LEN ];
	size_t i = GetIdentifierString( szTag, pszArgs );
	pszArgs += i;	// skip it.
	if (strlen(orig)> 1)
		DEBUG_ERR(("Undefined symbol '%s' ['%s']\n", szTag, orig));
	else
		DEBUG_ERR(("Undefined symbol '%s'\n", szTag));
	return 0;
}

llong CExpression::GetValMath( llong lVal, lpctstr & pExpr )
{
	ADDTOCALLSTACK("CExpression::GetValMath");
	GETNONWHITESPACE(pExpr);

	// Look for math type operator.
	switch ( pExpr[0] )
	{
		case '\0':
			break;
		case ')':  // expression end markers.
		case '}':
		case ']':
			pExpr++;	// consume this.
			break;
		case '+':
			pExpr++;
			lVal += GetVal( pExpr );
			break;
		case '-':
			pExpr++;
			lVal -= GetVal( pExpr );
			break;
		case '*':
			pExpr++;
			lVal *= GetVal( pExpr );
			break;
		case '|':
			pExpr++;
			if ( pExpr[0] == '|' )	// boolean ?
			{
				pExpr++;
				lVal = ( GetVal( pExpr ) || lVal );
			}
			else	// bitwise
				lVal |= GetVal( pExpr );
			break;
		case '&':
			pExpr++;
			if ( pExpr[0] == '&' )	// boolean ?
			{
				pExpr++;
				lVal = ( GetVal( pExpr ) && lVal );	// tricky stuff here. logical ops must come first or possibly not get processed.
			}
			else	// bitwise
				lVal &= GetVal( pExpr );
			break;
		case '/':
			pExpr++;
			{
				llong iVal = GetVal( pExpr );
				if ( ! iVal )
				{
					DEBUG_ERR(( "Exp_GetVal: Divide by 0\n" ));
					break;
				}
				lVal /= iVal;
			}
			break;
		case '%':
			pExpr++;
			{
				llong iVal = GetVal( pExpr );
				if ( ! iVal )
				{
					DEBUG_ERR(( "Exp_GetVal: Divide by 0\n" ));
					break;
				}
				lVal %= iVal;
			}
			break;
		case '^':
			pExpr ++;
			lVal ^= GetVal(pExpr);
			break;
		case '>': // boolean
			pExpr++;
			if ( pExpr[0] == '=' )	// boolean ?
			{
				pExpr++;
				lVal = ( lVal >= GetVal( pExpr ) );
			}
			else if ( pExpr[0] == '>' )	// shift
			{
				pExpr++;
				lVal >>= GetVal( pExpr );
			}
			else
				lVal = ( lVal > GetVal( pExpr ) );
			break;
		case '<': // boolean
			pExpr++;
			if ( pExpr[0] == '=' )	// boolean ?
			{
				pExpr++;
				lVal = ( lVal <= GetVal( pExpr ) );
			}
			else if ( pExpr[0] == '<' )	// shift
			{
				pExpr++;
				lVal <<= GetVal( pExpr );
			}
			else
				lVal = ( lVal < GetVal( pExpr ) );
			break;
		case '!':
			pExpr ++;
			if ( pExpr[0] != '=' )
				break; // boolean ! is handled as a single expresion.
			pExpr ++;
			lVal = ( lVal != GetVal( pExpr ) );
			break;
		case '=': // boolean
			while ( pExpr[0] == '=' )
				pExpr ++;
			lVal = ( lVal == GetVal( pExpr ) );
			break;
		case '@':
			pExpr++;
			{
				llong iVal = GetVal( pExpr );
				if ( (lVal == 0) && (iVal < 0) )
				{
					DEBUG_ERR(( "Exp_GetVal: Power of zero with negative exponent is undefined\n" ));
					break;
				}
				lVal = power(lVal, iVal);
			}
			break;
	}

	return lVal;
}

int g_getval_reentrant_check = 0;

llong CExpression::GetVal( lpctstr & pExpr )
{
	ADDTOCALLSTACK("CExpression::GetVal");
	// Get a value (default decimal) that could also be an expression.
	// This does not parse beyond a comma !
	//
	// These are all the type of expressions and defines we'll see:
	//
	//	all_skin_colors					// simple DEF value
	//	7933 						// simple decimal
	//	-100.0						// simple negative decimal
	//	.5						// simple decimal
	//	0.5						// simple decimal
	//	073a 						// hex value (leading zero and no .)
	//
	//	0 -1						// Subtraction. has a space separator. (Yes I know I hate this)
	//	{0-1}						// hyphenated simple range (GET RID OF THIS!)
	//		complex ranges must be in {}
	//	{ 3 6}						// simple range
	//	{ 400 1 401 1 } 				// weighted values (2nd val = 1)
	//	{ 1102 1148 1 }					// weighted range (3rd val < 10)
	//	{ animal_colors 1 no_colors 1 } 		// weighted range
	//	{ red_colors 1 {34 39} 1 }			// same (red_colors expands to a range)

	if ( pExpr == NULL )
		return 0;

	GETNONWHITESPACE( pExpr );

	g_getval_reentrant_check++;
	if ( g_getval_reentrant_check > 128 )
	{
		DEBUG_WARN(( "Deadlock detected while parsing '%s'. Fix the error in your scripts.\n", pExpr ));
		g_getval_reentrant_check--;
		return 0;
	}
	llong lVal = GetValMath(GetSingle(pExpr), pExpr);
	g_getval_reentrant_check--;

	return lVal;
}

int CExpression::GetRangeVals(lpctstr & pExpr, int64 * piVals, int iMaxQty)
{
	ADDTOCALLSTACK("CExpression::GetRangeVals");
	// Get a list of values.
	if ( pExpr == NULL )
		return 0;

	ASSERT(piVals);

	int iQty = 0;
	for (;;)
	{
		if ( !pExpr[0] ) break;
		if ( pExpr[0] == ';' )
			break;	// seperate field.
		if ( pExpr[0] == ',' )
			pExpr++;

		piVals[iQty] = GetSingle( pExpr );
		if ( ++iQty >= iMaxQty )
			break;
		if ( pExpr[0] == '-' && iQty == 1 )	// range separator. (if directly after, I know this is sort of strange)
		{
			pExpr++;	// ??? This is stupid. get rid of this and clean up it's use in the scripts.
			continue;
		}

		GETNONWHITESPACE(pExpr);

		// Look for math type operator.
		switch ( pExpr[0] )
		{
			case ')':  // expression end markers.
			case '}':
			case ']':
				pExpr++;	// consume this and end.
				return iQty;

			case '+':
			case '*':
			case '/':
			case '%':
			// case '^':
			case '<':
			case '>':
			case '|':
			case '&':
				piVals[iQty-1] = GetValMath( piVals[iQty-1], pExpr );
				break;
		}
	}

	return iQty;
}

int64 CExpression::GetRange(lpctstr & pExpr)
{
	ADDTOCALLSTACK("CExpression::GetRange");
	int64 lVals[256];		// Maximum elements in a list

	int iQty = GetRangeVals( pExpr, lVals, CountOf(lVals) );

	if (iQty == 0)
		return 0;
	if (iQty == 1) // It's just a simple value
		return lVals[0];
	if (iQty == 2) // It's just a simple range....pick one in range at random
		return Calc_GetRandLLVal2( minimum(lVals[0],lVals[1]), maximum(lVals[0],lVals[1]) );

	// I guess it's weighted values
	// First get the total of the weights

	int64 iTotalWeight = 0;
	int i = 1;
	for ( ; i < iQty; i+=2 )
	{
		if ( ! lVals[i] )	// having a weight of 0 is very strange !
			DEBUG_ERR(( "Weight of 0 in random set?\n" ));	// the whole table should really just be invalid here !
		iTotalWeight += lVals[i];
	}

	// Now roll the dice to see what value to pick
	iTotalWeight = Calc_GetRandLLVal(iTotalWeight) + 1;
	// Now loop to that value
	i = 1;
	for ( ; i<iQty; i+=2 )
	{
		iTotalWeight -= lVals[i];
		if ( iTotalWeight <= 0 )
			break;
	}

	return lVals[i-1];
}
