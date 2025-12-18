#include <vector>
#include <unordered_map>
#include <queue>
#include <climits>
#include<iostream>
using namespace std;

int foo(int source, unordered_map<int, vector<pair<int, int>>> graph)
{
    int cost = 0;
    priority_queue<pair<int, int>, std::vector<pair<int, int>>, std::greater<pair<int, int>>> pq;
    vector<bool> visited(graph.size(), false);
    pq.push({0, source});
    while (!pq.empty())
    {
        auto [w, u] = pq.top();
        pq.pop();
        if(visited[u]) continue;
        visited[u] = true;
        cost += w;
        for(auto [v, c] : graph[u]){
            if(!visited[v]){
                pq.push({c, v});
            }
        }
    }
    return cost;
}

int main() {
    std::cout << -10 % 6 << std::endl;
}
