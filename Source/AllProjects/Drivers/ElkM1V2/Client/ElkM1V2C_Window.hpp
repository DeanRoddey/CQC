//
// FILE NAME: ElkM1V2C_Window_.hpp
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
//  This file implements the main window for the Elk M1 V2 client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TElkM1V2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TElkM1V2CWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tElkM1V2C::EBaseValRes eCheckBaseVals
        (
            const   TWindow&                wndOwner
            ,       tElkM1V2C::TItemList&   colList
            , const TString&                strName
            , const TString&                strElkId
            , const tCIDLib::TCard4         c4Index
            , const tElkM1V2C::EItemTypes   eType
            ,       tCIDLib::TCard4&        c4ElkId
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TElkM1V2CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TElkM1V2CWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        ) override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bGetConnectData
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
        tCIDLib::TVoid AddNew();

        tCIDLib::TBoolean bCheckForChanges();

        tCIDLib::TVoid CheckStart
        (
                    TTextInStream&          strmSrc
            , const TString&                strExp
        );

        tCIDLib::TVoid ClearAllData();

        tCIDLib::TVoid ClearMods();

        tCIDLib::TVoid DeleteCurrent();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditCurrent();

        tCIDLib::TVoid GetNextLine
        (
                    TTextInStream&          strmSrc
            ,       TString&                strToFill
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
        );

        tCIDLib::TVoid LoadItemList
        (
            const   tElkM1V2C::EItemTypes   eType
        );

        tCIDLib::TVoid ParseBlock
        (
                    TTextInStream&          strmSrc
            , const tElkM1V2C::EItemTypes   eType
            , const TString&                strType
            ,       TItemInfo&              iiTemp
        );

        tCIDLib::TVoid SendChanges();

        tCIDLib::TVoid StoreMods();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colLists
        //  m_colModLists
        //      For each type of configurable item we need to keep track of the
        //      list of info objects for that type. We use an object array of
        //      vectors. Each array entry maps to one of the EItemTypes enum
        //      values. The individual lists are ref vectors of the base item
        //      info class.
        //
        //      We also keep another set which are the ones we actually work on
        //      during editing. The differences between them and the originals
        //      indicate what has been modified. And we can go back to the previous
        //      settings. Upon save, we move the mods to the other list again.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        //
        //
        //  m_c4LineNum
        //  m_xxxTmp
        //      These are temps to use during configuration parsing.
        // -------------------------------------------------------------------
        tElkM1V2C::TListList        m_colLists;
        tElkM1V2C::TListList        m_colModLists;

        TPushButton*                m_pwndAddButton;
        TPushButton*                m_pwndClearButton;
        TPushButton*                m_pwndDelButton;
        TPushButton*                m_pwndEditButton;
        TMultiColListBox*           m_pwndItemList;
        TPushButton*                m_pwndSendButton;
        TStaticMultiText*           m_pwndSettings;
        TMultiColListBox*           m_pwndTypeList;

        TStreamFmt                  m_sfmtSettingsL;
        TStreamFmt                  m_sfmtSettingsR;

        tCIDLib::TCard4             m_c4LineNum;
        tCIDLib::TStrList           m_colTokens;
        TString                     m_strTmp;
        TString                     m_strTmp2;
        TStringTokenizer            m_stokTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TElkM1V2CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

