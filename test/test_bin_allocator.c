#include "../src/bin.h"
#include "../src/allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// Test patterns to write to allocated memory
#define PATTERN_A 0xAA
#define PATTERN_B 0xBB
#define PATTERN_C 0xCC
#define PATTERN_D 0xDD

// Structure to track allocations
typedef struct {
    void *ptr;
    size_t size;
    uint8_t pattern;
} Allocation;

// Maximum number of allocations to track
#define MAX_ALLOCATIONS 1000

static Allocation allocations[MAX_ALLOCATIONS];
static size_t num_allocations = 0;

// Fill memory with a pattern
static void fill_memory(void *ptr, size_t size, uint8_t pattern) {
    memset(ptr, pattern, size);
}

// Verify memory contains the expected pattern
static bool verify_memory(void *ptr, size_t size, uint8_t pattern) {
    uint8_t *bytes = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        if (bytes[i] != pattern) {
            printf("Memory corruption detected at offset %zu: expected 0x%02X, got 0x%02X\n", 
                   i, pattern, bytes[i]);
            return false;
        }
    }
    return true;
}

// Add an allocation to our tracking array
static void track_allocation(void *ptr, size_t size, uint8_t pattern) {
    if (num_allocations >= MAX_ALLOCATIONS) {
        printf("ERROR: Too many allocations to track\n");
        return;
    }
    
    allocations[num_allocations].ptr = ptr;
    allocations[num_allocations].size = size;
    allocations[num_allocations].pattern = pattern;
    num_allocations++;
}

// Verify all tracked allocations still have their correct patterns
static bool verify_all_allocations() {
    bool all_valid = true;
    
    for (size_t i = 0; i < num_allocations; i++) {
        if (allocations[i].ptr != NULL) {
            if (!verify_memory(allocations[i].ptr, allocations[i].size, allocations[i].pattern)) {
                printf("Allocation %zu (ptr=%p, size=%zu, pattern=0x%02X) is corrupted!\n",
                       i, allocations[i].ptr, allocations[i].size, allocations[i].pattern);
                all_valid = false;
            }
        }
    }
    
    return all_valid;
}

// Test basic allocation and deallocation
static bool test_basic_allocation() {
    printf("Testing basic allocation...\n");
    
    // Test different sizes that should go into different bins
    size_t test_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128};
    size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    void *ptrs[8];
    
    // Allocate memory of different sizes
    for (size_t i = 0; i < num_sizes; i++) {
        ptrs[i] = bin_alloc(test_sizes[i]);
        if (!ptrs[i]) {
            printf("FAIL: bin_alloc(%zu) returned NULL\n", test_sizes[i]);
            return false;
        }
        
        // Fill with pattern and track
        fill_memory(ptrs[i], test_sizes[i], PATTERN_A + (i % 4));
        track_allocation(ptrs[i], test_sizes[i], PATTERN_A + (i % 4));
        
        printf("Allocated %zu bytes at %p\n", test_sizes[i], ptrs[i]);
    }
    
    // Verify all allocations are still intact
    if (!verify_all_allocations()) {
        printf("FAIL: Memory corruption detected after basic allocations\n");
        return false;
    }
    
    // Free all allocations
    for (size_t i = 0; i < num_sizes; i++) {
        struct Bin *bin = allocated_by_bin(ptrs[i]);
        if (!bin) {
            printf("FAIL: Could not find bin for allocation %p\n", ptrs[i]);
            return false;
        }
        bin_free(ptrs[i], bin);
        
        // Mark as freed in our tracking
        for (size_t j = 0; j < num_allocations; j++) {
            if (allocations[j].ptr == ptrs[i]) {
                allocations[j].ptr = NULL;
                break;
            }
        }
    }
    
    printf("PASS: Basic allocation test\n");
    return true;
}

// Test multiple allocations of the same size
static bool test_same_size_allocations() {
    printf("Testing multiple allocations of same size...\n");
    
    const size_t alloc_size = 16;
    const size_t num_allocs = 50;
    void *ptrs[50];
    
    // Allocate many blocks of the same size
    for (size_t i = 0; i < num_allocs; i++) {
        ptrs[i] = bin_alloc(alloc_size);
        if (!ptrs[i]) {
            printf("FAIL: bin_alloc(%zu) returned NULL on allocation %zu\n", alloc_size, i);
            return false;
        }
        
        // Fill with unique pattern based on index
        uint8_t pattern = (uint8_t)(i % 256);
        fill_memory(ptrs[i], alloc_size, pattern);
        track_allocation(ptrs[i], alloc_size, pattern);
        
        // Verify all previous allocations are still intact
        if (!verify_all_allocations()) {
            printf("FAIL: Memory corruption detected after allocation %zu\n", i);
            return false;
        }
    }
    
    printf("Successfully allocated %zu blocks of size %zu\n", num_allocs, alloc_size);
    
    // Free every other allocation
    for (size_t i = 0; i < num_allocs; i += 2) {
        struct Bin *bin = allocated_by_bin(ptrs[i]);
        if (!bin) {
            printf("FAIL: Could not find bin for allocation %p\n", ptrs[i]);
            return false;
        }
        bin_free(ptrs[i], bin);
        
        // Mark as freed
        for (size_t j = 0; j < num_allocations; j++) {
            if (allocations[j].ptr == ptrs[i]) {
                allocations[j].ptr = NULL;
                break;
            }
        }
    }
    
    // Verify remaining allocations are still intact
    if (!verify_all_allocations()) {
        printf("FAIL: Memory corruption detected after partial free\n");
        return false;
    }
    
    // Allocate new blocks (should reuse freed space)
    for (size_t i = 0; i < num_allocs; i += 2) {
        ptrs[i] = bin_alloc(alloc_size);
        if (!ptrs[i]) {
            printf("FAIL: bin_alloc(%zu) returned NULL on reallocation %zu\n", alloc_size, i);
            return false;
        }
        
        // Fill with different pattern
        uint8_t pattern = (uint8_t)((i + 128) % 256);
        fill_memory(ptrs[i], alloc_size, pattern);
        track_allocation(ptrs[i], alloc_size, pattern);
    }
    
    // Final verification
    if (!verify_all_allocations()) {
        printf("FAIL: Memory corruption detected after reallocation\n");
        return false;
    }
    
    // Clean up remaining allocations
    for (size_t i = 0; i < num_allocs; i++) {
        if (ptrs[i]) {
            struct Bin *bin = allocated_by_bin(ptrs[i]);
            if (bin) {
                bin_free(ptrs[i], bin);
            }
        }
    }
    
    // Clear tracking array
    num_allocations = 0;
    
    printf("PASS: Same size allocations test\n");
    return true;
}

// Test allocation boundaries and alignment
static bool test_allocation_boundaries() {
    printf("Testing allocation boundaries and alignment...\n");
    
    const size_t alloc_size = 32;
    const size_t num_allocs = 10;
    void *ptrs[10];
    
    // Allocate blocks and check they don't overlap
    for (size_t i = 0; i < num_allocs; i++) {
        ptrs[i] = bin_alloc(alloc_size);
        if (!ptrs[i]) {
            printf("FAIL: bin_alloc(%zu) returned NULL\n", alloc_size);
            return false;
        }
        
        // Check alignment (should be at least pointer-aligned)
        if ((uintptr_t)ptrs[i] % sizeof(void*) != 0) {
            printf("FAIL: Allocation %p is not properly aligned\n", ptrs[i]);
            return false;
        }
        
        // Fill entire allocation with pattern
        fill_memory(ptrs[i], alloc_size, PATTERN_A + (i % 4));
        
        // Check for overlaps with previous allocations
        for (size_t j = 0; j < i; j++) {
            uintptr_t start1 = (uintptr_t)ptrs[i];
            uintptr_t end1 = start1 + alloc_size;
            uintptr_t start2 = (uintptr_t)ptrs[j];
            uintptr_t end2 = start2 + alloc_size;
            
            // Check if ranges overlap
            if (!(end1 <= start2 || end2 <= start1)) {
                printf("FAIL: Allocations overlap! %p-%p and %p-%p\n",
                       (void*)start1, (void*)end1, (void*)start2, (void*)end2);
                return false;
            }
        }
        
        track_allocation(ptrs[i], alloc_size, PATTERN_A + (i % 4));
    }
    
    // Verify all allocations
    if (!verify_all_allocations()) {
        printf("FAIL: Memory corruption in boundary test\n");
        return false;
    }
    
    // Clean up
    for (size_t i = 0; i < num_allocs; i++) {
        struct Bin *bin = allocated_by_bin(ptrs[i]);
        if (bin) {
            bin_free(ptrs[i], bin);
        }
    }
    
    num_allocations = 0;
    
    printf("PASS: Allocation boundaries test\n");
    return true;
}

// Test stress scenario with mixed sizes
static bool test_mixed_size_stress() {
    printf("Testing mixed size stress scenario...\n");
    
    const size_t num_iterations = 100;
    void *active_ptrs[50];
    size_t active_sizes[50];
    uint8_t active_patterns[50];
    size_t num_active = 0;
    
    srand(42); // Fixed seed for reproducible results
    
    for (size_t iter = 0; iter < num_iterations; iter++) {
        if (num_active < 50 && (num_active == 0 || rand() % 2)) {
            // Allocate new block
            size_t size = 1 << (rand() % 7); // 1, 2, 4, 8, 16, 32, 64
            void *ptr = bin_alloc(size);
            
            if (!ptr) {
                printf("FAIL: bin_alloc(%zu) returned NULL at iteration %zu\n", size, iter);
                return false;
            }
            
            uint8_t pattern = (uint8_t)(rand() % 256);
            fill_memory(ptr, size, pattern);
            
            active_ptrs[num_active] = ptr;
            active_sizes[num_active] = size;
            active_patterns[num_active] = pattern;
            num_active++;
            
            track_allocation(ptr, size, pattern);
            
        } else if (num_active > 0) {
            // Free random block
            size_t idx = rand() % num_active;
            
            struct Bin *bin = allocated_by_bin(active_ptrs[idx]);
            if (!bin) {
                printf("FAIL: Could not find bin for allocation %p\n", active_ptrs[idx]);
                return false;
            }
            
            bin_free(active_ptrs[idx], bin);
            
            // Remove from tracking
            for (size_t j = 0; j < num_allocations; j++) {
                if (allocations[j].ptr == active_ptrs[idx]) {
                    allocations[j].ptr = NULL;
                    break;
                }
            }
            
            // Remove from active list
            active_ptrs[idx] = active_ptrs[num_active - 1];
            active_sizes[idx] = active_sizes[num_active - 1];
            active_patterns[idx] = active_patterns[num_active - 1];
            num_active--;
        }
        
        // Verify all active allocations every 10 iterations
        if (iter % 10 == 0) {
            if (!verify_all_allocations()) {
                printf("FAIL: Memory corruption detected at iteration %zu\n", iter);
                return false;
            }
        }
    }
    
    // Final verification
    if (!verify_all_allocations()) {
        printf("FAIL: Memory corruption detected at end of stress test\n");
        return false;
    }
    
    // Clean up remaining allocations
    for (size_t i = 0; i < num_active; i++) {
        struct Bin *bin = allocated_by_bin(active_ptrs[i]);
        if (bin) {
            bin_free(active_ptrs[i], bin);
        }
    }
    
    num_allocations = 0;
    
    printf("PASS: Mixed size stress test\n");
    return true;
}

int main() {
    printf("Starting bin allocator tests...\n\n");
    
    bool all_passed = true;
    
    all_passed &= test_basic_allocation();
    printf("\n");
    
    all_passed &= test_same_size_allocations();
    printf("\n");
    
    all_passed &= test_allocation_boundaries();
    printf("\n");
    
    all_passed &= test_mixed_size_stress();
    printf("\n");
    
    if (all_passed) {
        printf("🎉 ALL TESTS PASSED! Your bin allocator appears to be working correctly.\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED! There are issues with your bin allocator.\n");
        return 1;
    }
}
