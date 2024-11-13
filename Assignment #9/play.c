#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "quiz.h"

int play(quiz_t *quiz) {
    quiz->n++;           /* Increment question number */
    int attempts = 3;
    int points = 8;
    quiz->max += 8;      /* Increase possible max score */

    printf("%s\n\n", quiz->question);
    printf("[a] %s\n", quiz->choices[0]);
    printf("[b] %s\n", quiz->choices[1]);
    printf("[c] %s\n", quiz->choices[2]);
    printf("[d] %s\n", quiz->choices[3]);

    char input[10];
    while (attempts > 0) {
        printf("(%dpt) > ", points);
        if (!fgets(input, sizeof(input), stdin)) {
            /* EOF detected */
            return 1;  /* Signal to main() to exit the loop */
        }
        /* Remove newline character */
        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0') {
            /* Empty input, prompt again */
            continue;
        }

        char choice = tolower(input[0]);
        int choice_index = choice - 'a';

        if (choice_index < 0 || choice_index > 3) {
            printf("Invalid choice, please select a, b, c, or d.\n");
            continue;
        }

        if (choice_index == quiz->correct_choice) {
            quiz->score += points;
            printf("Congratulations, answer [%c] is correct. Your current score is %d/%d points.\n\n",
                   choice, quiz->score, quiz->max);
            return 0;
        } else {
            printf("Answer [%c] is wrong, try again.\n", choice);
            attempts--;
            points /= 2;
            if (points == 0) {
                points = 1;  /* Ensure points do not become zero */
            }
        }
    }

    /* If user runs out of attempts */
    printf("Sorry, you ran out of attempts. The correct answer was [%c].\n\n",
           'a' + quiz->correct_choice);
    return 0;
}
