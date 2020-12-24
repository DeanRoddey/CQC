//
// FILE NAME: ElkM1V2C_LoadInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2014
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
//  This is the header for the ElkM1V2C_LoadInfo.cpp file, which implements
//  a small class that we use to track the information about a configured
//  load.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLoadInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TLoadInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLoadInfo();

        TLoadInfo
        (
            const   TLoadInfo&              iiSrc
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TLoadInfo&              iiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TLoadInfo&              iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TLoadInfo& iiSrc) const
        {
            return !operator==(iiSrc);
        }


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bComp
        (
            const   TItemInfo&              iiToComp
        )   const;

        tCIDLib::TVoid FormatCfg
        (
                    TTextOutStream&         strmTar
        )   const;

        tCIDLib::TVoid FormatSettings
        (
                    TTextOutStream&         strmTar
            , const TStreamFmt&             sfmtName
            , const TStreamFmt&             sfmtValue
        )   const;

        tCIDLib::TVoid ParseCfg
        (
            const   tCIDLib::TCard4         c4Id
            ,       tCIDLib::TStrList&      colTokens
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_bDimmer
        //      Each one is marked as a dimmer or switch.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDimmer;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TLoadInfo,TItemInfo)
        DefPolyDup(TLoadInfo)
};

#pragma CIDLIB_POPPACK


