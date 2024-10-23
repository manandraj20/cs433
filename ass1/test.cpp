#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <bits/stdc++.h>
using namespace std;

#define ll long long
// Direction vectors for top, bottom, left, right movements
int dRow[] = {-1, 1, 0, 0};
int dCol[] = {0, 0, -1, 1};

// BFS function to find the shortest distance to the nearest '0' cell
int bfs(const vector<vector<ll int>> &grid, int m, int n, int startRow, int startCol) {
    // If the start cell is '0', return 0 immediately
    if (grid[startRow][startCol] == 0) return 0;
    if (grid[startRow][startCol] == -1) return -1;
    
    // Visited grid to track visited cells
    vector<vector<bool>> visited(m, vector<bool>(n, false));
    
    // BFS queue storing {row, col, distance}
    queue<pair<pair<int, int>, int>> q;
    
    // Start from the given (i, j) cell
    q.push({{startRow, startCol}, 0});
    visited[startRow][startCol] = true;
    
    // Perform BFS
    while (!q.empty()) {
        auto [cell, dist] = q.front();
        int row = cell.first;
        int col = cell.second;
        q.pop();
        
        // Check all 4 directions
        for (int i = 0; i < 4; i++) {
            int newRow = row + dRow[i];
            int newCol = col + dCol[i];
            
            // If the new position is within bounds
            if (newRow >= 0 && newRow < m && newCol >= 0 && newCol < n && !visited[newRow][newCol]) {
                // If the cell contains '0', return the distance
                if (grid[newRow][newCol] == 0) {
                    return dist + 1;
                }
                // If the cell is not blocked (-1), continue BFS
                if (grid[newRow][newCol] != -1) {
                    visited[newRow][newCol] = true;
                    q.push({{newRow, newCol}, dist + 1});
                }
            }
        }
    }
    
    // If no '0' is found, return -1
    return -1;
}

int main() {
    int m, n;
    // Input grid dimensions
    cin >> m >> n;
    
    vector<vector<ll int>> grid(m, vector<ll int>(n));
    
    // Input the grid values
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> grid[i][j];
        }
    }

    // Iterate over every cell and call BFS
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            ll int distance = bfs(grid, m, n, i, j);
            cout << distance << " ";
        }
        cout << endl;
    }
    
    return 0;
}
