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
    short m_sizex[UCHAR_MAX];
    short m_sizey[UCHAR_MAX];
    short m_sectorsize[UCHAR_MAX];
    //bool m_maps[UCHAR_MAX];		// list of supported maps
    uchar m_mapnum[UCHAR_MAX];		// real map number (0 for 0 and 1, 2 for 2, and so on) - file name
    uchar m_mapid[UCHAR_MAX];		// map id used by the client
public:
    CServerMapDiffCollection * m_pMapDiffCollection;

protected:
    bool m_mapsinitalized[256];


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
    bool Load(uchar iMap, short maxx, short maxy, short sectorsize, uchar realmapnum, uchar mapid);
    ///@}
    /** @name Operations:
     */
    ///@{
protected:
    bool DetectMapSize(int iMap);
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
