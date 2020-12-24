//
// FILE NAME: CQCInst_StatusPacket.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2001
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
//  This file defines a simple class that is used to pass status information
//  from the installer thread to the status dialog. These information packets
//  are queued by the thread and picked up by a timer in the install dialog
//  and displayed.
//
//  Currently it just includes an enum that indicates the step of the install
//  that is about to be done, bit it might be expanded later to pass more
//  info.
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
//   CLASS: TCQCStatusPacket
//  PREFIX: spack
// ---------------------------------------------------------------------------
class TCQCStatusPacket : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStatusPacket();

        TCQCStatusPacket
        (
            const   tCQCInst::EInstSteps    eStep
        );

        TCQCStatusPacket
        (
            const   TCQCStatusPacket&       spackSrc
        );

        ~TCQCStatusPacket();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStatusPacket& operator=
        (
            const   TCQCStatusPacket&       spackSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCInst::EInstSteps eStep() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eStep
        //      The step of the installation about to be started. This is used
        //      to display status messages and move the progress bar.
        // -------------------------------------------------------------------
        tCQCInst::EInstSteps    m_eStep;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStatusPacket,TObject)
};

#pragma CIDLIB_POPPACK


