/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef kGenericError
	#define kGenericError		-1
#endif

extern char	*gErrorMessage;


void SetErrorMessage(const char *theErrorMessage);
void SetErrorMessageAndAppendLongInt(const char *theErrorMessage,const long theLongInt);
void SetErrorMessageAndCStrAndLongInt(const char *theErrorMessage,const char * theCStr,const long theLongInt);
void SetErrorMessageAndCStr(const char *theErrorMessage,const char * theCStr);
void AppendCStrToErrorMessage(const char *theErrorMessage);
void AppendLongIntToErrorMessage(const long theLongInt);


char *GetErrorMessage(void);
OSErr GetErrorMessageInNewHandle(Handle *inoutHandle);
OSErr GetErrorMessageInExistingHandle(Handle inoutHandle);
OSErr AppendErrorMessageToHandle(Handle inoutHandle);


#ifdef __EXCEPTIONS_ENABLED__
	void ThrowErrorMessageException(void);
#endif



//	A bunch of evil macros that would be uneccessary if I were always using C++ !

#define SetErrorMessageAndBailIfNil(theArg,theMessage)								\
{																					\
	if (theArg == nil)																\
	{																				\
		SetErrorMessage(theMessage);												\
		errCode = kGenericError;													\
		goto EXITPOINT;																\
	}																				\
}


#define SetErrorMessageAndBail(theMessage)											\
{																					\
		SetErrorMessage(theMessage);												\
		errCode = kGenericError;													\
		goto EXITPOINT;																\
}


#define SetErrorMessageAndLongIntAndBail(theMessage,theLongInt)						\
{																					\
		SetErrorMessageAndAppendLongInt(theMessage,theLongInt);						\
		errCode = kGenericError;													\
		goto EXITPOINT;																\
}


#define SetErrorMessageAndLongIntAndBailIfError(theErrCode,theMessage,theLongInt)	\
{																					\
	if (theErrCode != noErr)														\
	{																				\
		SetErrorMessageAndAppendLongInt(theMessage,theLongInt);						\
		errCode = theErrCode;														\
		goto EXITPOINT;																\
	}																				\
}


#define SetErrorMessageCStrLongIntAndBailIfError(theErrCode,theMessage,theCStr,theLongInt)	\
{																					\
	if (theErrCode != noErr)														\
	{																				\
		SetErrorMessageAndCStrAndLongInt(theMessage,theCStr,theLongInt);			\
		errCode = theErrCode;														\
		goto EXITPOINT;																\
	}																				\
}


#define SetErrorMessageAndCStrAndBail(theMessage,theCStr)							\
{																					\
	SetErrorMessageAndCStr(theMessage,theCStr);										\
	errCode = kGenericError;														\
	goto EXITPOINT;																	\
}


#define SetErrorMessageAndBailIfError(theErrCode,theMessage)						\
{																					\
	if (theErrCode != noErr)														\
	{																				\
		SetErrorMessage(theMessage);												\
		errCode = theErrCode;														\
		goto EXITPOINT;																\
	}																				\
}


#define SetErrorMessageAndLongIntAndBailIfNil(theArg,theMessage,theLongInt)			\
{																					\
	if (theArg == nil)																\
	{																				\
		SetErrorMessageAndAppendLongInt(theMessage,theLongInt);						\
		errCode = kGenericError;													\
		goto EXITPOINT;																\
	}																				\
}


#define BailIfError(theErrCode)														\
{																					\
	if ((theErrCode) != noErr)														\
	{																				\
		goto EXITPOINT;																\
	}																				\
}


#define SetErrCodeAndBail(theErrCode)												\
{																					\
	errCode = theErrCode;															\
																					\
	goto EXITPOINT;																	\
}


#define SetErrorCodeAndMessageAndBail(theErrCode,theMessage)						\
{																					\
	SetErrorMessage(theMessage);													\
	errCode = theErrCode;															\
	goto EXITPOINT;																	\
}


#define BailNow()																	\
{																					\
	errCode = kGenericError;														\
	goto EXITPOINT;																	\
}


#ifdef __cplusplus
}
#endif
