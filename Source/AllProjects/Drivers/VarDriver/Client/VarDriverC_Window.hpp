//
// FILE NAME: CQCHostMonC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This file implements the main window for the variable driver interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TVarDriverCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TVarDriverCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVarDriverCWnd() = delete;

        TVarDriverCWnd(const TVarDriverCWnd&) = delete;

        TVarDriverCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TVarDriverCWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TVarDriverCWnd& operator=(const TVarDriverCWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TVoid Connected() override;

        tCIDLib::TVoid DoCleanup() override;

        tCIDLib::TVoid LostConnection() override;

        tCIDLib::TVoid UpdateDisplayData() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   override;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid ImportXML();

        tCIDLib::TVoid LoadList();

        tCIDLib::TVoid UploadCfg
        (
            const   TVarDrvCfg&             cfgToUpload
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldSerialNum
        //      We poll the value of the selected field, so we need to keep
        //      a field serial number around. It gets set to zero when the
        //      selected field changes, then is used to know if the field
        //      value has changed (in the timer callback.)
        //
        //  m_cfgData
        //      Our copy of the driver configuration, which contains the
        //      current fields that have been defined for the instance of
        //      the server driver we are talking to.
        //
        //  m_eFldType
        //      The type of the currently selected field, if any. If not, it will
        //      be set to the _Count vlalue.
        //
        //  m_mbufPoll
        //      A memory buffer we use in our poll method, to avoid creating
        //      and destroying one constantly.
        //
        //  m_pwndXXX
        //      Some typed pointers to some of our widgets that we need to
        //      interact with.
        //
        //  m_strCurDefVal
        //  m_strCurVal
        //      Used in our polling, to store the polled values for the
        //      selected field. These will be picked up by the GUI thread
        //      update and displayed. The default value is updated directly
        //      into the field config as well, so that it'll be there if
        //      we come back to this field later. But it's stored here for
        //      the GUI update callback.
        //
        //  m_strToPoll
        //      The name of the field to poll. When the user moves the list
        //      box selection, this is updated with the name of that field.
        //      The bDoPoll() method will poll this field. If this string is
        //      empty, it does nothing.
        //
        //  m_tmFmt
        //      We special case the display of time field values, so we keep
        //      around a time object with the desired format string set on it.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldSerialNum;
        TVarDrvCfg          m_cfgData;
        tCQCKit::EFldTypes  m_eFldType;
        THeapBuf            m_mbufPoll;
        TStaticText*        m_pwndDataType;
        TPushButton*        m_pwndDefButton;
        TStaticText*        m_pwndDefValue;
        TPushButton*        m_pwndEditButton;
        TMultiColListBox*   m_pwndFldList;
        TPushButton*        m_pwndImpButton;
        TStaticMultiText*   m_pwndLimits;
        TStaticText*        m_pwndPersist;
        TPushButton*        m_pwndSetButton;
        TStaticMultiText*   m_pwndValue;
        TString             m_strCurDefVal;
        TString             m_strCurVal;
        TString             m_strToPoll;
        TTime               m_tmFmt;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVarDriverCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

