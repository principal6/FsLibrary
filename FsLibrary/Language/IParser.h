#pragma once


#ifndef FS_I_PARSER_H
#define FS_I_PARSER_H


#include <CommonDefinitions.h>

#include <FsLibrary/Container/Tree.h>

#include <FsLibrary/Language/LanguageCommon.h>


namespace fs
{
	namespace Language
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
				{ "Grouper mismatch"                   ,  "���� �ݴ� ��ȣ�� ���� ��ġ���� �ʽ��ϴ�."                                       },
				{ "No matching grouper"                ,  "���� ��ȣ�� �ְ� �ݴ� ��ȣ�� �����ϴ�."                                         },
				{ "Wrong predecessor"                  ,  "�տ� �� ���� ���������� ���� �ʽ��ϴ�."                                         },
				{ "Wrong successor"                    ,  "�ڿ� �� ���� ���������� ���� �ʽ��ϴ�."                                         },
				{ "Lack of code"                       ,  "�� �־�� �� �ڵ尡 �����ϴ�."                                                  },
				{ "Repetition of code"                 ,  "�ڵ尡 �ߺ��˴ϴ�."													           },
				{ "Symbol not found"				   ,  "�ش� �ɺ��� ã�� �� �����ϴ�."												   },
				{ "Wrong scope"				           ,  "�̰��� ����� �� �����ϴ�."												       },
			};

		protected:
			static_assert(static_cast<uint32>(ErrorType::COUNT) == ARRAYSIZE(kErrorTypeStringArray));
			static constexpr const char*					convertErrorTypeToTypeString(const ErrorType errorType);
			static constexpr const char*					convertErrorTypeToContentString(const ErrorType errorType);

			class ErrorMessage
			{
			public:
															ErrorMessage();
															ErrorMessage(const SymbolTableItem& symbolTableItem, const ErrorType errorType);
															ErrorMessage(const SymbolTableItem& symbolTableItem, const ErrorType errorType, const char* const additionalExplanation);

			private:
				const uint64								_sourceAt;
				std::string									_message;
			};

		public:
															IParser(ILexer& lexer);
			virtual											~IParser() = default;

		public:
			virtual const bool								execute() abstract;

		public:
			const fs::Tree<SyntaxTreeItem>&					getSyntaxTree() const noexcept;
			const std::string								getSyntaxTreeString() noexcept;

		private:
			void											getSyntaxTreeStringInternal(const uint64 headSpace, const TreeNodeAccessor<SyntaxTreeItem>& node, const uint64 depth, std::string& outResult) noexcept;

		protected:
			void											reset();

		protected:
			const bool										needToContinueParsing() const noexcept;

		protected:
			void											advanceSymbolPositionXXX(const uint64 advanceCount);

		protected:
			const bool										hasSymbol(const uint64 symbolPosition) const noexcept;
			const uint64									getSymbolPosition() const noexcept;
			SymbolTableItem&								getSymbol(const uint64 symbolPosition) const noexcept;

		protected:
			const bool										findNextSymbol(const uint64 symbolPosition, const char* const cmp, uint64& outSymbolPosition) const noexcept;
			const bool										findNextSymbol(const uint64 symbolPosition, const SymbolClassifier symbolClassifier, uint64& outSymbolPosition) const noexcept;
			const bool										findNextSymbolEither(const uint64 symbolPosition, const char* const cmp0, const char* const cmp1, uint64& outSymbolPosition) const noexcept;
			const bool										findNextDepthMatchingCloseSymbol(const uint64 openSymbolPosition, const char* const closeSymbolString, uint64& outSymbolPosition) const noexcept;
		
		protected:
			void											reportError(const SymbolTableItem& symbolTableItem, const ErrorType errorType);
			void											reportError(const SymbolTableItem& symbolTableItem, const ErrorType errorType, const char* const additionalExplanation);
			const bool										hasReportedErrors() const noexcept;

		protected:
			fs::TreeNodeAccessor<SyntaxTreeItem>			getSyntaxTreeRootNode() noexcept;
			
		protected:
			ILexer&											_lexer;
			std::vector<SymbolTableItem>&					_symbolTable;

		private:
			fs::Tree<SyntaxTreeItem>						_syntaxTree;
		
		private:
			uint64											_symbolAt;
			std::vector<ErrorMessage>						_errorMessageArray;
			static const SymbolTableItem					kRootSymbol;
		};
	}
}


#include <FsLibrary/Language/IParser.inl>


#endif // !FS_I_PARSER_H
