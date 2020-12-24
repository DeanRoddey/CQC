//
// FILE NAME: CQCTreeBrws_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_RemBrowseDlg_.hpp"
#include    "CQCTreeBrws_DrvWizDlg_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCTreeBrws,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTreeBrws
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCTreeBrws: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCTreeBrws::TFacCQCTreeBrws() :

    TGUIFacility
    (
        L"CQCTreeBrws"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
{
}

TFacCQCTreeBrws::~TFacCQCTreeBrws()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCTreeBrws: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Invokes the driver configuration wizard dialog.
//
tCIDLib::TBoolean
TFacCQCTreeBrws::bDriverWizard( const   TWindow&                    wndOwner
                                , const TString&                    strHost
                                ,       TCQCDriverObjCfg&           cqcdcEdit
                                , const tCIDLib::TBoolean           bReconfigMode
                                , const tCIDLib::TStrList&          colMonikers
                                , const tCIDLib::TStrList* const    pcolPorts
                                , const TCQCSecToken&               sectUser)
{
    TCQCDrvWizDlg dlgWiz(strHost, bReconfigMode, sectUser);
    return dlgWiz.bRun(wndOwner, cqcdcEdit, colMonikers, pcolPorts);
}


//
//  A wrapper around the browser dialog, so that we don't have to expose it to the outside
//  world. This only really works on the data server part of the hierarchy. They pass in a
//  type relative path, which we pass on to the dialog. We get back a type relative path,
//  and return that.
//
//  That, in turn, can be relative to another path they pass. This is to support the use of
//  things like relative template paths. If pstrRelToPath is passed, then strPath value can
//  be a path relative to that. We pass that on to the dialog who understands that. We may
//  get back a full type relative path or something relative to the 'rel to' path.
//
tCIDLib::TBoolean
TFacCQCTreeBrws::bSelectFile(const  TWindow&                    wndOwner
                            , const TString&                    strTitle
                            , const tCQCRemBrws::EDTypes        eDType
                            , const TString&                    strPath
                            , const TCQCUserCtx&                cuctxToUse
                            , const tCQCTreeBrws::EFSelFlags    eFlags
                            ,       TString&                    strSelected
                            ,       MRemBrwsBrwsCB* const       prbbcEvents
                            , const TString* const              pstrRelToPath)
{
    TRemBrwsDlg dlgBrowse;
    return dlgBrowse.bRunDlg
    (
        wndOwner
        , strTitle
        , strPath
        , cuctxToUse
        , eDType
        , eFlags
        , strSelected
        , prbbcEvents
        , pstrRelToPath
    );
}


