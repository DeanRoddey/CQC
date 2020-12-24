//
// FILE NAME: BacroCRTS_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

namespace tBarcoCRTS
{
    // -----------------------------------------------------------------------
    //  Our view of the commands available to send to the Barco. These are
    //  what we use to interact with the TBarcoMsg class. Internally, it will
    //  transform it to the actual on the wire value.
    //
    //  NOTE:   There are multiple tables which this enum is used to index,
    //          so update them if you change this enum!
    // -----------------------------------------------------------------------
    enum class ECmds
    {
        Standby
        , Num0
        , Num1
        , Num2
        , Num3
        , Num4
        , Num5
        , Num6
        , Num7
        , Num8
        , Num9
        , ForceToggle
        , QueryCurSource
        , QueryStatus
        , QueryVersion

        , Count
        , Min           = Standby
        , Max           = QueryVersion
    };


    // -----------------------------------------------------------------------
    //  The reasons why a message was not recieved
    // -----------------------------------------------------------------------
    enum class EMsgResults
    {
        Ok
        , NoReply
        , PartialReply
        , BadCheckSum
        , BadCmd
        , SendFailed
    };
}

StdEnumTricks(tBarcoCRTS::ECmds)

