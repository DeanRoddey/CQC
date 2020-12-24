//
// FILE NAME: CQCMedia_ClassLoader_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/30/2006
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
//  This file defines our own macro engine class loader. We create a few
//  CML level classes of our own and need a class loader to get them installed
//  into the macro engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCMediaRTClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class TCQCMediaRTClassLoader : public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMediaRTClassLoader() noexcept;

        TCQCMediaRTClassLoader(const TCQCMediaRTClassLoader&) = delete;
        TCQCMediaRTClassLoader(TCQCMediaRTClassLoader&&) = delete;

        ~TCQCMediaRTClassLoader();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMediaRTClassLoader& operator=(const TCQCMediaRTClassLoader&) = delete;
        TCQCMediaRTClassLoader& operator=(TCQCMediaRTClassLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        )   final;
};

#pragma CIDLIB_POPPACK

