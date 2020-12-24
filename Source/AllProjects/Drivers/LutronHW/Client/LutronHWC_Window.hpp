//
// FILE NAME: LutronHWC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This file implements the main window for the Lutron Homeworks client driver.
//  We basically maintain some lists, of dimmers, LEDs, buttons, ect..., and
//  allow the user to edit, add, and remove them.
//
//  Our server side driver, like any CML based drivers that have to share config with
//  a client side interface, uses a formatted string to exchange the config data. We
//  parse out the data we get from him and load a list of our item objects.
//
//  During actual editing, we don't bother keeping the item list updated. We just store
//  the data in the multi-column list box. When they want to save, we pull the data out
//  and create a new list, sorted in the same order as the original. We then compare
//  the two lists to see if there are changes.
//
//  We impose the following rules on the data:
//
//      1. Names are alpha numeric, no punctuation or spaces
//      2. We don't allow duplicate names
//      3. We don't allow duplicate Lutron type/id/number combos
//      4. We insure that the name is non-empty and that the address is at
//         least a syntactically viable Lutron id combination.
//
//  To make it easier to catch dups we store a Card8 user data value on each item
//  in the list which is in the form:
//
//      (eType << 56) | (Num << 48) | Addr
//
//  Where Addr is the encoded bytes of the address.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLutronHWCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TLutronHWCWnd : public TCQCDriverClient, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLutronHWCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TLutronHWCWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   override;

        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

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
        //  Private data types
        // -------------------------------------------------------------------
        typedef TVector<THWCItem>           TItemList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddNew();

        tCIDLib::TBoolean bGetCfgLine
        (
                    TTextInStream&          strmCfg
            ,       TString&                strToFill
            , const TString&                strExpPref
            ,       tCIDLib::TCard4&        c4LineNum
        );

        tCIDLib::TBoolean bLoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            ,       TItemList&              colToFill
            ,       TString&                strErr
            ,       tCIDLib::TCard4&        c4ErrLine
        );

        tCIDLib::TBoolean bSendChanges
        (
            const   TItemList&              colToSend
            ,       TString&                strErrMsg
        );

        tCIDLib::TBoolean bStoreItems
        (
                    TItemList&              colToFill
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard4&        c4ErrInd
        )   const;

        tCIDLib::TBoolean bTestDupAddr
        (
            const   tLutronHWC::EItemTypes  eType
            , const TString&                strAddr
            , const TString&                strNum
            ,       tCIDLib::TCard8&        c8Addr
            ,       tCIDLib::TCard4&        c4AddrBytes
            ,       tCIDLib::TCard1&        c1Num
            , const tCIDLib::TCard4         c4ListIndex
            ,       TString&                strErrMsg
        )   const;

        tCIDLib::TVoid DeleteAll();

        tCIDLib::TVoid DeleteCur();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colItems
        //      A collection of item objects that we parse out of the configuration
        //      data we get from the server. We sort it by type and then by name so that
        //      we have a known ordering for later checks for changes.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        // -------------------------------------------------------------------
        TItemList           m_colItems;
        TPushButton*        m_pwndAdd;
        TPushButton*        m_pwndDel;
        TPushButton*        m_pwndDelAll;
        TMultiColListBox*   m_pwndList;
        TStaticText*        m_pwndStatus;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLutronHWCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

