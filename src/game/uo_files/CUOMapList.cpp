#include "../../common/CServerMap.h"
#include "../../common/CUOInstall.h"
#include "../CServerConfig.h"
#include "CUOMapList.h"
#include "CUOMap.h"

// CUOMapList:: Constructors, Destructor, Asign operator.
#define UO_MAP_FILES (uchar)5   // Amount of map files

CUOMapList::CUOMapList()
{
    memset(m_mapsinitalized, 0, sizeof(UCHAR_MAX));
    memset(m_sizex, 0, sizeof(UCHAR_MAX));
    memset(m_sizey, 0, sizeof(UCHAR_MAX));
    memset(m_mapnum, -1, sizeof(UCHAR_MAX));
    memset(m_mapid, -1, sizeof(UCHAR_MAX));
    memset(m_sectorsize, 0, sizeof(UCHAR_MAX));
    //_mMaps = new CUOMap*[UCHAR_MAX];

    for (uchar i = 0; i <= UO_MAP_FILES; ++i)
    {
        Load(i, 0, 0, 0, i, i);
    }

    m_pMapDiffCollection = nullptr;
}

// CUOMapList:: Modifiers.

void CUOMapList::Init()
{
    for ( uchar i = 0; i < UCHAR_MAX; i++ )
    {
        if ( _mMaps[i] )	// map marked as available. check whatever it's possible
        {
            //	check coordinates first
            if (m_mapnum[i] == -1)
            {
                continue;
            }
        }
    }

    if ( g_Cfg.m_fUseMapDiffs && !m_pMapDiffCollection )
        m_pMapDiffCollection = new CServerMapDiffCollection();


}

bool CUOMapList::Load(uchar map, char *args)
{
    if (( map < 0 ) || ( map > 255 ))
    {
        g_Log.EventError("Invalid map #%d couldn't be initialized.\n", map);
        return false;
    }
    else if ( !m_mapsinitalized[map] )	// disable double intialization
    {
        tchar * ppCmd[5];	// maxx,maxy,sectorsize,mapnum[like 0 for map0/statics0/staidx0],mapid
        size_t iCount = Str_ParseCmds(args, ppCmd, CountOf(ppCmd), ",");

        if ( iCount <= 0 )	// simple MAPX= same as disabling the map
        {
            _mMaps[map] = nullptr;
        }
        else
        {
            short	maxx = 0, maxy = 0, sectorsize = 0;
            uchar realmapnum = 0, mapid = UCHAR_MAX;
            switch (iCount)
            {
                case 5:
                    mapid = (uchar)ATOI(ppCmd[4]);
                case 4:
                    realmapnum = (uchar)ATOI(ppCmd[3]);
                case 3:
                    sectorsize = (short)ATOI(ppCmd[2]);
                case 2:
                    maxy = (short)ATOI(ppCmd[1]);
                case 1:
                    maxx = (short)ATOI(ppCmd[0]);
                default:
                    break;
            }

            // zero settings of anything except the real map num means
            if ( maxx )					// skipping the argument
            {
                if (( maxx < 8 ) || ( maxx % 8 ))
                {
                    g_Log.EventError("MAP%d: X coord must be multiple of 8 (%d is invalid, %d is still effective).\n", map, maxx, m_sizex[map]);
                }
                else m_sizex[map] = maxx;
            }
            if ( maxy )
            {
                if (( maxy < 8 ) || ( maxy % 8 ))
                {
                    g_Log.EventError("MAP%d: Y coord must be multiple of 8 (%d is invalid, %d is still effective).\n", map, maxy, m_sizey[map]);
                }
                else m_sizey[map] = maxy;
            }
            if ( sectorsize > 0 )
            {
                if (( sectorsize < 8 ) || ( sectorsize % 8 ))
                {
                    g_Log.EventError("MAP%d: Sector size must be multiple of 8 (%d is invalid, %d is still effective).\n", map, sectorsize, m_sectorsize[map]);
                }
                else if (( m_sizex[map]%sectorsize ) || ( m_sizey[map]%sectorsize ))
                {
                    g_Log.EventError("MAP%d: Map dimensions [%d,%d] must be multiple of sector size (%d is invalid, %d is still effective).\n", map, m_sizex[map], m_sizey[map], sectorsize, m_sectorsize[map]);
                }
                else m_sectorsize[map] = sectorsize;
            }
            if ( realmapnum >= 0 )
                m_mapnum[map] = realmapnum;
            if ( mapid == UCHAR_MAX)
                m_mapid[map] = mapid;
            else
                m_mapid[map] = map;
        }
        _mMaps[map] = new CUOMap(map, m_mapnum[map], m_sizex[map], m_sizey[map], m_sectorsize[map]);
        _mMaps[map]->init();
        m_mapsinitalized[map] = true;
    }
    return true;
}

bool CUOMapList::Load(uchar map, short maxx, short maxy, short sectorsize, uchar realmapnum, uchar mapid)
{
    m_sizex[map] = maxx;
    m_sizey[map] = maxy;
    m_sectorsize[map] = sectorsize;
    m_mapnum[map] = realmapnum;
    m_mapid[map] = mapid;
    return true;
}

// CUOMapList:: Operations.

bool CUOMapList::DetectMapSize(int map)
{
    if (!_mMaps[(uchar)map])
        return false;

    int	index = m_mapnum[(uchar)map];
    if ( index < 0 )
        return false;

    if (g_Install.m_Maps[index].IsFileOpen() == false)
        return false;

    //
    //	#0 - map0.mul			(felucca, 6144x4096 or 7168x4096, 77070336 or 89915392 bytes)
    //	#1 - map0 or map1.mul	(trammel, 6144x4096 or 7168x4096, 77070336 or 89915392 bytes)
    //	#2 - map2.mul			(ilshenar, 2304x1600, 11289600 bytes)
    //	#3 - map3.mul			(malas, 2560x2048, 16056320 bytes)
    //	#4 - map4.mul			(tokuno islands, 1448x1448, 6421156 bytes)
    //	#5 - map5.mul			(ter mur, 1280x4096, 16056320 bytes)
    //

    switch (index)
    {
        case 0: // map0.mul
        case 1: // map1.mul
            if (g_Install.m_Maps[index].GetLength() == 89915392 ||			// ML-sized
                !strcmpi(g_Install.m_Maps[index].GetFileExt(), ".uop"))		// UOP are all ML-sized
                //g_Install.m_Maps[index].GetLength() == 89923808)			// (UOP packed)
            {
                if (m_sizex[map] <= 0)		m_sizex[map] = 7168;
                if (m_sizey[map] <= 0)		m_sizey[map] = 4096;
            }
            else
            {
                if (m_sizex[map] <= 0)		m_sizex[map] = 6144;
                if (m_sizey[map] <= 0)		m_sizey[map] = 4096;
            }

            if (m_sectorsize[map] <= 0)	m_sectorsize[map] = 64;
            break;

        case 2: // map2.mul
            if (m_sizex[map] <= 0)		m_sizex[map] = 2304;
            if (m_sizey[map] <= 0)		m_sizey[map] = 1600;
            if (m_sectorsize[map] <= 0)	m_sectorsize[map] = 64;
            break;

        case 3: // map3.mul
            if (m_sizex[map] <= 0)		m_sizex[map] = 2560;
            if (m_sizey[map] <= 0)		m_sizey[map] = 2048;
            if (m_sectorsize[map] <= 0)	m_sectorsize[map] = 64;
            break;

        case 4: // map4.mul
            if (m_sizex[map] <= 0)		m_sizex[map] = 1448;
            if (m_sizey[map] <= 0)		m_sizey[map] = 1448;
            if (m_sectorsize[map] <= 0)	m_sectorsize[map] = 64;
            break;

        case 5: // map5.mul
            if (m_sizex[map] <= 0)		m_sizex[map] = 1280;
            if (m_sizey[map] <= 0)		m_sizey[map] = 4096;
            if (m_sectorsize[map] <= 0)	m_sectorsize[map] = 64;
            break;

        default:
            DEBUG_ERR(("Unknown map index %d with file size of %" PRIuSIZE_T " bytes. Please specify the correct size manually.\n", index, g_Install.m_Maps[index].GetLength()));
            break;
    }

    return (m_sizex[map] > 0 && m_sizey[map] > 0 && m_sectorsize[map] > 0);
}

void CUOMapList::Close()
{
    CUOMap *pMap = nullptr;
    for (uint iMap = 0; iMap < 255; ++iMap)
    {
        pMap = _mMaps[(uchar)iMap];
        if (!pMap)
        {
            continue;
        }
        delete pMap;
        _mMaps[(uchar)iMap] = nullptr;
    }

    if (m_pMapDiffCollection != nullptr)
    {
        delete m_pMapDiffCollection;
        m_pMapDiffCollection = nullptr;
    }
}

bool CUOMapList::IsMapSupported(int map) const
{
    if (( map < 0 ) || ( map > 255 ))
        return false;
    return( _mMaps.at((uchar)map) != nullptr);
}

CUOMap * CUOMapList::GetMap(uchar iMap)
{
    return _mMaps[iMap];
}

bool CUOMapList::IsInitialized(int map) const
{
    return (m_mapsinitalized[map]);
}
