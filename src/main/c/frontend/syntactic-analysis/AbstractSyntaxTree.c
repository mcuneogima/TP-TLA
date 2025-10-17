#include "AbstractSyntaxTree.h"
#include <stdlib.h>
#include <string.h>

static Logger * _logger = NULL;

void _shutdownAbstractSyntaxTreeModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
    _logger = createLogger("AbstractSyntaxTree");
    return _shutdownAbstractSyntaxTreeModule;
}

/* ====== Constructores ====== */

HtmlNode *createTagNode(const char *tagName, HtmlNode *children) {
    logDebugging(_logger, "Creating Tag Node: <%s>", tagName);
    HtmlNode *node = calloc(1, sizeof(HtmlNode));
    node->type = NODE_TAG;
    node->tagName = strdup(tagName);
    node->children = children;
    return node;
}

HtmlNode *createTextNode(char *text) {
    logDebugging(_logger, "Creating Text Node: \"%s\"", text);
    HtmlNode *node = calloc(1, sizeof(HtmlNode));
    node->type = NODE_TEXT;
    node->text = text;
    return node;
}

HtmlNode *appendSibling(HtmlNode *list, HtmlNode *newNode) {
    if (!list) return newNode;
    HtmlNode *curr = list;
    while (curr->next) curr = curr->next;
    curr->next = newNode;
    return list;
}

HtmlNode *appendChild(HtmlNode *parent, HtmlNode *child) {
    if (!parent->children) parent->children = child;
    else appendSibling(parent->children, child);
    return parent;
}

Attribute *createAttribute(char *name, char *value) {
    logDebugging(_logger, "Creating Attribute: %s=\"%s\"", name, value);
    Attribute *attr = calloc(1, sizeof(Attribute));
    attr->name = name;
    attr->value = value;
    return attr;
}

Program *createHtmlProgram(HtmlNode *root) {
    logDebugging(_logger, "Creating Program Node (root = %s)", 
                 root && root->tagName ? root->tagName : "NULL");
    Program *p = calloc(1, sizeof(Program));
    p->root = root;
    return p;
}

/* ====== Destructores ====== */

void destroyAttribute(Attribute *attr) {
    while (attr) {
        Attribute *next = attr->next;
        if (attr->name) free(attr->name);
        if (attr->value) free(attr->value);
        free(attr);
        attr = next;
    }
}

void destroyHtmlNode(HtmlNode *node) {
    if (!node) return;
    if (node->attributes) {
        destroyAttribute(node->attributes);
        node->attributes = NULL;
    }
    
    if (node->children) {
        destroyHtmlNode(node->children);
        node->children = NULL;
    }
    
    if (node->next) {
        destroyHtmlNode(node->next);
        node->next = NULL;
    }
    
    if (node->tagName) {
        free(node->tagName);
        node->tagName = NULL;
    }
    
    if (node->text) {
        free(node->text);
        node->text = NULL;
    }
    free(node);
}

void destroyProgram(Program *program) {
    if (!program) return;
	if (program->root) {
        destroyHtmlNode(program->root);
        program->root = NULL;
    }
    free(program);
}
