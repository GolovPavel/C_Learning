#include "list.h"
 
 static List* list_new(int val) {
    List *list = (List*)malloc(sizeof(List));
        list->val = val;
            list->next = NULL;
                return list;
                }
                 
                 void list_add(List **list, int val) {
                    for(; *list != NULL; list = &(*list)->next);
                        *list = list_new(val);
                        }
                         
                         void list_node_del(List **node) {
                            free(*node);
                                (*node) = (*node)->next;
                                 }
                                  
                                   void list_del_by_value(List **list, int val) {
                                        for(; *list != NULL; list = &(*list)->next) {
                                                if((*list)->val == val) {
                                                            list_node_del(list);
                                                                        break;
                                                                                }
                                                                                    }
                                                                                    }
                                                                                     
                                                                                      void list_free(List *list) {
                                                                                        for(; list != NULL; list = list->next)
                                                                                                free(list);
                                                                                                 }
