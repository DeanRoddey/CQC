//
// FILE NAME: CQCWebRIVA_ImgMap.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2017
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This file implements the RIVA client image map class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebRIVA_.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TRIVAImgMap
//  PREFIX: rimap
// ---------------------------------------------------------------------------

// -------------------------------------------------------------------
//  TRIVAImgMap: Constructors and Destructor
// -------------------------------------------------------------------
TRIVAImgMap::TRIVAImgMap() :

    m_colImgs
    (
        109
        , TStringKeyOps(kCIDLib::False)
        , &TRIVAImgItem::objExtractKey
        , tCIDLib::EMTStates::Safe
    )
{
}

TRIVAImgMap::~TRIVAImgMap()
{
}


// -------------------------------------------------------------------
//  TRIVAImgMap: Public, non-virtual methods
// -------------------------------------------------------------------

//
//  Returns true if it added a new img item, else false if it updated an existing one.
//
tCIDLib::TBoolean
TRIVAImgMap::bAddUpdate(const TString& strImgPath, const tCIDLib::TCard4 c4SerialNum)
{
    TRIVAImgItem* pimgiAdd = m_colImgs.pobjFindByKey(strImgPath);
    if (pimgiAdd)
    {
        pimgiAdd->objValue(c4SerialNum);
        return kCIDLib::False;
    }

    m_colImgs.objAdd(TRIVAImgItem(strImgPath, c4SerialNum));
    return kCIDLib::True;
}


//
//  Tries to find the passed image path in our map. The path is case insensitive. If
//  found it returns the image item, else null.
//
const TRIVAImgItem* TRIVAImgMap::pimiFind(const TString& strImgPath) const
{
    return m_colImgs.pobjFindByKey(strImgPath);
}


tCIDLib::TVoid TRIVAImgMap::Remove(const TString& strImgPath)
{
    // Tell it not to throw if not found. It should be, but let's be tolerant
    m_colImgs.bRemoveKey(strImgPath, kCIDLib::False);
}


tCIDLib::TVoid TRIVAImgMap::Reset()
{
    m_colImgs.RemoveAll();
}
