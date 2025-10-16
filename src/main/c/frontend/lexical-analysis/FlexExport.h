#ifndef FLEX_EXPORT_HEADER
#define FLEX_EXPORT_HEADER

#include "../../support/type/FlexContext.h"
#include "../../support/type/LexicalAnalyzer.h"
#include <stdbool.h>

/**
 * Hook that allows to export a static function or variable from the inside of
 * Flex infrastructure, in this case, the current context (a.k.a. start
 * condition). This source exists only because Flex uses static for methods
 * that are in its public API, a clearly flawed design decision.
 */

FlexContext flexCurrentContext(LexicalAnalyzer * lexicalAnalyzer);

void flexEnterContext(LexicalAnalyzer * lexicalAnalyzer, FlexContext flexContext);

bool flexHasBuffer(LexicalAnalyzer * lexicalAnalyzer);

void flexLeaveContext(LexicalAnalyzer * lexicalAnalyzer);

#endif