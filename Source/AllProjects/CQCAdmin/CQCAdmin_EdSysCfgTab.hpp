//
// FILE NAME: CQCAdmin_EdSysCfgTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/05/2016
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
//  This is the header for the CQCClient_EdSysCfgTab.cpp file, which implements a
//  tab window that just contains a scroll area window, within which is a simple
//  static image window. This is used for read-only viewing of image.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TESCGlobalTab;
class TESCRoomsTab;

// ---------------------------------------------------------------------------
//   CLASS: TValErrInfo
//  PREFIX: verr
// ---------------------------------------------------------------------------
class TValErrInfo
{
    public :
        // -------------------------------------------------------------------
        //  Public classa types
        // -------------------------------------------------------------------
        typedef TVector<TValErrInfo>        TErrList;
        enum class ETypes
        {
            General
            , Room
            , RoomCfg
        };


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TValErrInfo(const   TCQCSysCfgRmInfo&   scriSrc
                    , const tCIDLib::TCard4     c4TabId
                    , const tCIDCtrls::TWndId   widFocus
                    , const TString&            strErr
                    , const tCIDLib::TCard4     c4Index) :

            m_c4Index(c4Index)
            , m_c4TabId(c4TabId)
            , m_eType(ETypes::Room)
            , m_strErr(strErr)
            , m_strName(scriSrc.strName())
            , m_strUniqueId(scriSrc.strUniqueId())
            , m_widFocus(widFocus)
        {
        }

        TValErrInfo(const   tCIDCtrls::TWndId   widFocus
                    , const TString&            strErr
                    , const tCIDLib::TCard4     c4Index = 0
                    , const ETypes              eType = ETypes::General) :

            m_c4Index(c4Index)
            , m_c4TabId(kCIDLib::c4MaxCard)
            , m_eType(eType)
            , m_strErr(strErr)
            , m_strName(L"General Config")
            , m_widFocus(widFocus)
        {
        }

        TValErrInfo(const TValErrInfo&) = default;
        TValErrInfo(TValErrInfo&&) = default;

        ~TValErrInfo() = default;

        TValErrInfo& operator=(const TValErrInfo&) = default;
        TValErrInfo& operator=(TValErrInfo&&) = default;

        tCIDLib::TVoid FormatSepText
        (
                    TString&    strToFill
        )   const;

        tCIDLib::TCard4     m_c4Index;
        tCIDLib::TCard4     m_c4TabId;
        ETypes              m_eType;
        TString             m_strErr;
        TString             m_strName;
        TString             m_strUniqueId;
        tCIDCtrls::TWndId   m_widFocus;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTabBase
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTabBase : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Public constructors and destructor
        // -------------------------------------------------------------------
        TCQCESCTabBase() = delete;
        TCQCESCTabBase(const TCQCESCTabBase&) = delete;
        TCQCESCTabBase(TCQCESCTabBase&&) = delete;

        ~TCQCESCTabBase();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTabBase& operator=(const TCQCESCTabBase&) = delete;
        TCQCESCTabBase& operator=(TCQCESCTabBase&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid DisableCtrls();

        virtual tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        );

        virtual tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        ) = 0;

        virtual tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TCQCSysCfgRmInfo& scriRoom();

        const TCQCSysCfgRmInfo& scriRoom() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCESCTabBase
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
            , const TString&                strTabId
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddError
        (
            const   TCQCSysCfgRmInfo&       scriSrc
            ,       TValErrInfo::TErrList&  colErrs
            , const tCIDLib::TMsgId         midErr
            , const tCIDCtrls::TWndId       widFocus = 0
            , const tCIDLib::TCard4         c4Index = kCIDLib::c4MaxCard
        );

        tCIDLib::TVoid AddError
        (
            const   TCQCSysCfgRmInfo&       scriSrc
            ,       TValErrInfo::TErrList&  colErrs
            , const tCIDLib::TMsgId         midErr
            , const MFormattable&           mfmtbToken1
            , const tCIDCtrls::TWndId       widFocus = 0
            , const tCIDLib::TCard4         c4Index = kCIDLib::c4MaxCard
        );

        tCIDLib::TVoid AddError
        (
            const   TCQCSysCfgRmInfo&       scriSrc
            ,       TValErrInfo::TErrList&  colErrs
            , const tCIDLib::TMsgId         midErr
            , const MFormattable&           mfmtbToken1
            , const MFormattable&           mfmtbToken2
            , const tCIDCtrls::TWndId       widFocus = 0
            , const tCIDLib::TCard4         c4Index = kCIDLib::c4MaxCard
        );

        tCIDLib::TBoolean bFuncEnabled
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
        )   const;

        TCQCFldCache& cfcDevs();

        TCQCSysCfg& scfgEdit();

        const TString& strFuncId
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
        )   const;

        const TString& strFuncTitle
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcfcDevs
        //  m_pscfgEdit
        //      We get a pointer to the system configuration that is being edited, and a
        //      field cache for any field info stuff we need to do.
        //
        //  m_strRoomUID
        //      The unique id of the room that we are currently editing. If there's
        //      no selected room, then it's empty.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4TabIndex;
        TCQCFldCache*   m_pcfcDevs;
        TCQCSysCfg*     m_pscfgEdit;
        TString         m_strRoomUID;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTabBase,TTabWindow)
};



// ---------------------------------------------------------------------------
//  CLASS: TEdSysCfgTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TEdSysCfgTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        enum class EAddOpts
        {
            AddToNone
            , AddToAll
            , AddToExisting
        };


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TEdSysCfgTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TEdSysCfgTab(const TEdSysCfgTab&) = delete;
        TEdSysCfgTab(TEdSysCfgTab&&) = delete;

        ~TEdSysCfgTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSysCfgTab& operator=(const TEdSysCfgTab&) = delete;
        TEdSysCfgTab& operator=(TEdSysCfgTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TCQCSysCfg&             scfgEdit
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;



    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoValidation
        (
            const   tCIDLib::TBoolean       bNoPassNot
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cfcDevs
        //      Quite a bit of device/field attribute querying is done here, so we create
        //      a field cache and make it available to all of the tabs so that we aren't
        //      constantly banging the server for info.
        //
        //  m_pwndXXX
        //      Some of our controls we want to interact with in a typed way. We don't
        //      own them.
        //
        //  m_scfgEdit
        //      Our copy of the system configuration data that we are editing.
        //
        //  m_scfgOrg
        //      A copy of the data so that we can test for changes
        // -------------------------------------------------------------------
        TCQCFldCache    m_cfcDevs;
        TPushButton*    m_pwndGenerate;
        TESCGlobalTab*  m_pwndGlobTab;
        TPushButton*    m_pwndRevert;
        TESCRoomsTab*   m_pwndRoomsTab;
        TTabbedWnd*     m_pwndTabs;
        TPushButton*    m_pwndValidate;
        TCQCSysCfg      m_scfgEdit;
        TCQCSysCfg      m_scfgOrg;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSysCfgTab, TContentTab)
};

