//
// FILE NAME: CQCGKit_DrvInfoTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/11/2001
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
//  This is the header for a simple class that loads up information from a driver
//  configuration object and displays it within a tab window. So one of these can
//  be inserted into any tabbed window to display driver info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class T2ColSectList;

class CQCGKITEXPORT TCQCDrvInfoTab : public TTabWindow
{
    public :
       // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDrvInfoTab() = delete;

        TCQCDrvInfoTab
        (
            const   tCIDLib::TBoolean       bCanClose
        );

        TCQCDrvInfoTab
        (
            const  TString&                 strMoniker
            , const TCQCDriverObjCfg&       cqcdcInfo
            , const tCIDLib::TBoolean       bCanClose
        );

        TCQCDrvInfoTab(const TCQCDrvInfoTab&) = delete;
        TCQCDrvInfoTab(TCQCDrvInfoTab&&) = delete;

        ~TCQCDrvInfoTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDrvInfoTab& operator=(const TCQCDrvInfoTab&) = delete;
        TCQCDrvInfoTab& operator=(TCQCDrvInfoTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearInfo();

        tCIDLib::TVoid UpdateInfo
        (
            const  TString&                 strMoniker
            , const TCQCDriverObjCfg&       cqcdcInfo
        );


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
        )   final;

        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cqcdcInfo
        //      The driver info we are to display
        //
        //  m_pwndList
        //      Our list window that we display the content in
        //
        //  m_strMoniker
        //      The moniker that this info is for. This is just for display.
        // -------------------------------------------------------------------
        TCQCDriverObjCfg    m_cqcdcInfo;
        T2ColSectList*      m_pwndList;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvInfoTab, TTabWindow)
};

#pragma CIDLIB_POPPACK
