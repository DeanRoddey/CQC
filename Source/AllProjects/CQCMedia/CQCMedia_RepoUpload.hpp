//
// FILE NAME: CQCMedia_RepoUpload.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/06/2006
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
//  This is a file that is used by the CQSLRepoS and MediaRepoMgr facilities so that
//  they don't have to have a shared library of their own. It provides some shared
//  data that is used in the data upload process, which is not done via the ORB
//  interface because it's used for bulk upload of media data.
//
//  It's not included in our facilty header. The two facilities just include it
//  directly, since only they care about it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

namespace kCQSLRepoUpl
{
    // The command ids and special values
    constexpr tCIDLib::TCard4 c4MagicVal1       = 0xF84AFF43;
    constexpr tCIDLib::TCard4 c4MagicVal2       = 0x947CE341;

    //
    //  File and media types, which are combined together. If file type DB, then the
    //  media type doesn't matter, so use None.
    //
    constexpr tCIDLib::TCard4 c4FileType_DB     = 0x0001;
    constexpr tCIDLib::TCard4 c4FileType_Image  = 0x0002;
    constexpr tCIDLib::TCard4 c4FileType_Track  = 0x0003;

    constexpr tCIDLib::TCard4 c4MediaType_None  = 0x0000;
    constexpr tCIDLib::TCard4 c4MediaType_Movie = 0x0100;
    constexpr tCIDLib::TCard4 c4MediaType_Music = 0x0200;

    //
    //  Parm1 -
    //      The file sequence number, each new thing uploaded is given a sequential
    //      number for sanity checking.
    //
    //  Parm2 -
    //      The size of the file about to be sent.
    //
    //  Parm3 -
    //      1st Byte    - File Type
    //      2nd Byte    - Media type
    //      High Word   - The id of the thing, if applicable (not for DB file)
    //
    constexpr tCIDLib::TCard4 c4Cmd_StartFile   = 0x800D744A;

    // More packets after StartFile. Parm1 is the packet sequence number
    constexpr tCIDLib::TCard4 c4Cmd_FileData    = 0x86CD51AC;

    // Start off the whole upload process. Parm1 is the upload cookie
    constexpr tCIDLib::TCard4 c4Cmd_StartUpload = 0xD848A383;

    // From server back to client. No parameters, body is flattened TError object
    constexpr tCIDLib::TCard4 c4Cmd_SrvError    = 0x85B4AA81;

    // If a track or image
    constexpr tCIDLib::TCard4 c4Cmd_EndFile     = 0xF80043AE;

    // No parameters on these
    constexpr tCIDLib::TCard4 c4Cmd_EndUpload   = 0xF80043AE;
    constexpr tCIDLib::TCard4 c4Cmd_Ack         = 0x9844FD83;
    constexpr tCIDLib::TCard4 c4Cmd_FinalAck    = 0x84E432A1;
}

namespace tCQSLRepoUpl
{
    // This packet is used to transmit messages. It must be byte packed
    #pragma CIDLIB_PACK(1)
    struct THdr
    {
        tCIDLib::TCard4 c4MagicVal;
        tCIDLib::TCard4 c4DataCS;
        tCIDLib::TCard4 c4DataBytes;
        tCIDLib::TCard4 c4Cmd;
        tCIDLib::TCard4 c4Parm1;
        tCIDLib::TCard4 c4Parm2;
        tCIDLib::TCard4 c4Parm3;
        tCIDLib::TCard4 c4MagicVal2;
    };
    #pragma CIDLIB_POPPACK
}

#pragma CIDLIB_POPPACK

