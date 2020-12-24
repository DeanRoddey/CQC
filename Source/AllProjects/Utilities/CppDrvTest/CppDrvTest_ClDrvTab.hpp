//
// FILE NAME: CppDrvTest_ClDrvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/03/2018
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
//  This is a tab window that goes inserted into the main tabbed window of our
//  interface. This one is simple and just acts as a container for the client side
//  driver interface. If the driver has one, then one of these tabs is created and
//  it's passed to CQCIGKit as the parent window for the newly created client side
//  interface window. We just have to keep that window sized to fit our tab area.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TClDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TClDrvTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TClDrvTab();

        TClDrvTab(const TTabWindow&) = delete;

        ~TClDrvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TClDrvTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadDriver();

        tCIDLib::TBoolean bStopDriver();

        TCQCDriverClient* pwndDriver()
        {
            return m_pwndDriver;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndDriver
        //      CQCIGKit creates a main window to hold the controls that the client
        //      side driver interface creates. We store that away here for later
        //      use, mostly just so we can keep it sized to fit our tab area.
        // -------------------------------------------------------------------
        TCQCDriverClient*   m_pwndDriver;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TClDrvTab, TTabWindow)
};

#pragma CIDLIB_POPPACK
