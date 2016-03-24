
#pragma once
#ifndef _INC_CPARTY_H
#define _INC_CPARTY_H

#include "../common/CArray.h"
#include "../common/CRect.h"
#include "../common/CScriptObj.h"
#include "../common/CString.h"
#include "../common/CVarDefMap.h"
#include "../common/grayproto.h"
#include "../chars/CCharRefArray.h"
#include "../CServTime.h"


class PacketSend;

class CPartyDef : public CGObListRec, public CScriptObj
{
	// a list of characters in the party.
#define MAX_CHAR_IN_PARTY 10

public:
	static const char *m_sClassName;
	static lpctstr const sm_szVerbKeys[];
	static lpctstr const sm_szLoadKeys[];
	static lpctstr const sm_szLoadKeysM[];
private:
	CCharRefArray m_Chars;
	CGString m_sName;
	CVarDefMap m_TagDefs;
	CVarDefMap m_BaseDefs;		// New Variable storage system
	CGString m_pSpeechFunction;

public:
	lpctstr GetDefStr( lpctstr pszKey, bool fZero = false ) const;
	int64 GetDefNum( lpctstr pszKey, bool fZero = false ) const;
	void SetDefNum(lpctstr pszKey, int64 iVal, bool fZero = true);
	void SetDefStr(lpctstr pszKey, lpctstr pszVal, bool fQuoted = false, bool fZero = true);
	void DeleteDef(lpctstr pszKey);

private:
	bool SendMemberMsg( CChar * pCharDest, PacketSend * pPacket );
	void SendAll( PacketSend * pPacket );
	// List manipulation
	size_t AttachChar( CChar * pChar );
	size_t DetachChar( CChar * pChar );

public:
	CPartyDef( CChar * pCharInvite, CChar * pCharAccept );

private:
	CPartyDef(const CPartyDef& copy);
	CPartyDef& operator=(const CPartyDef& other);

public:
	static bool AcceptEvent( CChar * pCharAccept, CGrayUID uidInviter, bool bForced = false );
	static bool DeclineEvent( CChar * pCharDecline, CGrayUID uidInviter );

	bool IsPartyFull() const;
	bool IsInParty( const CChar * pChar ) const;
	bool IsPartyMaster( const CChar * pChar ) const;
	CGrayUID GetMaster();


	// Refresh status for party members
	void AddStatsUpdate( CChar * pChar, PacketSend * pPacket );
	// List sending wrappers
	bool SendRemoveList( CChar * pCharRemove, bool bFor );
	bool SendAddList( CChar * pCharDest );
	// Party message sending wrappers
	bool MessageEvent( CGrayUID uidDst, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
	// void MessageAll( CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
	// bool MessageMember( CGrayUID uidDst, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
	// Sysmessage sending wrappers
	void SysMessageAll( lpctstr pText );

	// Commands
	bool Disband( CGrayUID uidMaster );
	bool RemoveMember( CGrayUID uidRemove, CGrayUID uidCommand );
	void AcceptMember( CChar * pChar );
	void SetLootFlag( CChar * pChar, bool fSet );
	bool GetLootFlag( const CChar * pChar );
	bool SetMaster( CChar * pChar );

	// -------------------------------

	lpctstr GetName() const { return static_cast<lpctstr>(m_sName); }
	bool r_GetRef( lpctstr & pszKey, CScriptObj * & pRef );
	bool r_WriteVal( lpctstr pszKey, CGString & sVal, CTextConsole * pSrc );
	bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	bool r_LoadVal( CScript & s );
	bool r_Load( CScript & s );
};


#endif // _INC_CPARTY_H
