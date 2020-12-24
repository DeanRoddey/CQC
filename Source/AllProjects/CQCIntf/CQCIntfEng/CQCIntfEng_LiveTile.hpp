//
// FILE NAME: CQCIntfEng_LiveTile.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/19/2013
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
//  This is the header for the CQCIntfEng_Tile.cpp file, which implements the
//  TCQCIntfLiveTile class. This is a specialized derivative of the overlay class
//  which provides a non-interactive means of dynamically displaying one of a set
//  of available templates depending on the values of up to 2 fields evaluated by
//  pu to two expressions.
//
//  The contents within it cannot be interacted with and it can't be scrolled. It
//  itself just responds to a click with an OnClick event so that it can act as a
//  command button.
//
//  For each possible template that can loaded into it, the user defines one or
//  two expressions that evaluate the one or two fields (each expression is
//  associated with one of the fields.) They can also define a global variable and
//  a value to write to it whe that template is selected.
//
// CAVEATS/GOTCHAS:
//
//  1)  Our parent class still has its scrolling related events, but that's ok
//      since they will never get used. We won't ever scroll, and we override
//      the RTV generating method and just return a standard RTV object instead,
//      so none of the scrolling related RTVs are seen.
//
//  2)  By default the multi-field mixin class will accept any fields, which is
//      fine by us.
//
//  3)  We hook the high level (as opposed to widget level) hit testing method,
//      and just return ourself if the click is within our boundary, so that no
//      children ever get to see it. Because we say it's us, the parent container
//      class never recurses.
//
//  4)  In order to prevent the loading of an overlay to happen before we are
//      done initialization, we use an 'update ready' flag which is set to false
//      on init (and before we automatically load a new template.) Our PostInit
//      method will set to true. At that point, our value update callback will
//      start passing calls to the field mixin. It will then start calling us
//      back on the field change method, and then we could possibly load another
//      one.
//
//  5)  Because initialization happens again every time a template is loaded, we
//      have to distinguish between ours that only should happen once, and the
//      parent class' init. So we have another flag that is used to only do init
//      (primarily field registration) once. If we registered our fields every time
//      then we'd constantly cycle between not ready, then load, then we'd see not
//      ready because of re-registration, then a good value, and on and on.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfTemplate;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfOverlay
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfLiveTile

    : public TCQCIntfOverlay, public MCQCIntfMultiFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public data types
        //
        //  We need a small object to hold our per-template configuration info.
        // -------------------------------------------------------------------
        class CQCINTFENGEXPORT TTmplInfo
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TTmplInfo();

                TTmplInfo(const TTmplInfo& tiSrc);

                ~TTmplInfo();

                tCIDLib::TVoid operator=(const TTmplInfo& tiSrc);


                // -----------------------------------------------------------
                //  Public data members
                // -----------------------------------------------------------
                TCQCExpression          m_expr1;
                TCQCExpression          m_expr2;
                tCQCKit::ELogOps        m_eLogOp;

                TString                 m_strTemplate;
                TString                 m_strVarName;
                TString                 m_strVarVal;
        };
        using TTmplList = TVector<TTmplInfo>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4FmtSrcTmplsAttr
        (
            const   TTmplList&              colToFill
            ,       TMemBuf&                mbufTar
        );

        static tCIDLib::TVoid ParseSrcTmplsAttr
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            ,       TTmplList&              colToFill
        );



        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfLiveTile();

        TCQCIntfLiveTile
        (
            const   TCQCIntfLiveTile&       iwdgSrc
        );

        ~TCQCIntfLiveTile();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfLiveTile& operator=
        (
            const   TCQCIntfLiveTile&       iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   override;

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

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        TCQCIntfWidget* piwdgHit
        (
            const   TPoint&                 pntTest
            , const tCQCIntfEng::EHitFlags  eFlags = tCQCIntfEng::EHitFlags::Widgets
        )   override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TCard4         c4FldIndex
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoClickFeedback();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFldsRegistered
        //      We have to insure that we register our own fields only once after
        //      initialization. So we have to use a flag, because the field reg
        //      callback happens every time we load a new template.
        //
        //  m_bLastReady
        //      Any time we get a field change, we check to see if all of the fields
        //      have a good value. If not, we clear our template contents, else we
        //      evaluate the fields and see if we need to change. To avoid a lot of
        //      fiddling about, we keep the last ready/not ready state we saw so that
        //      we knwo if it changes.
        //
        //  m_bUpdateOK
        //      We have a bit of a race issue in that our field change guy will
        //      start getting field change updates before loading is complete. We
        //      can't allow it to actually load at that point. We have to wait until
        //      we are past the PostInit callback.
        //
        //  m_colTmpls
        //      A list of configured templates
        //
        //  m_strFld1
        //  m_strFld2
        //      The two fields we support. One or two of them may be enabled.
        //
        //  m_strCurTemplate
        //      The template we are currently displaying, so that we know if a new
        //      one has to be loaded.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFldsRegistered;
        tCIDLib::TBoolean   m_bLastReady;
        tCIDLib::TBoolean   m_bUpdateOK;
        TTmplList           m_colTmpls;
        TString             m_strFld1;
        TString             m_strFld2;
        TString             m_strCurTemplate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfLiveTile, TCQCIntfOverlay)
        DefPolyDup(TCQCIntfLiveTile)
};

#pragma CIDLIB_POPPACK


