%top{
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"
}

%{
extern CompilationStatus TagOpenLexemeAction(TokenLabel label);
extern CompilationStatus TagCloseLexemeAction(TokenLabel label);
extern CompilationStatus SelfClosingTagLexemeAction(TokenLabel label);
extern CompilationStatus TextLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus CommentLexemeAction();
extern CompilationStatus UnknownLexemeAction();
extern CompilationStatus EOFLexemeAction();

extern CompilationStatus AttributeNameLexemeAction();
extern CompilationStatus AttributeValueLexemeAction();
%}

%{
/* Estado actual del tag abierto */
static TokenLabel _currentOpenTag = UNKNOWN;
%}

/* Contextos */
%x COMMENT
%x TAG_ATTRS
%x ATTR_VALUE

/* Patrones reutilizables */
whitespace   [ \t\r\n]+
identifier   [A-Za-z_:][A-Za-z0-9_:.-]*
dqstring     \"([^\"\\]|\\.)*\"
sqstring     \'([^\'\\]|\\.)*\'

%%

<!--                          { BEGIN(COMMENT); return CommentLexemeAction(); }
<COMMENT>-->                 { BEGIN(INITIAL); return IgnoredLexemeAction(); }
<COMMENT>[^-]+               { return IgnoredLexemeAction(); }
<COMMENT>-+                  { return IgnoredLexemeAction(); }

/* ==== TAGS DE APERTURA ==== */
/* Cuando abrimos un tag, pasamos a un estado donde podemos tener atributos */
"<html>"                     { return TagOpenLexemeAction(HTML_OPEN); }
"<head>"                     { return TagOpenLexemeAction(HEAD_OPEN); }
"<body>"                     { return TagOpenLexemeAction(BODY_OPEN); }

"<div>"                      { return TagOpenLexemeAction(DIV_OPEN); }
"<ul>"                       { return TagOpenLexemeAction(UL_OPEN); }
"<li>"                       { return TagOpenLexemeAction(LI_OPEN); }
"<p>"                        { return TagOpenLexemeAction(P_OPEN); }
"<button>"                   { return TagOpenLexemeAction(BUTTON_OPEN); }
"<header>"                   { return TagOpenLexemeAction(HEADER_OPEN); }
"<footer>"                   { return TagOpenLexemeAction(FOOTER_OPEN); }

"<h1>"                       { return TagOpenLexemeAction(H1_OPEN); }
"<h2>"                       { return TagOpenLexemeAction(H2_OPEN); }
"<h3>"                       { return TagOpenLexemeAction(H3_OPEN); }
"<h4>"                       { return TagOpenLexemeAction(H4_OPEN); }
"<h5>"                       { return TagOpenLexemeAction(H5_OPEN); }
"<h6>"                       { return TagOpenLexemeAction(H6_OPEN); }

/* ==== TAGS CON ATRIBUTOS ==== */
/* Tags que pueden tener atributos. Entramos a estado TAG_ATTRS */
"<img"   { _currentOpenTag = IMG_OPEN; BEGIN(TAG_ATTRS); return TagOpenLexemeAction(IMG_OPEN); }
"<input" { _currentOpenTag = INPUT_OPEN; BEGIN(TAG_ATTRS); return TagOpenLexemeAction(INPUT_OPEN); }

/* ==== ATRIBUTOS ==== */
<TAG_ATTRS>{whitespace}      { return IgnoredLexemeAction(); }
<TAG_ATTRS>{identifier}      { return AttributeNameLexemeAction(); }
<TAG_ATTRS>"="               { return IgnoredLexemeAction(); }
<TAG_ATTRS>{dqstring}        { return AttributeValueLexemeAction(); }
<TAG_ATTRS>{sqstring}        { return AttributeValueLexemeAction(); }

/* Cerramos el tag */
<TAG_ATTRS>[ \t]*">" {
    BEGIN(INITIAL);
    _currentOpenTag = UNKNOWN;
    return IgnoredLexemeAction();
}

<TAG_ATTRS>[ \t]*/">" {
    BEGIN(INITIAL);
    CompilationStatus status;
    if (_currentOpenTag == IMG_OPEN)
        status = SelfClosingTagLexemeAction(IMG_SELF);
    else if (_currentOpenTag == INPUT_OPEN)
        status = SelfClosingTagLexemeAction(INPUT_SELF);
    else
        status = SelfClosingTagLexemeAction(UNKNOWN);
    _currentOpenTag = UNKNOWN;
    return status;
}

/* ==== SELF-CLOSING ==== */
"<input/>"                   { return SelfClosingTagLexemeAction(INPUT_SELF); }
"<img/>"                     { return SelfClosingTagLexemeAction(IMG_SELF); }
"<head/>"                    { return SelfClosingTagLexemeAction(HEAD_SELF); }

/* ==== TAGS DE CIERRE ==== */
"</html>"                    { return TagCloseLexemeAction(HTML_CLOSE); }
"</body>"                    { return TagCloseLexemeAction(BODY_CLOSE); }
"</div>"                     { return TagCloseLexemeAction(DIV_CLOSE); }
"</ul>"                      { return TagCloseLexemeAction(UL_CLOSE); }
"</li>"                      { return TagCloseLexemeAction(LI_CLOSE); }
"</p>"                       { return TagCloseLexemeAction(P_CLOSE); }
"</button>"                  { return TagCloseLexemeAction(BUTTON_CLOSE); }
"</header>"                  { return TagCloseLexemeAction(HEADER_CLOSE); }
"</footer>"                  { return TagCloseLexemeAction(FOOTER_CLOSE); }
"</h1>"                      { return TagCloseLexemeAction(H1_CLOSE); }
"</h2>"                      { return TagCloseLexemeAction(H2_CLOSE); }
"</h3>"                      { return TagCloseLexemeAction(H3_CLOSE); }
"</h4>"                      { return TagCloseLexemeAction(H4_CLOSE); }
"</h5>"                      { return TagCloseLexemeAction(H5_CLOSE); }
"</h6>"                      { return TagCloseLexemeAction(H6_CLOSE); }

/* ==== TEXTO ==== */
[^<>\n\t\r]+                 { return TextLexemeAction(); }

{whitespace}                 { return IgnoredLexemeAction(); }

/* ==== ERROR ==== */
.                            { return UnknownLexemeAction(); }

<<EOF>>                      { return EOFLexemeAction(); }
%%
