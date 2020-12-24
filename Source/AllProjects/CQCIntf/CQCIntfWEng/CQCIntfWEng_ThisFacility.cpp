//
// FILE NAME: CQCIntfEng_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This is the implementation file for the facility class. We have to
//  bringin some of the widget specific headers that are usually only
//  included into their own cpp files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfWEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIntfWEng,TGUIFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfEng
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TInt4 TFacCQCIntfWEng::i4ErrInvokeCode()
{
    return s_i4ErrInvokeCode;
}


tCIDLib::TInt4 TFacCQCIntfWEng::i4ErrSelCode()
{
    return s_i4ErrSelCode;
}


// Let's the containing app give us some ids that we'll use in posts back
tCIDLib::TVoid
TFacCQCIntfWEng::SetErrCBCodes( const   tCIDLib::TInt4  i4ErrSelId
                                , const tCIDLib::TInt4  i4ErrInvokeId)
{
    s_i4ErrInvokeCode = i4ErrInvokeId;
    s_i4ErrSelCode = i4ErrSelId;
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCIntfWEng::TFacCQCIntfWEng() :

    TGUIFacility
    (
        L"CQCIntfWEng"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_bNoInput(kCIDLib::False)
{
}

TFacCQCIntfWEng::~TFacCQCIntfWEng()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfWEng: Constructors and Destructor
// ---------------------------------------------------------------------------

// Get/set the no input mode flag
tCIDLib::TBoolean TFacCQCIntfWEng::bNoInputMode() const
{
    return m_bNoInput;
}

tCIDLib::TBoolean TFacCQCIntfWEng::bNoInputMode(const tCIDLib::TBoolean bToSet)
{
    m_bNoInput = bToSet;
    return m_bNoInput;
}


tCIDLib::TBoolean
TFacCQCIntfWEng::bSaveTemplate( const   TCQCIntfTemplate&   iwdgToSave
                                , const TWindow&            wndOwner
                                , const TString&            strTitle
                                , const tCIDLib::TBoolean   bCheckForOverwrite
                                , const TCQCUserCtx&        cuctxToUse)
{
    try
    {
        // Get an data server client proxy
        TDataSrvClient dsclSave;

        // It must have been assigned a name before saving
        if (iwdgToSave.strTemplateName().bIsEmpty())
        {
            TErrBox msgbErr
            (
                strTitle, facCQCIntfEng().strMsg(kIEngMsgs::midStatus_NoTmplName)
            );
            msgbErr.ShowIt(wndOwner);
            return kCIDLib::False;
        }

        // See if this name exists, and confirm overwrite if so
        if (bCheckForOverwrite)
        {
            if (dsclSave.bFileExists(iwdgToSave.strTemplateName()
                                    , tCQCRemBrws::EDTypes::Template))
            {
                TYesNoBox msgbAsk
                (
                    strTitle
                    , facCQCIntfEng().strMsg
                      (
                        kIEngMsgs::midQ_OverwriteTmpl, iwdgToSave.strTemplateName()
                      )
                );
                if (!msgbAsk.bShowIt(wndOwner))
                    return kCIDLib::False;
            }
        }

        // Flatten this guy to a memory stream
        THeapBuf mbufData(64 * 1024UL);
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << iwdgToSave << kCIDLib::FlushIt;

        tCIDLib::TCard4         c4NewSerNum;
        tCIDLib::TEncodedTime   enctLastChange;
        tCIDLib::TKVPFList      colXMeta;
        dsclSave.WriteTemplate
        (
            iwdgToSave.strTemplateName()
            , c4NewSerNum
            , enctLastChange
            , mbufData
            , strmOut.c4CurSize()
            , iwdgToSave.eMinRole()
            , iwdgToSave.strNotes()
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , cuctxToUse.sectUser()
        );
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfWEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Oh well, didn't work. Display the error to the user
        TExceptDlg dlgbFail
        (
            wndOwner
            , strTitle
            , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_TmplWriteFailed)
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



TCQCIntfViewWnd* TFacCQCIntfWEng::
pwndMakeNewInterface(       TWindow&                    wndParent
                    , const TCQCIntfTemplate&           iwdgTemplate
                    , const tCIDCtrls::TWndId           widToUse
                    ,       TDataSrvClient&             dsclToUse
                    , const TString&                    strTitle
                    ,       MCQCIntfAppHandler* const   pmiahToUse
                    , const tCIDLib::TBoolean           bSuppressWarnings
                    , const TCQCUserCtx&                cuctxToUse)
{
    // Set the mouse to wait mode while we do this
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

    //
    //  Create a new interface window. Put a janitor on it, so that it gets
    //  cleaned up if we bail out of here with a failure. It is invisible,
    //  so we won't get any paints until we get the data ready.
    //
    TCQCIntfViewWnd* pwndNew = new TCQCIntfViewWnd();
    TWndJanitor<TCQCIntfViewWnd> janClient(pwndNew);

    // Now create the window. It's still invisible at this point
    TArea areaClient;
    wndParent.QueryClientArea(areaClient, kCIDLib::True);
    pwndNew->Create(wndParent, widToUse, areaClient);

    //
    //  Tell the window to do the set up now. It will initialize the widgets,
    //  register any fields with the polling engine, start up the value scan
    //  timer, run on preload and on load actions, etc...
    //
    tCQCIntfEng::TErrList colErrs;
    try
    {
        pwndNew->bInitialize
        (
            wndParent
            , dsclToUse
            , colErrs
            , strTitle
            , pmiahToUse
            , iwdgTemplate
            , cuctxToUse
        );
    }

    catch(TError& errToCatch)
    {
        //
        //  It failed, so destroy the window and don't set it. We keep the
        //  old one.
        //
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        try
        {
            pwndNew->Destroy();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete pwndNew;
        return 0;
    }

    // Give the interface window the focus
    pwndNew->TakeFocus();

    // Get the new template interface
    TCQCIntfTemplate& iwdgNew = pwndNew->civTarget().iwdgBaseTmpl();

    // Make it visible now
    pwndNew->SetVisibility(kCIDLib::True);

    //
    //  If there were validation errors, and they aren't supressed, show
    //  them. We pass a zero for the report window, since we never do
    //  editing type validation display for an on load type of scenario.
    //  Only the designer does that during an explicit validation pass.
    //
    if (!colErrs.bIsEmpty() && !bSuppressWarnings)
        ShowErrors(wndParent, 0, colErrs, iwdgNew.strTemplateName());

    return janClient.pwndOrphan();
}


//
//  A helper to display validation errors that are returned from a validation
//  pass or on loading of a new template.
//
tCIDLib::TVoid
TFacCQCIntfWEng::ShowErrors(const   TWindow&                wndParent
                            ,       TWindow* const          pwndReport
                            , const tCQCIntfEng::TErrList&  colErrs
                            , const TString&                strTmplName)
{
    if (!TFacCQCIntfEng::bDesMode()
    &&  !TFacCQCIntfWEng::i4ErrInvokeCode()
    &&  !TFacCQCIntfWEng::i4ErrSelCode())
    {
        return;
    }

    //
    //  Display the error window to let the user view or edit the
    //  errors.
    //
    TCQCValErrDlg dlgErr;
    dlgErr.RunDlg(wndParent, pwndReport, colErrs, strTmplName);
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfWEng: Private, static data members
// ---------------------------------------------------------------------------
tCIDLib::TInt4      TFacCQCIntfWEng::s_i4ErrSelCode(0);
tCIDLib::TInt4      TFacCQCIntfWEng::s_i4ErrInvokeCode(0);

