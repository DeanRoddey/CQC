//
// FILE NAME: ElkM1C_VoltInfo.hpp
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
//  This is the header for the ElkM1C_VoltInfo.cpp file, which implements
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
//  PREFIX: vi
// ---------------------------------------------------------------------------
class TVoltInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVoltInfo();

        TVoltInfo
        (
            const   TVoltInfo&              viToCopy
        );


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TVoltInfo&              viToAssign
        );

        tCIDLib::TBoolean operator==
        (
            const   TVoltInfo&              viToComp
        )   const;

        tCIDLib::TBoolean operator!=(const TVoltInfo& viToComp) const
        {
            return !operator==(viToComp);
        }


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUsed() const;

        tCIDLib::TFloat8 f8AVal() const;

        tCIDLib::TFloat8 f8BVal() const;

        tCIDLib::TFloat8 f8CVal() const;

        tCIDLib::TFloat8 f8HighLim() const;

        tCIDLib::TFloat8 f8LowLim() const;

        tCIDLib::TVoid FormatCfg
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid FormatEQ
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid FormatLims
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bUsed
            , const tCIDLib::TFloat8        f8AVal
            , const tCIDLib::TFloat8        f8BVal
            , const tCIDLib::TFloat8        f8CVal
            , const tCIDLib::TFloat8        f8LowLim
            , const tCIDLib::TFloat8        f8HighLim
        );


    private :
        // -------------------------------------------------------------------
        //  Private class members
        //
        //  m_bUsed
        //      Indicates whether this voltage is configured for use by the
        //      driver or not.
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
        tCIDLib::TBoolean   m_bUsed;
        tCIDLib::TFloat8    m_f8AVal;
        tCIDLib::TFloat8    m_f8BVal;
        tCIDLib::TFloat8    m_f8CVal;
        tCIDLib::TFloat8    m_f8HighLim;
        tCIDLib::TFloat8    m_f8LowLim;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TVoltInfo,TObject)
};

#pragma CIDLIB_POPPACK

