#include "BisonActions.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

HtmlNode *TagSemanticAction(const char *tagName, HtmlNode *children) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	HtmlNode *node = createTagNode(tagName, children);
	logDebugging(_logger, "Created Tag Node <%s>", tagName);
	return node;
}

HtmlNode *TextSemanticAction(char *text) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	HtmlNode *node = createTextNode(text);
	logDebugging(_logger, "Created Text Node \"%s\"", text);
	return node;
}

HtmlNode *AppendSiblingSemanticAction(HtmlNode *list, HtmlNode *newNode) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	HtmlNode *result = appendSibling(list, newNode);
	logDebugging(_logger, "Appended sibling node");
	return result;
}

Attribute *AttributeSemanticAction(const char *name, const char *value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Attribute *attr = createAttribute(name, value);
	logDebugging(_logger, "Created Attribute: %s=\"%s\"", name, value);
	return attr;
}

Program *ProgramSemanticAction(HtmlNode *root) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program *program = createHtmlProgram(root);
	if (_compilerState)
		_compilerState->abstractSyntaxtTree = program;
	logDebugging(_logger, "ProgramSemanticAction completed");
	return program;
}

Attribute *appendAttribute(Attribute *list, Attribute *newAttr) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    if (!list) return newAttr;
    Attribute *curr = list;
    while (curr->next) curr = curr->next;
    curr->next = newAttr;
    logDebugging(_logger, "Appended attribute %s", newAttr->name);
    return list;
}

HtmlNode *TagWithAttributesSemanticAction(const char *tagName, HtmlNode *children, Attribute *attrs) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    HtmlNode *node = createTagNode(tagName, children);
    node->attributes = attrs;
    logDebugging(_logger, "Created Tag Node <%s> with attributes", tagName);
    return node;
}

