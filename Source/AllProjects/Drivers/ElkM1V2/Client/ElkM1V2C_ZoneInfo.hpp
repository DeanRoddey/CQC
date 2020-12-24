//
// FILE NAME: ElkM1V2C_ZoneInfo.hpp
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
//  This is the header for the ElkM1V2C_ZoneInfo.cpp file, which implements
//  a small class that we use to track the information about a configured
//  zone.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZoneInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TZoneInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZoneInfo();

        TZoneInfo
        (
            const   TZoneInfo&              iiSrc
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TZoneInfo&              iiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZoneInfo&              iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TZoneInfo& iiSrc) const
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
        //  m_bSendTrigs
        //      Each zone can be marked as to whether it should send event
        //      triggers or not.
        //
        //  m_eType
        //      The type of zone this one should be treated as.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSendTrigs;
        tElkM1V2C::EZoneTypes   m_eType;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZoneInfo,TItemInfo)
        DefPolyDup(TZoneInfo)
};

#pragma CIDLIB_POPPACK


