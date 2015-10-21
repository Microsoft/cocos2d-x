/****************************************************************************
Copyright (c) 2010 cocos2d-x.org
Copyright (c) Microsoft Open Technologies, Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "CCFileUtilsWinRT.h"
#include "CCWinRTUtils.h"
#include "platform/CCCommon.h"
//#include <Shlobj.h>

#define NOMINMAX
#include <Windows.h>

using namespace std;

NS_CC_BEGIN

static std::string s_pszResourcePath;

static void _checkPath()
{
    if (s_pszResourcePath.empty())
    {
		// TODO: needs to be tested
		s_pszResourcePath = CCFileUtilsWinRT::getAppPath() + '\\' + "Assets\\Resources\\";
    }
}

CCFileUtils* CCFileUtils::sharedFileUtils()
{
    if (s_sharedFileUtils == NULL)
    {
        s_sharedFileUtils = new CCFileUtilsWinRT();
        s_sharedFileUtils->init();
    }
    return s_sharedFileUtils;
}

CCFileUtilsWinRT::CCFileUtilsWinRT()
{
}

bool CCFileUtilsWinRT::init()
{
    _checkPath();
    m_strDefaultResRootPath = s_pszResourcePath;
    return CCFileUtils::init();
}

#if 1
bool CCFileUtilsWinRT::isFileExist(const std::string& strFilePath)
{
    bool ret = false;
    FILE * pf = 0;

    std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    { // Not absolute path, add the default root path at the beginning.
        strPath.insert(0, m_strDefaultResRootPath);
    }

    const char* path = strPath.c_str();

	if (path && strlen(path) && (pf = fopen(path, "rb")))
    {
        ret = true;
        fclose(pf);
    }
    return ret;
}
#else

bool CCFileUtilsWinRT::isFileExist(const std::string& strFilePath)
{
	WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

	std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    { // Not absolute path, add the default root path at the beginning.
        strPath.insert(0, m_strDefaultResRootPath);
    }

#if 0
    std::string pathKey = strPath;
    pathKey = CCFileUtils::sharedFileUtils()->fullPathForFilename(pathKey.c_str());
    std::replace( pathKey.begin(), pathKey.end(), '/', '\\'); 

#endif // 0


	hFind = FindFirstFileEx(CCUtf8ToUnicode(strPath.c_str(), -1).c_str(), FindExInfoStandard, &FindFileData,
             FindExSearchNameMatch, NULL, 0);


    return hFind != INVALID_HANDLE_VALUE;
#if 0
    if (0 == strFilePath.length())
    {
        return false;
    }
    
    std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    { // Not absolute path, add the default root path at the beginning.
        strPath.insert(0, m_strDefaultResRootPath);
    }
    return GetFileAttributesA(strPath.c_str()) != -1 ? true : false;
#endif
}
#endif

static std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
    std::string ret;
    if (!strUtf8.empty())
    {
        std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
        int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
        if (nNum)
        {
            char* ansiString = new char[nNum + 1];
            ansiString[0] = 0;

            nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

            ret = ansiString;
            delete[] ansiString;
        }
        else
        {
            CCLOG("Wrong convert to Ansi code:0x%x", GetLastError());
        }
    }

    return ret;
}

std::string CCFileUtilsWinRT::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return UTF8StringToMultiByte(filenameUtf8);
}

long CCFileUtilsWinRT::getFileSize(const std::string &filepath)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(StringUtf8ToWideChar(filepath).c_str(), GetFileExInfoStandard, &fad))
    {
        return 0; // error condition, could call GetLastError to find out more
    }
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return (long)size.QuadPart;
}


bool CCFileUtilsWinRT::removeFile(const std::string &path)
{
    std::wstring wpath = StringUtf8ToWideChar(path);
    if (DeleteFile(wpath.c_str()))
    {
        return true;
    }
    else
    {
        CCLOG("Remove file failed with error: %d", GetLastError());
        return false;
    }
}

bool CCFileUtilsWinRT::isAbsolutePath(const std::string& strPath)
{
    if (   strPath.length() > 2 
        && ( (strPath[0] >= 'a' && strPath[0] <= 'z') || (strPath[0] >= 'A' && strPath[0] <= 'Z') )
        && strPath[1] == ':')
    {
        return true;
    }
    return false;
}

string CCFileUtilsWinRT::getWritablePath()
{
	auto localFolderPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
	return std::string(PlatformStringToString(localFolderPath)) + '\\';
}

string CCFileUtilsWinRT::getAppPath()
{
	Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
	return std::string(PlatformStringToString(package->InstalledLocation->Path));
}

NS_CC_END
