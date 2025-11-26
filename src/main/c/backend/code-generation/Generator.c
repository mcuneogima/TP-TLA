#include "Generator.h"

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;
static unsigned int _componentCounter = 0;
static unsigned int _headerCounter = 0;
static unsigned int _footerCounter = 0;

/** Component structure to store extracted components */
typedef struct ComponentList {
    char * name;
    HtmlNode * content;
    HtmlNode * originalNode;
    struct ComponentList * next;
} ComponentList;

static ComponentList * _components = NULL;

/** Shutdown module's internal state. */
void _shutdownGeneratorModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: Generator...");
        
        // Free component list
        ComponentList * current = _components;
        while (current != NULL) {
            ComponentList * next = current->next;
            free(current->name);
            free(current);
            current = next;
        }
        _components = NULL;
        
        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeGeneratorModule() {
    _logger = createLogger("Generator");
    _componentCounter = 0;
    _headerCounter = 0;
    _footerCounter = 0;
    _components = NULL;
    return _shutdownGeneratorModule;
}

/** PRIVATE FUNCTIONS */

static char * _indentation(const unsigned int indentationLevel);
static void _generatePrologue(void);
static void _generateEpilogue(void);
static void _generateProgram(Program * program);
static void _generateNode(const unsigned int indentationLevel, HtmlNode * node);
static void _generateTag(const unsigned int indentationLevel, HtmlNode * node);
static void _generateText(const unsigned int indentationLevel, HtmlNode * node);
static void _generateAttributes(const unsigned int indentationLevel, Attribute * attributes);
static void _generateInput(const unsigned int indentationLevel, HtmlNode * node);
static void _generateImage(const unsigned int indentationLevel, HtmlNode * node);
static void _output(const unsigned int indentationLevel, const char * const format, ...);
static const char * _mapTagToComposable(const char * tagName);
static const char * _getTextStyle(const char * tagName);
static char * _getAttributeValue(Attribute * attributes, const char * name);
static int _matchesPattern(HtmlNode * node, const char * pattern);
static void _generatePattern(const unsigned int indentationLevel, HtmlNode * node, const char * pattern);
static void _addComponent(const char * name, HtmlNode * content, HtmlNode * originalNode);
static void _generateComponent(const unsigned int indentationLevel, const char * name, HtmlNode * content);
static void _generateAllComponents(void);
static void _generateCustomCard(const unsigned int indentationLevel, HtmlNode * node);
static void _generateLabeledInput(const unsigned int indentationLevel, HtmlNode * node);
static void _generateImageWithFooter(const unsigned int indentationLevel, HtmlNode * node);
static char * _extractTextContent(HtmlNode * node);
static void _generateBaseComponents(void);
static int _isEmptyOrQuotes(const char * str);
static void _collectComponents(const unsigned int indentationLevel, HtmlNode * node);
static char * _findComponentByNode(HtmlNode * node);

/**
 * Checks if a string is NULL, empty, or just "".
 */
static int _isEmptyOrQuotes(const char * str) {
    if (str == NULL) return 1;
    if (strlen(str) == 0) return 1;
    if (strcmp(str, "\"\"") == 0) return 1;
    return 0;
}

/**
 * Maps HTML tag names to Kotlin Compose equivalents.
 */
static const char * _mapTagToComposable(const char * tagName) {
    if (strcmp(tagName, "div") == 0) return "Column";
    if (strcmp(tagName, "header") == 0) return "Column";
    if (strcmp(tagName, "footer") == 0) return "Column";
    if (strcmp(tagName, "body") == 0) return "Column";
    if (strcmp(tagName, "ul") == 0) return "Column";
    if (strcmp(tagName, "li") == 0) return "Row";
    if (strcmp(tagName, "button") == 0) return "Button";
    if (strcmp(tagName, "p") == 0) return "Text";
    if (strcmp(tagName, "span") == 0) return "Text";
    if (strcmp(tagName, "h1") == 0) return "Text";
    if (strcmp(tagName, "h2") == 0) return "Text";
    if (strcmp(tagName, "h3") == 0) return "Text";
    if (strcmp(tagName, "h4") == 0) return "Text";
    if (strcmp(tagName, "h5") == 0) return "Text";
    if (strcmp(tagName, "h6") == 0) return "Text";
    
    logWarning(_logger, "Unknown tag '%s', defaulting to Column", tagName);
    return "Column";
}

/**
 * Returns the appropriate text style for heading tags.
 */
static const char * _getTextStyle(const char * tagName) {
    if (strcmp(tagName, "h1") == 0) return "MaterialTheme.typography.headlineLarge";
    if (strcmp(tagName, "h2") == 0) return "MaterialTheme.typography.headlineMedium";
    if (strcmp(tagName, "h3") == 0) return "MaterialTheme.typography.headlineSmall";
    if (strcmp(tagName, "h4") == 0) return "MaterialTheme.typography.titleLarge";
    if (strcmp(tagName, "h5") == 0) return "MaterialTheme.typography.titleMedium";
    if (strcmp(tagName, "h6") == 0) return "MaterialTheme.typography.titleSmall";
    if (strcmp(tagName, "p") == 0) return "MaterialTheme.typography.bodyLarge";
    if (strcmp(tagName, "span") == 0) return "MaterialTheme.typography.bodyMedium";
    return NULL;
}

/**
 * Gets an attribute value by name.
 */
static char * _getAttributeValue(Attribute * attributes, const char * name) {
    Attribute * current = attributes;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Extracts text content from a node recursively.
 */
static char * _extractTextContent(HtmlNode * node) {
    if (node == NULL) {
        return strdup("");
    }
    
    if (node->type == NODE_TEXT) {
        return strdup(node->text);
    }
    
    if (node->children != NULL) {
        return _extractTextContent(node->children);
    }
    
    return strdup("");
}

/**
 * Checks if a div matches a specific pattern.
 * Patterns: "h2+p" (CustomCard), "p+input" (LabeledInput), "img+p" (ImageWithFooter)
 */
static int _matchesPattern(HtmlNode * node, const char * pattern) {
    if (node == NULL || node->type != NODE_TAG || strcmp(node->tagName, "div") != 0) {
        return 0;
    }
    
    HtmlNode * child1 = node->children;
    if (child1 == NULL || child1->type != NODE_TAG) {
        return 0;
    }
    
    HtmlNode * child2 = child1->next;
    if (child2 == NULL || child2->type != NODE_TAG) {
        return 0;
    }
    
    if (strcmp(pattern, "h2+p") == 0) {
        return strcmp(child1->tagName, "h2") == 0 && strcmp(child2->tagName, "p") == 0;
    }
    
    if (strcmp(pattern, "p+input") == 0) {
        return strcmp(child1->tagName, "p") == 0 && strcmp(child2->tagName, "input") == 0;
    }
    
    if (strcmp(pattern, "img+p") == 0) {
        return strcmp(child1->tagName, "img") == 0 && strcmp(child2->tagName, "p") == 0;
    }
    
    return 0;
}

/**
 * Generates a CustomCard component call.
 */
static void _generateCustomCard(const unsigned int indentationLevel, HtmlNode * node) {
    HtmlNode * h2 = node->children;
    HtmlNode * p = h2->next;
    
    char * title = _extractTextContent(h2);
    char * description = _extractTextContent(p);
    
    _output(indentationLevel, "CustomCard(\n");
    _output(indentationLevel + 1, "title = \"%s\",\n", title);
    _output(indentationLevel + 1, "description = \"%s\"\n", description);
    _output(indentationLevel, ")\n");
    
    free(title);
    free(description);
}

/**
 * Generates a LabeledInput component call.
 */
static void _generateLabeledInput(const unsigned int indentationLevel, HtmlNode * node) {
    HtmlNode * p = node->children;
    HtmlNode * input = p->next;
    
    char * label = _extractTextContent(p);
    char * placeholder = _getAttributeValue(input->attributes, "placeholder");
    char * type = _getAttributeValue(input->attributes, "type");
    
    // Attributes come with quotes, check if empty or just ""
    if (_isEmptyOrQuotes(placeholder)) placeholder = "\"\"";
    if (_isEmptyOrQuotes(type)) type = "\"text\"";
    
    _output(indentationLevel, "LabeledInput(\n");
    _output(indentationLevel + 1, "label = \"%s\",\n", label);
    _output(indentationLevel + 1, "placeholder = %s,\n", placeholder);
    _output(indentationLevel + 1, "inputType = %s\n", type);
    _output(indentationLevel, ")\n");
    
    free(label);
}

/**
 * Generates an ImageWithFooter component call.
 */
static void _generateImageWithFooter(const unsigned int indentationLevel, HtmlNode * node) {
    HtmlNode * img = node->children;
    HtmlNode * p = img->next;
    
    char * src = _getAttributeValue(img->attributes, "src");
    char * alt = _getAttributeValue(img->attributes, "alt");
    char * caption = _extractTextContent(p);
    
    // Attributes come with quotes, check if empty or just ""
    if (_isEmptyOrQuotes(src)) src = "\"\"";
    if (_isEmptyOrQuotes(alt)) alt = "\"\"";
    
    _output(indentationLevel, "ImageWithFooter(\n");
    _output(indentationLevel + 1, "imageUrl = %s,\n", src);
    _output(indentationLevel + 1, "contentDescription = %s,\n", alt);
    _output(indentationLevel + 1, "caption = \"%s\"\n", caption);
    _output(indentationLevel, ")\n");
    
    free(caption);
}

/**
 * Adds a component to the list.
 */
static void _addComponent(const char * name, HtmlNode * content, HtmlNode * originalNode) {
    ComponentList * newComponent = (ComponentList *)malloc(sizeof(ComponentList));
    newComponent->name = strdup(name);
    newComponent->content = content;
    newComponent->originalNode = originalNode;
    newComponent->next = _components;
    _components = newComponent;
}

/**
 * Generates base reusable components.
 */
static void _generateBaseComponents(void) {
    // CustomCard
    _output(0, "@Composable\n");
    _output(0, "fun CustomCard(title: String, description: String) {\n");
    _output(1, "Card(\n");
    _output(2, "modifier = Modifier.fillMaxWidth().padding(8.dp)\n");
    _output(1, ") {\n");
    _output(2, "Column(modifier = Modifier.padding(16.dp)) {\n");
    _output(3, "Text(text = title, style = MaterialTheme.typography.headlineMedium)\n");
    _output(3, "Spacer(modifier = Modifier.height(8.dp))\n");
    _output(3, "Text(text = description, style = MaterialTheme.typography.bodyLarge)\n");
    _output(2, "}\n");
    _output(1, "}\n");
    _output(0, "}\n\n");
    
    // LabeledInput
    _output(0, "@Composable\n");
    _output(0, "fun LabeledInput(label: String, placeholder: String, inputType: String) {\n");
    _output(1, "Column(modifier = Modifier.fillMaxWidth().padding(8.dp)) {\n");
    _output(2, "Text(text = label, style = MaterialTheme.typography.bodyLarge)\n");
    _output(2, "Spacer(modifier = Modifier.height(4.dp))\n");
    _output(2, "val keyboardType = when (inputType) {\n");
    _output(3, "\"email\" -> KeyboardType.Email\n");
    _output(3, "\"number\" -> KeyboardType.Number\n");
    _output(3, "\"password\" -> KeyboardType.Password\n");
    _output(3, "\"phone\" -> KeyboardType.Phone\n");
    _output(3, "else -> KeyboardType.Text\n");
    _output(2, "}\n");
    _output(2, "TextField(\n");
    _output(3, "value = \"\",\n");
    _output(3, "onValueChange = { },\n");
    _output(3, "placeholder = { Text(text = placeholder, color = Color.Gray) },\n");
    _output(3, "keyboardOptions = KeyboardOptions(keyboardType = keyboardType),\n");
    _output(3, "modifier = Modifier.fillMaxWidth()\n");
    _output(2, ")\n");
    _output(1, "}\n");
    _output(0, "}\n\n");
    
    // ImageWithFooter
    _output(0, "@Composable\n");
    _output(0, "fun ImageWithFooter(imageUrl: String, contentDescription: String, caption: String) {\n");
    _output(1, "Column(modifier = Modifier.fillMaxWidth().padding(8.dp)) {\n");
    _output(2, "// AsyncImage would be used here with Coil library\n");
    _output(2, "// For now, placeholder\n");
    _output(2, "Box(\n");
    _output(3, "modifier = Modifier.fillMaxWidth().height(200.dp)\n");
    _output(3, "    .background(Color.LightGray)\n");
    _output(2, ") {\n");
    _output(3, "Text(text = \"Image: $imageUrl\", modifier = Modifier.align(Alignment.Center))\n");
    _output(2, "}\n");
    _output(2, "Spacer(modifier = Modifier.height(8.dp))\n");
    _output(2, "Text(text = caption, style = MaterialTheme.typography.bodyMedium)\n");
    _output(1, "}\n");
    _output(0, "}\n\n");
}

/**
 * Generates a component function.
 */
static void _generateComponent(const unsigned int indentationLevel, const char * name, HtmlNode * content) {
    _output(0, "@Composable\n");
    _output(0, "fun %s() {\n", name);
    
    if (content != NULL) {
        _generateNode(1, content);
    }
    
    _output(0, "}\n\n");
}

/**
 * Finds a component name by its original node.
 */
static char * _findComponentByNode(HtmlNode * node) {
    ComponentList * current = _components;
    while (current != NULL) {
        if (current->originalNode == node) {
            return current->name;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Generates all collected components.
 */
static void _generateAllComponents(void) {
    ComponentList * current = _components;
    while (current != NULL) {
        _generateComponent(0, current->name, current->content);
        current = current->next;
    }
}

/**
 * Creates the prologue of the generated output.
 */
static void _generatePrologue(void) {
    _output(0, "%s",
        "import androidx.compose.foundation.background\n"
        "import androidx.compose.foundation.layout.*\n"
        "import androidx.compose.foundation.text.KeyboardOptions\n"
        "import androidx.compose.material3.*\n"
        "import androidx.compose.runtime.*\n"
        "import androidx.compose.ui.Alignment\n"
        "import androidx.compose.ui.Modifier\n"
        "import androidx.compose.ui.graphics.Color\n"
        "import androidx.compose.ui.text.input.KeyboardType\n"
        "import androidx.compose.ui.unit.dp\n\n"
    );
    
    _generateBaseComponents();
}

/**
 * Creates the epilogue of the generated output.
 */
static void _generateEpilogue(void) {
    _output(0, "}\n");
}

/**
 * First pass: collects header and footer components without generating output.
 */
static void _collectComponents(const unsigned int indentationLevel, HtmlNode * node) {
    if (node == NULL) {
        return;
    }
    
    if (node->type == NODE_TAG) {
        if (node->tagName != NULL) {
            // Collect header components
            if (strcmp(node->tagName, "header") == 0) {
                char componentName[64];
                sprintf(componentName, "HeaderComponent%d", ++_headerCounter);
                _addComponent(componentName, node->children, node);
            }
            
            // Collect footer components
            if (strcmp(node->tagName, "footer") == 0) {
                char componentName[64];
                sprintf(componentName, "FooterComponent%d", ++_footerCounter);
                _addComponent(componentName, node->children, node);
            }
            
            // Recursively collect from children
            if (node->children != NULL) {
                _collectComponents(indentationLevel + 1, node->children);
            }
        }
    }
    
    // Process siblings
    if (node->next != NULL) {
        _collectComponents(indentationLevel, node->next);
    }
}

/**
 * Generates the output of the program.
 */
static void _generateProgram(Program * program) {
    if (program == NULL || program->root == NULL) {
        logError(_logger, "Program or root node is NULL");
        return;
    }
    
    logDebugging(_logger, "Generating program from AST...");
    
    // First pass: collect components (header/footer)
    _collectComponents(1, program->root);
    
    // Generate all collected components
    _generateAllComponents();
    
    // Generate main view
    _output(0, "@Composable\n");
    _output(0, "fun GeneratedView() {\n");
    
    // Second pass: generate actual code
    _generateNode(1, program->root);
}

/**
 * Generates the output of a node (recursively).
 */
static void _generateNode(const unsigned int indentationLevel, HtmlNode * node) {
    if (node == NULL) {
        return;
    }
    
    switch (node->type) {
        case NODE_TAG:
            _generateTag(indentationLevel, node);
            break;
        case NODE_TEXT:
            _generateText(indentationLevel, node);
            break;
        default:
            logError(_logger, "Unknown node type: %d", node->type);
            break;
    }
    
    // Process siblings
    if (node->next != NULL) {
        _generateNode(indentationLevel, node->next);
    }
}

/**
 * Generates the output of a tag node.
 */
static void _generateTag(const unsigned int indentationLevel, HtmlNode * node) {
    if (node->tagName == NULL) {
        logError(_logger, "Tag name is NULL");
        return;
    }
    
    // Skip html, head, title tags
    if (strcmp(node->tagName, "html") == 0 || 
        strcmp(node->tagName, "head") == 0 || 
        strcmp(node->tagName, "title") == 0) {
        if (node->children != NULL) {
            _generateNode(indentationLevel, node->children);
        }
        return;
    }
    
    // Handle header and footer as components
    if (strcmp(node->tagName, "header") == 0) {
        char * componentName = _findComponentByNode(node);  
        if (componentName != NULL) {
            _output(indentationLevel, "%s()\n", componentName);
        }
        return;
    }
    
    if (strcmp(node->tagName, "footer") == 0) {
        char * componentName = _findComponentByNode(node);  
        if (componentName != NULL) {
            _output(indentationLevel, "%s()\n", componentName);
        }
        return;
    }
    
    // Check for pattern matching in div
    if (strcmp(node->tagName, "div") == 0) {
        if (_matchesPattern(node, "h2+p")) {
            _generateCustomCard(indentationLevel, node);
            return;
        }
        if (_matchesPattern(node, "p+input")) {
            _generateLabeledInput(indentationLevel, node);
            return;
        }
        if (_matchesPattern(node, "img+p")) {
            _generateImageWithFooter(indentationLevel, node);
            return;
        }
    }
    
    // Handle input
    if (strcmp(node->tagName, "input") == 0) {
        _generateInput(indentationLevel, node);
        return;
    }
    
    // Handle img
    if (strcmp(node->tagName, "img") == 0) {
        _generateImage(indentationLevel, node);
        return;
    }
    
    // Handle br
    if (strcmp(node->tagName, "br") == 0) {
        _output(indentationLevel, "Spacer(modifier = Modifier.height(8.dp))\n");
        return;
    }
    
    const char * composable = _mapTagToComposable(node->tagName);
    const char * textStyle = _getTextStyle(node->tagName);
    
    // Handle text-based tags
    if (strcmp(composable, "Text") == 0) {
        _output(indentationLevel, "%s(\n", composable);
        
        if (node->children != NULL && node->children->type == NODE_TEXT) {
            _output(indentationLevel + 1, "text = \"%s\"", node->children->text);
        } else {
            _output(indentationLevel + 1, "text = \"\"");
        }
        
        if (textStyle != NULL) {
            _output(0, ",\n");
            _output(indentationLevel + 1, "style = %s\n", textStyle);
        } else {
            _output(0, "\n");
        }
        
        _output(indentationLevel, ")\n");
        return;
    }
    
    // Handle Button
    if (strcmp(composable, "Button") == 0) {
        _output(indentationLevel, "Button(\n");
        _output(indentationLevel + 1, "onClick = { /* TODO */ }\n");
        _output(indentationLevel, ") {\n");
        
        if (node->children != NULL) {
            _generateNode(indentationLevel + 1, node->children);
        }
        
        _output(indentationLevel, "}\n");
        return;
    }
    
    // Handle container tags
    _output(indentationLevel, "%s", composable);
    
    if (node->attributes != NULL) {
        _output(0, "(\n");
        _generateAttributes(indentationLevel + 1, node->attributes);
        _output(indentationLevel, ")");
    }
    
    _output(0, " {\n");
    
    if (node->children != NULL) {
        _generateNode(indentationLevel + 1, node->children);
    }
    
    _output(indentationLevel, "}\n");
}

/**
 * Generates an input field.
 */
static void _generateInput(const unsigned int indentationLevel, HtmlNode * node) {
    char * type = _getAttributeValue(node->attributes, "type");
    char * placeholder = _getAttributeValue(node->attributes, "placeholder");
    
    // Attributes come with quotes already
    if (_isEmptyOrQuotes(type)) type = "\"text\"";
    if (_isEmptyOrQuotes(placeholder)) placeholder = "\"\"";
    
    _output(indentationLevel, "TextField(\n");
    _output(indentationLevel + 1, "value = \"\",\n");
    _output(indentationLevel + 1, "onValueChange = { },\n");
    _output(indentationLevel + 1, "placeholder = { Text(text = %s, color = Color.Gray) },\n", placeholder);
    _output(indentationLevel + 1, "keyboardOptions = KeyboardOptions(keyboardType = ");
    
    if (strcmp(type, "\"email\"") == 0) {
        _output(0, "KeyboardType.Email");
    } else if (strcmp(type, "\"number\"") == 0) {
        _output(0, "KeyboardType.Number");
    } else if (strcmp(type, "\"password\"") == 0) {
        _output(0, "KeyboardType.Password");
    } else if (strcmp(type, "\"phone\"") == 0) {
        _output(0, "KeyboardType.Phone");
    } else {
        _output(0, "KeyboardType.Text");
    }
    
    _output(0, ")\n");
    _output(indentationLevel, ")\n");
}

/**
 * Generates an image.
 */
static void _generateImage(const unsigned int indentationLevel, HtmlNode * node) {
    char * src = _getAttributeValue(node->attributes, "src");
    char * alt = _getAttributeValue(node->attributes, "alt");
    
    // Attributes come with quotes already
    if (_isEmptyOrQuotes(src)) src = "\"\"";
    if (_isEmptyOrQuotes(alt)) alt = "\"\"";
    
    _output(indentationLevel, "// Image: src=%s, alt=%s\n", src, alt);
    _output(indentationLevel, "Box(\n");
    _output(indentationLevel + 1, "modifier = Modifier.fillMaxWidth().height(200.dp)\n");
    _output(indentationLevel + 2, ".background(Color.LightGray)\n");
    _output(indentationLevel, ") {\n");
    _output(indentationLevel + 1, "Text(text = \"Image: \" + %s, modifier = Modifier.align(Alignment.Center))\n", src);
    _output(indentationLevel, "}\n");
}

/**
 * Generates the output of a text node.
 */
static void _generateText(const unsigned int indentationLevel, HtmlNode * node) {
    if (node->text == NULL || strlen(node->text) == 0) {
        return;
    }
    
    char * text = node->text;
    while (*text == ' ' || *text == '\n' || *text == '\t') {
        text++;
    }
    
    if (strlen(text) == 0) {
        return;
    }
    
    _output(indentationLevel, "Text(\"%s\")\n", text);
}

/**
 * Generates attributes as Compose modifiers.
 */
static void _generateAttributes(const unsigned int indentationLevel, Attribute * attributes) {
    if (attributes == NULL) {
        return;
    }
    
    _output(indentationLevel, "modifier = Modifier\n");
}

/**
 * Generates an indentation string for the specified level.
 */
static char * _indentation(const unsigned int level) {
    return indentation(_indentationCharacter, level, _indentationSize);
}

/**
 * Outputs a formatted string to standard output.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
    va_list arguments;
    va_start(arguments, format);
    char * indent = _indentation(indentationLevel);
    char * effectiveFormat = concatenate(2, indent, format);
    vfprintf(stdout, effectiveFormat, arguments);
    fflush(stdout);
    free(effectiveFormat);
    free(indent);
    va_end(arguments);
}

/** PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
    logDebugging(_logger, "Generating Kotlin Compose output...");
    _generatePrologue();
    _generateProgram(compilerState->abstractSyntaxtTree);
    _generateEpilogue();
    logDebugging(_logger, "Generation is done.");
}