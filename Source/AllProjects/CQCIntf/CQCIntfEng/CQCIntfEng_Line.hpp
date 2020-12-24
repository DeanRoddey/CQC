//
// FILE NAME: CQCIntfEng_Line.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2013
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
//  This is the header for the CQCIntfEng_Line.cpp file, which implements
//  implements the TCQCIntfLine class. This guy draws horizontal or vertical
//  lines, with optional line ends.
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
//   CLASS: TCQCIntfLine
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfLine : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfLine();

        TCQCIntfLine
        (
            const   TCQCIntfLine&       iwdgToCopy
        );

        ~TCQCIntfLine();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfLine& operator=
        (
            const   TCQCIntfLine&       iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList& colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHorizontal() const;

        tCIDLib::TBoolean bHorizontal
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bThick() const;

        tCIDLib::TBoolean bThick
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCIntfEng::ELineEnds eEndType() const;

        tCQCIntfEng::ELineEnds eEndType
        (
            const   tCQCIntfEng::ELineEnds  eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDCtrls::EWndAreas eHitTest
        (
            const   TPoint&                 pntTest
        )   const;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Draw
        (
                    TGraphDrawDev&          gdevTarget
            , const TPoint&                 pntStart
            , const TPoint&                 pntEnd
            , const TRGBClr&                rgbLine
            , const tCIDLib::TBoolean       bThick
            , const tCIDLib::TBoolean       bDoEnds
        );

        tCIDLib::TVoid DrawCircleEnds
        (
                    TGraphDrawDev&          gdevTarget
            , const TPoint&                 pntStart
            , const TPoint&                 pntEnd
            , const TRGBClr&                rgbFill
            , const tCIDLib::TBoolean       bShadow
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHorizontal
        //      Controls whether it's a horizontal or vertical line.
        //
        //  m_bThick
        //      The line can be either thin or thick. Thin is a single pixel
        //      wide and thick is 2.
        //
        //  m_eEndType
        //      Indicates the type of end that the line should have.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bHorizontal;
        tCIDLib::TBoolean       m_bThick;
        tCQCIntfEng::ELineEnds  m_eEndType;
        TPoint                  m_pntEnd;
        TPoint                  m_pntStart;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfLine,TCQCIntfWidget)
        DefPolyDup(TCQCIntfLine)
};

#pragma CIDLIB_POPPACK



