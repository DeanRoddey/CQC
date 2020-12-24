//
// FILE NAME: CQCDrvDev_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/14/2003
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
//  This is the header for the facility class of this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDrvDev
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCDrvDev : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCDrvDev();

        TFacCQCDrvDev(const TFacCQCDrvDev&) = delete;
        TFacCQCDrvDev(TFacCQCDrvDev&&) = delete;

        ~TFacCQCDrvDev();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCDrvDev& operator=(const TFacCQCDrvDev&) = delete;
        TFacCQCDrvDev& operator=(TFacCQCDrvDev&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    private :
        // -------------------------------------------------------------------
        //  Private non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoLogon();

        tCIDLib::TBoolean bParseParms
        (
                    tCIDLib::TBoolean&      bIgnoreState
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cuctxToUse
        //      The user context for the logged in user, which we have to
        //      pass to various things.
        //
        //  m_strAppTitle
        //      This is used in all the popups, so preload it once and keep
        //      it around.
        //
        //  m_wndMainFrame
        //      Our main frame window.
        // -------------------------------------------------------------------
        TCQCUserCtx         m_cuctxToUse;
        tCIDLib::TIPPortNum m_ippnHTTP;
        TString             m_strTitleText;
        TCQCDrvDevFrameWnd  m_wndMainFrame;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCDrvDev,TGUIFacility)
};

