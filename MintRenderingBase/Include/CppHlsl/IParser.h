#pragma once


#ifndef MINT_I_PARSER_H
#define MINT_I_PARSER_H


#include <MintCommon/Include/CommonDefinitions.h>

#include <MintContainer/Include/Vector.h>
#include <MintContainer/Include/Tree.h>

#include <MintRenderingBase/Include/CppHlsl/LanguageCommon.h>


namespace mint
{
    namespace CppHlsl
    {
        class ILexer;


        class IParser abstract
        {
        protected:
            enum class ErrorType
            {
                GrouperMismatch,
                NoMatchingGrouper,
                WrongPredecessor,
                WrongSuccessor,
                LackOfCode,
                RepetitionOfCode,
                SymbolNotFound,
                WrongScope,

                COUNT
            };

        private:
            static constexpr const char* const kErrorTypeStringArray[][2]
            {
                { "Grouper mismatch"                    ,  "열고 닫는 기호가 서로 일치하지 않습니다."                                      },
                { "No matching grouper"                 ,  "여는 기호만 있고 닫는 기호가 없습니다."                                        },
                { "Wrong predecessor"                   ,  "앞에 온 것이 문법적으로 맞지 않습니다."                                        },
                { "Wrong successor"                     ,  "뒤에 온 것이 문법적으로 맞지 않습니다."                                        },
                { "Lack of code"                        ,  "더 있어야 할 코드가 없습니다."                                                 },
                { "Repetition of code"                  ,  "코드가 중복됩니다."                                                            },
                { "Symbol not found"                    ,  "해당 심볼을 찾을 수 없습니다."                                                 },
                { "Wrong scope"                         ,  "이곳에 사용할 수 없습니다."                                                    },
            };

        protected:
            static_assert(static_cast<uint32>(ErrorType::COUNT) == ARRAYSIZE(kErrorTypeStringArray));
            static constexpr const char*                    convertErrorTypeToTypeString(const ErrorType errorType);
            static constexpr const char*                    convertErrorTypeToContentString(const ErrorType errorType);

            class ErrorMessage
            {
            public:
                                                            ErrorMessage();
                                                            ErrorMessage(const SymbolTableItem& symbolTableItem, const ErrorType errorType);
                                                            ErrorMessage(const SymbolTableItem& symbolTableItem, const ErrorType errorType, const char* const additionalExplanation);

            private:
                const uint32                                _sourceAt;
                std::string                                 _message;
            };

        public:
                                                            IParser(ILexer& lexer);
            virtual                                         ~IParser() = default;

        public:
            virtual const bool                              execute() abstract;

        protected:
            void                                            reset();

        protected:
            const bool                                      continueParsing() const noexcept;

        protected:
            void                                            advanceSymbolPositionXXX(const uint32 advanceCount);

        protected:
            const bool                                      hasSymbol(const uint32 symbolPosition) const noexcept;
            const uint32                                    getSymbolPosition() const noexcept;
            SymbolTableItem&                                getSymbol(const uint32 symbolPosition) const noexcept;

        protected:
            const bool                                      findNextSymbol(const uint32 symbolPosition, const char* const cmp, uint32& outSymbolPosition) const noexcept;
            const bool                                      findNextSymbol(const uint32 symbolPosition, const SymbolClassifier symbolClassifier, uint32& outSymbolPosition) const noexcept;
            const bool                                      findNextSymbolEither(const uint32 symbolPosition, const char* const cmp0, const char* const cmp1, uint32& outSymbolPosition) const noexcept;
            const bool                                      findNextDepthMatchingGrouperCloseSymbol(const uint32 openSymbolPosition, uint32* const outCloseSymbolPosition = nullptr) const noexcept;
        
        protected:
            void                                            reportError(const SymbolTableItem& symbolTableItem, const ErrorType errorType);
            void                                            reportError(const SymbolTableItem& symbolTableItem, const ErrorType errorType, const char* const additionalExplanation);
            const bool                                      hasReportedErrors() const noexcept;

        protected:
            ILexer&                                         _lexer;
            mint::Vector<SymbolTableItem>&                  _symbolTable;

        private:
            uint32                                          _symbolAt;
            mint::Vector<ErrorMessage>                      _errorMessageArray;
            static const SymbolTableItem                    kRootSymbol;
        };
    }
}


#include <MintRenderingBase/Include/CppHlsl/IParser.inl>


#endif // !MINT_I_PARSER_H
