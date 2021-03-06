﻿#pragma once


#ifndef MINT_STRING_UTIL_H
#define MINT_STRING_UTIL_H


#include <string>

#include <MintCommon/Include/CommonDefinitions.h>

#include <MintContainer/Include/Vector.h>


namespace mint
{
    struct StringRange final
    {
                        StringRange();
                        StringRange(const uint32 offset);
                        StringRange(const uint32 offset, const uint32 length);

    public:
        const bool      isLengthSet() const noexcept;

    public:
        uint32          _offset;
        uint32          _length;
    };


    template <uint32 Size>
    void                formatString(char(&buffer)[Size], const char* format, ...);
    void                formatString(char* const buffer, const uint32 bufferSize, const char* format, ...);
    
    template <uint32 Size>
    void                formatString(wchar_t(&buffer)[Size], const wchar_t* format, ...);
    void                formatString(wchar_t* const buffer, const uint32 bufferSize, const wchar_t* format, ...);


    namespace StringUtil
    {
        const bool      isNullOrEmpty(const char* const rawString);
        const bool      isNullOrEmpty(const wchar_t* const rawWideString);
        
        const uint32    strlen(const char* const rawString);
        const uint32    wcslen(const wchar_t* const rawWideString);
        const uint32    find(const char* const source, const char* const target, const uint32 offset = 0);
        const bool      strcmp(const char* const a, const char* const b);
        
        template <uint32 DestSize>
        void            strcpy(char(&dest)[DestSize], const char* const source);

        void            convertWideStringToString(const std::wstring& source, std::string& destination);
        void            convertStringToWideString(const std::string& source, std::wstring& destination);
        void            excludeExtension(std::string& inoutText);

        static void     tokenize(const std::string& inputString, const char delimiter, mint::Vector<std::string>& outArray);
        static void     tokenize(const std::string& inputString, const mint::Vector<char>& delimiterArray, mint::Vector<std::string>& outArray);
        static void     tokenize(const std::string& inputString, const std::string& delimiterString, mint::Vector<std::string>& outArray);
    }
}


#include <MintContainer/Include/StringUtil.inl>


#endif // !MINT_STRING_UTIL_H
