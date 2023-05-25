#include "Includes.h"

typedef struct Node_s{
    char *str;
    uint *ruleOccurances;   // number of times each rule occurs in str
    uint totalOccurances;
    struct Node_s **child;
}Node;

typedef struct NodeList_s{
    Node *node;
    struct NodeList_s *next;
}NodeList;

void printExamples(char *prog)
{
    const char *example[] = {
        "\"AA->B\" \"BA->CA\" \"AC->B\" \"CB->AB\" ACABBACABCB",
        "\"0->1\" \"1->2\" \"2->3\" 000",
        "\"01->10\" \"11->00\" 10101011",
        "\"101<->010\" \"111->0\" 101010101000010",
        "\"0->1\" \"1->2\" \"2->3\" \"3->4\" \"4->5\" \"5->6\" \"6->7\" \"7->8\" \"8->9\" \"9->0\" 00"
    };
    for(uint i = 0; i < 5; i++)
        printf("\nExample %u:\n%s %s\n", i+1, prog, example[i]);
}

// returns the number of times find occurs in str
uint numMatches(const char *str, const char *find)
{
    const char *c = str;
    uint ret = 0;
    do{
        if((c = strstr(c, find))!=NULL){
            c++;
            ret++;
        }
    }while(c);
    return ret;
}

// returns a pointer to the start of the nth occurence of find in str
const char* getMatchN(const char *str, const char *find, const uint n)
{
    const char *c = str;
    for(uint i = 0; i < n; i++){
        if((c = strstr(c, find))==NULL){
            printf("Could not get match number %u/%u of %s in %s!", i, n, find, str);
            exit(-1);
        }
        c++;
    }
    return strstr(c, find);
}

// returns the length of a string where replace has been substituted for one instance of find in str
uint newStrLen(const char *str, const char *find, const char *replace)
{
    const int ret = strlen(str)+(strlen(replace)-strlen(find));
    if(ret<0){
        printf("err: replaceLen < 0\n");
        printf("str: \"%s\", find: \"%s\", replace: \"%s\"\n", str, find, replace);
        exit(-1);
    }
    return (uint)ret;
}

// constructs a new string substituting replace for the Nth occourance of find in str
char* replaceN(char *str, const char *find, const char *replace, const uint n)
{
    char *ret = calloc(1, newStrLen(str, find, replace));

    // get nth match
    const char *matchPos = getMatchN(str, find, n);

    // get length up to the nth match
    const uint beforeMatchLen = matchPos-str;
    strncat(ret, str, beforeMatchLen);
    strcat(ret, replace);
    strcat(ret, matchPos+strlen(find));
    return ret;
}

// searches through list and returns the first node with matching str, or NULL if no match found
Node* searchStrList(NodeList *list, const char *str)
{
    while(list != NULL){
        if(strcmp(list->node->str, str) == 0)
            return list->node;
        list = list->next;
    }
    return NULL;
}

// counts the number of positions where the rule could be applied for every rule
void createOccurances(Node *n, Rule *list)
{
    const uint len = ruleListLen(list);
    n->ruleOccurances = calloc(len, sizeof(uint));
    for(uint i = 0; i < len; i++){
        n->ruleOccurances[i] = numMatches(n->str, rule->find);
        n->totalOccurances += n->ruleOccurances[i];
        rule = rule->next;
    }
}

// true if no more rules can be applied
bool isNormalForm(const char *str, Rule *list)
{
    while(list){
        if(numMatches(str, list) > 0)
            return false;
    }
    return true;
}

void printNormalForms(NodeList *nodes, Rule *list)
{
    uint count = 0;
    while(nodes != NULL){
        if(isNormalForm(nodes->node->str, list))
            printf("Normal form %u: %s\n", ++count, nodes->node->str);
        nodes = nodes->next;
    }
}

NodeList* rewrite(Node *n, Rule *list, NodeList *nodes)
{
    static uint count = 0;
    printf("str: %s, count: %u\n", n->str, count++);
    createOccurances(n, list);
    n->child = calloc(n->totalOccurances, sizeof(Node*));
    uint current = 0;

    while(list){
        for(uint i = 0; i < n->ruleOccurances[list->num]; i++){
            char *newstr = replaceN(n->str, list.find, list.replace, i);
            if((n->child[current+i] = searchStrList(nodes, newstr))==NULL){
                n->child[current+i] = calloc(1, sizeof(Node));
                n->child[current+i]->str = newstr;
                NodeList *head = calloc(1, sizeof(NodeList));
                head->next = nodes;
                head->node = n->child[current+i];
                nodes = rewrite(n->child[current+i], list, head);
            }else{
                free(newstr);
            }
        }
        current+=n->ruleOccurances[list->num];
        list = list->next;
    }

    if(current != n->totalOccurances){
        printf("Total child nodes generated != n->totalOccurances\n");
        exit(-1);
    }
    return nodes;
}

void freeList(NodeList *list)
{
    while(list != NULL){
        NodeList *newhead = list->next;
        free(list->node->str);
        free(list->node->ruleOccurances);
        free(list->node->child);
        free(list->node);
        free(list);
        list = newhead;
    }
}

int main(int argc, char **argv)
{
    Rule *list = ruleListParse(argc, argv);
    ruleListPrint(list);

    NodeList *nodes = calloc(1, sizeof(NodeList));
    nodes->node = calloc(1, sizeof(Node));
    nodes->node->str = strdup(argv[argc-1]);

    nodes = rewrite(nodes->node, list, nodes);
    printNormalForms(nodes, list);

    freeList(nodes);
    ruleListFree(list);

    return 0;
}
