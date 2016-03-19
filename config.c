#include <assert.h>
#include <yaml.h>

#include "config.h"

struct showcase_config config;

static void config_parse_page(yaml_document_t *document, yaml_node_t *node, struct showcase_page *page)
{
    assert(node->type == YAML_MAPPING_NODE);

    for (yaml_node_pair_t *child = node->data.mapping.pairs.start; child < node->data.mapping.pairs.top; child++) {
        yaml_node_t *key = yaml_document_get_node(document, child->key);
        yaml_node_t *value = yaml_document_get_node(document, child->value);

        assert(key->type == YAML_SCALAR_NODE);
        assert(value->type == YAML_SCALAR_NODE);

        if (!strncmp((char*)key->data.scalar.value, "wait_time", key->data.scalar.length))
            page->wait_time = atoi((char*)value->data.scalar.value);
        else if (!strncmp((char*)key->data.scalar.value, "url", key->data.scalar.length))
            page->url = strndup((char*)value->data.scalar.value, value->data.scalar.length);
    }
}

static void config_parse_pages(yaml_document_t *document, yaml_node_t *node)
{
    assert(node->type == YAML_SEQUENCE_NODE);

    size_t pages_num = node->data.sequence.items.top - node->data.sequence.items.start;
    config.pages_num = pages_num;
    config.pages = calloc(pages_num, sizeof(struct showcase_page));

    for (int i = 0; i < pages_num; i++) {
        yaml_node_t *child = yaml_document_get_node(document, node->data.sequence.items.start[i]);
        config_parse_page(document, child, &config.pages[i]);
    }
}

static void config_parse_root(yaml_document_t *document, yaml_node_t *node)
{
    assert(node->type == YAML_MAPPING_NODE);

    for (yaml_node_pair_t *child = node->data.mapping.pairs.start; child < node->data.mapping.pairs.top; child++) {
        yaml_node_t *key = yaml_document_get_node(document, child->key);
        yaml_node_t *value = yaml_document_get_node(document, child->value);

        assert(key->type == YAML_SCALAR_NODE);

        if (!strncmp((char*)key->data.scalar.value, "wait_time", key->data.scalar.length)) {
            assert(value->type == YAML_SCALAR_NODE);
            config.wait_time = atoi((char*)value->data.scalar.value);
        } else if (!strncmp((char*)key->data.scalar.value, "pages", key->data.scalar.length)) {
            config_parse_pages(document, value);
        }
    }
}

#define CONFIG_FILE "/etc/showcase.yaml"

void config_parse()
{
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser))
        goto parser_error;

    FILE *file = fopen(CONFIG_FILE, "rb");
    if (!file) {
        fprintf(stderr, "Can't find configuration file " CONFIG_FILE "\n");
        return;
    }

    assert(file);
    yaml_parser_set_input_file(&parser, file);

    yaml_document_t document;
    if (!yaml_parser_load(&parser, &document))
        goto parser_error;

    config_parse_root(&document, yaml_document_get_root_node(&document));

    yaml_document_delete(&document);
parser_error:
    /* Display a parser error message. */

    switch (parser.error)
    {
        case YAML_MEMORY_ERROR:
            fprintf(stderr, "Memory error: Not enough memory for parsing\n");
            break;

        case YAML_READER_ERROR:
            if (parser.problem_value != -1) {
                fprintf(stderr, "Reader error: %s: #%X at %zu\n", parser.problem,
                        parser.problem_value, parser.problem_offset);
            }
            else {
                fprintf(stderr, "Reader error: %s at %zu\n", parser.problem,
                        parser.problem_offset);
            }
            break;

        case YAML_SCANNER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Scanner error: %s at line %zu, column %zu\n"
                        "%s at line %zu, column %zu\n", parser.context,
                        parser.context_mark.line+1, parser.context_mark.column+1,
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Scanner error: %s at line %zu, column %zu\n",
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            break;

        case YAML_PARSER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Parser error: %s at line %zu, column %zu\n"
                        "%s at line %zu, column %zu\n", parser.context,
                        parser.context_mark.line+1, parser.context_mark.column+1,
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Parser error: %s at line %zu, column %zu\n",
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            break;

        case YAML_COMPOSER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Composer error: %s at line %zu, column %zu\n"
                        "%s at line %zu, column %zu\n", parser.context,
                        parser.context_mark.line+1, parser.context_mark.column+1,
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Composer error: %s at line %zu, column %zu\n",
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            break;

        default:
            /* Couldn't happen. */
            fprintf(stderr, "Internal error\n");
            break;
    }

    yaml_parser_delete(&parser);
}

void config_free()
{
    for (int i = 0; i < config.pages_num; i++)
        free((char*)config.pages[i].url);
    free(config.pages);
}
