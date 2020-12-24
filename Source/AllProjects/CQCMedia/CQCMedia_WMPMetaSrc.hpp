//
// FILE NAME: CQCMedia_WMPWrapper.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2006
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
//  This is the header for the CQCMedia_WMPWrapper.cpp file, which implements
//  a wrapper around MS WMP media player API for getting to media metadata
//  via WMP. Inside here we are using raw COM stuff, which we hide from the
//  outside world.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCWMPMetaSrc
// PREFIX: wmpms
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCWMPMetaSrc : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCWMPMetaSrc();

        TCQCWMPMetaSrc(const TCQCWMPMetaSrc&) = delete;

        ~TCQCWMPMetaSrc();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TCQCWMPMetaSrc& operator=(const TCQCWMPMetaSrc&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompleteLoad
        (
            const   TString&                strPath
            ,       TMediaCollect&          mcolToFill
            ,       TJPEGImage&             imgCover
            ,       tCQCMedia::TItemList&   colItems
            ,       tCIDLib::TStrList&      colCatNames
            ,       TString&                strInfoURL
        );

        tCIDLib::TVoid StartLoad
        (
            const   TString&                strPath
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pData
        //      We use an opaque pointer and just cast it internally to it's
        //      actual type as required.
        // -------------------------------------------------------------------
        tCIDLib::TVoid* m_pData;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCWMPMetaSrc,TObject)
};

#pragma CIDLIB_POPPACK


