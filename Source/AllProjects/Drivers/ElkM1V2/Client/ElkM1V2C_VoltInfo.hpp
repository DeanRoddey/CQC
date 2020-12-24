//
// FILE NAME: ElkM1V2C_VoltInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/30/2008
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
//  This is the header for the ElkM1V2C_VoltInfo.cpp file, which implements
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
//   CLASS: TVoltInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TVoltInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVoltInfo();

        TVoltInfo
        (
            const   TVoltInfo&              iiSrc
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TVoltInfo&              iiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TVoltInfo&              iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TVoltInfo& iiSrc) const
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
        //  m_f8XVal
        //      The three quadratic equation components that are used to map
        //      the raw voltage to a useful value.
        //
        //  m_f8HighLim
        //  m_f8LowLim
        //      The high and low limits for the field, which are defined by
        //      the user since we cannot figure out what they would be
        //      automatically.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_f8AVal;
        tCIDLib::TFloat8    m_f8BVal;
        tCIDLib::TFloat8    m_f8CVal;
        tCIDLib::TFloat8    m_f8HighLim;
        tCIDLib::TFloat8    m_f8LowLim;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TVoltInfo,TItemInfo)
        DefPolyDup(TVoltInfo)
};

#pragma CIDLIB_POPPACK

