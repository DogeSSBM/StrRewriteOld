#ifndef RULE_H
#define RULE_H

typedef struct Rule{
    uint num;
    char *find;
    char *replace;
    struct Rule *next;
}Rule;

void rulePrint(Rule *rule)
{
    printf("{\"%s\"->\"%s\"}\n", rule->find, rule->replace);
}

void ruleListPrint(Rule *rule)
{
    printf("----------------------------------\n");
    while(rule){
        printf("rule[%u] = ", rule->num);
        rulePrint(rule);
        rule = rule->next;
    }
    printf("----------------------------------\n");
}

Rule* ruleFree(Rule *rule)
{
    if(!rule)
        return NULL;
    if(rule->find)
        free(rule->find);
    if(rule->replace)
        free(rule->replace);
    Rule *next = rule->next;
    free(rule);
    return next;
}

void ruleListFree(Rule *rule)
{
    while(rule)
        rule = ruleFree(rule);
}

uint ruleListLen(Rule *rule)
{
    uint ret = 0;
    while(rule != NULL){
        rule = rule->next;
        ret++;
    }
    return ret;
}

Rule* ruleAppend(Rule *head, Rule *tail)
{
    if(!head)
        return tail;
    Rule *cur = head;
    while(cur->next){
        if(!tail){
            return head;
        }
        if(strcmp(cur->find, tail->find) == 0 && strcmp(cur->replace, tail->replace) == 0){
            tail = ruleFree(tail);
            cur = head;
        }else{
            cur = cur->next
        }
    };
    cur->next = tail;
    while(tail){
        tail->num = cur->num+1;
        cur = cur->next;
        tail = tail->next;
    }
    return head;
}

Rule* ruleNew(char *find, char *replace)
{
    Rule *rule = calloc(1, sizeof(Rule));
    rule->find = find;
    rule->replace = replace;
}

Rule* ruleParse(Rule *list, const char *str)
{
    char *arrow = strstr(str, "->");
    if(!arrow)
        panic("Rules must contain an arrow \"->\"", arrow);

    uint flen = arrow-str;
    char *find = malloc(flen+1);
    char *replace = malloc(strlen(arrow+2)+1);
    strncpy(find, str, flen);
    strcpy(replace, arrow+2);

    return list = ruleAppend(list, ruleNew(find, replace));
}

Rule ruleListParse(const int argc, char **argv)
{
    if(argc < 3){
        printf("Usage:\n%s <rule 1> [rule 2] [rule ...] <input string>\n", argv[0]);
        printf("\trules: find->replace \n");
        printExamples(argv[0]);
        exit(-1);
    }

    Rule *list = NULL;
    for(uint i = 1; i < argc-1; i++)
        list = ruleParse(list, argv[i]);

    return list;
}

#endif /* end of include guard: RULE_H */
