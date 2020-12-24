//
// FILE NAME: CQCIGKit_AddToPackDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/07/2010
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
//  This is the header for the CQCIGKit_AddToPackDlg.cpp file, which implements a dialog
//  that lets a user add files to a package. The
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TAddToPackDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------
class TAddToPackDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TAddToPackDlg();

        ~TAddToPackDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TCQCPackage&            packTar
            , const TString&                strInitPath
            , const tCIDLib::TBoolean       bEncrypted
            , const tCQCRemBrws::EDTypes    eDType
            , const TCQCUserCtx&            cuctxToUse
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddContent();

        tCIDLib::TBoolean bAddFileToPack
        (
            const   TString&                strRelPath
        );

        tCIDCtrls::EEvResponses eBrwsHandler
        (
                    TTreeBrowseInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEncrypted
        //      Indicates whether they want the package to be encrypted or not.
        //
        //  m_cuctxToUse
        //      The calling program must provide us with a valid user context that we
        //      can use to access the data server.
        //
        //  m_dsclLoad
        //      We need a data server client to download the macro files that are added to
        //      the package.
        //
        //  m_eDType
        //      The type of data we allow to be added to the package.
        //
        //  m_eFType
        //      The type of files we are adding, which we translate from the data type that
        //      we get from the caller.
        //
        //  m_pcuctxToUse
        //      The caller must provide us with a security context so we can do server
        //      operations to get the content to add.
        //
        //  m_ppackTar
        //      The target package to manage.
        //
        //  m_pwndXXX
        //      We store a few typed pointers to our widgets for convenience. We don't own
        //      them, we just reference them.
        //
        //  m_strBasePath
        //      The base hierarchical path for our data type. Anything that is of potential
        //      interest has to be below that.
        //
        //  m_strInitPath
        //      The caller can pass in an initial path, which must be under the appropriate
        //      part of the hierarchy for the indicated type. It should be a fully
        //      qualified hierarchical path.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bEncrypted;
        tCQCRemBrws::EDTypes    m_eDType;
        tCQCKit::EPackFlTypes   m_eFType;
        TDataSrvClient          m_dsclLoad;
        const TCQCUserCtx*      m_pcuctxToUse;
        TCQCPackage*            m_ppackTar;
        TPushButton*            m_pwndAdd;
        TPushButton*            m_pwndCancel;
        TMultiColListBox*       m_pwndList;
        TPushButton*            m_pwndRemove;
        TPushButton*            m_pwndRevert;
        TPushButton*            m_pwndSave;
        TCQCTreeTypeBrowser*    m_pwndTree;
        TString                 m_strBasePath;
        TString                 m_strInitPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAddToPackDlg,TDlgBox)
};


#pragma CIDLIB_POPPACK



