// ----------------------------------------------------------------------------
//  FILE: CQCPollEng_Shared.hpp
//  DATE: Tue, Jan 19 17:39:54 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tCQCPollEng
{
    
    // ------------------------------------------------------------------------
    //  Used by the eNewFieldInfo method, to indicate to that method what
    //  it should do with the new field info it is getting. See the base
    //  widget class header comments for what these mean.
    //                  
    // ------------------------------------------------------------------------
    enum class EFldActs
    {
        EFldAct_CheckField
        , EFldAct_SetField
        , EFldAct_AssocField
        , Count
        , Min = EFldAct_CheckField
        , Max = EFldAct_AssocField
    };
    [[nodiscard]] CQCPOLLENGEXPORT tCIDLib::TBoolean bIsValidEnum(const EFldActs eVal);

    
    // ------------------------------------------------------------------------
    //  The results of registering a field with the engine to be
    //  polled
    //                  
    // ------------------------------------------------------------------------
    enum class EFldRegRes
    {
        OK
        , NotFound
        , BadName
        , WrongAccess
        , NoReadAccess
        , Count
        , Min = OK
        , Max = NoReadAccess
    };
    [[nodiscard]] CQCPOLLENGEXPORT EFldRegRes eXlatEFldRegRes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCPOLLENGEXPORT const TString& strXlatEFldRegRes(const EFldRegRes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    CQCPOLLENGEXPORT const TString& strLoadEFldRegRes(const EFldRegRes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCPOLLENGEXPORT tCIDLib::TBoolean bIsValidEnum(const EFldRegRes eVal);

    
    // ------------------------------------------------------------------------
    //  Most clients will want to keep up with the state of each
    //  field value they are having the engine poll. So we provide
    //  this obvious set of values for such a state. Generally they
    //  will use the prefab TCQCFldPollInfo class which does all
    //  this work for them.
    //  
    //  Invalid     : Something about the moniker/field names were found
    //                to be invalid. Pathological but if it happene we
    //                want to just stop trying to do anything with this
    //                one. We start out in WaitReg, and only get moved
    //                here if found to be invalid.
    //  WaitReg     : Waiting to get the field registered with the engine.
    //                So it may not be available yet.
    //  Rejected    : We registered, but the field is either not readable
    //                or the client marked as rejected because it no longer
    //                meets their requirements (perhaps due to a change in
    //                the field definition.) It will only come out of this
    //                state if we see a driver level change or more, in whihc
    //                case we'll go back to WaitReg again to give them another
    //                chance.
    //                it will stay that way until we see a change in the
    //  FldError    : We are registered but the field is in error state.
    //  WaitValue   : We are registered and waiting for a value to come in.
    //  Ready       : We have a good value and just need to watch for changes
    //                  
    // ------------------------------------------------------------------------
    enum class EFldStates
    {
        Invalid
        , WaitReg
        , Rejected
        , FldError
        , WaitValue
        , Ready
        , Count
        , Min = Invalid
        , Max = Ready
    };
    CQCPOLLENGEXPORT const TString& strLoadEFldStates(const EFldStates eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCPOLLENGEXPORT tCIDLib::TBoolean bIsValidEnum(const EFldStates eVal);

}

#pragma CIDLIB_POPPACK




