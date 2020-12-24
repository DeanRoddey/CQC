//
// FILE NAME: CQCKit_DevClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/27/2014
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
//  This is the header for the CQCDrvDev_DevClass.cpp file, which implements
//  the TCQCDevClass class. It's used to store device class info we download
//  from the data server. These are used to validate V2 compliance. When a new
//  session is opened, we see what device classes the new driver uses and pull
//  those down and store them if we don't have them already.
//
//  We also implement a small class that holds the field info. We could almost
//  use the TCQCFldDef class, but it's not quite right for our needs, particularly
//  wrt to field access. We need the allowable types of access, whereas the field
//  def holds teh actual field access type. We keep the field class internal since
//  it's not required by users of the device class interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCDevClFld;


// ---------------------------------------------------------------------------
//  CLASS: TCQCDevClass
// PREFIX: dvcls
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDevClass : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Public types
        //
        //  Fixed   - The names are defined by the device class
        //  Free    - No fixed names, no pattern, whatever the user wants for
        //              suffix part of the name. There will be a single field
        //              description and all must match it except for the name
        //              which is ignored.
        //  Pattern - Pattern based. Every field must match one of the field
        //              name patterns, and then the rest of its settings. All
        //              must be pattern based.
        // -------------------------------------------------------------------
        enum class ENameTypes
        {
            Fixed
            , Free
            , Pattern

            , Count
        };


        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString strDTDText;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4LoadAIOFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strIOName
            , const tCIDLib::TBoolean       bInput
            , const tCIDLib::TBoolean       bFloat
            , const TString&                strLim
        );

        static tCIDLib::TCard4 c4LoadAudioFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
        );

        static tCIDLib::TCard4 c4LoadAVProcFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
            , const TString&                strCurLim
            , const TString&                strSetLim
            , const tCIDLib::TBoolean       bSetWriteAlways
        );

        static tCIDLib::TCard4 c4LoadDevInfoFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
        );

        static tCIDLib::TCard4 c4LoadDIOFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strIOName
            , const tCIDLib::TBoolean       bInput
        );

        static tCIDLib::TCard4 c4LoadLightFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strLightName
            , const tCIDLib::TBoolean       bDimmer
            , const tCIDLib::TBoolean       bSwitch
        );

        static tCIDLib::TCard4 c4LoadMTransFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
        );

        static tCIDLib::TCard4 c4LoadNowPlayingFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
        );

        static tCIDLib::TCard4 c4LoadPowerFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
        );

        static tCIDLib::TCard4 c4LoadProjFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
            , const TString&                strARLim
            , const tCIDLib::TBoolean       bARWriteAlways
        );

        static tCIDLib::TCard4 c4LoadSecAreaFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strAreaName
            , const TString&                strArmLim
        );

        static tCIDLib::TCard4 c4LoadSecZoneFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strZoneName
            , const tCIDLib::TBoolean       bIsMotion
        );

        static tCIDLib::TCard4 c4LoadSwitcherFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
            , const TString&                strCurSrcLim
            , const TString&                strSetSrcLim
            , const tCIDLib::TBoolean       bSetWriteAlways
        );

        static tCIDLib::TCard4 c4LoadThermoFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
            , const TString&                strTempLim
            , const TString&                strHSPLim
            , const TString&                strLWPLim
            , const TString&                strFanLim
            , const TString&                strFanOpLim
            , const TString&                strModeLim
            , const TString&                strOpModeLim
        );

        static tCIDLib::TCard4 c4LoadTunerFlds
        (
                    tCQCKit::TFldDefList&   colToFill
            , const TString&                strSubUnit
            , const TString&                strPreLim
            , const tCIDLib::TBoolean       bWritePreset
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDevClass();

        TCQCDevClass
        (
            const   TCQCDevClass&           dvclsSrc
        );

        ~TCQCDevClass();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDevClass& operator=
        (
            const   TCQCDevClass&           dvclsSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMultiUnit() const;

        tCIDLib::TBoolean bNeedsPower() const;

        tCIDLib::TBoolean bParseFrom
        (
            const   tCQCKit::EDevClasses    eClass
            , const TMemBuf&                mbufSrc
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TBoolean bValidateFlds
        (
                    tCQCKit::TFldDefList&   colFields
            ,       tCIDLib::TStrCollect&   colErrors
        )   const;

        tCIDLib::TCard4 c4FmtVersion() const;

        tCQCKit::EDevClasses eDevClass() const;

        ENameTypes eNameType() const;

        const TString& strClassDesc() const;

        const TString& strClassName() const;

        const TString& strClassPref() const;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        //
        //  We make this list a ref list, so that we can keep our field class
        //  internal and unexported. Anything that reduces downstream rebuilds is
        //  good.
        // -------------------------------------------------------------------
        using TFldDefSet = TRefKeyedHashSet<TCQCFldDef, TString, TStringKeyOps>;
        using TFldList = TRefVector<TCQCDevClFld>;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckField
        (
            const   TCQCDevClFld&           dvclsfcCur
            , const TCQCFldDef&             flddTest
            ,       tCIDLib::TStrCollect&   colErrors
            ,       TString&                strError
            , const tCIDLib::TBoolean       bReportFail
        )   const;

        tCIDLib::TBoolean bCheckSubUnit
        (
            const   TString&                strCheck
            ,       tCIDLib::TStrCollect&   colErrors
        )   const;



        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bMultiUnit
        //      Indicates if multi-unit naming is allowed. It almost always is but
        //      just in case.
        //
        //  m_bNeedsPower
        //      Some device classes require that the Power device class also be
        //      implemented. We store that flag here.
        //
        //  m_c4FmtVersion
        //      The format version of the device class, in case we should change
        //      it in the future. For now it's always 1, which is what we support.
        //
        //  m_colOurFlds
        //      We find all the fields related to our device class in the incoming list
        //      and store them here, so that we have a list of just our own. It's a
        //      hash set keyed on the field name, so it also lets us quickly find one
        //      and insure that fields exist.
        //
        //      This one has to be mutable so we can reload it in the validation
        //      call.
        //
        //  m_eNameType
        //      The naming scheme used by this device class.
        //
        //  m_pcolFldList
        //      The defintion for the fields required by this class. It's by ref and
        //      a pointer to it, so that we can hide the field class internally.
        //
        //  m_eDevClass
        //      The device class that this object represented.
        //
        //  m_strClassDescr
        //  m_strClassName
        //  m_strClassPref
        //      The basic class info from the CML file.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bMultiUnit;
        tCIDLib::TBoolean       m_bNeedsPower;
        tCIDLib::TCard4         m_c4FmtVersion;
        mutable TFldDefSet      m_colOurFlds;
        tCQCKit::EDevClasses    m_eDevClass;
        ENameTypes              m_eNameType;
        TFldList*               m_pcolFldList;
        TString                 m_strClassDesc;
        TString                 m_strClassName;
        TString                 m_strClassPref;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDevClass,TObject)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCV2Validator
// PREFIX: cv2v
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCV2Validator : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCV2Validator();

        TCQCV2Validator(const TCQCV2Validator&) = delete;

        ~TCQCV2Validator();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCV2Validator& operator=(const TCQCV2Validator&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoV2Validation
        (
            const   TCQCDriverCfg&          cqcdcTest
            , const tCQCKit::TFldDefList&   colFields
            ,       tCIDLib::TStrCollect&   colErrors
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FindDevClass
        (
            const   tCQCKit::EDevClasses    eClass
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colDevClasses
        //      The list of device classes we have downloaded so far.
        // -------------------------------------------------------------------
        TVector<TCQCDevClass>   m_colDevClasses;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCV2Validator,TObject)
};

#pragma CIDLIB_POPPACK

