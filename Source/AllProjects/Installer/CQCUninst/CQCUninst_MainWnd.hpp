//
// FILE NAME: CQCUninst_MainWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/16/2003
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
//  This window does the actual uninstall work. It starts a thread which does
//  the work and which posts posts messages back to the dialog to tell it what
//  is happening. The thread saves up any errors and the window shows them at the
//  end. Until then it indicates errors by setting the icon beside each item to
//  indicate errors occured.
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
//   CLASS: TMainFrameWnd
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        TMainFrameWnd(const TMainFrameWnd&) = delete;

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMainFrameWnd& operator=(const TMainFrameWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateMainWnd();

        tCIDLib::TBoolean bRemoveDir() const;

        const TString& strRemoveDirCmd() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TBag<TError>    TErrList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes ePollThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRemoveDir
        //      We store the state of the remove directory check box here upon starting
        //      the uninstall.
        //
        //  m_colCols
        //      For adding items to the list.
        //
        //  m_colErrs
        //      Any errors that the worker thread sees, so we can show them at the end.
        //
        //  m_eState
        //      We update this to track our state over time.
        //
        //  m_pathDelCmd
        //      If removing the directory the bgn thread sets up this path and spits out
        //      the delection batch file there. We'll invoke it on the way out.
        //
        //  m_pwndXXX
        //      Some typed pointers to important widgets that we have to interact with
        //      enough to want to avoid type casting.
        //
        //  m_thrWorker
        //      The thread that does the actual work and posts events to us to let us know
        //      what is going on.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bRemoveDir;
        tCIDLib::TStrList   m_colCols;
        TErrList            m_colErrs;
        tCQCUninst::EStates m_eState;
        TPathStr            m_pathDelCmd;
        TPushButton*        m_pwndCancel;
        TStaticMultiText*   m_pwndInstruct;
        TCheckedMCListWnd*  m_pwndList;
        TCheckBox*          m_pwndRemoveDir;
        TPushButton*        m_pwndStart;
        TThread             m_thrWorker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd, TFrameWnd)
};

#pragma CIDLIB_POPPACK



