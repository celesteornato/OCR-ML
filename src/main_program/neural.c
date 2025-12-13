#include "neural.h"
#include "grayscale.h"
#include <assert.h>
#include <err.h>
#include <math.h>
#include <matrix.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <threads.h>
#include <time.h>

#define countof(A) (sizeof(A) / sizeof(*A))

static double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}
static double dsigmoid(double x)
{
    return x * (1 - x);
}
/* static double relu(double x) */
/* { */
/*     return x >= 0 ? x : 0; */
/* } */
/* static double drelu(double x) */
/* { */
/*     return x >= 0 ? 1 : 0; */
/* } */

static double rnd(void)
{
    return ((double)random() / ((double)(1ULL << 31) - 1.0)) - 0.5;
}

static double (*output_func)(double) = sigmoid;
static double (*output_delta)(double) = dsigmoid;
static double (*hidden_func)(double) = sigmoid;
static double (*hidden_delta)(double) = dsigmoid;

void neural_save_weights(struct neural_network *nn, const char path[static 1])
{
    FILE *fileptr = fopen(path, "wb");
    if (fileptr == NULL)
    {
        errx(1, "Could not open file %s", path);
    }
    if (fwrite(nn, sizeof(*nn), 1, fileptr) != 1)
    {
        perror("Error while writing weights!");
        goto cleanup;
    }
cleanup:
    if (fclose(fileptr) != 0)
    {
        perror("Error while closing weight file!");
        thrd_exit(1);
    }
}

void neural_load_weights(struct neural_network *nn, const char path[static 1])
{
    FILE *fileptr = fopen(path, "rb");
    if (fileptr == NULL)
    {
        errx(1, "Could not open file %s", path);
    }
    if (fseek(fileptr, 0, SEEK_END) == -1)
    {
        perror("Could not fseek to end of weights file");
        goto cleanup;
    }
    (void)fseek(fileptr, 0, SEEK_SET);
    if (fread(nn, sizeof(*nn), 1, fileptr) != 1)
    {
        perror("Error while reading weights!");
        goto cleanup;
    }

cleanup:
    if (fclose(fileptr) != 0)
    {
        perror("Error while closing weight file!");
        thrd_exit(1);
    }
}

/* This macro could not have been replaced by a static inline func, as it relies
 * on what would otherwise be variable sizes. */
#define RAND_INIT_LAYER(lay, lay_biases, lay_weight, layprev)                  \
    do                                                                         \
    {                                                                          \
        for (size_t i = 0; i < countof(lay); ++i)                              \
        {                                                                      \
            (lay_biases)[i] = rnd();                                           \
            (lay)[i] = rnd();                                                  \
            for (size_t j = 0; j < countof(layprev); ++j)                      \
            {                                                                  \
                (lay_weight)[i][j] = rnd();                                    \
            }                                                                  \
        }                                                                      \
    } while (0)

static void randomize_layers(struct neural_network *nn)
{
    RAND_INIT_LAYER(nn->layer1, nn->layer1_biases, nn->layer1_weights,
                    nn->input);
    RAND_INIT_LAYER(nn->layer2, nn->layer2_biases, nn->layer2_weights,
                    nn->layer1);
    RAND_INIT_LAYER(nn->output, nn->output_biases, nn->output_weights,
                    nn->layer2);
}

static void forward_pass(struct neural_network *nn,
                         const uint_fast8_t input[static INPUT_SIZE])
{
    /* memset(nn->layer1, 0, sizeof(nn->layer1)); */
    /* memset(nn->layer2, 0, sizeof(nn->layer2)); */
    /* memset(nn->output, 0, sizeof(nn->output)); */

    memcpy(nn->input, input, sizeof(nn->input));

    // We then compute the product
    for (size_t i = 0; i < LAYER1_SIZE; ++i)
    {
        double dot = line_dot8(nn->input, nn->layer1_weights[i], INPUT_SIZE);
        nn->layer1[i] = dot - nn->layer1_biases[i];
    }

    line_map(nn->layer1, LAYER1_SIZE, hidden_func);

    for (size_t i = 0; i < LAYER2_SIZE; ++i)
    {
        double dot = line_dot(nn->layer1, nn->layer2_weights[i], LAYER1_SIZE);
        nn->layer2[i] = dot - nn->layer2_biases[i];
    }
    line_map(nn->layer2, LAYER2_SIZE, hidden_func);

    for (size_t i = 0; i < OUTPUT_SIZE; ++i)
    {
        double dot = line_dot(nn->layer2, nn->output_weights[i], LAYER2_SIZE);
        nn->output[i] = dot - nn->output_biases[i];
    }
    line_map(nn->output, OUTPUT_SIZE, output_func);
}

char neural_find_logic(struct neural_network *nn, const char path[static 1])
{
    uint_fast8_t input[INPUT_SIZE] = {0};
    path_to_bytes(path, input, 32, 32);
    forward_pass(nn, input);

    return (char)('a' + max_i(nn->output, countof(nn->output)));
}

/* Same thing here, we cannot replace this with a static inline. I'm sorry. */
#define PROPAG(lay, lay_d, nlay_d, nlay_w)                                     \
    do                                                                         \
    {                                                                          \
        for (size_t i = 0; i < countof(lay); ++i)                              \
        {                                                                      \
            double sum = 0;                                                    \
            for (size_t j = 0; j < countof(nlay_d); ++j)                       \
            {                                                                  \
                sum += (nlay_w)[j][i] * (nlay_d)[j];                           \
            }                                                                  \
            (lay_d)[i] = hidden_delta((lay)[i]) * sum;                         \
        }                                                                      \
    } while (0)

#define APPLY(lay_w, lay_d, lay_b, play)                                       \
    do                                                                         \
    {                                                                          \
        for (size_t i = 0; i < countof(lay_w); ++i)                            \
        {                                                                      \
            for (size_t j = 0; j < countof(play); ++j)                         \
            {                                                                  \
                (lay_w)[i][j] += LEARNING_RATE * (lay_d)[i] * (play)[j];       \
            }                                                                  \
            (lay_b)[i] += LEARNING_RATE * (lay_d)[i];                          \
        }                                                                      \
    } while (0)

static void back_propagate(struct neural_network *nn,
                           double expected[static OUTPUT_SIZE])
{
    double otp_delta[OUTPUT_SIZE] = {0};
    double layer2_delta[LAYER2_SIZE] = {0};
    double layer1_delta[LAYER1_SIZE] = {0};

    // This one is a special case, so we don't put it in the macro.
    for (size_t i = 0; i < OUTPUT_SIZE; ++i)
    {
        double otp = nn->output[i];
        otp_delta[i] = (expected[i] - otp) * output_delta(otp);
    }

    PROPAG(nn->layer2, layer2_delta, otp_delta, nn->output_weights);
    PROPAG(nn->layer1, layer1_delta, layer2_delta, nn->layer2_weights);

    APPLY(nn->output_weights, otp_delta, nn->output_biases, nn->layer2);
    APPLY(nn->layer2_weights, layer2_delta, nn->layer2_biases, nn->layer1);
    APPLY(nn->layer1_weights, layer1_delta, nn->layer1_biases, nn->input);
}

static bool must_stop = false;
void sigusr_handle(int _)
{
    (void)_;
    must_stop = true;
}
void neural_train(struct neural_network *nn)
{
    srandom((unsigned int)time(NULL));
    randomize_layers(nn);

    (void)signal(SIGUSR1, sigusr_handle);

    size_t best_correct = 0;
    size_t hasnt_beaten_correct_count = 0;

    for (int i = 0; i < EPOCHS && !must_stop; ++i)
    {
        printf("\nEntering Epoch %d: %d%% to the end\n", i, (100 * i) / EPOCHS);
        size_t correct = 0;

        const char *letter_path = "assets/letters";
        if ((random() % 15) == 0)
        {
            letter_path = "assets/comparison";
	    printf("Accursed generation!😱😱😱😱 \n");
        }

        for (int j = 0; j < DATASET_SIZE; ++j)
        {
#ifdef DEBUGPRINT
            if (j == DATASET_SIZE - 1 || (j % (DATASET_SIZE / 100)) == 0)
            {
                printf("\r\tEpoch %d: %d%%", i, (100 * j) / DATASET_SIZE);
                fflush(stdout);
            }
#endif
            size_t letter_idx = (size_t)(random() % 26);
            char expected_letter = (char)('a' + letter_idx);

            char path[128] = {0};
            assert(snprintf(path, sizeof(path), "%s/%c/%ld.bmp", letter_path,
                            expected_letter,
                            random() % DATASET_PER_INPUT) < (long)sizeof(path));

            uint_fast8_t input[INPUT_SIZE];
            if (!path_to_bytes(path, input, 32, 32))
            {
                continue;
            }

            double expected[OUTPUT_SIZE] = {0};
            expected[letter_idx] = 1;

            forward_pass(nn, input);
            back_propagate(nn, expected);

            size_t obtained = max_i(nn->output, countof(nn->output));
            if (obtained == letter_idx)
            {
                correct += 1;
            }
        }
#ifdef DEBUGPRINT
        printf("\nEpoch %d: accuracy of %.1f%%\n", i,
               (100.0 * (double)correct) / (double)DATASET_SIZE);
#endif

        if (correct > best_correct)
        {
            best_correct = correct;
            hasnt_beaten_correct_count = 0;
            continue;
        }
        hasnt_beaten_correct_count += 1;
        if (hasnt_beaten_correct_count >= 3)
        {
            break; //  Early stopping
        }
    }
}
