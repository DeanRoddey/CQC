//
// FILE NAME: CQSLRepoImp_Helpers.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/31/2007
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
//  This is the header for a namespace that just provides some general
//  purpose helper methods, to keep them out of the main logic.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQSLRepoImp.hpp"


// ---------------------------------------------------------------------------
//  NAMESPACE: TRepoImpHelpers
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TRepoImpHelpers::bGetFile(  const   TString&            strPrompt
                            ,       TString&            strFile
                            , const tCIDLib::TBoolean   bMustExist)
{
    tCIDLib::TBoolean   bRet;
    while (kCIDLib::True)
    {
        *pconOut  << kCIDLib::NewLn << strPrompt << L" (Enter for none): ";
        pconOut->Flush();
        *pconIn >> strFile;
        *pconOut << kCIDLib::EndLn;

        // If empty, return false
        if (strFile.bIsEmpty())
        {
            bRet = kCIDLib::False;
            break;
        }

        if (TFileSys::bExists(strFile, tCIDLib::EDirSearchFlags::NormalFiles))
        {
            // We have it, so break out
            bRet = kCIDLib::True;
            break;
        }
         else
        {
            if (!bMustExist)
            {
                if (bGetYNAnswer(L"The file doesn't exit. Create it?"))
                {
                    bRet = kCIDLib::True;
                    break;
                }
            }
        }

        *pconOut << L"That file was not found. Please try again."
                 << kCIDLib::EndLn;
    }
    return bRet;
}


tCIDLib::TBoolean
TRepoImpHelpers::bGetPath(  const   TString&            strPrompt
                            ,       TString&            strPath
                            , const tCIDLib::TBoolean   bMustExist)
{
    tCIDLib::TBoolean   bRet;
    while (kCIDLib::True)
    {
        *pconOut  << kCIDLib::NewLn << strPrompt << L" (Enter for none): ";
        pconOut->Flush();
        *pconIn >> strPath;
        *pconOut << kCIDLib::EndLn;

        // If empty, return false
        if (strPath.bIsEmpty())
        {
            bRet = kCIDLib::False;
            break;
        }

        //
        //  If it doesn't exist, then see if it must exist. If not, then
        //  ask if they want to create it. If it does exist, just break
        //  out with this path. Else, we have to make them try again.
        //
        if (TFileSys::bIsDirectory(strPath))
        {
            // We have it, so break out
            bRet = kCIDLib::True;
            break;
        }
         else
        {
            if (!bMustExist)
            {
                if (bGetYNAnswer(L"The directory doesn't exit. Create it?"))
                {
                    bRet = kCIDLib::True;
                    TFileSys::MakePath(strPath);
                    break;
                }
            }
        }

        // It doesn't exist, but must, so tell them to tray again
        *pconOut << L"That directory was not found. Please try again\n"
                 << kCIDLib::EndLn;
    }
    return bRet;
}


//
//  A helper to gen up a client proxy for the repo driver's specialized
//  administration interface.
//
tCIDLib::TBoolean
TRepoImpHelpers::bGetRepo(  const   TString&                    strPrompt
                            ,       TString&                    strMoniker
                            ,       tCQSLRepoImp::TRepoClient&  orbcRepoMgr)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (kCIDLib::True)
    {
        *pconOut  << kCIDLib::NewLn << strPrompt << L" (Enter for none): ";
        pconOut->Flush();
        *pconIn >> strMoniker;
        *pconOut << kCIDLib::EndLn;

        // If empty, return false
        if (strMoniker.bIsEmpty())
            break;

        //
        //  Try to get info about this driver. If we can, make sure it is
        //  of the right type.
        //
        try
        {
            TCQCDriverObjCfg cqcdcTest;
            facCQCKit().QueryDriverCfg(strMoniker, cqcdcTest);

            if ((cqcdcTest.strMake() != L"CQSL")
            ||  (cqcdcTest.strModel() != L"CQSLRepo"))
            {
                *pconOut << L"That is not a CQSL repository driver. Please "
                            L"try again" << kCIDLib::EndLn;
                continue;

            }

            //
            //  It's of the right type, so let's get a client repo
            //  manager interface for it. The binding is provided by
            //  the proxy client, but we have to do a token replacement
            //  of the moniker in it.
            //
            TString strMgrBinding = TCQSLRepoMgrClientProxy::strBindingScope;
            strMgrBinding.eReplaceToken(strMoniker, L'm');
            strMgrBinding.Append(L"/");
            strMgrBinding.Append(TCQSLRepoMgrClientProxy::strBindingName);

            //
            //  Get a name server proxy and then get the object id for this
            //  binding path. If we get the info, then create a proxy and
            //  store it on the return counted pointer object.
            //
            TOrbObjId ooidTmp;
            if (facCIDOrbUC().bGetNSObject(strMgrBinding, ooidTmp))
            {
                orbcRepoMgr.SetPointer
                (
                    new TCQSLRepoMgrClientProxy(ooidTmp, TString::strEmpty())
                );
                bRet = kCIDLib::True;
            }
             else
            {
                *pconOut << L"The repository's management interface could "
                            L"not be obtained!" << kCIDLib::EndLn;
            }
            break;
        }

        catch(const TError& errToCatch)
        {

            *pconOut << L"That driver moniker was not found. Please try "
                        L"again. ERROR="
                     << kCIDLib::NewLn
                     << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        }
    }
    return bRet;
}


tCIDLib::TBoolean TRepoImpHelpers::bGetYNAnswer(const TString& strQ)
{
    tCIDLib::TBoolean bRet;
    TString strInput;
    while (kCIDLib::True)
    {
        *pconOut  << kCIDLib::NewLn << strQ << L"? (Y/N): ";
        pconOut->Flush();
        *pconIn >> strInput;
        *pconOut << kCIDLib::EndLn;

        if (strInput.c4Length() != 1)
        {
            *pconOut  << L"\nPlease enter either Y or N."
                      << kCIDLib::EndLn;
            continue;
        }

        strInput.ToUpper();
        if (strInput[0] == L'Y')
        {
            bRet = kCIDLib::True;
            break;
        }
         else if (strInput[0] == L'N')
        {
            bRet = kCIDLib::False;
            break;
        }
         else
        {
            *pconOut << L"\nEnter either Y or N. An empty string will cancel"
                     << kCIDLib::EndLn;
        }
    }
    return bRet;
}



tCIDLib::TCard4
TRepoImpHelpers::c4GetIndex(const   TString&        strPrompt
                            , const tCIDLib::TCard4 c4Min
                            , const tCIDLib::TCard4 c4Max)
{
    tCIDLib::TCard4 c4Ret;
    TString         strInput;

    while (kCIDLib::True)
    {
        *pconOut  << kCIDLib::NewLn << strPrompt << L" (" << c4Min
                  << L" to " << c4Max << L", 99 to cancel): ";
        pconOut->Flush();
        *pconIn >> strInput;
        *pconOut << kCIDLib::EndLn;

        // If empty, we canceled
        if (strInput.bIsEmpty())
        {
            c4Ret = kCIDLib::c4MaxCard;
            break;
        }

        try
        {
            c4Ret = strInput.c4Val(tCIDLib::ERadices::Dec);
            if (((c4Ret >= c4Min) && (c4Ret <= c4Max)) || (c4Ret == 99))
                break;

            *pconOut << L"The value was outside the valid range. Try again"
                     << kCIDLib::EndLn;
        }

        catch(...)
        {
            *pconOut << L"The value was invalid. An empty string will cancel"
                     << kCIDLib::EndLn;
        }
    }
    return c4Ret;
}


