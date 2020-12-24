//
// FILE NAME: CQCIGKit_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/19/2004
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
//  This is the types header for this facility
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

namespace tCQCIGKit
{
    // -----------------------------------------------------------------------
    //  Blanker modes we support
    // -----------------------------------------------------------------------
    enum class EBlankModes
    {
        Normal
        , Clock
        , SlideShow
    };


    // -----------------------------------------------------------------------
    //  Used to get an option from the user about how to deal with the deletion
    //  of a conditional command from an action.
    // -----------------------------------------------------------------------
    enum class ECmdDelOpts
    {
        Cancel
        , DelAll
        , KeepCmds
    };


    // -----------------------------------------------------------------------
    //  Used by the key mapped action dialog
    // -----------------------------------------------------------------------
    enum class EKMActModes
    {
        Edit
        , Execute
    };


    // -----------------------------------------------------------------------
    //  Alias for the factory function that we dynamically locate in a client driver
    //  facility. Each client driver exports a global function with this signature,
    //  using "C' conventions. Its name is provided as a constant in our constants
    //  namespace. This method will create a client window and make itself a child of
    //  the passed window.
    //
    //  The window must be a derivative of TCQCDriverClient, and a pointer to the new
    //  window is returned. The caller is responsible for cleaning it.
    // -----------------------------------------------------------------------
    using TClientDriverFuncPtr = TCQCDriverClient* (*)
    (
                TWindow&            wndParent
        , const TArea&              areaInit
        , const tCIDCtrls::TWndId   widToUse
        , const TCQCDriverObjCfg&   cqcdcToLoad
        , const TCQCUserCtx&        cuctxToUse
    );
}

