#pragma once


#ifndef FS_LANGUAGE_COMMON_H
#define FS_LANGUAGE_COMMON_H


#include <CommonDefinitions.h>


namespace fs
{
	namespace Language
	{
		class Lexer;


		enum class SymbolClassifier : uint32
		{
			Delimiter,

			NumberLiteral,				// [0-9]+[.]*[0-9]*

			Keyword,					// if, else, class, ...

			GROUPER_BEGINS,
			//
			Grouper_Open,				// () {} []   �� <> �� ����!!!
			Grouper_Close,				// () {} []   �� <> �� ����!!!
			//
			GROUPER_ENDS,

			StringQuote,				// ' "

			StatementTerminator,		// ;

			Punctuator,					// ,

			OPERATOR_BEGINS,			// �� ��¥�� operator �� ù ���ڵ� �ݵ�� operator ��!?
			//
			AssignmentOperator,
			RelationalOperator,
			ArithmeticOperator,
			LogicalOperator,
			BitwiseOperator,
			MemberAccessOperator,
			OperatorCandiate,
			//
			OPERATOR_ENDS,

			Identifier,					// [a-zA-Z_][a-zA-Z_0-9]+

			RESERVED
		};
		static_assert(SymbolClassifier::Delimiter			<	SymbolClassifier::NumberLiteral		);
		static_assert(SymbolClassifier::NumberLiteral		<	SymbolClassifier::Keyword			);
		static_assert(SymbolClassifier::Keyword				<	SymbolClassifier::GROUPER_BEGINS	);
		static_assert(SymbolClassifier::GROUPER_BEGINS		<	SymbolClassifier::GROUPER_ENDS		);
		static_assert(SymbolClassifier::OPERATOR_BEGINS		<	SymbolClassifier::OPERATOR_ENDS		);
		static_assert(SymbolClassifier::OPERATOR_ENDS		<	SymbolClassifier::Identifier		);
		

		class SymbolTableItem
		{
			friend Lexer;

		public:
									SymbolTableItem()
										: _sourceAt{ 0 }
										, _symbolClassifier{ SymbolClassifier::Identifier }
										, _symbolIndex{ 0 }
									{
										__noop;
									}

									SymbolTableItem(const uint64 sourceAt, const SymbolClassifier symbolClassifier, const std::string& symbolString)
										: _sourceAt{ sourceAt }
										, _symbolClassifier{ symbolClassifier }
										, _symbolString{ symbolString }
										, _symbolIndex{ 0 }
									{
										__noop;
									}

									FS_INLINE void clearData()
									{
										// _sourceAt �� �׳� ���ܵд�.
										_symbolClassifier = SymbolClassifier::Identifier;
										_symbolString.clear();
										_symbolIndex = 0;
									}

									FS_INLINE const uint64 index() const noexcept
									{
										return _symbolIndex;
									}

		public:
			uint64					_sourceAt;
			SymbolClassifier		_symbolClassifier;
			std::string				_symbolString;

		private:
			uint64					_symbolIndex;
		};


		using SyntaxClassifierEnumType = uint32;
		class SyntaxTreeItem
		{
		public:
			SyntaxTreeItem()
				: _syntaxClassifier{ kUint32Max }
				, _symbolTableItemIndex{ 0 }
				, _symbolTableItem{ nullptr }
			{
				__noop;
			}

			SyntaxTreeItem(const SymbolTableItem& symbolTableItem)
				: _syntaxClassifier{ kUint32Max }
				, _symbolTableItemIndex{ symbolTableItem.index() }
				, _symbolTableItem{ &symbolTableItem }
			{
				__noop;
			}

			SyntaxTreeItem(const SymbolTableItem& symbolTableItem, const SyntaxClassifierEnumType syntaxClassifier)
				: _syntaxClassifier{ syntaxClassifier }
				, _symbolTableItemIndex{ symbolTableItem.index() }
				, _symbolTableItem{ &symbolTableItem }
			{
				__noop;
			}

		private:
			SyntaxClassifierEnumType	_syntaxClassifier;
			uint64					_symbolTableItemIndex;

		public:
			const SymbolTableItem*	_symbolTableItem;
		};
	}
}
#endif // !FS_LANGUAGE_COMMON_H
