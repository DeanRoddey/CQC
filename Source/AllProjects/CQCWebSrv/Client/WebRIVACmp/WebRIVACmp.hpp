//
// FILE NAME: WebRIVAComp.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/30/2017
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
//  This little program is used to generate shared content between the WebRIVA library
//  facility on the Web Server side, and the WebRIVA typescript program that implements
//  the client side.
//
//  There's a good bit of data transmitted and having to keep it all in sync would be
//  annoying and error prone. We have an XML file that defines the types, constants
//  and structures involved. We parse that and spit out code into both of the other
//  project's directories.
//
//  Each structure is marked as CS or SC, which means sent from client to server, sent
//  from server to client. This controls what code we generate.
//
//  For structures marked as CS, then we generate:
//
//  1. A TS method that will take the required values and build them into a JSON object
//      to be sent to the server
//  2. A C++ method that will extract out the values from the JSON object into a set of
//      output parameters of appropriate type.
//
//  For structures marked as SC, then we generate:
//
//  1. A C++ method that will take the required parameters and build them into the
//      appropriate JSON object with the correct members.
//
//  * Sadly we can't generate a generic extractor method on the TS side, because TS has no
//  output parameters. We could only put the values into a generic object as attributes,
//  but that's what the JSON object already is. So the TS side has to manually pull the
//  values out in each msg handler, based on a knowledge of what the value names are.
//
//
//  The JSON structures will include a message type, which will allow either side to know
//  which type of message it is and call the correct handler method to process it. That code
//  will know which extraction method to call to get the values out into locals for processing.
//
//  They also include a sequence number, which is an unsigned 32 bit value. Each side keeps
//  an in and out sequence number. It assigns each successive number to outgoing msgs,
//  starting at 1, and checks incoming messages against its incoming sequence number. If it
//  gets an out of sequence msg, it knows something really bad happened and it closes the
//  connection so that it can restart and recover.
//
//  We want to minimize transmission overhead, so all of the JSON names are very short, which
//  is fine since it's all programmatically managed for the most. The TS side does have to
//  look up the values using these short names though, since TS's lack of output parameters
//  means we can't create extraction methods for each message.
//
//  There is no nesting, so the message is just a flat list of values. The only 'sub-structs'
//  are a known list of common things, like areas, colors, points, and sizes. These are sent
//  as single values with their own types. They are just simple formatted text versions of
//  those things, which the receiving side will just parse out using simple split type
//  operations.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ----------------------------------------------------------------------------
//  Includes for underlying stuff we need
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CQCKit.hpp"



// ----------------------------------------------------------------------------
//  Facility types
// ----------------------------------------------------------------------------
namespace tWebRIVACmp
{
}


// ----------------------------------------------------------------------------
//  Our own header includes
// ----------------------------------------------------------------------------
#include    "WebRIVACmp_Data.hpp"
#include    "WebRIVACmp_CppGen.hpp"
#include    "WebRIVACmp_TSGen.hpp"

