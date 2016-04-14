#pragma once
#ifndef _INC_UOFILES_CUOINDEXREC_H
#define _INC_UOFILES_CUOINDEXREC_H

#include "../../common/common.h"

// All these structures must be byte packed.
#if defined _WINDOWS && (!__MINGW32__)
// Microsoft dependant pragma
#pragma pack(1)
#define PACK_NEEDED
#else
// GCC based compiler you can add:
#define PACK_NEEDED __attribute__ ((packed))
#endif

class CUOVersionBlock;

/**
* 12 byte block = used for table indexes. (staidx0.mul,multi.idx,anim.idx)
*/
struct CUOIndexRec
{
private:
    dword m_dwOffset;  ///< 0xFFFFFFFF = nothing here ! else pointer to something (CUOStaticItemRec possibly)
    dword m_dwLength; ///< Length of the object in question.
public:
    word m_wVal3;  ///< Varied uses. ex. GumpSizey
    word m_wVal4;  ///< Varied uses. ex. GumpSizex
public:
    /** @name Modifiers:
     */
    ///@{
    void Init();
    void CopyIndex( const CUOVersionBlock * pVerData );
    void SetupIndex( dword dwOffset, dword dwLength );
    ///@}
    /** @name Operations:
     */
    ///@{
    dword GetFileOffset() const;
    dword GetBlockLength() const;
    bool IsVerData() const;
    bool HasData() const;
    ///@}

} PACK_NEEDED;


// Turn off structure packing.
#if defined _WINDOWS && (!__MINGW32__)
#pragma pack()
#else
#undef PACK_NEEDED
#endif

#endif //_INC_UOFILES_CUOINDEXREC_H
