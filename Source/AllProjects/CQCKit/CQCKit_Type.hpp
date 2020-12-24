//
// FILE NAME: CQCKit_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2000
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
//  This is the non-class types header for the facility.
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
//  Toolkit types namespace
// ---------------------------------------------------------------------------
namespace tCQCKit
{
    // -----------------------------------------------------------------------
    //  Each client side driver can tell the underlying framework how active
    //  its data is. This will let the underlying framework to adjust itself
    //  appropriately in terms of poll speed and reconnect speed. This makes
    //  things more efficient.
    //
    //  The 'Interactive' level is reserved for devices that are used in a
    //  very interactive way, such as DVD players and such, i.e. those devices
    //  that, in normal use, are often constantly under direct manual user
    //  control and therefore need to react very crisply.
    //
    //  Server side drivers are in control of their own polling and such, so
    //  they don't use this at this time, though they might later on if more
    //  of the server side logic is factored down into a base class.
    //
    //  These could be persisted so add at the end!
    // -----------------------------------------------------------------------
    enum class EActLevels
    {
        Low
        , Medium
        , High
        , Interactive

        , Count
        , Min           = Low
        , Max           = Interactive
    };


    // -----------------------------------------------------------------------
    //  Some action engines allow nested actions (caused by another action) or
    //  non-nested ones. This type is used to indicate which a given invoked
    //  action should be. It effectively controls whether the action is pushed
    //  to the top of a stack or stuck on the bottom.
    // -----------------------------------------------------------------------
    enum class EActOrders
    {
        NonNested
        , Nested
    };


    // -----------------------------------------------------------------------
    //  Used by some action engines, when they support nested and non-nested
    //  actions. This tells the caller what was done in response to the
    //  invocation of an action.
    //
    //  NoAction -  There were no commands configured for the indicated
    //              event id, so nothing was posted and no action is required.
    //  Process  -  The action stack was empty, so a new processing cycle
    //              needs to be started.
    //  Already  -  The context was already processing commands, so there's
    //              no need to call the invoke command because it's already
    //              running.
    // -----------------------------------------------------------------------
    enum class EActPostRes
    {
        NoAction
        , Process
        , Already
    };


    // -----------------------------------------------------------------------
    //  Commands have a type that indicates two things. One is whether it has
    //  any side effects or not and the other is whether it returns true/false
    //  status.) Conditionals with no side effects can only be used in
    //  conditional opcodes. Non-conditionals cannot be used in conditionals
    //  at all. Others can be used either as a regular command opcode or in a
    //  conditional opcode.
    // -----------------------------------------------------------------------
    enum class ECmdTypes
    {
        None                = 0x0
        , Conditional       = 0x1
        , SideEffect        = 0x2
        , Both              = 0x3
    };


    // -----------------------------------------------------------------------
    //  Driver initialization results. This is what is returned by drivers
    //  from their init call, which is called by CQCServer when it loads the
    //  driver.
    // -----------------------------------------------------------------------
    enum class EDrvInitRes
    {
        Failed
        , WaitConfig
        , WaitCommRes

        , Count
        , Min               = Failed
        , Max               = WaitCommRes
    };


    // -----------------------------------------------------------------------
    //  As a convenience to the client side, the field limit classes provide
    //  a recommended visual representation indicator, i.e. the type of widget
    //  that would best display a field of that type, range, limit of values,
    //  or so forth.
    // -----------------------------------------------------------------------
    enum class EOptFldReps
    {
        None
        , Check
        , Combo
        , SelDialog
        , Spin
        , Slider
        , Text
        , Time

        , Count
    };


    // -----------------------------------------------------------------------
    //  This is passed to the token replacement stuff
    //
    //  NoEscape    - Doesn't attempt to
    //  TestOnly    - Tests that it's valid syntax but doesn't really expand
    //  GVarsOnly   - Only allow global variable tokens (no locals, RTVs, fields...)
    // -----------------------------------------------------------------------
    enum class ETokRepFlags : tCIDLib::TCard2
    {
        None            = 0x0000
        , NoEscape      = 0x0001
        , TestOnly      = 0x0002
        , GVarsOnly     = 0x0004
    };


    // -----------------------------------------------------------------------
    //  Typedef for the factory function that we dynamically locate in the
    //  server driver DLL/SharedLib. Each server driver exports a global
    //  function with this signature, using "C'conventions. Its name is
    //  provided as a constant in kCQCKit. It returns an object that is
    //  derived from the CQC server side Orb derived class.
    // -----------------------------------------------------------------------
    typedef TCQCServerBase* (*TSrvDriverFuncPtr)
    (
        const   TCQCDriverObjCfg&   cqcdcInfo
    );
}

#pragma CIDLIB_POPPACK

