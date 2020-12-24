//
// FILE NAME: CQCIntfEng_DigitalClock.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/06/2002
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
//  This is the header for the CQCIntfEng_DigitalClock.cpp file, which
//  implements a simple derivative of the base text widget. Each derivative
//  just has to provide the text to display, and the parent class does the
//  display work. In our case, we just provide it with a fixed string that
//  we are set up to display.
//
//  We also define our configuration tab windwo here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfDigitalClockTab;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfDigitalClock
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfDigitalClock

    : public TCQCIntfWidget, public MCQCIntfFontIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfDigitalClock();

        TCQCIntfDigitalClock
        (
            const   TCQCIntfDigitalClock&     iwdgToCopy
        );

        ~TCQCIntfDigitalClock();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfDigitalClock& operator=
        (
            const   TCQCIntfDigitalClock&     iwdgToAssign
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
            ,       tCQCIntfEng::TErrList&  colErrs
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
        tCIDLib::TVoid Set
        (
            const   tCQCIntfEng::EClockFmts eFmt
            , const tCIDLib::TBoolean       bRawText
            , const tCIDLib::TBoolean       bShowBorders
            , const tCIDLib::TBoolean       bShowPanels
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TCQCIntfDigitalClockTab;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaUpdate
            , const tCIDLib::TBoolean       bDrawBorder
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRawText
        //      Indicates whether they just want the raw text display as a
        //      string, or if they want to use the spaced display. If this
        //      is set, then the show borders/panels settings are ignored.
        //
        //  m_bShowBorders
        //  m_bShowPanels
        //      Indicates whehter we draw the panels around each number or not,
        //      and whether we draw the border around the panels. These are
        //      only used if the raw text flag is not set.
        //
        //  m_eFmt
        //      The 12/24 hour clock.
        //
        //  m_tmValue
        //      Just used at runtime, to update each time we update and get
        //      the current time and format.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bRawText;
        tCIDLib::TBoolean       m_bShowBorders;
        tCIDLib::TBoolean       m_bShowPanels;
        tCQCIntfEng::EClockFmts m_eFmt;
        TTime                   m_tmValue;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfDigitalClock,TCQCIntfWidget)
        DefPolyDup(TCQCIntfDigitalClock)
};

#pragma CIDLIB_POPPACK



