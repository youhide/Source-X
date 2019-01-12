/**
* @file CUOMap.cpp
*
*/

#include "CUOMap.h"
#include "../CSector.h"
#include <iostream>


CUOMap::CUOMap()
{
    ADDTOCALLSTACK("CUOMap::CUOMap");
    _iMapIndex = UCHAR_MAX;    // 255 = disabled
    _iMapFileID = 0;
    _iSizeX = 0;
    _iSizeY = 0;
    _iSectorSize = 0;
    _iSectorQty = 0;
    _iSectorsActive = 0;
}

CUOMap::~CUOMap()
{
    ADDTOCALLSTACK("CUOMap::~CUOMap");
    ASSERT(_iSectorQty);
    CSector *pSector = nullptr;
    for (int x = 0; x < _iSectorQty; ++x)
    {
        pSector = _vSectors[x];
        if (!pSector)
        {
            continue;
        }
        pSector->Close();
        delete pSector;
    }
}

bool CUOMap::init(uchar iMapID, uchar iRealID, short iSizeX, short iSizeY, short iSectorSize)
{
    ADDTOCALLSTACK("CUOMap::Init");
    _iMapIndex = iMapID;
    _iMapFileID = iRealID;
    _iSizeX = iSizeX;
    _iSizeY = iSizeY;
    _iSectorSize = iSectorSize;
    if (_iSectorSize == 0)
    {
        _iSectorSize = 64;
    }
    _iSectorQty = GetSectorCols() * GetSectorRows();

    CSector * pSector = nullptr;
    short iSectorsX = GetSectorCols();
    short iSectorsY = GetSectorRows();
    _vSectors.reserve(GetSectorQty());
    int icount = 0;

    for (short iCoordY = 0; iCoordY < iSectorsY; ++iCoordY)
    {
        for (short iCoordX = 0; iCoordX < iSectorsX; ++iCoordX)
        {            
            _vSectors.push_back(new CSector(this, iSectorsX, iSectorsY, icount++));
        }
    }
    ASSERT((int)_vSectors.size() == _iSectorQty);

    icount = (int)_vSectors.size();
    // Additional for just to call Init (which calls SetAdjacentSectors, and should be done after generating all sectors).
    for (short x = 0; x < icount; ++x)
    {
        pSector = _vSectors[x];
        ASSERT(pSector);  // Should never trigger.
        pSector->SetAdjacentSectors();
    }
    return true;
}

uchar CUOMap::GetMapIndex()
{
    return _iMapIndex;
}

uchar CUOMap::GetMapFileID()
{
    return _iMapFileID;
}

void CUOMap::SetMapID(uchar iMapID)
{
    _iMapIndex = iMapID;
}

void CUOMap::SetRealID(uchar iRealID)
{
    _iMapFileID = iRealID;
}

void CUOMap::LinkRegion(CRegion * pRegion, int & iLinkedSectors)
{
    ADDTOCALLSTACK("CUOMap::LinkRegion");
    for (auto &pSector : _vSectors)
    {
        if (!pSector)
        {
            continue;
        }
        if (pRegion->IsOverlapped(pSector->GetRect()))
        {
            //	Yes, this sector overlapped, so add it to the sector list
            if (!pSector->LinkRegion(pRegion))
            {
                g_Log.EventError("Linking sector #%d in map %d for region %s failed (fatal for this region).\n", pSector->GetIndex(), GetMapIndex(), pRegion->GetName());
                return;
            }
            iLinkedSectors++;
        }
    }
}

void CUOMap::UnrealizeRegion(CRegion * pRegion, int& iLinkedSectors)
{
    ADDTOCALLSTACK("CUOMap::UnrealizeRegion");  //TODO: rename this to match 'LinkRegion'
    CSector *pSector = nullptr;
    int iMax = GetSectorQty();
    for (int x = 0; x < iMax; ++x)
    {
        pSector = _vSectors[x];
        if (!pSector)
        {
            continue;
        }
        if (!pRegion->IsOverlapped(pSector->GetRect()))
        {
            continue;
        }
        if (pSector->UnLinkRegion(pRegion))
        {
            iLinkedSectors--;
        }
    }
}

short CUOMap::GetSizeX()
{
    return _iSizeX;
}

short CUOMap::GetSizeY()
{
    return _iSizeY;
}

short CUOMap::GetSectorSize()
{
    return _iSectorSize;
}

int CUOMap::GetSectorQty()
{
    return _iSectorQty;
}

short CUOMap::GetSectorRows()
{
    ADDTOCALLSTACK("CUOMap::GetSectorRows");
    return _iSizeY / _iSectorSize;
}

short CUOMap::GetSectorCols()
{
    ADDTOCALLSTACK("CUOMap::GetSectorCols");
    return _iSizeX / _iSectorSize;
}

int CUOMap::GetSectorsActive()
{
    return _iSectorsActive;
}

CSector * CUOMap::GetSector(short x, short y)
{
    ADDTOCALLSTACK("CUOMap::GetSector(x,y)");
    int index = (y * GetSectorRows()) + x;
    if (index < 0 || index >= _iSectorQty)
    {
        // ASSERT?
        return nullptr;
    }
    return GetSector(index);
}

CSector * CUOMap::GetSectorByCoords(short x, short y)
{
    ADDTOCALLSTACK("CUOMap::GetSectorByCoords");
    return GetSector(x / GetSectorCols(), y / GetSectorRows());
}

CSector * CUOMap::GetSector(int x)
{
    ADDTOCALLSTACK("CUOMap::GetSector(i)");
    return _vSectors[x];
}