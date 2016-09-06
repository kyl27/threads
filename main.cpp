#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <chrono>
#include <thread>

#include <omp.h>

#include <sys/time.h>


const int NUM_THREADS = 10;

struct Node {
    int val;
    Node *left;
    Node *right;
};

Node *generateTree(int depth, int val) {
    Node *node = new Node();
    node->val = val;

    if (depth > 0) {
        node->left = generateTree(depth - 1, val);
        node->right = generateTree(depth - 1, val);
    }
    return node;
}

void destroyTree(Node *node) {
    if (node != NULL) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

void openMpSum(Node *node, int *sum) {
    if (node != NULL) {
        int left_sum = 0, right_sum = 0;
        #pragma omp task shared(left_sum)
        openMpSum(node->left, &left_sum);
        #pragma omp task shared(right_sum)
        openMpSum(node->right, &right_sum);
        #pragma omp taskwait

        std::this_thread::sleep_for(std::chrono::milliseconds(node->val));
        *sum = node->val + left_sum + right_sum;
        return;
    }
    *sum = 0;
}

void parallelSum(Node *node, int depth, int *sum) {
    if (node != NULL) {
        int left_sum = 0, right_sum = 0;
        if (std::pow(2, depth) < NUM_THREADS) {
            std::thread left (parallelSum, node->left, depth + 1, &left_sum);
            parallelSum(node->right, depth + 1, &right_sum);
            left.join();
        } else {
            parallelSum(node->left, depth + 1, &left_sum);
            parallelSum(node->right, depth + 1, &right_sum);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(node->val));
        *sum = node->val + left_sum + right_sum;
        return;
    }
    *sum = 0;;
}

double wall_time_ms() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return 1000.0 * time.tv_sec + 0.001 * time.tv_usec;
}

int main() {

    omp_set_num_threads(NUM_THREADS);

    int depth, val;

    std::cout << "Tree depth: " << std::endl;
    std::cin >> depth;
    std::cout << "Node value: " << std::endl;
    std::cin >> val;

    double start = wall_time_ms();
    Node *root = generateTree(depth, val);
    double elapsed = wall_time_ms() - start;

    std::cout << "Tree generated with depth " << depth
        << " and node value " << val
        << " in " << elapsed << " ms" << std::endl;

    start = wall_time_ms();
    int sum;
    #pragma omp parallel
    {
        #pragma omp single
        openMpSum(root, &sum);
    }
    elapsed = wall_time_ms() - start;

    std::cout << "openMpSum completed in " << elapsed << "ms" << std::endl;
    std::cout << "Sum: " << sum << std::endl;

    start = wall_time_ms();
    parallelSum(root, 0, &sum);
    elapsed = wall_time_ms() - start;

    std::cout << "parallelSum completed in " << elapsed << "ms" << std::endl;
    std::cout << "Sum: " << sum << std::endl;

    destroyTree(root);

    std::cout << "Tree destroyed." << std::endl;
}
