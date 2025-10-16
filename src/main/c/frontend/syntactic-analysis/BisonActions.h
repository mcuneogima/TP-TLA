#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule();

/**
 * Bison semantic actions.
 */

HtmlNode *TagSemanticAction(const char *tagName, HtmlNode *children);
HtmlNode *TextSemanticAction(char *text);
HtmlNode *AppendSiblingSemanticAction(HtmlNode *list, HtmlNode *newNode);
Attribute *AttributeSemanticAction(const char *name, const char *value);
Program *ProgramSemanticAction(HtmlNode *root);
Attribute *appendAttribute(Attribute *list, Attribute *newAttr);
HtmlNode *TagWithAttributesSemanticAction(const char *tagName, HtmlNode *children, Attribute *attrs);


#endif
