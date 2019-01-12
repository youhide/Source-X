#include "../../common/CServerMap.h"
#include "../../common/CUOInstall.h"
#include "../CServerConfig.h"
#include "CUOMapList.h"
#include "CUOMap.h"

// CUOMapList:: Constructors, Destructor, Asign operator.
#define UO_MAP_FILES (uchar)5   // Amount of map files

CUOMapList::CUOMapList()
{
    m_pMapDiffCollection = nullptr;
}

// CUOMapList:: Modifiers.

void CUOMapList::Init()
{
    if (g_Cfg.m_fUseMapDiffs && !m_pMapDiffCollection)
    {
        m_pMapDiffCollection = new CServerMapDiffCollection();
    }
}

bool CUOMapList::Load(uchar map, char *args)
{
    if (( map < 0 ) || ( map >= UCHAR_MAX))
    {
        g_Log.EventError("Invalid map #%d couldn't be initialized.\n", map);
        return false;
    }
    if (_mMaps[map] == nullptr)
    {
        _mMaps[map] = new CUOMap();
    }
    if ( _mMaps[map]->GetMapIndex() == UCHAR_MAX )	// disable double intialization
    {
        tchar * ppCmd[5];	// iSizeX,iSizeY,iSectorSize,mapnum[like 0 for map0/statics0/staidx0],iMapIndex
        size_t iCount = Str_ParseCmds(args, ppCmd, CountOf(ppCmd), ",");

        short iSizeX = 0;
        short iSizeY = 0;
        short iSectorSize = 0;
        uchar iMapFileID = 0;   // default to map0.
        uchar iMapIndex = UCHAR_MAX;    // 255 = yet disabled
        DetectMapSize(map, iSizeX, iSizeY, iSectorSize);
        if ( iCount > 0 )	// no params = disabled map.
        {
            short itmp = 0;
            switch (iCount)
            {
                case 5:
                {
                    iMapIndex = (uchar)ATOI(ppCmd[4]);
                    if (iMapIndex >= UCHAR_MAX)
                    {
                        g_Log.EventError("MAP%d: Setting map's id to an invalid value (%d is invalid, defaulting to %d).\n", map, iMapIndex, map);
                        iMapIndex = map;
                    }
                }
                case 4:
                {
                    itmp = (short)ATOI(ppCmd[3]);
                    if (itmp > UCHAR_MAX ) // would be better to check against #UO_MAP_FILES, but that would prevent some possible additional maps in custom clients or future new maps added.
                    {
                        g_Log.EventError("MAP%d: Setting map's file id to an invalid value (%d is invalid, defaulting to %d).\n", map, itmp, 0);
                        itmp = 0;
                    }
                    iMapFileID = (uchar)itmp;
                }
                case 3:
                {
                    itmp = (short)ATOI(ppCmd[2]);
                    if (itmp == -1) // Auto-detect
                    {
                        itmp = iSectorSize; // TODO: this is not yet a real working autodetecting code, needs to have calculations to generate default values
                    }
                    if ((itmp < 8) || (itmp % 8))
                    {
                        g_Log.EventError("MAP%d: Sector size must be multiple of %d (%d is invalid).\n", map, iSectorSize, itmp);
                        itmp = iSectorSize;
                    }
                    else if ((iSizeX%itmp) || (iSizeY%itmp))
                    {
                        g_Log.EventError("MAP%d: Map dimensions [%d,%d] must be multiple of sector size [%d] (%d is invalid).\n", map, iSizeX, iSizeY, iSectorSize, itmp);
                        itmp = iSectorSize;
                    }
                    iSectorSize = itmp;
                }
                case 2:
                {
                    itmp = (short)ATOI(ppCmd[1]);
                    if ((itmp < 8) || (itmp % 8))
                    {
                        g_Log.EventError("MAP%d: Y coord must be multiple of 8 (%d is invalid, %d is still valid).\n", map, itmp, iSizeY);
                        itmp = iSizeY;
                    }
                    iSizeY = itmp;
                }
                case 1:
                {
                    itmp = (short)ATOI(ppCmd[0]);
                    if ((itmp < 8) || (itmp % 8))
                    {
                        g_Log.EventError("MAP%d: X coord must be multiple of 8 (%d is invalid, %d is still valid).\n", map, itmp, iSizeX);
                        itmp = iSizeX;
                    }
                    iSizeX = itmp;
                }
                default:
                    break;
            }
        }
        ASSERT(_mMaps[map]);
        _mMaps[map]->init(map, iMapFileID, iSizeX, iSizeY, iSectorSize);
    }
    return true;
}

// CUOMapList:: Operations.

bool CUOMapList::DetectMapSize(int map, short &iSizeX, short &iSizeY, short &iSectorSize)
{
    if (!_mMaps[(uchar)map])
    {
        return false;
    }

    int	index = map;// _mMaps[(uchar)map]->GetMapIndex();
    bool fMLSizedMap = true;
    /*if (index < 0 || index >= UCHAR_MAX)
    {
        return false;
    }*/

    /*if (g_Install.m_Maps[index].IsFileOpen() == false)
    {
        if (g_Install.m_Maps[index].GetLength() == 89915392 ||			// ML-sized
            !strcmpi(g_Install.m_Maps[index].GetFileExt(), ".uop"))		// UOP are all ML-sized
            //g_Install.m_Maps[index].GetLength() == 89923808)			// (UOP packed)
        {
            fMLSizedMap = false;
        }
        //return false;
    }*/

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
            {
                if (fMLSizedMap)
                {
                    if (iSizeX <= 0)
                    {
                        iSizeX = 7168;
                    }
                    if (iSizeY <= 0)
                    {
                        iSizeY = 4096;
                    }
                }
                else
                {
                    if (iSizeX <= 0)
                    {
                        iSizeX = 6144;
                    }
                    if (iSizeY <= 0)
                    {
                        iSizeY = 4096;
                    }
                }

                if (iSectorSize <= 0)
                {
                    iSectorSize = 64;
                }
            }
            break;

        case 2: // map2.mul
            {
                if (iSizeX <= 0)
                {
                    iSizeX = 2304;
                }
                if (iSizeY <= 0)
                {
                    iSizeY = 1600;
                }
                if (iSectorSize <= 0)
                {
                    iSectorSize = 16;
                }
            }
            break;

        case 3: // map3.mul
            {
                if (iSizeX <= 0)
                {
                    iSizeX = 2560;
                }
                if (iSizeY <= 0)
                {
                    iSizeY = 2048;
                }
                if (iSectorSize <= 0)
                {
                    iSectorSize = 16;
                }
            }
            break;

        case 4: // map4.mul
            {
                if (iSizeX <= 0)
                {
                    iSizeX = 1448;
                }
                if (iSizeY <= 0)
                {
                    iSizeY = 1448;
                }
                if (iSectorSize <= 0)
                {
                    iSectorSize = 11;
                }
            }
            break;

        case 5: // map5.mul
            {
                if (iSizeX <= 0)
                {
                    iSizeX = 1280;
                }
                if (iSizeY <= 0)
                {
                    iSizeY = 4096;
                }
                if (iSectorSize <= 0)
                {
                    iSectorSize = 32;
                }
            }
            break;

        default:
        {
            DEBUG_ERR(("Unknown map index %d with file size of %" PRIuSIZE_T " bytes. Please specify the correct size manually.\n", index, g_Install.m_Maps[index].GetLength()));
            break;
        }
    }

    return (iSizeX > 0 && iSizeY > 0 && iSectorSize > 0);
}

void CUOMapList::Close()
{
    CUOMap *pMap = nullptr;
    for (uint iMap = 0; iMap < UCHAR_MAX; ++iMap)
    {
        pMap = GetMap((uchar)iMap);
        if (!pMap)
        {
            continue;
        }
        delete pMap;
        _mMaps.erase((uchar)iMap);
        //_mMaps[(uchar)iMap] = nullptr;
    }

    if (m_pMapDiffCollection != nullptr)
    {
        delete m_pMapDiffCollection;
        m_pMapDiffCollection = nullptr;
    }
}

bool CUOMapList::IsMapSupported(int map) const
{
    if ((map < 0) || (map >= 255))
    {
        return false;
    }
    auto it = _mMaps.find((uchar)map);
    if (it == _mMaps.end())
    {
        return nullptr;
    }
    return ((*it).second != nullptr);
}

CUOMap * CUOMapList::GetMap(uchar iMap)
{
    auto it = _mMaps.find((uchar)iMap);
    if (it == _mMaps.end())
    {
        return nullptr;
    }
    return (*it).second;
}

bool CUOMapList::IsInitialized(int map) const
{
    auto it = _mMaps.find((uchar)map);
    if (it == _mMaps.end())
    {
        return false;
    }
    return ((*it).second->GetMapIndex() != UCHAR_MAX);
}
