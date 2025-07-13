#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Node types for the expression tree
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_SIN,
    NODE_COS
} NodeType;

// Expression tree node
typedef struct Node {
    NodeType type;
    double value;           // For NUMBER nodes
    struct Node *left;      // Left child
    struct Node *right;     // Right child
} Node;

// Individual in the population
typedef struct {
    Node *tree;
    double fitness;
} Individual;

// Global allocator functions
static void *(*g_allocator)(size_t) = NULL;
static void (*g_deallocator)(void *) = NULL;

// Target function: f(x) = x^2 + 2*x + 1
static double target_function(double x) {
    return x * x + 2 * x + 1;
}

// Create a new node
static Node *create_node(NodeType type) {
    Node *node = (Node *)g_allocator(sizeof(Node));
    if (!node) return NULL;
    
    node->type = type;
    node->value = 0.0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Free a tree recursively
static void free_tree(Node *node) {
    if (!node) return;
    
    free_tree(node->left);
    free_tree(node->right);
    g_deallocator(node);
}

// Generate a random tree with given depth
static Node *generate_random_tree(int max_depth, int current_depth) {
    if (current_depth >= max_depth || (current_depth > 0 && rand() % 3 == 0)) {
        // Create terminal node
        Node *node;
        if (rand() % 2 == 0) {
            // Number node
            node = create_node(NODE_NUMBER);
            if (node) node->value = (double)(rand() % 21 - 10); // -10 to 10
        } else {
            // Variable node (x)
            node = create_node(NODE_VARIABLE);
        }
        return node;
    }

    // Create function node
    NodeType types[] = {NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV};
    NodeType type = types[rand() % 4];
    
    Node *node = create_node(type);
    if (!node) return NULL;
    
    // Binary operators
    node->left = generate_random_tree(max_depth, current_depth + 1);
    node->right = generate_random_tree(max_depth,current_depth + 1);

    // Check if children were created successfully
    if (!node->left || !node->right) {
        free_tree(node);
        return NULL;
    }
    
    return node;
}

// Evaluate an expression tree
static double evaluate_tree(Node *node, double x) {
    if (!node) return 0.0;
    
    switch (node->type) {
        case NODE_NUMBER:
            return node->value;
        case NODE_VARIABLE:
            return x;
        case NODE_ADD:
            return evaluate_tree(node->left, x) + evaluate_tree(node->right, x);
        case NODE_SUB:
            return evaluate_tree(node->left, x) - evaluate_tree(node->right, x);
        case NODE_MUL:
            return evaluate_tree(node->left, x) * evaluate_tree(node->right, x);
        case NODE_DIV: {
            double right_val = evaluate_tree(node->right, x);
            if (fabs(right_val) < 1e-10) return 1.0; // Avoid division by zero
            return evaluate_tree(node->left, x) / right_val;
        }
        default:
            return 0.0;
    }
}

// Copy a tree (deep copy)
static Node *copy_tree(Node *node) {
    if (!node) return NULL;
    
    Node *copy = create_node(node->type);
    if (!copy) return NULL;
    
    copy->value = node->value;
    copy->left = copy_tree(node->left);
    copy->right = copy_tree(node->right);
    
    return copy;
}

// Calculate fitness (lower is better)
static double calculate_fitness(Node *tree) {
    double error = 0.0;
    int num_points = 20;
    
    for (int i = 0; i < num_points; i++) {
        double x = (double)i / (num_points - 1) * 4.0 - 2.0; // x from -2 to 2
        double predicted = evaluate_tree(tree, x);
        double actual = target_function(x);
        double diff = predicted - actual;
        error += diff * diff;
    }
    
    return error / num_points; // Mean squared error
}

// Tournament selection
static int tournament_selection(Individual *population, int pop_size, int tournament_size) {
    int best = rand() % pop_size;
    
    for (int i = 1; i < tournament_size; i++) {
        int candidate = rand() % pop_size;
        if (population[candidate].fitness < population[best].fitness) {
            best = candidate;
        }
    }
    
    return best;
}

// Crossover two trees
static Node *crossover(Node *parent1, Node *parent2) {
    if (!parent1 || !parent2) return NULL;
    
    // Simple crossover: randomly choose subtrees
    if (rand() % 2 == 0) {
        Node *child = copy_tree(parent1);
        // Replace a random subtree with one from parent2
        if (child && child->left && rand() % 2 == 0) {
            free_tree(child->left);
            child->left = copy_tree(parent2->right ? parent2->right : parent2);
        } else if (child && child->right) {
            free_tree(child->right);
            child->right = copy_tree(parent2->left ? parent2->left : parent2);
        }
        return child;
    } else {
        return copy_tree(parent2);
    }
}

// Mutate a tree
static void mutate(Node *node, double mutation_rate) {
    if (!node) return;
    
    if ((double)rand() / RAND_MAX < mutation_rate) {
        if (node->type == NODE_NUMBER) {
            node->value = (double)(rand() % 21 - 10);
        } else if (node->type >= NODE_ADD && node->type <= NODE_DIV) {
            // Change operator
            NodeType types[] = {NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV};
            node->type = types[rand() % 4];
        }
    }
    
    mutate(node->left, mutation_rate);
    mutate(node->right, mutation_rate);
}

// Main genetic programming function
void genetic_program(void *(*allocator)(size_t), void (*deallocator)(void *),
                     size_t generations, size_t pop_size, unsigned int seed) {
    g_allocator = allocator;
    g_deallocator = deallocator;
    
    srand(seed);
    
    // Allocate population
    Individual *population = (Individual *)allocator(sizeof(Individual) * pop_size);
    Individual *new_population = (Individual *)allocator(sizeof(Individual) * pop_size);
    
    if (!population || !new_population) {
        printf("Failed to allocate population\n");
        return;
    }

    
    // Initialize population
    for (size_t i = 0; i < pop_size; i++) {
        Node * tree = generate_random_tree(3, 0); // Reduced depth
        population[i].tree = tree;
        if (!population[i].tree) {
            // Create a simple fallback tree
            population[i].tree = create_node(NODE_NUMBER);
            if (population[i].tree) {
                population[i].tree->value = 1.0;
            }
        }
        if (population[i].tree) {
            population[i].fitness = calculate_fitness(population[i].tree);
        } else {
            population[i].fitness = 1e6; // Very bad fitness
        }
    }
    
    printf("Starting GP with %zu generations, population size %zu\n", generations, pop_size);
    
    // Evolution loop
    for (size_t gen = 0; gen < generations; gen++) {
        // Find best individual
        double best_fitness = population[0].fitness;
        int best_idx = 0;
        for (size_t i = 1; i < pop_size; i++) {
            if (population[i].fitness < best_fitness) {
                best_fitness = population[i].fitness;
                best_idx = i;
            }
        }
        
        if (gen % 10 == 0) {
            printf("Generation %zu: Best fitness = %.6f\n", gen, best_fitness);
        }
        
        // Create new population
        for (size_t i = 0; i < pop_size; i++) {
            if (i == 0) {
                // Elitism: keep best individual
                new_population[i].tree = copy_tree(population[best_idx].tree);
            } else {
                // Selection and crossover
                int parent1_idx = tournament_selection(population, pop_size, 3);
                int parent2_idx = tournament_selection(population, pop_size, 3);
                
                new_population[i].tree = crossover(population[parent1_idx].tree, 
                                                 population[parent2_idx].tree);
                
                // Mutation
                mutate(new_population[i].tree, 0.1);
            }
            
            new_population[i].fitness = calculate_fitness(new_population[i].tree);
        }
        
        // Free old population trees
        for (size_t i = 0; i < pop_size; i++) {
            free_tree(population[i].tree);
        }
        
        // Swap populations
        Individual *temp = population;
        population = new_population;
        new_population = temp;
    }
    
    // Final results
    double best_fitness = population[0].fitness;
    for (size_t i = 1; i < pop_size; i++) {
        if (population[i].fitness < best_fitness) {
            best_fitness = population[i].fitness;
        }
    }
    
    printf("Final best fitness: %.6f\n", best_fitness);
    
    // Cleanup
    for (size_t i = 0; i < pop_size; i++) {
        free_tree(population[i].tree);
    }
    
    deallocator(population);
    deallocator(new_population);
}
