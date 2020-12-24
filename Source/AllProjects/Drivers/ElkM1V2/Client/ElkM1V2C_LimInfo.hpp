//
// FILE NAME: ElkM1V2C_LimInfo.hpp
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
//  This is the header for the ElkM1V2C_LimInfo.cpp file, which implements
//  an intermediate class that a few different ultimate info classes derive from,
//  the ones that have low/high limits. This lets us process some of their stuff
//  generically.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLimInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TLimInfo : public TItemInfo
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TLimInfo();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TLimInfo&               iiSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TLimInfo& iiSrc) const
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
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSignedLims() const;



        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_bSignedLims
        //      The derived class indicates whether his limits should be signed
        //      or unsigned.
        //
        //  m_i4HighLim
        //  m_i4LowLim
        //      The high/low range limits that the user sets for this item.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bSignedLims;
        tCIDLib::TInt4      m_i4LowLim;
        tCIDLib::TInt4      m_i4HighLim;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TLimInfo
        (
            const   tElkM1V2C::EItemTypes   eType
            , const tCIDLib::TBoolean       bSignedLims
            , const tCIDLib::TInt4          i4DefLow
            , const tCIDLib::TInt4          i4DefHigh
        );

        TLimInfo
        (
            const   TLimInfo&               iiSrc
        );

        tCIDLib::TVoid operator=
        (
            const   TLimInfo&               iiSrc
        );


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TLimInfo,TItemInfo)
        DefPolyDup(TLimInfo)
};

#pragma CIDLIB_POPPACK



