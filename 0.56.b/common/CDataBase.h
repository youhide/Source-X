//
//	CDataBase
//		mySQL wrapper for easier data operations witheen in-game server
//
#ifndef CDATABASE_H
#define	CDATABASE_H

#include "../common/graycom.h"
#include <mysql.h>
#include <errmsg.h>	// mysql standart include
#include "../common/CScriptObj.h"
#include "../sphere/mutex.h"

#ifdef _WIN32
	#pragma comment(lib, "libmySQL")
#else
	#pragma comment(lib, "libmysqlclient")
#endif

#define	MIN_MYSQL_VERSION_ALLOW	40115

class CDataBase : public CScriptObj
{
public:
	static const char *m_sClassName;
	//	construction
	CDataBase();
	~CDataBase();
	bool Connect(const char *user, const char *password, const char *base = "", const char *host = "localhost");
	bool Connect();
	void Close();							//	close link with db

	//	select
	bool	query(const char *query, CVarDefMap & mapQueryResult);			//	proceeds the query for SELECT
	bool	__cdecl queryf(CVarDefMap & mapQueryResult, char *fmt, ...);
	bool	exec(const char *query);			//	executes query (pretty faster) for ALTER, UPDATE, INSERT, DELETE, ...
	bool	__cdecl execf(char *fmt, ...);

	//	set / get / info methods
	bool	isConnected();
	UINT	getLastId();					//	get last id generated by auto-increment field
	void	addQueryResult(CGString, CScriptTriggerArgs *);

public:
	bool OnTick();
	int FixWeirdness();

	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );

	LPCTSTR GetName() const
	{
		return "SQL_OBJ";
	}

public:
	CVarDefMap	m_QueryResult;
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];

private:
	typedef std::pair<CGString, CScriptTriggerArgs *> FunctionArgsPair_t;
	typedef std::queue<FunctionArgsPair_t> QueueFunction_t;

protected:
	bool	_bConnected;					//	are we online?
	MYSQL	*_myData;						//	mySQL link
	QueueFunction_t m_QueryArgs;

private:
	SimpleMutex m_resultMutex;
	ManualThreadLock stlqueryLock;
	bool addQuery(bool, LPCTSTR, LPCTSTR);
};

#endif