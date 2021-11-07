//
// FILE NAME: CQCIntfEng_TimeImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/10/2014
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
//  This is the header for the CQCIntfEng_TimeImg.cpp file, which implements
//  a widget that is specialized primarily for the display of analog clock hands
//  but could be used in various other ways. It is associated with a time, either
//  local time or a time field (we have variations for each and a base class that
//  just gets told what time to display.)
//
//  It can be configured to display either an hour image or a minute image or both.
//  Based on the current hour and/or minute, it selects an image from a given scope
//  with a specific pattern that includes the minute or hour number, so something
//  like:
//
//      \User\MyClock\Hands\Min_%(n, 2, 0)
//      \User\MyClock\Hands\Hour_%(n)
//
//  The first would look for an image in the form, Min_02 or Min_53. The latter
//  would not assume any leading zeros, so Hour_0 or Hour_10.
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
//   CLASS: TCQCIntfTimeImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfTimeImgBase : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCIntfTimeImgBase() = delete;

        ~TCQCIntfTimeImgBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUseHourImg() const;

        tCIDLib::TBoolean bUseHourImg
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard1 c1Opacity() const;

        tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TBoolean bUseMinImg() const;

        tCIDLib::TBoolean bUseMinImg
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TString& strNPatHour() const;

        const TString& strNPatHour
        (
            const   TString&                strToSet
        );

        const TString& strPathHour() const;

        const TString& strPathHour
        (
            const   TString&                strToSet
        );

        const TString& strNPatMin() const;

        const TString& strNPatMin
        (
            const   TString&                strToSet
        );

        const TString& strPathMin() const;

        const TString& strPathMin
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfTimeImgBase
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeStr
        );

        TCQCIntfTimeImgBase
        (
            const   TCQCIntfTimeImgBase&    iwdgToCopy
        );

        TCQCIntfTimeImgBase& operator=
        (
            const   TCQCIntfTimeImgBase&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetTime
        (
            const   tCIDLib::TEncodedTime   enctNew
            , const tCIDLib::TBoolean       bNoRedraw
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadImg
        (
            const   tCIDLib::TCard4         c4Number
            , const tCIDLib::TBoolean       bHour
        );

        tCIDLib::TVoid LoadImg
        (
                    TDataSrvClient&         dsclInit
            , const tCIDLib::TCard4         c4Number
            , const tCIDLib::TBoolean       bHour
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseHourImg
        //  m_bUseMinImg
        //      Indicates which of the images we are configured to use.
        //
        //  m_c1Opacity
        //      The constant opacity for the images, which we let the user adjust.
        //
        //  m_c4LastHour
        //  m_c4LastMin
        //      The last hour and minute we were given. These allow us to only update
        //      if they change. We initialize them to 99, which is beyond any legal
        //      value and guaranteed to make us transition as soon as we get one.
        //
        //  m_cptrHourImg
        //  m_cptrMinImg
        //      The are the current hour and minute images, i.e. the last one that
        //      we loaded based on the incoming time the derived class gave us.
        //
        //  m_strNPatHour
        //  m_strNPatMin
        //      The patterns that we use to build up the actual image name based on
        //      the hour or minute number.
        //
        //  m_strPathHour
        //  m_strPathMin
        //      The image repo path in which we look for our our and minuute images.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bUseHourImg;
        tCIDLib::TBoolean   m_bUseMinImg;
        tCIDLib::TCard1     m_c1Opacity;
        tCIDLib::TCard4     m_c4LastHour;
        tCIDLib::TCard4     m_c4LastMin;
        TIntfImgCachePtr    m_cptrHourImg;
        TIntfImgCachePtr    m_cptrMinImg;
        TString             m_strNPatHour;
        TString             m_strNPatMin;
        TString             m_strPathHour;
        TString             m_strPathMin;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTimeImgBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldTimeImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldTimeImg

    : public TCQCIntfTimeImgBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldTimeImg();

        TCQCIntfFldTimeImg
        (
            const   TCQCIntfFldTimeImg&     iwdgToCopy
        );

        ~TCQCIntfFldTimeImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldTimeImg& operator=
        (
            const   TCQCIntfFldTimeImg&     iwdgToAssign
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
        )   final;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFldTimeImg,TCQCIntfTimeImgBase)
        DefPolyDup(TCQCIntfFldTimeImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLocalTimeImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfLocalTimeImg : public TCQCIntfTimeImgBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfLocalTimeImg();

        TCQCIntfLocalTimeImg
        (
            const   TCQCIntfLocalTimeImg&   iwdgSrc
        );

        ~TCQCIntfLocalTimeImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfLocalTimeImg& operator=
        (
            const   TCQCIntfLocalTimeImg&   iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const final;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   final;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   final;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const final;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   final;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_enctNextUpdate
        //      Since the parent only needs to update once a minute, we avoid alot
        //      of extra work by just remembering when the next minute will be and
        //      only calling our parent when we go past it.
        // -------------------------------------------------------------------
        tCIDLib::TEncodedTime   m_enctNextUpdate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfLocalTimeImg,TCQCIntfTimeImgBase)
        DefPolyDup(TCQCIntfLocalTimeImg)
};


#pragma CIDLIB_POPPACK


