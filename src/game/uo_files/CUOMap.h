/**
* @file CUOMap.h
*
*/

#ifndef _INC_CUOMAP_H
#define _INC_CUOMAP_H

#include "uofiles_types.h"
#include <map>

class CSector;
class CUOMap
{
private:

    uchar _iMapID;          // ID given in the ini for this map.
    uchar _iRealID;         // ID of the file map used as base.

    short _iSizeX;          // Size of the map for X coord.
    short _iSizeY;          // Size of the map for Y coord.

    short _iSectorSize;     // Size in squares for each sector.
    int _iSectorQty;        // Total amount of sectors.
    int _iSectorsActive;    // Awaken sectors.
    std::vector<CSector*> _vSectors;    // index of sectors.

public:
    uchar GetMapID();
    uchar GetRealID();

    short GetSizeX();
    short GetSizeY();

    short GetSectorSize();
    int GetSectorQty();
    short GetSectorRows();
    short GetSectorCols();
    int GetSectorsActive();
    CSector * GetSector(short x, short y);
    CSector * GetSectorByCoords(short x, short y);
    CSector *GetSector(int x);
protected:
    friend class CUOMapList;
    friend class CUOInstall;
    friend class CRegion;
    CUOMap(uchar iMapID, uchar iRealID, short iSizeX, short iSizeY, short iSectorSize);
    ~CUOMap();
    bool init();
    void SetMapID(uchar iMapID);
    void SetRealID(uchar iRealID);
    void LinkRegion(CRegion* pRegion, int& iLinkedSectors);
    void UnrealizeRegion(CRegion *pRegion, int& iLinkedSectors);
};





#endif // _INC_CUOMAP_H
