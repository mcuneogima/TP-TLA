#ifndef SEMANTIC_VALUE_HEADER
#define SEMANTIC_VALUE_HEADER

#include "TokenLabel.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../frontend/syntactic-analysis/BisonParser.h"

/**
 * The type of a Bison semantic value, that is, the meaning attached to a token.
 * Aquí definimos los tipos concretos que puede almacenar.
 */
typedef union SemanticValue {
	int integer;          // para números
	int intValue;         // alias más claro
	char *string;    // para nombres de tags, texto, atributos, etc.
	void *object;         // para AST nodes u otros punteros
} SemanticValue;

#endif
