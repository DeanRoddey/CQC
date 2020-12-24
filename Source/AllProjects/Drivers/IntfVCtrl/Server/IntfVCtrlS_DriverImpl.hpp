//
// FILE NAME: IntfVCtrlS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2004
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
//  This is the header file for the actual driver class. We just inherit from
//  the IDL generated base server class, and provide implementations for the
//  virtual methods it created for us.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TIntfVCtrlSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TIntfVCtrlSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfVCtrlSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TIntfVCtrlSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TIntfVCtrlSDriver();
        TIntfVCtrlSDriver(const TIntfVCtrlSDriver&);
        tCIDLib::TVoid operator=(const TIntfVCtrlSDriver&);


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TCntPtr<TIntfCtrlClientProxy>   TIVCtrlProxy;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldXXX
        //      We look up our fields after registration so that we can
        //      use by id methods, for efficiency.
        //
        //  m_orbcProxy
        //      The client proxy object that we used to talk to the interface
        //      driver.
        //
        //  m_strPollTmpX
        //      Some strings to use during polling so that we are not
        //      constantly creating and destroying them.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Action;
        tCIDLib::TCard4     m_c4FldId_CurBaseTmpl;
        tCIDLib::TCard4     m_c4FldId_CurTopTmpl;
        tCIDLib::TCard4     m_c4FldId_LayerCount;
        tCIDLib::TCard4     m_c4FldId_LoadOverlay;
        tCIDLib::TCard4     m_c4FldId_LoadTemplate;
        tCIDLib::TCard4     m_c4FldId_Maximized;
        tCIDLib::TCard4     m_c4FldId_PlayWAV;
        tCIDLib::TCard4     m_c4FldId_SetVariable;
        TIVCtrlProxy        m_orbcProxy;
        TString             m_strPollTmp1;
        TString             m_strPollTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfVCtrlSDriver, TCQCServerBase)
};

#pragma CIDLIB_POPPACK


