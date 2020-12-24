//
// FILE NAME: CQCTreeBrws_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This file defines facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTreeBrws
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TFacCQCTreeBrws : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCTreeBrws();

        TFacCQCTreeBrws(const TFacCQCTreeBrws&) = delete;
        TFacCQCTreeBrws(TFacCQCTreeBrws&&) = delete;

        ~TFacCQCTreeBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCTreeBrws& operator=(const TFacCQCTreeBrws&) = delete;
        TFacCQCTreeBrws& operator=(TFacCQCTreeBrws&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDriverWizard
        (
            const   TWindow&                wndOwner
            , const TString&                strHost
            ,       TCQCDriverObjCfg&       cqcdcEdit
            , const tCIDLib::TBoolean       bReconfigMode
            , const tCIDLib::TStrList&      colMonikers
            , const tCIDLib::TStrList* const pcolPorts
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bSelectFile
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCRemBrws::EDTypes    eDType
            , const TString&                strPath
            , const TCQCUserCtx&            cuctxToUse
            , const tCQCTreeBrws::EFSelFlags eFlags
            ,       TString&                strSelected
            ,       MRemBrwsBrwsCB* const   prbbcEvents = nullptr
            , const TString* const          pstrRelToPath = nullptr
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCTreeBrws,TGUIFacility)
};

#pragma CIDLIB_POPPACK


