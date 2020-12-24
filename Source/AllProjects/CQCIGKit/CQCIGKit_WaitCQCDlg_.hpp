//
// FILE NAME: CQCIGKit_WaitCQCDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/05/2004
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
//  This is the header for the CQCGKit_WaitCQCDlg.cpp file, which implements
//  a small dailog box that is used by some client tools to indicate to the
//  user that they are waiting for the CQC back end services to start up. This
//  can be a problem in Kiosk mode and for advanced users who move very quickly
//  to start up client tools before the back end might have finished coming
//  up.
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
//   CLASS: TWaitCQCDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TWaitCQCDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWaitCQCDlg();

        ~TWaitCQCDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strBinding
            , const tCIDLib::TCard4         c4WaitFor
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TWaitCQCDlg(const TWaitCQCDlg&);
        tCIDLib::TVoid operator=(const TWaitCQCDlg&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eRunThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_enctEnd
        //      Just before we run the modal loop we calc the end time and store
        //      it here so that the timer can know when it needs to terminate
        //      the dialog.
        //
        //  m_pwndXXX
        //      Type pointers to some of our widgets, for convenience.
        //
        //  m_strBinding
        //      The name server binding that we wait to show up.
        //
        //  m_strTitle
        //      The title text to put in the title bar, which they give us
        //      also.
        //
        //  m_thrWaiter
        //      To keep the dialog responsive, we do the checking on a separate
        //      thread.
        // -------------------------------------------------------------------
        tCIDLib::TEncodedTime   m_enctEnd;
        TPushButton*            m_pwndCancelButton;
        TString                 m_strBinding;
        TString                 m_strTitle;
        TThread                 m_thrWaiter;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TWaitCQCDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


