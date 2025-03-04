Copyright @lucabotez

# Dimensional kNN

## Overview
**Dimensional kNN** implements a **k-d tree** for efficiently performing **nearest neighbor (NN) and range search (RS) queries** in multi-dimensional space. It supports **fast insertion, searching, and sorting** of points using dynamic memory allocation.

## Features
- **Load data** into a k-d tree from a file.
- **Find the nearest neighbor (NN)** to a given point.
- **Perform range search (RS)** to find points within a given boundary.
- **Efficient memory management**, with dynamic allocation and deallocation.

## Commands

### 1) LOAD
- **Usage:** `LOAD <filename>`
- **Description:** Loads a set of points from a file and inserts them into a k-d tree.

### 2) NN (Nearest Neighbor)
- **Usage:** `NN <point_coordinates>`
- **Description:** Finds the closest point(s) to the given coordinates using **Euclidean distance**.

### 3) RS (Range Search)
- **Usage:** `RS <lower_bounds> <upper_bounds>`
- **Description:** Finds all points within a specified range defined by the lower and upper bounds for each dimension.

### 4) EXIT
- **Usage:** `EXIT`
- **Description:** Frees all dynamically allocated memory and terminates the program.

## Implementation Details
- **k-d Tree Structure:**
  - Each node stores multi-dimensional point data.
  - Splitting occurs along different dimensions at each level.
- **Nearest Neighbor Search:**
  - Uses **recursive traversal** to find the closest points.
  - Optimized by pruning unnecessary branches.
- **Range Search:**
  - Searches for points within a user-defined boundary.
  - Results are **sorted lexicographically** for consistent output.
- **Performance Considerations:**
  - **Insertion & Search Complexity:** **O(log N)** for balanced trees.
  - **Memory Efficiency:** Dynamic allocation minimizes waste.
