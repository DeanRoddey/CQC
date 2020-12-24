//
// FILE NAME: CQCInst_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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
//  This is the header for the facility class for the installer.
//
//  There are a few places where we might need to get the user to log in as an admin
//  to prove he has the right to do something. We don't want to make them do it
//  multiple times, so we provide a call to get user credentials if we already have
//  them or ask the user to log in if we don't.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCInst
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCInst : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCInst();

        TFacCQCInst(const TFacCQCInst&) = delete;
        TFacCQCInst(TFacCQCInst&&) = delete;

        ~TFacCQCInst();


        // -------------------------------------------------------------------
        //  Unimplimented
        // -------------------------------------------------------------------
        TFacCQCInst& operator=(const TFacCQCInst&) = delete;
        TFacCQCInst& operator=(TFacCQCInst&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckTarget
        (
                    TWindow&                wndParent
            , const TString&                strTarget
            ,       TCQCVerInstallInfo&     viiToFill
        );

        tCIDLib::TBoolean bForceNewSysId() const;

        const TVector<TInstWarning>& colWarnings() const;

        tCIDLib::TVoid DupPath
        (
            const   TString&                strSrc
            , const TString&                strTarget
            , const tCIDLib::TBoolean       bRecurse
            , const tCIDLib::TBoolean       bOverwrite = kCIDLib::True
        );

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDKernel::EWSrvStarts eStartMode() const;

        tCIDKernel::EWSrvStarts eStartMode
        (
            const   tCIDKernel::EWSrvStarts eToSet
        );

        tCIDLib::TVoid LogInstallMsg
        (
            const   tCIDLib::TCh* const     pszMsg
        );

        tCIDLib::TVoid LogInstallMsg
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid LogInstallMsg
        (
            const   TString&                strMsg
            , const MFormattable&           fmtblToken1
        );

        tCIDLib::TVoid QueueWarning
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid QueueWarning
        (
            const   TString&                strMsg
            , const TError&                 errExcept
        );

        tCIDLib::TVoid QueueWarning
        (
            const   TString&                strMsg
            , const TString&                strResource
        );

        tCIDLib::TVoid QueueWarning
        (
            const   TString&                strMsg
            , const TString&                strResource
            , const TError&                 errExcept
        );

        const TString& strLogPath() const;

        TMainFrameWnd& wndMain()
        {
            CIDAssert(m_pwndMainFrm != nullptr, L"Main window is not set yet");
            return *m_pwndMainFrm;
        }


    private :
        // -------------------------------------------------------------------
        //  Private types and constants
        // -------------------------------------------------------------------
        const tCIDLib::TCard4 c4MaxErrors = 128;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInit();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eStartMode
        //      We ask the user what startup mode they want to set on the service. That's
        //      stored here until it's time for the installer thread to do the service setup.
        //
        //  m_colWarnings
        //      We queue up non-fatal warnings here so that they can be displayed to the
        //      user before we commit to the upgrade.
        //
        //  m_pathLogs
        //      The path where we put the logs. It's put here so that it can be reported to
        //      the user at the end if we fail.
        //
        //  m_pwndMainFrm
        //      Our main frame window. It does all the real work. It needs to be a pointer
        //      because we don't want it initializing in the startup thread context, which
        //      is when this facility object will be constructed.
        // -------------------------------------------------------------------
        tCIDKernel::EWSrvStarts m_eStartMode;
        TVector<TInstWarning>   m_colWarnings;
        TPathStr                m_pathLogs;
        TMainFrameWnd*          m_pwndMainFrm;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCInst,TFacility)
};

#pragma CIDLIB_POPPACK



