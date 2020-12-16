#pragma once


#ifndef FS_CPP_PARSER_H
#define FS_CPP_PARSER_H


#include <CommonDefinitions.h>

#include <Language/IParser.h>
#include <Language/LanguageCommon.h>


namespace fs
{
	namespace Language
	{
		class ILexer;


		enum CppSyntaxClassifier : SyntaxClassifierEnumType
		{
			CppSyntaxClassifier_Preprocessor_Include,

			CppSyntaxClassifier_Literal_Number,
			CppSyntaxClassifier_Literal_String,
			CppSyntaxClassifier_Literal_TrueFalse,
			CppSyntaxClassifier_Literal_Nullptr,

			CppSyntaxClassifier_ClassStruct_Keyword,
			CppSyntaxClassifier_ClassStruct_Identifier,
			CppSyntaxClassifier_ClassStruct_AccessModifier,
			CppSyntaxClassifier_ClassStruct_Constructor,
			CppSyntaxClassifier_ClassStruct_Constructor_InitializerList,
			CppSyntaxClassifier_ClassStruct_Destructor,
			CppSyntaxClassifier_ClassStruct_MemberVariable,
			CppSyntaxClassifier_ClassStruct_MemberVariableIdentifier,
			CppSyntaxClassifier_Alignas,
			CppSyntaxClassifier_Alignas_Alignment,
			
			CppSyntaxClassifier_Type,
			CppSyntaxClassifier_Type_Specification,
			CppSyntaxClassifier_Type_PointerType,
			CppSyntaxClassifier_Type_ReferenceType,
			CppSyntaxClassifier_Type_RvalueReferenceType,
			CppSyntaxClassifier_Type_Value,
			CppSyntaxClassifier_Type_Alias,
			
			CppSyntaxClassifier_Function_Name,
			CppSyntaxClassifier_Function_Instructions,
			CppSyntaxClassifier_Function_Parameter,
			CppSyntaxClassifier_Function_Return,
			CppSyntaxClassifier_Function_Return_Value,

			CppSyntaxClassifier_Namespace,

			CppSyntaxClassifier_INVALID,
		};
		

		enum class CppMainInfo_TypeFlags : SyntaxMainInfoType
		{
			NONE			= 0,

			Const			= 1 <<  0,
			Constexpr		= 1 <<  1,
			Mutable			= 1 <<  2,
			Static			= 1 <<  3,
			ThreadLocal		= 1 <<  4,
			Short			= 1 <<  5,
			Long			= 1 <<  6,
			LongLong		= 1 <<  7,
			Unsigned		= 1 <<  8,
		};

		enum class CppMainInfo_FunctionAttributeFlags : SyntaxMainInfoType
		{
			NONE			= 0,

			Const			= 1 << 0,
			Noexcept		= 1 << 1,
			Override		= 1 << 2,
			Final			= 1 << 3,
			Abstract		= 1 << 4,
			Default			= 1 << 5,
			Delete			= 1 << 6,
		};


		enum class CppSubInfo_AccessModifier : SyntaxSubInfoType
		{
			NONE		= 0,
			
			Public		= 1,
			Protected	= 2,
			Private		= 3,
		};


		enum class CppTypeOf : uint8
		{
			INVALID,
			BuiltInType,
			LiteralType,
			UserDefinedType,
		};

		enum class CppTypeNodeParsingMethod : uint8
		{
			Expression,
			ClassStructMember,
			FunctionParameter,
		};

		enum class CppUserDefinedTypeInfo : uint8
		{
			Default,
			Abstract,
			Derived,
			DerivedFinal,
		};


		static const CppSubInfo_AccessModifier	convertStringToCppClassStructAccessModifier(const std::string& input);


		class CppTypeTableItem final
		{
		public:
											CppTypeTableItem()	= default;
											CppTypeTableItem(const SymbolTableItem& typeSymbol, const CppMainInfo_TypeFlags& typeFlags);
											CppTypeTableItem(const SymbolTableItem& typeSymbol, const CppUserDefinedTypeInfo& userDefinedTypeInfo);
											CppTypeTableItem(const SyntaxTreeItem& typeSyntax);
											~CppTypeTableItem()	= default;

		public:
			const std::string&				getTypeName() const noexcept;

		public:
			void							setTypeSize(const uint32 typeSize) noexcept;
			const uint32					getTypeSize() const noexcept;

		private:
			SymbolTableItem					_typeSymbol;
			CppMainInfo_TypeFlags			_typeFlags;
			CppUserDefinedTypeInfo			_userDefinedTypeInfo;

		private:
			uint32							_typeSize;
		};


		struct CppTypeModifierSet
		{
		public:
			bool	_isConst		= false;	// const �� �ߺ� ����!!
			bool	_isConstexpr	= false;	// For non-Parameter
			bool	_isMutable		= false;	// For ClassStruct
			bool	_isStatic		= false;	// For non-Parameter
			bool	_isThreadLocal	= false;	// For Expression
			bool	_isShort		= false;
			uint8	_longState		= 0;		// 0: none, 1: long, 2: long long
			uint8	_signState		= 0;		// 0: default signed, 1: explicit signed, 2: explicit unsgined (signed, unsigned �� �ߺ� �����ϳ� ������ �Ұ�!)

		public:
			const CppMainInfo_TypeFlags			getTypeFlags() const noexcept;
		};


		class CppHlslParser final : public IParser
		{
		public:
														CppHlslParser(ILexer& lexer);
			virtual										~CppHlslParser();

		public:
			void										preExecute();
			virtual const bool							execute() override final;

		private:
			const bool									parseCode(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, uint64& outAdvanceCount);
		
		private:
			const bool									parseClassStruct(const bool isStruct, const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, uint64& outAdvanceCount);
			const bool									parseClassStructMember(const SymbolTableItem& classIdentifierSymbol, const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, CppSubInfo_AccessModifier& inOutAccessModifier, uint64& outAdvanceCount, bool& outContinueParsing);
			const bool									parseClassStructInitializerList(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount);
			const bool									parseClassStructInitializerList_Item(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount, bool& outContinueParsing);
		
		private:
			const bool									parseFunctionParameters(const bool isDeclaration, const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode);
			const bool									parseFunctionParameters_Item(const bool isDeclaration, const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount);
			const bool									parseFunctionInstructions(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount);
		
		private:
			const bool									parseExpression(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount);
		
		private:
			const bool									isTypeChunk(const uint64 symbolPosition, uint64& outPostTypeChunkPosition);
			
			// Identifier ������ �Ľ�
			const bool									parseTypeNode(const CppTypeNodeParsingMethod parsingMethod, const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, TreeNodeAccessor<SyntaxTreeItem>& outTypeNode, uint64& outAdvanceCount);
			const bool									parseTypeNode_CheckModifiers(const CppTypeNodeParsingMethod parsingMethod, const uint64 symbolPosition, CppTypeModifierSet& outTypeModifierSet, uint64& outAdvanceCount);
		
		private:
			const bool									parseAlignas(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& ancestorNode, uint64& outAdvanceCount);
			const bool									parseUsing(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, uint64& outAdvanceCount);
		
		private:
			const bool									parseNamespace(const uint64 symbolPosition, TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, uint64& outAdvanceCount);

		private:
			TreeNodeAccessor<SyntaxTreeItem>			findNamespaceNode(const std::string& namespaceFullIdentifier) const noexcept;
			std::string									getNamespaceNodeFullIdentifier(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode) const noexcept;
			std::string									getNamespaceNodeFullIdentifier(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, const std::string& subNamespaceIdentifier) const noexcept;

		private:
			TreeNodeAccessor<SyntaxTreeItem>			findNamespaceNodeInternal(const TreeNodeAccessor<SyntaxTreeItem>& parentnamespaceNode, const std::string& namespaceIdentifier) const noexcept;
			const bool									isNamespaceNode(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode) const noexcept;

		private:
			static const CppSyntaxClassifier			convertSymbolToAccessModifierSyntax(const SymbolTableItem& symbol) noexcept;
			static const CppSyntaxClassifier			convertLiteralSymbolToSyntax(const SymbolTableItem& symbol) noexcept;
		
		public:
			void										registerTypeTemplate(const std::string& typeFullIdentifier, const uint32 typeSize);
		
		private:
			const uint64								registerType(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, const CppTypeTableItem& type);
			std::string									getTypeFullIdentifier(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, const std::string& typeIdentifier) const noexcept;

		private:
			const bool									registerTypeAlias(const std::string& typeAlias, const uint64 typeIndex);
		
		private:
			const bool									isSymbolType(const SymbolTableItem& typeSymbol) const noexcept;
			const bool									isSymbolTypeInternal(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, const SymbolTableItem& typeSymbol) const noexcept;
			const bool									isIdentifierType(const std::string& typeFullIdentifier) const noexcept;
			const bool									isBuiltInTypeXXX(const std::string& symbolString) const noexcept;
			const bool									isUserDefinedTypeXXX(const std::string& typeFullIdentifier) const noexcept;
			const std::string&							getUnaliasedSymbolStringXXX(const SymbolTableItem& symbol) const noexcept;
			const CppTypeOf								getTypeOfSymbol(const TreeNodeAccessor<SyntaxTreeItem>& namespaceNode, const SymbolTableItem& symbol) const noexcept;

		private:
			TreeNodeAccessor<SyntaxTreeItem>			_globalNamespaceNode;
			TreeNodeAccessor<SyntaxTreeItem>			_currentScopeNamespaceNode;

		private:
			std::vector<CppTypeTableItem>				_typeTable;
			std::unordered_map<std::string, uint64>		_typeTableUmap;

			std::unordered_map<std::string, uint64>		_typeAliasTableUmap;
		
		private:
			std::unordered_map<std::string, int8>		_builtInTypeUmap;
		
		private:
			static const SymbolTableItem				kInitializerListSymbol;
			static const SymbolTableItem				kMemberVariableListSymbol;
			static const SymbolTableItem				kParameterListSymbol;
			static const SymbolTableItem				kInstructionListSymbol;
			static const SymbolTableItem				kInvalidGrammarSymbol;
			static const SymbolTableItem				kImplicitIntTypeSymbol;
			static const SymbolTableItem				kGlobalNamespaceSymbol;
		};
	}
}


#include <Language/CppHlslParser.inl>


#endif // !FS_CPP_PARSER_H
