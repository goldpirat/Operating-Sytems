#ifndef QUIZ_H
#define QUIZ_H

typedef struct {
    unsigned n;          
    unsigned score;      
    unsigned max;        
    char *question;      
    char *answer;        
    char *choices[4];    
    int correct_choice; 
} quiz_t;

/* Function prototypes remain the same */

extern char* fetch(char *url);
extern int parse(quiz_t *quiz, char *msg);
extern int play(quiz_t *quiz);

#endif
