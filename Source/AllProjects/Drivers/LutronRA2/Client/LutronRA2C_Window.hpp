//
// FILE NAME: LutronRA2C_Window_.hpp
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
//  This file implements the main window for the Lutron Radio RA2 client driver.
//  We basically maintain some lists, of dimmers, LEDs, buttons, ect..., and
//  allow the user to edit, add, remove, items on these lists.
//
//  To avoid redundant storage of collections driving list boxes, we just use
//  one of the templatized list window item classes to hold the items directly
//  in the list boxes. The LutronRA2C_Item files implement the small item
//  class we use to represent the items in our lists. THey have mostly the
//  same information.
//
//  Our server side driver, like any CML based drivers that have to share
//  config with a client side interface, uses a formatted string to exchange
//  the configuration data. We parse out the data we get from him and fill
//  our lists. We let the user edit as desired and if they press the Send
//  button, we format our edited stuff back out into the same format and
//  send it to the server side driver.
//
//  We impose the following rules on the data:
//
//      1. Names are alpha numeric, no punctuation or spaces
//      2. We don't allow duplicate names
//      3. We don't allow duplicate Lutron integration/component ids
//      4. We insure that the name is non-empty and that the address is at
//         least a viable Lutron id combination.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLutronRA2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TLutronRA2CWnd : public TCQCDriverClient, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLutronRA2CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TLutronRA2CWnd();


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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   final;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   final;

        tCIDLib::TVoid Connected() final;

        tCIDLib::TVoid DoCleanup() final;

        tCIDLib::TVoid LostConnection() final;

        tCIDLib::TVoid UpdateDisplayData() final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TItemList = TVector<TRA2CItem>;


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

        tCIDLib::TVoid DeleteAll();

        tCIDLib::TVoid DeleteCur();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid StoreItems
        (
                    TItemList&              colToFill
        )   const;



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

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLutronRA2CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

