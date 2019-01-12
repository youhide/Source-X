/**
* @file CUOMapList.h
*
*/

#ifndef _INC_CUOMAPLIST_H
#define _INC_CUOMAPLIST_H

#include <map>
#include "../../common/common.h"

// All these structures must be byte packed.
#if defined(_WIN32) && defined(_MSC_VER)
	// Microsoft dependant pragma
	#pragma pack(1)
	#define PACK_NEEDED
#else
	// GCC based compiler you can add:
	#define PACK_NEEDED __attribute__ ((packed))
#endif


class CUOMap;
class CServerMapDiffCollection;

extern class CUOMapList
{
private:
    std::map<uchar,CUOMap*> _mMaps;
public:
    CServerMapDiffCollection * m_pMapDiffCollection;

public:
    /** @name Constructors, Destructor, Asign operator:
     */
    ///@{
    CUOMapList();
private:
    CUOMapList(const CUOMapList& copy);
    CUOMapList& operator=(const CUOMapList& other);
    ///@}
public:
    /** @name Modifiers:
     */
    ///@{
    void Init();
    bool Load(uchar iMap, char *args);
    ///@}
    /** @name Operations:
     */
    ///@{
protected:
    bool DetectMapSize(int iMap, short &iSizeX, short &iSizeY, short &iSectorSize);
    friend class CWorld;
    void Close();
public:
    bool IsMapSupported(int iMap) const;
    CUOMap* GetMap(uchar iMap);
    bool IsInitialized(int iMap) const;
    ///@}
} g_MapList;


// Turn off structure packing.
#if defined(_WIN32) && defined(_MSC_VER)
	#pragma pack()
#else
	#undef PACK_NEEDED
#endif

#endif //_INC_CUOMAPLIST_H
