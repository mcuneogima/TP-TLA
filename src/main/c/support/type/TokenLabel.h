#ifndef TOKEN_LABEL_HEADER
#define TOKEN_LABEL_HEADER

/**
 * The type of a Bison token label, that is, an identifier of a token, that
 * transports the lexeme and semantic value as a whole.
 */
typedef signed int TokenLabel;

/**
 * Token labels recognized by the HTML frontend.
 * Deben coincidir con los %token del parser (Bison).
 */
enum {
    UNKNOWN = 0,
    IGNORED,
    EOF_TOKEN,
    HTML_OPEN, HTML_CLOSE,
    HEAD_OPEN, HEAD_SELF,
    BODY_OPEN, BODY_CLOSE,
    DIV_OPEN, DIV_CLOSE,
    UL_OPEN, UL_CLOSE,
    LI_OPEN, LI_CLOSE,
    P_OPEN, P_CLOSE,
    BUTTON_OPEN, BUTTON_CLOSE,
    INPUT_OPEN, INPUT_SELF,
    IMG_OPEN, IMG_SELF,
    HEADER_OPEN, HEADER_CLOSE,
    FOOTER_OPEN, FOOTER_CLOSE,
    H1_OPEN, H1_CLOSE,
    H2_OPEN, H2_CLOSE,
    H3_OPEN, H3_CLOSE,
    H4_OPEN, H4_CLOSE,
    H5_OPEN, H5_CLOSE,
    H6_OPEN, H6_CLOSE,
    ATTRIBUTE_NAME,
    ATTRIBUTE_VALUE,
    TEXT,
    COMMENT
};

#endif
