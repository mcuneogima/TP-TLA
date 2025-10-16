#ifndef ABSTRACT_SYNTAX_TREE_HTML_HEADER
#define ABSTRACT_SYNTAX_TREE_HTML_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

ModuleDestructor initializeAbstractSyntaxTreeModule();

typedef enum {
    NODE_HTML,
    NODE_TAG,
    NODE_TEXT,
    NODE_ATTRIBUTE,
    NODE_ROOT
} NodeType;

typedef struct Attribute Attribute;
typedef struct HtmlNode HtmlNode;
typedef struct Program Program;

struct Attribute {
    char *name;
    char *value;
    Attribute *next;
};

struct HtmlNode {
    NodeType type;
    char *tagName;
    char *text;
    Attribute *attributes;
    HtmlNode *children;
    HtmlNode *next;
};

struct Program {
    HtmlNode *root;
};

/* Constructores */
HtmlNode *createTagNode(const char *tagName, HtmlNode *children);
HtmlNode *createTextNode(char *text);
HtmlNode *appendSibling(HtmlNode *list, HtmlNode *newNode);
HtmlNode *appendChild(HtmlNode *parent, HtmlNode *child);
Attribute *createAttribute(const char *name, const char *value);
Program *createHtmlProgram(HtmlNode *root);

/* Destructores */
void destroyHtmlNode(HtmlNode *node);
void destroyAttribute(Attribute *attr);
void destroyProgram(Program *program);

#endif