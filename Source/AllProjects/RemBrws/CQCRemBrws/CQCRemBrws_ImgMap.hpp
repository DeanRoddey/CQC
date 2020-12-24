//
// FILE NAME: CQCRemBrws_ImgMap.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/09/2008
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
//  This file defines a simple class that is used by the data server to
//  provide image maps, i.e. it can map images from one image repository
//  scope to another, under different names.
//
//  We define this class here so that we can provide a simple command line
//  tool to verify image maps, which we couldn't do if it was defined in the
//  data server itself. There's a static method to parse a map file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCImgMap
//  PREFIX: cimap
// ---------------------------------------------------------------------------
class CQCREMBRWSEXP TCQCImgMap : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TNameMap = tCIDLib::TKVHashSet;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid ParseMapFile
        (
                    TTextInStream&          strmSrc
            , const TString&                strFileName
            ,       TCQCImgMap&             cimapToFill
        );

        static const TString& strKey
        (
            const   TCQCImgMap&             imapSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCImgMap();

        TCQCImgMap
        (
            const   TString&                strSrcPath
            , const TString&                strTarPath
        );

        TCQCImgMap(const TCQCImgMap&) = default;
        TCQCImgMap(TCQCImgMap&&) = default;

        ~TCQCImgMap();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQCImgMap& operator=(const TCQCImgMap&) = default;
        TCQCImgMap& operator=(TCQCImgMap&&) = default;


        // -------------------------------------------------------------------
        //  Public members
        // -------------------------------------------------------------------
        TNameMap    m_colMap;
        TString     m_strSrcPath;
        TString     m_strTarPath;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCImgMap,TObject)
};

#pragma CIDLIB_POPPACK


