//
// FILE NAME: CQCIntfEng_WeatherChImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2004
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
//  This is the header for the CQCIntfEng_WeatherChImg.cpp file, which
//  implements a widget that is specialized for display of the images that
//  map to the status values in the Weather Channel's XML data feed. They
//  provide (for current and forecast days) a status value that indicates
//  things like cloudy, clear, overcast, windy, etc... and they provide a
//  set of images that map to those values. We know that those images are
//  in the image repository at a given place, so we can automatically display
//  the right image based on the value of a field that reports that
//  information. The field is numeric and the images are named 1, 2, 3, etc...
//  so we just take the known repository scope and add the formatted value
//  of the image.
//
//  The driver will set the field to 999 if the value is not available, in
//  which case we show a 'not applicable' image.
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
//   CLASS: TCQCIntfWeatherChImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWeatherChImg

    : public TCQCIntfSpecialImg, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfWeatherChImg();

        TCQCIntfWeatherChImg
        (
            const   TCQCIntfWeatherChImg&   iwdgToCopy
        );

        ~TCQCIntfWeatherChImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfWeatherChImg& operator=
        (
            const   TCQCIntfWeatherChImg&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        TBitmap bmpSample() const override;

        tCIDLib::TCard1 c1Opacity() const override;

        tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
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
        tCIDLib::TVoid BuildImgName
        (
            const   tCIDLib::TCard4         c4Value
            , const TSize&                  szToUse
            ,       TString&                strName
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cptrImg
        //      When the value changes, we load up the image we need.
        //
        //  m_c1Opacity
        //      Constant opacity set for our widget. This is set via a
        //      configuration tab.
        // -------------------------------------------------------------------
        TIntfImgCachePtr    m_cptrImg;
        tCIDLib::TCard1     m_c1Opacity;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfWeatherChImg,TCQCIntfSpecialImg)
        DefPolyDup(TCQCIntfWeatherChImg)
};

#pragma CIDLIB_POPPACK


