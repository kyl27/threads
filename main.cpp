#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>

#include <sys/time.h>


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

int sequentialSum(Node *node) {
    if (node != NULL) {
        int left_sum = 0, right_sum = 0;
        #pragma omp task shared(left_sum)
        left_sum = sequentialSum(node->left);
        #pragma omp task shared(right_sum)
        right_sum = sequentialSum(node->right);
        #pragma omp taskwait

        return node->val + left_sum + right_sum;
        std::this_thread::sleep_for(std::chrono::milliseconds(node->val));
    }
    return 0;
}

void parallelSum(Node *node, int& sum) {
    if (node != NULL) {
        int left_sum = 0, right_sum = 0;
        std::thread left (parallelSum, node->left, std::ref(left_sum));
        parallelSum(node->right, right_sum);
        left.join();
        sum = node->val + left_sum + right_sum;
        return;
    }
    sum = 0;;
}

double wall_time_ms() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return 1000.0 * time.tv_sec + 0.001 * time.tv_usec;
}

int main() {
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
    int sum = sequentialSum(root);
    elapsed = wall_time_ms() - start;

    std::cout << "Sequential DFS completed in " << elapsed << "ms" << std::endl;
    std::cout << "Sum: " << sum << std::endl;

    start = wall_time_ms();
    parallelSum(root, sum);
    elapsed = wall_time_ms() - start;

    std::cout << "Parallel DFS completed in " << elapsed << "ms" << std::endl;
    std::cout << "Sum: " << sum << std::endl;

    destroyTree(root);

    std::cout << "Tree destroyed." << std::endl;
}
