//
// FILE NAME: CQCGKit_GetVersion_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2003
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
//  This is the header for the CQCGKit_GetVersionDlg.cpp file, which implements a dialog
//  that allows the user to enter a CQC version, in the form M.m.r (Major.minor.revision),
//  which is returned as a TCard8 value in which the versions are packed in the standard
//  CIDLib way.
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
//   CLASS: TGetVersionDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TGetVersionDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGetVersionDlg();

        TGetVersionDlg(const TGetVersionDlg&) = delete;
        TGetVersionDlg(TGetVersionDlg&&) = delete;

        ~TGetVersionDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGetVersionDlg& operator=(const TGetVersionDlg&) = delete;
        TGetVersionDlg& operator=(TGetVersionDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            ,       tCIDLib::TCard8&        c8ToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c8RetVal
        //      A place to put the parsed version until we can get it into
        //      the caller's return parm.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_strTitle
        //      This dialog is used in multiple places so the caller provides
        //      the title bar text.
        // -------------------------------------------------------------------
        tCIDLib::TCard8 m_c8RetVal;
        TPushButton*    m_pwndAccept;
        TPushButton*    m_pwndCancel;
        TEntryField*    m_pwndVersion;
        TString         m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TGetVersionDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



