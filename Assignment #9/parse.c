#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quiz.h"
#include <jansson.h>

/* Function to unescape HTML entities */
void unescape_html(char *str) {
    char *p = str;
    char *q = str;

    while (*p) {
        if (*p == '&') {
            if (strncmp(p, "&quot;", 6) == 0) {
                *q++ = '"';
                p += 6;
            } else if (strncmp(p, "&apos;", 6) == 0) {
                *q++ = '\'';
                p += 6;
            } else if (strncmp(p, "&amp;", 5) == 0) {
                *q++ = '&';
                p += 5;
            } else if (strncmp(p, "&lt;", 4) == 0) {
                *q++ = '<';
                p += 4;
            } else if (strncmp(p, "&gt;", 4) == 0) {
                *q++ = '>';
                p += 4;
            } else if (strncmp(p, "&#039;", 6) == 0) {
                *q++ = '\'';
                p += 6;
            } else {
                /* Unknown entity, copy as is */
                *q++ = *p++;
            }
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
}

int parse(quiz_t *quiz, char *msg) {
    json_error_t error;
    json_t *root, *results, *question_obj;

    root = json_loads(msg, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error: %s\n", error.text);
        return -1;
    }

    results = json_object_get(root, "results");
    if (!json_is_array(results)) {
        json_decref(root);
        return -1;
    }

    question_obj = json_array_get(results, 0);
    if (!json_is_object(question_obj)) {
        json_decref(root);
        return -1;
    }

    const char *question_text = json_string_value(json_object_get(question_obj, "question"));
    const char *correct_answer = json_string_value(json_object_get(question_obj, "correct_answer"));
    json_t *incorrect_answers = json_object_get(question_obj, "incorrect_answers");

    if (!question_text || !correct_answer || !json_is_array(incorrect_answers)) {
        json_decref(root);
        return -1;
    }

    quiz->question = strdup(question_text);
    if (!quiz->question) {
        json_decref(root);
        return -1;
    }
    unescape_html(quiz->question);

    /* Prepare an array to hold all four choices */
    char *choices[4];
    size_t num_incorrect = json_array_size(incorrect_answers);
    if (num_incorrect != 3) {
        json_decref(root);
        free(quiz->question);
        return -1;
    }

    choices[0] = strdup(correct_answer);
    if (!choices[0]) {
        json_decref(root);
        free(quiz->question);
        return -1;
    }
    unescape_html(choices[0]);

    /* Copy incorrect answers */
    for (int i = 0; i < 3; i++) {
        const char *incorrect = json_string_value(json_array_get(incorrect_answers, i));
        if (!incorrect) {
            json_decref(root);
            free(quiz->question);
            for (int j = 0; j <= i; j++) {
                free(choices[j]);
            }
            return -1;
        }
        choices[i + 1] = strdup(incorrect);
        if (!choices[i + 1]) {
            json_decref(root);
            free(quiz->question);
            for (int j = 0; j <= i; j++) {
                free(choices[j]);
            }
            return -1;
        }
        unescape_html(choices[i + 1]);
    }

    /* Shuffle choices and keep track of the correct answer's index */
    int indices[4] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    for (int i = 0; i < 4; i++) {
        quiz->choices[i] = choices[indices[i]];
        if (indices[i] == 0) {
            quiz->correct_choice = i;  /* Correct answer index */
        }
    }

    json_decref(root);
    return 0;
}
