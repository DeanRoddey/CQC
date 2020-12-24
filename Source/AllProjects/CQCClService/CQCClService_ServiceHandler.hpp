//
// FILE NAME: CQCClService_ServiceHandler.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/20/2013
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
//  This is the header for the service handler for this service, to allow us
//  to respond to requests from the system service manager.
//
//  Note that this guy shouldn't contain any object members, since it will be
//  created before low level initialization is complete. So it could cause
//  problems if it constructed any client members and they try to do things
//  that depending on that low level CIDLib initialization being done.
//
//  All of the stuff is over in the facility class and it is faulted in when
//  our bStartServiceThread() method is called by the service manager, by
//  which time init is done.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCClServiceService
//  PREFIX: srvh
// ---------------------------------------------------------------------------
class TCQCClServiceService : public TWin32ServiceHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCClServiceService();

        TCQCClServiceService(const TCQCClServiceService&) = delete;
        TCQCClServiceService(TCQCClServiceService&&) = delete;

        ~TCQCClServiceService();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCClServiceService& operator=(const TCQCClServiceService&) = delete;
        TCQCClServiceService& operator=(TCQCClServiceService&&) = delete;


        // -------------------------------------------------------------------
        //  Inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bStartServiceThread() final;

        tCIDLib::TBoolean bStopServiceThread() final;

        tCIDLib::TBoolean bSuspendRequest() final;

        tCIDLib::TVoid ServiceOp
        (
            const   tCIDKernel::EWSrvOps    eOperation
        )   final;

        tCIDLib::TVoid ServiceOp
        (
            const   tCIDKernel::EWSrvPwrOps ePowerOp
        )   final;
};

#pragma CIDLIB_POPPACK


