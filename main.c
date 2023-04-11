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

typedef struct{
    char *find;
    char *replace;
}Rule;

typedef struct RuleList_s{
    uint ruleNum;
    char *find;
    char *replace;
    struct RuleList_s *next;
}RuleList;

typedef struct{
    uint numRules;
    Rule *rule;
}RuleSet;

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

void printRule(const Rule rule)
{
    printf("{\"%s\"->\"%s\"}\n", rule.find, rule.replace);
}

void printRuleSet(const RuleSet rs)
{
    printf("----------------------------------\n");
    for(uint i = 0; i < rs.numRules; i++){
        printf("rule[%u] = ", i);
        printRule(rs.rule[i]);
    }
    printf("----------------------------------\n");
}

void freeRule(const Rule rule)
{
    if(rule.find != NULL)
        free(rule.find);
    if(rule.replace != NULL)
        free(rule.replace);
}

void freeRuleSet(RuleSet rs)
{
    if(rs.numRules == 0)
        return;
    for(uint i = 0; i < rs.numRules; i++)
        freeRule(rs.rule[i]);
    free(rs.rule);
}

uint ruleListLen(RuleList *list)
{
    uint ret = 0;
    while(list != NULL){
        list = list->next;
        ret++;
    }
    return ret;
}

RuleSet convertRuleListToSet(RuleList *list)
{
    const uint len = ruleListLen(list);
    RuleSet ret = {
        .numRules = len,
        .rule = malloc(sizeof(Rule)*(len)),
    };
    RuleList *next;
    for(uint i = 0; i < len; i++){
        ret.rule[i].find = list->find;
        ret.rule[i].replace = list->replace;
        next = list->next;
        free(list);
        list = next;
    }
    if(next != NULL){
        printf("Error! RuleList longer than expected\n");
        exit(-1);
    }
    return ret;
}

RuleList* appendRuleList(char *find, char *replace, RuleList *list)
{
    RuleList *current = list;
    while(current != NULL){
        if(strcmp(find, current->find) == 0 && strcmp(replace, current->replace) == 0){
            free(find);
            free(replace);
            return list;
        }
        current = current->next;
    }
    RuleList *next = list;
    list = calloc(1, sizeof(RuleList));
    if(next != NULL)
        list->ruleNum = next->ruleNum+1;
    else
        list->ruleNum = 0;
    list->next = next;
    list->find = find;
    list->replace = replace;
    return list;
}

RuleList* parseSingleArrow(const char *str, RuleList *list)
{
    char *arrow = strstr(str, "->");
    if(arrow == NULL){
        printf("Error parsing \"%s\".\nRules must be written \"A<->B\" or \"A->B\"\n", str);
        exit(-1);
    }
    if(strstr(arrow+1, "->")!=NULL){
        printf("Error parsing \"%s\".\nRewrite rules must have only one \"->\"\n", str);
        exit(-1);
    }
    if(strstr(str, "<->")){
        printf("Error parsing \"%s\".\nRules can only contain\"<->\" or \"->\" not both\n", str);
        exit(-1);
    }
    uint flen = arrow-str;
    char *find = malloc(flen+1);
    char *replace = malloc(strlen(arrow+2)+1);
    strncpy(find, str, flen);
    strcpy(replace, arrow+2);
    return list = appendRuleList(find, replace, list);
}

RuleList* parseArrow(const char *str, RuleList *list)
{
    char *arrow = strstr(str, "<->");
    if(arrow == NULL)
        return parseSingleArrow(str, list);
    if(strstr(arrow+1, "<->")!=NULL){
        printf("Error parsing \"%s\".\nEquivalence rules must have only one \"<->\"\n", str);
        exit(-1);
    }
    char *singleArrow = strstr(str, "->");
    if(singleArrow != arrow+1 || strstr(arrow+2, "->")){
        printf("Error parsing \"%s\".\nRules can only contain\"<->\" or \"->\" not both\n", str);
        exit(-1);
    }
    uint flen = arrow-str;
    char *find1 = malloc(flen+1);
    char *replace1 = malloc(strlen(arrow+3)+1);
    strncpy(find1, str, flen);
    strcpy(replace1, arrow+3);

    char *find2 = strdup(replace1);
    char *replace2 = strdup(find1);

    return list = appendRuleList(find2, replace2, list = appendRuleList(find1, replace1, list));
}

RuleSet parseRuleSet(const uint argc, char **argv)
{
    if(argc < 3){
        printf("Usage:\n%s <rule 1> [rule 2] [rule ...] <input string>\n", argv[0]);
        printf("\trules: find->replace \n");
        printExamples(argv[0]);
        exit(-1);
    }

    RuleList *list = NULL;
    for(uint i = 1; i < argc-1; i++)
        list = parseArrow(argv[i], list);

    return convertRuleListToSet(list);
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
void createOccurances(Node *n, const RuleSet rs)
{
    n->ruleOccurances = calloc(rs.numRules, sizeof(uint));
    for(uint i = 0; i < rs.numRules; i++){
        n->ruleOccurances[i] = numMatches(n->str, rs.rule[i].find);
        n->totalOccurances += n->ruleOccurances[i];
    }
}

// true if no more rules can be applied
bool isNormalForm(const char *str, const RuleSet rs)
{
    for(uint i = 0; i < rs.numRules; i++){
        if(numMatches(str, rs.rule[i].find)>0)
            return false;
    }
    return true;
}

void printNormalForms(NodeList *list, const RuleSet rs)
{
    uint count = 0;
    while(list != NULL){
        if(isNormalForm(list->node->str, rs))
            printf("Normal form %u: %s\n", ++count, list->node->str);
        list = list->next;
    }
}

NodeList* rewrite(Node *n, const RuleSet rs, NodeList *list)
{
    static uint count = 0;
    printf("str: %s, count: %u\n", n->str, count++);
    createOccurances(n, rs);
    n->child = calloc(n->totalOccurances, sizeof(Node*));
    uint current = 0;
    for(uint i = 0; i < rs.numRules; i++){
        for(uint j = 0; j < n->ruleOccurances[i]; j++){
            char *newstr = replaceN(n->str, rs.rule[i].find, rs.rule[i].replace, j);
            if((n->child[current+j] = searchStrList(list, newstr))==NULL){
                n->child[current+j] = calloc(1, sizeof(Node));
                n->child[current+j]->str = newstr;
                NodeList *head = calloc(1, sizeof(NodeList));
                head->next = list;
                head->node = n->child[current+j];
                list = rewrite(n->child[current+j], rs, head);
            }else{
                free(newstr);
            }
        }
        current+=n->ruleOccurances[i];
    }
    if(current != n->totalOccurances){
        printf("Total child nodes generated != n->totalOccurances\n");
        exit(-1);
    }
    return list;
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
    RuleSet rs = parseRuleSet(argc, argv);
    printRuleSet(rs);
    NodeList *list = calloc(1, sizeof(NodeList));
    list->node = calloc(1, sizeof(Node));
    list->node->str = strdup(argv[argc-1]);
    list = rewrite(list->node, rs, list);
    printNormalForms(list, rs);
    freeList(list);
    freeRuleSet(rs);

    return 0;
}
