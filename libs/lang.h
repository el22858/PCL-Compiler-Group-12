#pragma once

#if !defined(BITS)
#	error no integer bit width provided (-DBITS=)
#endif

#if defined(PCL)
/* I/O */
#	define READ_CHAR     readChar
#	define READ_STRING   readString
#	define READ_BYTE     readByte
#	define READ_INTEGER  readInteger
#	define READ_BOOLEAN  readBoolean
#	define WRITE_CHAR    writeChar
#	define WRITE_STRING  writeString
#	define WRITE_BYTE    writeByte
#	define WRITE_INTEGER writeInteger
#	define WRITE_BOOLEAN writeBoolean
/* math */
#	define ABS    abs
#	define FABS   fabs
#	define SQRT   sqrt
#	define SIN    sin
#	define COS    cos
#	define TAN    tan
#	define ARCTAN arctan
#	define EXP    exp
#	define LN     ln
#	define PI     pi
/* type conversion */
#	define TRUNC trunc
#	define ROUND round
#	define ORD   ord
#	define CHR   chr
#elif defined(DANA)
/* I/O */
#	define READ_CHAR     readChar
#	define READ_STRING   readString
#	define READ_BYTE     readByte
#	define READ_INTEGER  readInteger
#	define WRITE_CHAR    writeChar
#	define WRITE_STRING  writeString
#	define WRITE_BYTE    writeByte
#	define WRITE_INTEGER writeInteger
/* type conversion */
#	define EXTEND extend
#	define SHRINK shrink
/* string manipulation */
#	define STRLEN strlen
#	define STRCMP strcmp
#	define STRCPY strcpy
#	define STRCAT strcat
#else
#	error no supported language selected (-D[LANG])
#endif
