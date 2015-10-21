/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org
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
#include "CCWinRTUtils.h"
#include "platform/CCFileUtils.h"
#include "support/user_default/CCUserDefault.h"
#include <Windows.h>
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>
#include <wrl\wrappers\corewrappers.h>

NS_CC_BEGIN



using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;



std::wstring CCUtf8ToUnicode(const char * pszUtf8Str, unsigned len/* = -1*/)
{
    std::wstring ret;
    do
    {
        if (! pszUtf8Str) break;
		// get UTF8 string length
		if (-1 == len)
		{
			len = strlen(pszUtf8Str);
		}
        if (len <= 0) break;

		// get UTF16 string length
		int wLen = MultiByteToWideChar(CP_UTF8, 0, pszUtf8Str, len, 0, 0);
		if (0 == wLen || 0xFFFD == wLen) break;
		
		// convert string  
        wchar_t * pwszStr = new wchar_t[wLen + 1];
        if (! pwszStr) break;
        pwszStr[wLen] = 0;
        MultiByteToWideChar(CP_UTF8, 0, pszUtf8Str, len, pwszStr, wLen + 1);
        ret = pwszStr;
        CC_SAFE_DELETE_ARRAY(pwszStr);
    } while (0);
    return ret;
}

std::string CCUnicodeToUtf8(const wchar_t* pwszStr)
{
	std::string ret;
	do
	{
		if(! pwszStr) break;
		size_t len = wcslen(pwszStr);
		if (len <= 0) break;
		
		size_t convertedChars = 0;
		char * pszUtf8Str = new char[len*3 + 1];
		WideCharToMultiByte(CP_UTF8, 0, pwszStr, len+1, pszUtf8Str, len*3 + 1, 0, 0);
		ret = pszUtf8Str;
		CC_SAFE_DELETE_ARRAY(pszUtf8Str);
	}while(0);

	return ret;
}

std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
    std::wstring ret;
    if (!strUtf8.empty())
    {
        int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
        if (nNum)
        {
            WCHAR* wideCharString = new WCHAR[nNum + 1];
            wideCharString[0] = 0;

            nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

            ret = wideCharString;
            delete[] wideCharString;
        }
        else
        {
            CCLOG("Wrong convert to WideChar code:0x%x", GetLastError());
        }
    }
    return ret;
}


std::string PlatformStringToString(Platform::String^ s) {
	std::wstring t = std::wstring(s->Data());
	return std::string(t.begin(),t.end());
}

Platform::String^ PlatformStringFromString(const std::string& s)
{
    std::wstring ws = StringUtf8ToWideChar(s);
    return ref new Platform::String(ws.data(), ws.length());
}


// Method to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
float ConvertDipsToPixels(float dips)
{
   static const float dipsPerInch = 96.0f;
   return floor(dips * DisplayProperties::LogicalDpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

float getScaledDPIValue(float v) {
	auto dipFactor = DisplayProperties::LogicalDpi / 96.0f;
	return v * dipFactor;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)

// Function that reads from a binary file asynchronously.
Concurrency::task<Platform::Array<byte>^> ReadDataAsync(Platform::String^ filename)
{
	using namespace Windows::Storage;
	using namespace Concurrency;
		
	auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		
	return create_task(folder->GetFileAsync(filename)).then([] (StorageFile^ file) 
	{
		return file->OpenReadAsync();
	}).then([] (Streams::IRandomAccessStreamWithContentType^ stream)
	{
		unsigned int bufferSize = static_cast<unsigned int>(stream->Size);
		auto fileBuffer = ref new Streams::Buffer(bufferSize);
		return stream->ReadAsync(fileBuffer, bufferSize, Streams::InputStreamOptions::None);
	}).then([] (Streams::IBuffer^ fileBuffer) -> Platform::Array<byte>^ 
	{
		auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
		Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);
		return fileData;
	});
}
#else



// Function that reads from a binary file asynchronously.
Concurrency::task<Platform::Array<byte>^> ReadDataAsync(Platform::String^ path)
{
	using namespace Windows::Storage;
	using namespace Concurrency;
		
	return create_task(StorageFile::GetFileFromPathAsync(path)).then([&](StorageFile^ f)
	{
		return FileIO::ReadBufferAsync(f);

	}).then([] (Streams::IBuffer^ fileBuffer) -> Platform::Array<byte>^ 
	{
		auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
		Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);
		return fileData;
	});
}

bool createMappedCacheFile(const std::string& srcFilePath, std::string& cacheFilePath, std::string ext)
{
    bool ret = false;
    auto folderPath = CCFileUtils::sharedFileUtils()->getWritablePath();
    cacheFilePath = folderPath + computeHashForFile(srcFilePath) + ext;
    std::string prevFile = CCUserDefault::sharedUserDefault()->getStringForKey(srcFilePath.c_str());

    if (prevFile == cacheFilePath) {
        ret = CCFileUtils::sharedFileUtils()->isFileExist(cacheFilePath);
    }
    else {
        CCFileUtils::sharedFileUtils()->removeFile(prevFile);
    }

    CCUserDefault::sharedUserDefault()->setStringForKey(srcFilePath.c_str(), cacheFilePath);
    return ret;
}

void destroyMappedCacheFile(const std::string& key)
{
    std::string value = CCUserDefault::sharedUserDefault()->getStringForKey(key.c_str());

    if (!value.empty()) {
        CCFileUtils::sharedFileUtils()->removeFile(value);
    }

    CCUserDefault::sharedUserDefault()->setStringForKey(key.c_str(), "");
}

std::string computeHashForFile(const std::string& filePath)
{
    std::string ret = filePath;
    int pos = std::string::npos;
    pos = ret.find_last_of('/');

    if (pos != std::string::npos) {
        ret = ret.substr(pos);
    }

    pos = ret.find_last_of('.');

    if (pos != std::string::npos) {
        ret = ret.substr(0, pos);
    }

    CREATEFILE2_EXTENDED_PARAMETERS extParams = { 0 };
    extParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extParams.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
    extParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extParams.dwSize = sizeof(extParams);
    extParams.hTemplateFile = nullptr;
    extParams.lpSecurityAttributes = nullptr;

    Microsoft::WRL::Wrappers::FileHandle file(CreateFile2(std::wstring(filePath.begin(), filePath.end()).c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extParams));

    if (file.Get() != INVALID_HANDLE_VALUE) {
        FILE_BASIC_INFO  fInfo = { 0 };
        if (GetFileInformationByHandleEx(file.Get(), FileBasicInfo, &fInfo, sizeof(FILE_BASIC_INFO))) {
            std::stringstream ss;
            ss << ret << "_";
            ss << fInfo.CreationTime.QuadPart;
            ss << fInfo.ChangeTime.QuadPart;
            ret = ss.str();
        }
    }

    return ret;
}

#endif


NS_CC_END
