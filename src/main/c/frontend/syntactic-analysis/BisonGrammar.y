%code requires {
    #include "AbstractSyntaxTree.h"
}

%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"
#include <string.h>

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {

	char *string;
	HtmlNode *node;
	Attribute *attribute;
	Program *program;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */

/** Terminals. */
%token <string> TEXT
%token <string> ATTRIBUTE_NAME ATTRIBUTE_VALUE
%token HTML_OPEN HTML_CLOSE
%token HEAD_OPEN HEAD_CLOSE
%token BODY_OPEN BODY_CLOSE
%token TITLE_OPEN TITLE_CLOSE
%token HEADER_OPEN HEADER_CLOSE
%token FOOTER_OPEN FOOTER_CLOSE
%token DIV_OPEN DIV_CLOSE
%token UL_OPEN UL_CLOSE
%token LI_OPEN LI_CLOSE
%token P_OPEN P_CLOSE
%token SPAN_OPEN SPAN_CLOSE
%token BUTTON_OPEN BUTTON_CLOSE
%token H1_OPEN H1_CLOSE
%token H2_OPEN H2_CLOSE
%token H3_OPEN H3_CLOSE
%token H4_OPEN H4_CLOSE
%token H5_OPEN H5_CLOSE
%token H6_OPEN H6_CLOSE
%token INPUT_OPEN IMG_OPEN
%token INPUT_SELF IMG_SELF HEAD_SELF BR_SELF
%token IGNORED UNKNOWN



/** Non-terminals. */
%type <node> html head_opt body elements element li_elements li_element
%type <program> program
%type <attribute> attribute attributes

%destructor { if ($$) destroyHtmlNode($$); } <node>
%destructor { if ($$) destroyAttribute($$); } <attribute>
%destructor { if ($$) free($$); } <string>


/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
      html                              { $$ = ProgramSemanticAction($1); }
    ;

html:
      HTML_OPEN head_opt body HTML_CLOSE { $$ = TagSemanticAction("html", appendSibling($2, $3)); }
    ;

head_opt:
      HEAD_OPEN elements HEAD_CLOSE     { $$ = TagSemanticAction("head", $2); }
	| HEAD_OPEN TITLE_OPEN TEXT TITLE_CLOSE HEAD_CLOSE
                                                    { 
                                                       HtmlNode *title = TagSemanticAction("title", TextSemanticAction($3));
                                                       $$ = TagSemanticAction("head", title);
                                                    }
    | HEAD_OPEN TITLE_OPEN TITLE_CLOSE HEAD_CLOSE   
                                                    { 
                                                       HtmlNode *title = TagSemanticAction("title", NULL);
                                                       $$ = TagSemanticAction("head", title);
                                                    }
    | %empty                            			{ $$ = NULL; }
    ;

body:
      BODY_OPEN elements BODY_CLOSE     { $$ = TagSemanticAction("body", $2); }
    ;

elements:
      element                           { $$ = $1; }
    | elements element                  { $$ = AppendSiblingSemanticAction($1, $2); }
    ;

element:
      P_OPEN TEXT P_CLOSE               { $$ = TagSemanticAction("p", TextSemanticAction($2)); }
    | P_OPEN P_CLOSE                    { $$ = TagSemanticAction("p", NULL); }
    | BUTTON_OPEN TEXT BUTTON_CLOSE     { $$ = TagSemanticAction("button", TextSemanticAction($2)); }
    | BUTTON_OPEN BUTTON_CLOSE          { $$ = TagSemanticAction("button", NULL); }
    | SPAN_OPEN TEXT SPAN_CLOSE         { $$ = TagSemanticAction("span", TextSemanticAction($2)); }
    | SPAN_OPEN SPAN_CLOSE              { $$ = TagSemanticAction("span", NULL); }
    | H1_OPEN TEXT H1_CLOSE             { $$ = TagSemanticAction("h1", TextSemanticAction($2)); }
    | H1_OPEN H1_CLOSE                  { $$ = TagSemanticAction("h1", NULL); }
    | H2_OPEN TEXT H2_CLOSE             { $$ = TagSemanticAction("h2", TextSemanticAction($2)); }
    | H2_OPEN H2_CLOSE                  { $$ = TagSemanticAction("h2", NULL); }
    | H3_OPEN TEXT H3_CLOSE             { $$ = TagSemanticAction("h3", TextSemanticAction($2)); }
    | H3_OPEN H3_CLOSE                  { $$ = TagSemanticAction("h3", NULL); }
    | H4_OPEN TEXT H4_CLOSE             { $$ = TagSemanticAction("h4", TextSemanticAction($2)); }
    | H4_OPEN H4_CLOSE                  { $$ = TagSemanticAction("h4", NULL); }
    | H5_OPEN TEXT H5_CLOSE             { $$ = TagSemanticAction("h5", TextSemanticAction($2)); }
    | H5_OPEN H5_CLOSE                  { $$ = TagSemanticAction("h5", NULL); }
    | H6_OPEN TEXT H6_CLOSE             { $$ = TagSemanticAction("h6", TextSemanticAction($2)); }
    | H6_OPEN H6_CLOSE                  { $$ = TagSemanticAction("h6", NULL); }
    | HEADER_OPEN elements HEADER_CLOSE { $$ = TagSemanticAction("header", $2); }
    | HEADER_OPEN HEADER_CLOSE          { $$ = TagSemanticAction("header", NULL); }
    | FOOTER_OPEN elements FOOTER_CLOSE { $$ = TagSemanticAction("footer", $2); }
    | FOOTER_OPEN FOOTER_CLOSE          { $$ = TagSemanticAction("footer", NULL); }
    | DIV_OPEN elements DIV_CLOSE       { $$ = TagSemanticAction("div", $2); }
    | DIV_OPEN DIV_CLOSE                { $$ = TagSemanticAction("div", NULL); }
    | IMG_OPEN attributes IMG_SELF      { $$ = TagWithAttributesSemanticAction("img", NULL, $2); }
    | INPUT_OPEN attributes INPUT_SELF  { $$ = TagWithAttributesSemanticAction("input", NULL, $2); }
    | BR_SELF                           { $$ = TagSemanticAction("br", NULL); }
    | TEXT                              { $$ = TextSemanticAction($1); }
	| UL_OPEN li_elements UL_CLOSE      { $$ = TagSemanticAction("ul", $2); }
	| UL_OPEN UL_CLOSE                  { $$ = TagSemanticAction("ul", NULL); }
	;

li_elements:
      li_element                       { $$ = $1; }
    | li_elements li_element           { $$ = AppendSiblingSemanticAction($1, $2); }
    ;

li_element:
      LI_OPEN TEXT LI_CLOSE            { $$ = TagSemanticAction("li", TextSemanticAction($2)); }
    | LI_OPEN LI_CLOSE                 { $$ = TagSemanticAction("li", NULL); }
    ;




	attribute:
		ATTRIBUTE_NAME ATTRIBUTE_VALUE
			{ $$ = AttributeSemanticAction($1, $2); }
		;

	attributes:
		%empty
			{ $$ = NULL; }
		| attributes attribute
			{ $$ = appendAttribute($1, $2); }
		;

%%
