//
// FILE NAME: ElkM1V2C_OutputInfo.hpp
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
//  This is the header for the ElkM1V2C_OutputInfo.cpp file, which implements
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
//   CLASS: TOutputInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TOutputInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TOutputInfo();

        TOutputInfo
        (
            const   TOutputInfo&              iiSrc
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TOutputInfo&              iiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TOutputInfo&              iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TOutputInfo& iiSrc) const
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
        RTTIDefs(TOutputInfo,TItemInfo)
        DefPolyDup(TOutputInfo)
};

#pragma CIDLIB_POPPACK



