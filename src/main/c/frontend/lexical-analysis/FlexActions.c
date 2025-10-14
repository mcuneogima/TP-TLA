#include "FlexActions.h"
#include "FlexExport.h"

/* MODULE INTERNAL STATE */

static bool _logIgnoredLexemes = true;
static InputBuffer * _inputBuffer = NULL;
static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownFlexActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: FlexActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	if (_inputBuffer != NULL) {
		destroyInputBuffer(_inputBuffer);
		_inputBuffer = NULL;
	}
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer) {
	_inputBuffer = NULL;
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("FlexActions");
	_logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
	return _shutdownFlexActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logTokenAction(const char * actionName, Token * token);

/**
 * Logs a lexical-analyzer action over a token in DEBUGGING level.
 */
static void _logTokenAction(const char * actionName, Token * token) {
	char * _lexeme = escape(token->lexeme);
	logDebugging(_logger, WARNING_COLOR "%s" DEFAULT_COLOR ": Token(context=%d, label=%d, length=%d, lexeme=%s\"%s\"%s, line=%d, semanticValue=%p)",
		actionName,
		token->context,
		token->label,
		token->length,
		INFORMATION_COLOR, _lexeme, DEFAULT_COLOR,
		token->line,
		token->semanticValue);
	free(_lexeme);
	_lexeme = NULL;
}

/* PUBLIC FUNCTIONS */

CompilationStatus TagOpenLexemeAction(TokenLabel label) {
    Token * token = createToken(_lexicalAnalyzer, label);
    logDebugging(_logger, "TagOpen: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

CompilationStatus TagCloseLexemeAction(TokenLabel label) {
    Token * token = createToken(_lexicalAnalyzer, label);
    logDebugging(_logger, "TagClose: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

CompilationStatus SelfClosingTagLexemeAction(TokenLabel label) {
    Token * token = createToken(_lexicalAnalyzer, label);
    logDebugging(_logger, "SelfClosing: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

CompilationStatus TextLexemeAction() {
    Token * token = createToken(_lexicalAnalyzer, TEXT);
    token->semanticValue->string = strdup(token->lexeme);
    logDebugging(_logger, "Text: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

CompilationStatus CommentLexemeAction() {
    logDebugging(_logger, "Comment ignored");
    return IN_PROGRESS;
}

CompilationStatus IgnoredLexemeAction() {
    return IN_PROGRESS;
}

CompilationStatus UnknownLexemeAction() {
    logError(_logger, "Unknown or invalid token found");
    return FAILED;
}

CompilationStatus EOFLexemeAction() {
    Token * token = createToken(_lexicalAnalyzer, EOF_TOKEN);
    pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return SUCCEEDED;
}

CompilationStatus AttributeNameLexemeAction() {
    Token *token = createToken(_lexicalAnalyzer, ATTRIBUTE_NAME);
    token->semanticValue->string = strdup(token->lexeme);
    logDebugging(_logger, "Attr name: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

CompilationStatus AttributeValueLexemeAction() {
    Token *token = createToken(_lexicalAnalyzer, ATTRIBUTE_VALUE);
    token->semanticValue->string = strdup(token->lexeme);
    logDebugging(_logger, "Attr value: %s", token->lexeme);
    CompilationStatus status = pushToken(_lexicalAnalyzer, token);
    destroyToken(token);
    return status;
}

