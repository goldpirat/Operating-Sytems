#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <signal.h>
#include "quiz.h"

/* Signal handler for CTRL-C */
void sigint_handler(int sig) {
    printf("\nThanks for playing today.\n");
    exit(0);
}

int main() {
    quiz_t quiz;
    memset(&quiz, 0, sizeof(quiz));

    /* Handle CTRL-C */
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("Answer multiple-choice questions about computer science.\n");
    printf("You score points for each correctly answered question.\n");
    printf("If you need multiple attempts to answer a question, the\n");
    printf("points you score for a correct answer go down.\n\n");

    while (1) {
        char *url = "https://opentdb.com/api.php?amount=1&category=18&type=multiple";
        char *response = fetch(url);
        if (!response) {
            fprintf(stderr, "Failed to fetch question\n");
            return 1;
        }

        if (parse(&quiz, response) != 0) {
            fprintf(stderr, "Failed to parse question\n");
            free(response);
            return 1;
        }
        free(response);

        int play_result = play(&quiz);

        /* Free dynamically allocated memory */
        for (int i = 0; i < 4; i++) {
            free(quiz.choices[i]);
            quiz.choices[i] = NULL;
        }
        free(quiz.question);
        quiz.question = NULL;
        free(quiz.answer);
        quiz.answer = NULL;

        if (play_result == 1) {
            /* User signaled EOF (CTRL-D) */
            printf("Your current score is %d/%d points.\n", quiz.score, quiz.max);
            printf("Thanks for playing today.\n");
            break;
        } else if (play_result != 0) {
            /* An error occurred */
            return 1;
        }

        /* Continue to next question */
    }

    printf("Your final score is %d/%d points.\n", quiz.score, quiz.max);
    return 0;
}
