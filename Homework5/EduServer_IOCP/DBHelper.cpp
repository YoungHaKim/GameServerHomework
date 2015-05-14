#include "stdafx.h"
#include "Exception.h"
#include "ThreadLocal.h"
#include "DBHelper.h"

//done: DbHelper의 static 멤버변수 초기화
SQLHENV DbHelper::mSqlHenv = nullptr;
SQL_CONN* DbHelper::mSqlConnPool = nullptr; ///< 워커스레드수만큼
int DbHelper::mDbWorkerThreadCount = 0;

DbHelper::DbHelper()
{
	CRASH_ASSERT(mSqlConnPool[LWorkerThreadId].mUsingNow == false);

	mCurrentSqlHstmt = mSqlConnPool[LWorkerThreadId].mSqlHstmt;
	mCurrentResultCol = 1;
	mCurrentBindParam = 1;
	CRASH_ASSERT(mCurrentSqlHstmt != nullptr);

	mSqlConnPool[LWorkerThreadId].mUsingNow = true;
}
/*

Unbinding Columns
To unbind a single column, an application calls SQLBindCol with ColumnNumber set to the number of that column and TargetValuePtr set to a null pointer. If ColumnNumber refers to an unbound column, SQLBindCol still returns SQL_SUCCESS.
To unbind all columns, an application calls SQLFreeStmt with fOption set to SQL_UNBIND. This can also be accomplished by setting the SQL_DESC_COUNT field of the ARD to zero.
*/

DbHelper::~DbHelper()
{
	//done: SQLFreeStmt를 이용하여 현재 SQLHSTMT 해제(unbind, 파라미터리셋, close 순서로)
	
	/*
	SQLFreeStmt stops processing associated with a specific statement, closes any open cursors associated with the statement, 
	discards pending results, or, optionally, frees all resources associated with the statement handle.

	SQL_ CLOSE: Closes the cursor associated with StatementHandle (if one was defined) and discards all pending results. 
	The application can reopen this cursor later by executing a SELECT statement again with the same or different parameter values. 
	If no cursor is open, this option has no effect for the application. SQLCloseCursor can also be called to close a cursor. For more information, see Closing the Cursor.

	SQL_DROP: This option is deprecated. A call to SQLFreeStmt with an Option of SQL_DROP is mapped in the Driver Manager to SQLFreeHandle.

	SQL_UNBIND: Sets the SQL_DESC_COUNT field of the ARD to 0, releasing all column buffers bound by SQLBindCol for the given StatementHandle. 
	This does not unbind the bookmark column; to do that, the SQL_DESC_DATA_PTR field of the ARD for the bookmark column is set to NULL. 
	Notice that if this operation is performed on an explicitly allocated descriptor that is shared by more than one statement, 
	the operation will affect the bindings of all statements that share the descriptor. For more information, see Overview of Retrieving Results (Basic).

	SQL_RESET_PARAMS: Sets the SQL_DESC_COUNT field of the APD to 0, releasing all parameter buffers set by SQLBindParameter for the given StatementHandle. 
	If this operation is performed on an explicitly allocated descriptor that is shared by more than one statement, 
	this operation will affect the bindings of all the statements that share the descriptor. For more information, see Binding Parameters.

	*/
	
	SQLFreeStmt(mCurrentSqlHstmt, SQL_UNBIND);
	SQLFreeStmt(mCurrentSqlHstmt, SQL_RESET_PARAMS);
	SQLFreeStmt(mCurrentSqlHstmt, SQL_CLOSE);
	
	//SQLDisconnect(mSqlConnPool[LWorkerThreadId].mSqlHdbc);
	
	mSqlConnPool[LWorkerThreadId].mUsingNow = false;
}

bool DbHelper::Initialize(const wchar_t* connInfoStr, int workerThreadCount)
{
	//done: mSqlConnPool, mDbWorkerThreadCount를 워커스레스 수에 맞추어 초기화
	mDbWorkerThreadCount = workerThreadCount;
	mSqlConnPool = new SQL_CONN[mDbWorkerThreadCount];

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mSqlHenv))
	{
		printf_s("DbHelper Initialize SQLAllocHandle failed\n");
		return false;
	}

	if (SQL_SUCCESS != SQLSetEnvAttr(mSqlHenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER))
	{
		printf_s("DbHelper Initialize SQLSetEnvAttr failed\n");
		return false;
	}
		

	/// 스레드별로 SQL connection을 풀링하는 방식. 즉, 스레드마다 SQL서버로의 연결을 갖는다.
	for (int i = 0; i < mDbWorkerThreadCount; ++i)
	{
		//done: SQLAllocHandle을 이용하여 SQL_CONN의 mSqlHdbc 핸들 사용가능하도록 처리
		if (SQLAllocHandle(SQL_HANDLE_DBC, mSqlHenv, &mSqlConnPool[i].mSqlHdbc) != SQL_SUCCESS)
		{
			printf_s("SQLAllocHandle failed\n");
			return false;
		}

		SQLSMALLINT resultLen = 0;

		/*
		SQLRETURN SQLDriverConnect(
		SQLHDBC         ConnectionHandle,
		SQLHWND         WindowHandle,
		SQLCHAR *       InConnectionString,
		SQLSMALLINT     StringLength1,
		SQLCHAR *       OutConnectionString,
		SQLSMALLINT     BufferLength,
		SQLSMALLINT *   StringLength2Ptr,
		SQLUSMALLINT    DriverCompletion);

		ConnectionHandle
		[Input] Connection handle.
		WindowHandle
		[Input] Window handle. The application can pass the handle of the parent window, if applicable, or a null pointer if either the window handle is not applicable or SQLDriverConnect will not present any dialog boxes.
		InConnectionString
		[Input] A full connection string (see the syntax in "Comments"), a partial connection string, or an empty string.
		StringLength1
		[Input] Length of *InConnectionString, in characters if the string is Unicode, or bytes if string is ANSI or DBCS.
		OutConnectionString
		[Output] Pointer to a buffer for the completed connection string. Upon successful connection to the target data source, this buffer contains the completed connection string. Applications should allocate at least 1,024 characters for this buffer.
		If OutConnectionString is NULL, StringLength2Ptr will still return the total number of characters (excluding the null-termination character for character data) available to return in the buffer pointed to by OutConnectionString.
		BufferLength
		[Input] Length of the *OutConnectionString buffer, in characters.
		StringLength2Ptr
		[Output] Pointer to a buffer in which to return the total number of characters (excluding the null-termination character) available to return in *OutConnectionString. If the number of characters available to return is greater than or equal to BufferLength, the completed connection string in *OutConnectionString is truncated to BufferLength minus the length of a null-termination character.
		DriverCompletion
		[Input] Flag that indicates whether the Driver Manager or driver must prompt for more connection information:
		SQL_DRIVER_PROMPT, SQL_DRIVER_COMPLETE, SQL_DRIVER_COMPLETE_REQUIRED, or SQL_DRIVER_NOPROMPT.
		(For additional information, see "Comments.")
		*/
		
		SQLWCHAR retconstring[1024];

		//done: SQLDriverConnect를 이용하여 SQL서버에 연결하고 그 핸들을 SQL_CONN의 mSqlHdbc에 할당
		SQLRETURN ret = SQLDriverConnect(mSqlConnPool[i].mSqlHdbc,
			NULL,
			(SQLWCHAR*)L"DRIVER={SQL Server};SERVER=PETER\\SQLEXPRESS; DATABASE=dbo; UID=sa; PWD=satest",
			SQL_NTS,
			retconstring,
			1024,
			NULL,
			SQL_DRIVER_NOPROMPT
			);

		/*SQLRETURN ret = SQLDriverConnect(&mSqlConnPool[i].mSqlHdbc,
			NULL,
			(SQLWCHAR*)connInfoStr,
			(SQLSMALLINT)wcslen(connInfoStr),
			NULL,
			0,
			&resultLen,
			SQL_DRIVER_NOPROMPT
			);*/

		if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
		{
			SQLWCHAR sqlState[1024] = { 0, } ;
			SQLINTEGER nativeError = 0;
			SQLWCHAR msgText[1024] = { 0, } ;
			SQLSMALLINT textLen = 0 ;

			SQLGetDiagRec(SQL_HANDLE_DBC, mSqlConnPool[i].mSqlHdbc, 1, sqlState, &nativeError, msgText, 1024, &textLen);

			wprintf_s(L"DbHelper Initialize SQLDriverConnect failed: %s \n", msgText);

			return false;
		}

		//done: SQLAllocHandle를 이용하여 SQL_CONN의 mSqlHstmt 핸들 사용가능하도록 처리
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, mSqlConnPool[i].mSqlHdbc, &mSqlConnPool[i].mSqlHstmt))
		{
			wprintf_s(L"SQLAllocHandle connection to statement handle failed \n");

			return false;
		}
	}

	return true;
}


void DbHelper::Finalize()
{
	for (int i = 0; i < mDbWorkerThreadCount; ++i)
	{
		SQL_CONN* currConn = &mSqlConnPool[i];
		if (currConn->mSqlHstmt)
			SQLFreeHandle(SQL_HANDLE_STMT, currConn->mSqlHstmt);

		if (currConn->mSqlHdbc)
			SQLFreeHandle(SQL_HANDLE_DBC, currConn->mSqlHdbc);
	}

	delete[] mSqlConnPool;


}

bool DbHelper::Execute(const wchar_t* sqlstmt)
{
	//done: mCurrentSqlHstmt핸들 사용하여 sqlstmt를 수행.  
	SQLRETURN ret = SQLExecDirect(mCurrentSqlHstmt, (SQLWCHAR*)sqlstmt, (SQLINTEGER)wcslen(sqlstmt));
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::FetchRow()
{
	//done: mCurrentSqlHstmt가 들고 있는 내용 fetch

	SQLRETURN ret = SQLFetch(mCurrentSqlHstmt);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		if (SQL_NO_DATA != ret)
		{
			PrintSqlStmtError();
		}
		
		return false;
	}

	return true;
}

/*
SQLRETURN SQLBindParameter(
SQLHSTMT        StatementHandle,		//[Input] Statement handle.
SQLUSMALLINT    ParameterNumber,		//[Input] Parameter number, ordered sequentially in increasing parameter order, starting at 1.
SQLSMALLINT     InputOutputType,		//[Input] The type of the parameter. For more information, see "InputOutputType Argument" in "Comments."
https://msdn.microsoft.com/query/dev12.query?appId=Dev12IDEF1&l=EN-US&k=k(sqlext%2FSQLBindParameter);k(SQLBindParameter);k(DevLang-C%2B%2B);k(TargetOS-Windows)&rd=true

SQLSMALLINT     ValueType,				//[Input] The C data type of the parameter. For more information, see "ValueType Argument" in "Comments."
https://msdn.microsoft.com/en-us/library/ms714556(v=vs.85).aspx

SQLSMALLINT     ParameterType,			//[Input] The SQL data type of the parameter. For more information, see "ParameterType Argument" in "Comments."
https://msdn.microsoft.com/en-us/library/ms710150(v=vs.85).aspx

SQLULEN         ColumnSize,				//[Input] The size of the column or expression of the corresponding parameter marker. For more information, see "ColumnSize Argument" in "Comments."
SQLSMALLINT     DecimalDigits,			//[Input] The decimal digits of the column or expression of the corresponding parameter marker. 
SQLPOINTER      ParameterValuePtr,		//[Deferred Input] A pointer to a buffer for the parameter's data. For more information, see "ParameterValuePtr Argument" in "Comments."
SQLLEN          BufferLength,			//[Input/Output] Length of the ParameterValuePtr buffer in bytes. 
SQLLEN *        StrLen_or_IndPtr);		//[Deferred Input] A pointer to a buffer for the parameter's length. 

An application calls SQLBindParameter to bind each parameter marker in an SQL statement. 
Bindings remain in effect until the application calls SQLBindParameter again, 
calls SQLFreeStmt with the SQL_RESET_PARAMS option, 
or calls SQLSetDescField to set the SQL_DESC_COUNT header field of the APD to 0.

*/

bool DbHelper::BindParamInt(int* param)
{
	//done: int형 파라미터 바인딩
	SQLRETURN ret = SQLBindParameter(
		mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 10, 0, param, 0, NULL);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamFloat(float* param)
{
	SQLRETURN ret = SQLBindParameter(mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT,
		SQL_C_FLOAT, SQL_REAL, 15, 0, param, 0, NULL);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamBool(bool* param)
{
	//done: bool형 파라미터 바인딩 - why not char type?
	SQLRETURN ret = SQLBindParameter(
		mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 3, 0, param, 0, NULL);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DbHelper::BindParamText(const wchar_t* text)
{

	//done: 유니코드 문자열 바인딩
	SQLRETURN ret = SQLBindParameter(
		mCurrentSqlHstmt, mCurrentBindParam++, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, wcslen(text), 0, (SQLPOINTER)text, 0, NULL);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}


void DbHelper::BindResultColumnInt(int* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_LONG, r, 4, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}
void DbHelper::BindResultColumnFloat(float* r)
{
	SQLLEN len = 0;
	
	//done: float형 결과 컬럼 바인딩
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_FLOAT, r, 4, &len);
	
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DbHelper::BindResultColumnBool(bool* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_TINYINT, r, 1, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}
void DbHelper::BindResultColumnText(wchar_t* text, size_t count)
{
	SQLLEN len = 0;
	//done: wchar_t*형 결과 컬럼 바인딩
	SQLRETURN ret = SQLBindCol(mCurrentSqlHstmt, mCurrentResultCol++, SQL_C_WCHAR, text, count, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}


void DbHelper::PrintSqlStmtError()
{
	SQLWCHAR sqlState[1024] = { 0, };
	SQLINTEGER nativeError = 0;
	SQLWCHAR msgText[1024] = { 0, };
	SQLSMALLINT textLen = 0;

	SQLGetDiagRec(SQL_HANDLE_STMT, mCurrentSqlHstmt, 1, sqlState, &nativeError, msgText, 1024, &textLen);

	wprintf_s(L"DbHelper SQL Statement Error: %ls \n", msgText);
}