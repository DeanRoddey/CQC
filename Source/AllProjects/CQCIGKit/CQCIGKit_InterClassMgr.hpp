//
// FILE NAME: CQCIGKit_InterClassMgr.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2003
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
//  This is the header for the CQCIGKit_ClassManager.cpp file, which implements the
//  TCQCInterClassMgr, which implements a CQC specific interactive class manager for the
//  macro engine. CQCMEng provides one for non-interactive use, but this one is used by GUI
//  apps that need to allow the user to select a macro, or manage macros.
//
//  Because this one is used in editors, we have to deal with encryption of classes. We don't
//  want to be able to access any encrypted macros if we are used by an editor app. So we
//  always tell the macro server that we don't want to see them. If he indicates we tried to
//  open an encrypted one, we'll return som bogus text content that indicates the file cannot
//  be opened.
//
//  What we do basically is just override the methods that are related to reading, writing,
//  selecting, and checking for the existence of, macro classes on the Master Server.
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
//  CLASS: TCQCInterClassMgr
// PREFIX: mecm
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TCQCInterClassMgr : public TObject, public MMEngClassMgr
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCInterClassMgr
        (
                    TWindow* const          pwndOwner
            , const TString&                strSelDlgTitle
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TBoolean       bReadOnly
            ,       MRemBrwsBrwsCB* const   prbbcApp
        );

        TCQCInterClassMgr(const TCQCInterClassMgr&) = delete;

        ~TCQCInterClassMgr();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCInterClassMgr& operator=(const TCQCInterClassMgr&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Shutdown();


    protected :
        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckIfExists
        (
            const   TString&                strToCheck
        )   override;

        tCIDLib::TVoid DoStore
        (
            const   TString&                strClassPath
            , const TString&                strText
        )   override;

        tCIDLib::TVoid DoUndoWriteMode
        (
            const   TString&                strClassPath
        )   override;

        tCIDLib::EOpenRes eDoSelect
        (
                    TString&                strToFill
            , const tCIDMacroEng::EResModes    eMode
        )   override;

        TTextInStream* pstrmDoLoad
        (
            const   TString&                strClassPath
            , const tCIDMacroEng::EResModes    eMode
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bReadOnly
        //      This indicates whether we invoke the selection dialog in read only
        //      mode or allow it to support the popup menu (the one used when filtering
        //      for a single type in this case.) When used in the standalone IDE it
        //      allows for editing.
        //
        //  m_cuctxToUse
        //      The caller has to provide us with the user context for the
        //      logged in user, which we need for any write operations.
        //
        //  m_prbbcApp
        //      A callback handler for the file selection popup, so that the contaning
        //      application can response to any edits made. If not invoked in read
        //      only mode, you would normally want to provide a handler so that the
        //      containing application can control/respond to what happens
        //
        //  m_pwndOwner
        //      The owner window that our selection dialog should use.
        //
        //  m_sectUser
        //      The caller must provide us with a security token to be used
        //      when we need to store macros.
        //
        //  m_strLastClassPath
        //      The last path something was opened from, so that we can start
        //      there next time. Initially it's set to the base path.
        //
        //  m_strSelDlgTitle
        //      The title text to give to the macro selection dialog.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bReadOnly;
        const TCQCUserCtx&  m_cuctxToUse;
        MRemBrwsBrwsCB*     m_prbbcApp;
        TWindow*            m_pwndOwner;
        TString             m_strLastClassPath;
        TString             m_strSelDlgTitle;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCInterClassMgr,TObject)
};

#pragma CIDLIB_POPPACK

