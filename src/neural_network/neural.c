#include <err.h>
#include <math.h>
#include <matrix.h>
#include <neural.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define countof(A) (sizeof(A) / sizeof(*A))

static double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}
static double rnd(void)
{
    return ((double)random() / ((double)(1ULL << 31) - 1.0)) - 0.5;
}
static bool rndb(void)
{
    return random() % 2;
}

/* This macro could not have been replaced by a static inline func, as it relies on what would
 * otherwise be variable sizes. */
#define RAND_INIT_LAYER(lay, lay_biases, lay_weight, layprev)                                      \
    do                                                                                             \
    {                                                                                              \
        for (size_t i = 0; i < countof(lay); ++i)                                                  \
        {                                                                                          \
            (lay_biases)[i] = rnd();                                                               \
            (lay)[i] = rnd();                                                                      \
            for (size_t j = 0; j < countof(layprev); ++j)                                          \
            {                                                                                      \
                (lay_weight)[i][j] = rnd();                                                        \
            }                                                                                      \
        }                                                                                          \
    } while (0)

static void randomize_layers(struct neural_network *nn)
{
    RAND_INIT_LAYER(nn->layer1, nn->layer1_biases, nn->layer1_weights, nn->input);
    RAND_INIT_LAYER(nn->layer2, nn->layer2_biases, nn->layer2_weights, nn->layer1);
    RAND_INIT_LAYER(nn->output, nn->output_biases, nn->output_weights, nn->layer2);
}

static void forward_pass(struct neural_network *nn, const double input[static INPUT_SIZE])
{
    memset(nn->layer1, 0, sizeof(nn->layer1));
    memset(nn->layer2, 0, sizeof(nn->layer2));
    memset(nn->output, 0, sizeof(nn->output));
    memcpy(nn->input, input, sizeof(nn->input));

    // We then compute the product
    for (size_t i = 0; i < LAYER1_SIZE; ++i)
    {
        double dot = line_dot(nn->input, nn->layer1_weights[i], INPUT_SIZE);
        nn->layer1[i] = dot;
    }
    line_subi(nn->layer1, nn->layer1_biases, LAYER1_SIZE);
    line_map(nn->layer1, LAYER1_SIZE, sigmoid);

    for (size_t i = 0; i < LAYER2_SIZE; ++i)
    {
        double dot = line_dot(nn->layer1, nn->layer2_weights[i], LAYER1_SIZE);
        nn->layer2[i] = dot;
    }
    line_subi(nn->layer2, nn->layer2_biases, LAYER2_SIZE);
    line_map(nn->layer2, LAYER2_SIZE, sigmoid);

    for (size_t i = 0; i < OUTPUT_SIZE; ++i)
    {
        double dot = line_dot(nn->layer2, nn->output_weights[i], LAYER2_SIZE);
        nn->output[i] = dot;
    }
    line_subi(nn->output, nn->output_biases, OUTPUT_SIZE);
    line_map(nn->output, OUTPUT_SIZE, sigmoid);
}

bool neural_find_logic(struct neural_network *nn, bool a, bool b)
{
    double input[] = {a, b};
    forward_pass(nn, input);
    // Processing the input already adds the bias into the output array directly,
    // so no need to add it again
    return nn->output[0] > 0.5;
}

/* Same thing here, we cannot replace this with a static inline. I'm sorry. */
#define PROPAGATE_LAYER(lay, lay_weight, lay_bias, layprev, delta, lr, expected, otp_d, otp_w)     \
    do                                                                                             \
    {                                                                                              \
        for (size_t j = 0; j < countof(lay); ++j)                                                  \
        {                                                                                          \
            double otp = (lay)[j];                                                                 \
            double sum = 0;                                                                        \
            for (size_t k = 0; k < OUTPUT_SIZE; ++k)                                               \
            {                                                                                      \
                sum += (otp_d)[k] * (otp_w)[k][j];                                                 \
            }                                                                                      \
            (delta)[j] = otp * (1 - otp) * sum;                                                    \
            for (size_t i = 0; i < countof(layprev); ++i)                                          \
            {                                                                                      \
                (lay_weight)[j][i] += (lr) * (delta)[j] * (layprev)[i];                            \
            }                                                                                      \
            (lay_bias)[j] -= (lr) * (delta)[j];                                                    \
        }                                                                                          \
    } while (0)

static void back_propagate(struct neural_network *nn, double expected[static OUTPUT_SIZE],
                           double learning_rate)
{
    double otp_delta[OUTPUT_SIZE] = {0};
    double layer2_delta[LAYER2_SIZE] = {0};
    double layer1_delta[LAYER1_SIZE] = {0};

    for (size_t j = 0; j < OUTPUT_SIZE; ++j)
    {
        double otp = nn->output[j];
        otp_delta[j] = otp * (1 - otp) * (expected[j] - otp);
        for (size_t i = 0; i < LAYER2_SIZE; ++i)
        {
            nn->output_weights[j][i] += learning_rate * otp_delta[j] * nn->layer2[i];
        }
        nn->output_biases[j] -= learning_rate * otp_delta[j];
    }
    PROPAGATE_LAYER(nn->layer2, nn->layer2_weights, nn->layer2_biases, nn->layer1, layer2_delta,
                    learning_rate, expected, otp_delta, nn->output_weights);
    PROPAGATE_LAYER(nn->layer1, nn->layer1_weights, nn->layer1_biases, nn->input, layer1_delta,
                    learning_rate, expected, otp_delta, nn->output_weights);
}

void train(struct neural_network *nn)
{
    srandom((unsigned int)time(NULL));
    randomize_layers(nn);
    for (size_t i = 0; i < TRAIN_ITERATIONS; ++i)
    {
#ifdef DEBUG
        double control_tt[] = {1, 1};
        double control_tf[] = {1, 0};

        if ((i % 4999) == 0)
        {
            double percent_progress = (double)i * (100.0 / (double)TRAIN_ITERATIONS);
            printf("%zu/%d (%f %%)\n", i, TRAIN_ITERATIONS, percent_progress);

            forward_pass(nn, control_tt);
            printf("\t True/True gives: %f\n", nn->output[0]);
            forward_pass(nn, control_tf);
            printf("\t True/False gives: %f\n", nn->output[0]);
        }
#endif

        bool a = rndb();
        bool b = rndb();

        double input[] = {a ? 1.0 : 0.0, b ? 1.0 : 0.0};
        double expected[] = {(a == b) ? 1.0 : 0.0};

        forward_pass(nn, input);
        back_propagate(nn, expected, 0.5);
    }
}
