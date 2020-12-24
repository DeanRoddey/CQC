//
// FILE NAME: CQCIR_Type.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This file implements the streaming methods for our enum types.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIR_.hpp"



// Translate a text version of an IR device cap flags to the enum value
tCQCIR::EIRDevCaps tCQCIR::eXlatEIRDevCaps(const TString& strToXlat)
{
    static const tCIDLib::TCh* const apszText[] =
    {
        L"None"
        , L"Blast"
        , L"BlastTrain"
        , L"Receive"
    };
    static const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(apszText);

    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (strToXlat == apszText[c4Index])
            break;
    }

    if (c4Index == c4Count)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_BadEnumValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strToXlat
            , TString(L"tCQCIR::EIRDevCaps")
        );
    }

    //
    //  The 0th slot is zero, so if it's zero we return none, else we
    //  shift up one by the index minus one.
    //
    if (c4Index)
        return tCQCIR::EIRDevCaps(0x1UL << (c4Index - 1));
    return tCQCIR::EIRDevCaps::None;
}

