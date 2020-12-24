//
// FILE NAME: ElkM1V2C_AreaInfo.hpp
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
//  This is the header for the ElkM1V2C_AreaInfo.cpp file, which implements
//  a small class that we use to track the information about a configured
//  voltage. We have to keep up with the tree quadratic equation components
//  and high/low field limit values for each one.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TAreaInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TAreaInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAreaInfo();

        TAreaInfo
        (
            const   TAreaInfo&              iiSrc
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TAreaInfo&              iiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TAreaInfo&              iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TAreaInfo& iiSrc) const
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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TAreaInfo,TItemInfo)
        DefPolyDup(TAreaInfo)
};

#pragma CIDLIB_POPPACK


