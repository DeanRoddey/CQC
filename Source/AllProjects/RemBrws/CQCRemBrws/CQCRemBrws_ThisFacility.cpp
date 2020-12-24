//
// FILE NAME: CQCRemBrws_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCRemBrws,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCRemBrws
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCRemBrws: Public, static data
// ---------------------------------------------------------------------------

//
//  This is a DTD for the reports we generate and the URN that the data server puts in
//  for the PUBLIC id.
//
const TString TFacCQCRemBrws::strReportDTDURN(L"urn:charmedquark.com:CQC-Report.DTD");
const TString TFacCQCRemBrws::strReportDTD
(
    L"<?xml encoding=\"$NativeWideChar$\"?>\n"
    L"\n"
    L"<!ELEMENT   File    EMPTY>\n"
    L"<!ATTLIST   File\n"
    L"            Name    CDATA #REQUIRED\n"
    L"            LastMod CDATA #REQUIRED\n"
    L"            SerNum CDATA #REQUIRED\n"
    L"            Paused (Y|N) #IMPLIED>\n"
    L"\n"
    L"<!ELEMENT   Scope   (File | Scope)*>\n"
    L"<!ATTLIST   Scope\n"
    L"            Name    CDATA #REQUIRED>\n"
    L"\n"
    L"<!ELEMENT   FileType    (File | Scope)*>\n"
    L"<!ATTLIST   FileType\n"
    L"            Type        (EvMonitor | GlobalAct | Image | Macro | SchEvent\n"
    L"                          | Template | TrgEvent) #REQUIRED>\n"
    L"\n"
    L"<!ELEMENT   LicComp EMPTY>\n"
    L"<!ATTLIST   LicComp\n"
    L"            Name CDATA #REQUIRED\n"
    L"            Enabled (Y|N) #REQUIRED>\n"
    L"\n"
    L"<!ELEMENT   LicComps (LicComp*)>\n"
    L"\n"
    L"<!ELEMENT   Driver EMPTY>\n"
    L"<!ATTLIST   Driver\n"
    L"            Moniker NMTOKEN #REQUIRED\n"
    L"            Make CDATA #REQUIRED\n"
    L"            Model CDATA #REQUIRED\n"
    L"            Version CDATA #REQUIRED\n"
    L"            Host CDATA #REQUIRED\n>"
    L"<!ELEMENT   Drivers (Driver*)>\n"
    L"\n"
    L"<!ELEMENT   Report      (LicComps, Drivers, FileType+)>\n"
    L"<!ATTLIST   Report\n"
    L"            GeneratedAt CDATA #REQUIRED\n"
    L"            Ver CDATA #REQUIRED\n"
    L"            Tier NMTOKEN #REQUIRED\n"
    L"            Cls NMTOKEN #REQUIRED\n"
    L"            Drvs NMTOKEN #REQUIRED\n"
    L"            Descr CDATA #IMPLIED>\n"
);


// ---------------------------------------------------------------------------
//  TFacCQCRemBrws: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCRemBrws::TFacCQCRemBrws() :

    TFacility
    (
        L"CQCRemBrws"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_objaFTExts(tCQCRemBrws::EDTypes::Count)
    , m_objaFTPrefs(tCQCRemBrws::EDTypes::Count)
    , m_objaFTSysPrefs(tCQCRemBrws::EDTypes::Count)
    , m_strBadChars(L"()!@$%^&*+-~`'\",?/\\|[]<>")
{
    //
    //  Set up extensions for those things that have it. These are 5.3.903 and forward
    //  versions, where we added a 2 to distinguish them from the old ad hoc formats.
    //  These 2 versions are all stored in our standard TChunkedFile format.
    //
    m_objaFTExts[tCQCRemBrws::EDTypes::EvMonitor] = L"CQCEvMon2";
    m_objaFTExts[tCQCRemBrws::EDTypes::GlobalAct] = L"CQCGAct2";
    m_objaFTExts[tCQCRemBrws::EDTypes::Image]     = L"CQCImgStore2";
    m_objaFTExts[tCQCRemBrws::EDTypes::Macro]     = L"MEngc2";
    m_objaFTExts[tCQCRemBrws::EDTypes::SchEvent]  = L"CQCSEv2";
    m_objaFTExts[tCQCRemBrws::EDTypes::Template]  = L"CQCIntf2";
    m_objaFTExts[tCQCRemBrws::EDTypes::TrgEvent]  = L"CQCTEv2";

    //
    //  Create the base and system versions of the path arrays. These really only apply to
    //  the data server based types, but we just do them all.
    //
    tCQCRemBrws::EDTypes eType = tCQCRemBrws::EDTypes::Min;
    while (eType <= tCQCRemBrws::EDTypes::Max)
    {
        m_objaFTPrefs[eType] = tCQCRemBrws::strAltXlatEDTypes(eType);
        m_objaFTSysPrefs[eType] = m_objaFTPrefs[eType];
        m_objaFTSysPrefs[eType].Append(L"/System");

        eType++;
    }
}

TFacCQCRemBrws::~TFacCQCRemBrws()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCRemBrws: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add a component to the passed path, handling the separator issues.
//
tCIDLib::TVoid TFacCQCRemBrws::AddPathComp(TString& strTar, const TString& strToAppend)
{
    // Handle special cases first. If nothing to append, then we are done
    if (strToAppend.bIsEmpty())
        return;

    // Remember if the source has a starting separator
    const tCIDLib::TBoolean bSrcSep(strToAppend.chFirst() == kCIDLib::chForwardSlash);

    // If the target is empty, then just append , adding a separator if needed.
    if (strTar.bIsEmpty())
    {
        if (!bSrcSep)
            strTar.Append(kCIDLib::chForwardSlash);
        strTar.Append(strToAppend);
        return;
    }

    //
    //  OK, nothing special case at this point. We have content in both. Remember if the
    //  target has a trailing sep.
    //
    const tCIDLib::TBoolean bTarSep(strTar.chLast() == kCIDLib::chForwardSlash);

    //
    //  If both have seps, append the stuff after the leading sep. If neither, then add
    //  one first. Else just append.
    //
    if (bSrcSep == bTarSep)
    {
        // They either both have or both don't have
        if (bSrcSep)
        {
            // Both have them, so only take the stuff after the leading src slash
            strTar.AppendSubStr(strToAppend, 1);
        }
        else
        {
            // Both don't so add a sep then append
            strTar.Append(kCIDLib::chForwardSlash);
            strTar.Append(strToAppend);
        }
    }
     else
    {
        // Only one has a separator, so it's the easy scenario
        strTar.Append(strToAppend);
    }
}


//
//  We modify the passed file to get rid of any characters that we don't want to allow
//  in our files. Some files that the OS may allow we don't want. We replace them with
//  a hyphen. And we remove any path or extension. This gets us down to a file name that
//  we can use on the data server.
//
//  So this is taking a FILE SYSTEM path. It keeps just the name part of the file, and
//  adjusts that.
//
tCIDLib::TVoid TFacCQCRemBrws::AdjustFileName(TPathStr& pathEdit)
{
    pathEdit.bRemovePath();
    pathEdit.bRemoveExt();

    const tCIDLib::TCard4 c4Count = pathEdit.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_strBadChars.bContainsChar(pathEdit[c4Index]))
            pathEdit.PutAt(c4Index, kCIDLib::chHyphenMinus);
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::AppendTypeExt(TString& strTar, const tCQCRemBrws::EDTypes eType)
{
    if (strTar.chLast() != kCIDLib::chPeriod)
        strTar.Append(kCIDLib::chPeriod);
    strTar.Append(strFlTypeExt(eType));
}


//
//  If the passed path starts with the base system path for the indicated file type,
//  then it's a system scope (or in a system scope.) We have another one for when
//  they only have the path and would have to just turn around and figure out the
//  type, which we can just do for them.
//
tCIDLib::TBoolean
TFacCQCRemBrws::bCheckIsSystemScope(const   TString&                strPath
                                    ,       tCQCRemBrws::EDTypes&   eType) const
{
    eType = eXlatPathType(strPath);
    return strPath.bStartsWithI(m_objaFTSysPrefs[eType]);
}

tCIDLib::TBoolean
TFacCQCRemBrws::bIsSystemScope( const   TString&                strPath
                                , const tCQCRemBrws::EDTypes    eType) const
{
    CIDAssert(eType < tCQCRemBrws::EDTypes::Count, L"Invalid file type enum");
    return strPath.bStartsWithI(m_objaFTSysPrefs[eType]);
}


//
//  If there's more than one path component, it will remove the last one, and the trailing
//  separator as well. If not it does nothing. If the last character is a separator, it first
//  removes that, then starts checking.
//
//  It returns true if it did anything.
//
tCIDLib::TBoolean TFacCQCRemBrws::bRemoveLastPathItem(TString& strPath)
{
    //
    //  Handle special cases, which avoids a lot of confusion below. If it's empty, or
    //  not fully qualified, or has no slashes, then it can't be valid.
    //
    tCIDLib::TCard4 c4At;
    if (strPath.bIsEmpty()
    ||  (strPath[0] != kCIDLib::chForwardSlash)
    ||  !strPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_BadPath
            , strPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // If it's the root, then we do nothing
    if (strPath == L"/")
        return kCIDLib::False;

    //
    //  If the last character is a slash, which it normally shouldn't be, then remove it.
    //  We've already checked for the special case of the root, so we can't mess up by doing
    //  it this way. Then find a new last slash. If none, then it can't be good.
    //
    if (strPath.chLast() == kCIDLib::chForwardSlash)
    {
        strPath.DeleteLast();

        // Find the new last slash
        if (!strPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kRBrwsErrs::errcRem_BadPath
                , strPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
    }

    // We shouldn't have ever ended up with the last slash at the start
    CIDAssert(c4At != 0, L"The last path slash should not be at index 0");

    // Let's cap it here
    strPath.CapAt(c4At);
    return kCIDLib::True;
}


//
//  Return the appropriate local base path for the file type. This is only for
//  the server side generally. It handles any special case redirections of the
//  the paths when in the development environment.
//
tCIDLib::TVoid
TFacCQCRemBrws::BasePathForType(const   tCQCRemBrws::EDTypes    eType
                                ,       TString&                strToFill) const
{
    //
    //  It's almost always the CQCData\DataServer dir. Except when in the
    //  development environment, and for some types of files. So optimize for
    //  the common scenario and override that below if necessary.
    //
    strToFill = facCQCKit().strDataDir();
    if (strToFill.chLast() != kCIDLib::chBackSlash)
        strToFill.Append(kCIDLib::chBackSlash);

    if (facCQCKit().bDevMode())
    {
        if (eType == tCQCRemBrws::EDTypes::Macro)
        {
            // It's under the source tree
            if (!TProcEnvironment::bFind(L"CQC_SRCTREE", strToFill))
            {
                facCQCRemBrws().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kRBrwsErrs::errcRem_NoSrcDir
                    , tCIDLib::ESeverities::ProcFatal
                    , tCIDLib::EErrClasses::NotFound
                );
            }

            if (strToFill.chLast() != kCIDLib::chBackSlash)
                strToFill.Append(kCIDLib::chBackSlash);
            strToFill.Append(L"Source\\AllProjects\\");
        }
         else
        {
            strToFill.Append(L"DataServer\\");
        }
    }
     else
    {
        strToFill.Append(L"DataServer\\");
    }
}


//
//  This converts a CML class path (in the dotted form, so MEng.System.Runtime.Whatever)
//  into the equivalent hierarchical path for the data server. So:
//
//  1. We replace MEng with Macro
//  2. We replace periods with forward slashes
//  3. We add append it to the base hierarchy path for macros
//
tCIDLib::TVoid
TFacCQCRemBrws::CMLClassPathToHPath(const TString& strClassPath, TString& strToFill) const
{
    strToFill = strFlTypePref(tCQCRemBrws::EDTypes::Macro);
    strToFill.AppendSubStr(strClassPath, 4);
    strToFill.bReplaceChar(kCIDLib::chPeriod, kCIDLib::chForwardSlash);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);
}

//
//  Similar to above but converts to relative path.
//
//  1. We start with /
//  2. We copy in everything from past the MEng. prefix
//  3. We convert periods to forward slashes
//
tCIDLib::TVoid
TFacCQCRemBrws::CMLClassPathToRelPath(const TString& strClassPath, TString& strToFill) const
{
    strToFill = L"/";
    strToFill.AppendSubStr(strClassPath, 5);
    strToFill.bReplaceChar(kCIDLib::chPeriod, kCIDLib::chForwardSlash);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);
}


// Create the type relative version of a data server path
tCIDLib::TVoid
TFacCQCRemBrws::CreateRelPath(  const   TString&                strHPath
                                , const tCQCRemBrws::EDTypes    eType
                                ,       TString&                strToFill)
{
    // Get the base path and remove that from the incoming file
    const TString& strPref = strFlTypePref(eType);

    // The passed path has to start with the prefix for the indicated data type
    if (!strHPath.bStartsWithI(strPref))
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_NotDTPath
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }

    //
    //  The HPath may be exactly the same, i.e. pointing at the top of that part of
    //  the hierarchy. If so, the relative path becomes /. Else we get out the part
    //  trailing the non-relative part.
    //
    if (strHPath.c4Length() == strPref.c4Length())
        strToFill = L"/";
    else
        strToFill.CopyInSubStr(strHPath, strPref.c4Length());

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);
}


//
//  Create the root /User hierarchical and local paths for a given type
tCIDLib::TVoid
TFacCQCRemBrws::CreateTypePaths(const   tCQCRemBrws::EDTypes    eType
                                ,       TString&                strHPath
                                ,       TString&                strLocalPath)
{
    // Create the HPath
    strHPath = strFlTypePref(eType);
    AddPathComp(strHPath, L"User");
    facCQCKit().PrepRemBrwsPath(strHPath);

    // And the local path
    BasePathForType(eType, strLocalPath);
    strLocalPath.AppendSubStr(strFlTypePref(eType), 11);
    strLocalPath.Append(L"\\User");
    strLocalPath.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);
}


//
//  Translate the passed path to one of the file types. It is assumed to be more
//  than just the prefix, so we only compare the prefix part.
//
tCQCRemBrws::EDTypes
TFacCQCRemBrws::eXlatPathType(  const   TString&            strToXlat
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    tCQCRemBrws::EDTypes  eRet = tCQCRemBrws::EDTypes::Min;
    while (eRet < tCQCRemBrws::EDTypes::Count)
    {
        const TString& strCur = m_objaFTPrefs[eRet];
        if (strToXlat.bCompareNI(strCur, strCur.c4Length()))
            break;
        eRet++;
    }

    if ((eRet == tCQCRemBrws::EDTypes::Count) && bThrowIfNot)
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_UnknownPathType
            , strToXlat
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );
    }
    return eRet;
}


//
//  Similar to above but also converts the path by removing the found prefix,
//  converting the slashes to regular file path slashes. We have one that will
//  return either the type relative path or the full local path, and another that
//  returns both.
//
//  These are really only for the data server to get incoming paths translated to
//  the forms he needs.
//
tCQCRemBrws::EDTypes
TFacCQCRemBrws::eConvertPath(const  TString&            strHToCvt
                            , const tCIDLib::TBoolean   bIsScope
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bGetFullPath) const
{
    tCQCRemBrws::EDTypes  eRet = tCQCRemBrws::EDTypes::Min;
    while (eRet < tCQCRemBrws::EDTypes::Count)
    {
        const TString& strCur = m_objaFTPrefs[eRet];
        if (strHToCvt.bCompareNI(strCur, strCur.c4Length()))
        {
            // Build up the appropriate path
            if (bGetFullPath)
            {
                // Start with the appropriate base path
                BasePathForType(eRet, strToFill);

                //
                //  Copy in after the the part of the DS path we want to keep. If it
                //  is a scope it's possible that it will be the top level scope for
                //  the type, and exactly equal to the prefix, so we have to check for
                //  that to avoid an index error.
                //
                if (strHToCvt.c4Length() > 11)
                    strToFill.AppendSubStr(strHToCvt, 11);

                // Add the correct extension for the type if not a scope
                if (!bIsScope)
                {
                    strToFill.Append(kCIDLib::chPeriod);
                    strToFill.Append(m_objaFTExts[eRet]);
                }

                // Adjust slashes to OS type
                strToFill.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);
            }
             else
            {
                //
                //  Just copy in the incoming  path after the /Customize part. Watch
                //  for the special case where the incoming is the top level for
                //  the type, without any trailing slash, in which case the relative
                //  path is "/". In that case it also MUST be a scope.
                //
                if (strHToCvt.c4Length() == strCur.c4Length())
                {
                    CIDAssert(bIsScope, L"The root path of a data type must be a scope");
                    strToFill = L"/";
                }
                 else
                {
                    strToFill.CopyInSubStr(strHToCvt, strCur.c4Length());
                }
            }
            break;
        }
        eRet++;
    }

    if (eRet == tCQCRemBrws::EDTypes::Count)
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_UnknownPathType
            , strHToCvt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );
    }

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);

    return eRet;
}


tCQCRemBrws::EDTypes
TFacCQCRemBrws::eConvertPath(const  TString&            strOrgHToCvt
                            , const tCIDLib::TBoolean   bIsScope
                            ,       TString&            strRelPath
                            ,       TString&            strLocalPath) const
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strToCvt;
    facCQCKit().PrepRemBrwsPath(strOrgHToCvt, strToCvt);

    tCQCRemBrws::EDTypes  eRet = tCQCRemBrws::EDTypes::Min;
    while (eRet < tCQCRemBrws::EDTypes::Count)
    {
        const TString& strCur = m_objaFTPrefs[eRet];
        if (strToCvt.bCompareNI(strCur, strCur.c4Length()))
        {
            //
            //  Create the relative path. If the incoming is the top level scope for
            //  the type and has no trailing separator, we have to check for that
            //  and make the relative path "/".
            //
            if (strToCvt.c4Length() == strCur.c4Length())
                strRelPath = L"/";
            else
                strRelPath.CopyInSubStr(strToCvt, strCur.c4Length());

            // Now the full path, start with the base path
            BasePathForType(eRet, strLocalPath);

            //
            //  Add the relative path part, we skip over the /Customize/ part of the
            //  path. This is really only used with the /Customize part of the
            //  hierarchy, though there may be other parts defined by the containing
            //  application.
            //
            strLocalPath.AppendSubStr(strToCvt, 11);

            // Now convert separators
            strLocalPath.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);

            // Add the correct extension for the type if not a scope
            if (!bIsScope)
            {
                strLocalPath.Append(kCIDLib::chPeriod);
                strLocalPath.Append(m_objaFTExts[eRet]);
            }
            break;
        }
        eRet++;
    }

    if (eRet == tCQCRemBrws::EDTypes::Count)
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_UnknownPathType
            , strToCvt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );
    }

    return eRet;
}


//
//  This converts a hierarchical path to the equivalent CML class path (in the
//  dotted form, so MEng.System.Runtime.Whatever) So:
//
//  1. We remove the macro hierarchy base path, which leaves it at /User or /System
//  2. We prepend it with MEng
//  3. We replace the forward slashes with periods
//
tCIDLib::TVoid
TFacCQCRemBrws::HPathToCMLClassPath(const TString& strHPath, TString& strToFill) const
{
    strToFill = L"MEng";
    strToFill.AppendSubStr
    (
        strHPath, strFlTypePref(tCQCRemBrws::EDTypes::Macro).c4Length()
    );

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);

    // And replace the slashes with periods
    strToFill.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chPeriod);
}


// Return a counted pointer wrapped data server proxy
tCQCRemBrws::TDSrvClProxy TFacCQCRemBrws::orbcDataSrv()
{
    return tCQCRemBrws::TDSrvClProxy
    (
        facCIDOrbUC().porbcMakeClient<TDataSrvAccClientProxy>
        (
            TDataSrvAccClientProxy::strBinding
        )
    );
}


// Get the base name part out of the source path
tCIDLib::TVoid
TFacCQCRemBrws::QueryNamePart(const TString& strOrgSrcPath, TString& strToFill)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strSrcPath;
    facCQCKit().PrepRemBrwsPath(strOrgSrcPath, strSrcPath);

    tCIDLib::TCard4 c4At;
    if (!strSrcPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_BadPath
            , strSrcPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    //
    //  If the last character is a slash, which it normally shouldn't be
    //  the we have to go back enother one.
    //
    if (strToFill.chLast() == kCIDLib::chForwardSlash)
    {
        if (!strSrcPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kRBrwsErrs::errcRem_BadPath
                , strSrcPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
    }

    // Copy out the name part
    strSrcPath.CopyOutSubStr(strToFill, c4At + 1);
}


//
//  Query the parent path part of a path, without any trailing slash. Though, if it is
//  a root path (/), then the string will not be changed.
//
//  We cannot deal with ./whatever type paths. We only deal with fully qualified paths. If
//  a relative path is passed, this will fail.
//
tCIDLib::TVoid
TFacCQCRemBrws::QueryParentPath(const TString& strOrgPath, TString& strToFill)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strPath;
    facCQCKit().PrepRemBrwsPath(strOrgPath, strPath);

    //
    //  Copy it and then call another method that will do what we want. We don't care what
    //  it returns.
    //
    strToFill = strPath;
    bRemoveLastPathItem(strToFill);
}


//
//  This converts a type relative macro path to the equivalent CML class path (in the
//  dotted form, so MEng.System.Runtime.Whatever) So from:
//
//      /User/MyMacros/MyMacro
//  to
//      MEng.User.MyMacros.MyMacro
//
//  So we:
//
tCIDLib::TVoid
TFacCQCRemBrws::RelPathToCMLClassPath(const TString& strOrgRelPath, TString& strToFill) const
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strRelPath;
    facCQCKit().PrepRemBrwsPath(strOrgRelPath, strRelPath);

    strToFill = L"MEng";
    strToFill.Append(strRelPath);

    // Now replace slashes with periods
    strToFill.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chPeriod);
}



// Return the descriptive name for the description
const TString&
TFacCQCRemBrws::strFlTypeDescr(const tCQCRemBrws::EDTypes eType) const
{
    CIDAssert(eType < tCQCRemBrws::EDTypes::Count, L"Invalid file type enum");
    return tCQCRemBrws::strAltXlatEDTypes(eType);
}


// Return the file extension for the indicated file type
const TString&
TFacCQCRemBrws::strFlTypeExt(const tCQCRemBrws::EDTypes  eType) const
{
    CIDAssert(eType < tCQCRemBrws::EDTypes::Count, L"Invalid file type enum");
    return m_objaFTExts[eType];
}


// Return the data server hierarchy prefix for the indicated file type
const TString&
TFacCQCRemBrws::strFlTypePref(const tCQCRemBrws::EDTypes  eType) const
{
    CIDAssert(eType < tCQCRemBrws::EDTypes::Count, L"Invalid file type enum");
    return m_objaFTPrefs[eType];
}


//
//  Splits a path (relative or full) into the parent part and the name part. The original
//  is capped, leaving it the parent part, and the remainder is copied to strName. If, by
//  some chance, the path is /, then name is empty.
//
tCIDLib::TVoid TFacCQCRemBrws::SplitPath(TString& strOrgPath, TString& strName)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strPathPart;
    facCQCKit().PrepRemBrwsPath(strOrgPath, strPathPart);

    //
    //  Copy the path part to the name, then remove the last path item
    //  leaving the parent path, and copy it back to the original. Can't do
    //  it directly on the original since it may have been normalized to
    //  some degree above.
    //
    strName = strPathPart;
    bRemoveLastPathItem(strPathPart);
    strOrgPath = strPathPart;

    // And cut that much out of the copy to leave the name
    if (!strName.bIsEmpty())
        strName.Cut(0, strPathPart.c4Length() + 1);
}


// Convert a path from the type specific format to the full hierarhical format
tCIDLib::TVoid
TFacCQCRemBrws::ToHPath(const   TString&                strRelPath
                        , const tCQCRemBrws::EDTypes    eType
                        ,       TString&                strToFill)
{
    strToFill = strFlTypePref(eType);
    strToFill.Append(strRelPath);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);
}

tCIDLib::TVoid
TFacCQCRemBrws::ToHPath(const   TString&                strScope
                        , const TString&                strFile
                        , const tCQCRemBrws::EDTypes    eType
                        ,       TString&                strToFill)
{
    strToFill = strFlTypePref(eType);
    strToFill.Append(strScope);
    if (strToFill.chLast() != kCIDLib::chForwardSlash)
        strToFill.Append(kCIDLib::chForwardSlash);
    strToFill.Append(strFile);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strToFill);
}
