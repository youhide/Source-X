/**
* @file CUOMap.cpp
*
*/

#include "CUOMap.h"
#include "../CSector.h"
#include <iostream>


CUOMap::CUOMap(uchar iMapID, uchar iRealID, short iSizeX, short iSizeY, short iSectorSize)
{
    _iMapID = iMapID;
    _iRealID = iRealID;
    _iSizeX = iSizeX;
    _iSizeY = iSizeY;
    _iSectorSize = iSectorSize;
    if (_iSectorSize == 0)
    {
        _iSectorSize = 64;
    }
    _iSectorQty = GetSectorCols() * GetSectorRows();
    _iSectorsActive = 0;
}

CUOMap::~CUOMap()
{
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

bool CUOMap::init()
{
    CSector * pSector = nullptr;
    short iSizeX = GetSectorCols();
    short iSizeY = GetSectorRows();
    _vSectors.reserve(GetSectorQty());
    int icount = 0;

    for (short iCoordY = 0; iCoordY < iSizeY; ++iCoordY)
    {
        for (short iCoordX = 0; iCoordX < iSizeX; ++iCoordX)    
        {            
            _vSectors.push_back(new CSector(this, iCoordX, iCoordY, icount++));          
        }
    }
    ASSERT((int)_vSectors.size() == _iSectorQty);

    // Additional for just to call Init (which calls SetAdjacentSectors, and should be done after generating all sectors).
    for (short x = 0; x < iSizeX; ++x)
    {
        pSector = _vSectors[x];
        ASSERT(pSector);  // Should never trigger.
        pSector->SetAdjacentSectors();
    }
    return true;
}

uchar CUOMap::GetMapID()
{
    return _iMapID;
}

uchar CUOMap::GetRealID()
{
    return _iRealID;
}

void CUOMap::SetMapID(uchar iMapID)
{
    _iMapID = iMapID;
}

void CUOMap::SetRealID(uchar iRealID)
{
    _iRealID = iRealID;
}

void CUOMap::LinkRegion(CRegion * pRegion, int & iLinkedSectors)
{

    CSector *pSector = nullptr;
    int iMax = GetSectorCols() * GetSectorRows();
    for (int x = 0; x < iMax; ++x)
    {
        pSector = _vSectors[x];
        if (!pSector)
        {
            continue;
        }
        if (pRegion->IsOverlapped(pSector->GetRect()))
        {
            //	Yes, this sector overlapped, so add it to the sector list
            if (!pSector->LinkRegion(pRegion))
            {
                g_Log.EventError("Linking sector #%d in map %d for region %s failed (fatal for this region).\n", x, GetMapID(), pRegion->GetName());
                return;
            }
            iLinkedSectors++;
        }
    }
}

void CUOMap::UnrealizeRegion(CRegion * pRegion, int& iLinkedSectors)
{
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
    return _iSizeY / _iSectorSize;
}

short CUOMap::GetSectorCols()
{
    return _iSizeX / _iSectorSize;
}

int CUOMap::GetSectorsActive()
{
    return _iSectorsActive;
}

CSector * CUOMap::GetSector(short x, short y)
{
    int index = (y * GetSectorRows()) + x;
    if (index < 0 || index > _iSectorQty)
    {
        // ASSERT?
        return nullptr;
    }
    return _vSectors[index];
}

CSector * CUOMap::GetSectorByCoords(short x, short y)
{
    return GetSector(x / GetSectorCols(), y / GetSectorRows());
}

CSector * CUOMap::GetSector(int x)
{
    return _vSectors[x];
}