#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct BFSResult {
    vector<int> dist;
    vector<int> parent;
};

class Graph {
public:
    explicit Graph(int n = 0) : adj(n) {}

    void resize(int n) { adj.assign(n, {}); }

    void addEdge(int u, int v) {
        if (u < 0 || v < 0 || u >= (int)adj.size() || v >= (int)adj.size()) return;
        adj[u].push_back(v);
    }

    int size() const { return (int)adj.size(); }

    const vector<int>& neighbors(int u) const { return adj[u]; }

private:
    vector<vector<int>> adj;
};

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

class RouteSearch {
public:
    bool loadData(const string& cityFileName, const string& flightFileName) {
        ifstream cityFile(cityFileName);
        if (!cityFile) {
            cerr << "Error: cannot open " << cityFileName << endl;
            return false;
        }

        string line;
        cityNames.clear();
        cityToId.clear();

        while (getline(cityFile, line)) {
            line = trim(line);
            if (line.empty()) continue;
            cityToId[line] = (int)cityNames.size();
            cityNames.push_back(line);
        }
        cityFile.close();

        graph.resize((int)cityNames.size());

        ifstream flightFile(flightFileName);
        if (!flightFile) {
            cerr << "Error: cannot open " << flightFileName << endl;
            return false;
        }

        int from = -1;
        while (getline(flightFile, line)) {
            if (trim(line).empty()) continue;

            if (line.rfind("From:", 0) == 0) {
                string city = trim(line.substr(5));
                if (!hasCity(city)) {
                    cerr << "Warning: unknown source city in flight file: " << city << endl;
                    from = -1;
                    continue;
                }
                from = getCityId(city);
            } else if (line.rfind("To", 0) == 0) {
                string city = trim(line.substr(5));
                if (from != -1 && hasCity(city)) graph.addEdge(from, getCityId(city));
            } else {
                string city = trim(line);
                if (from != -1 && hasCity(city)) graph.addEdge(from, getCityId(city));
            }
        }
        flightFile.close();
        return true;
    }

    bool hasCity(const string& name) const {
        return cityToId.find(name) != cityToId.end();
    }

    int getCityId(const string& name) const {
        auto it = cityToId.find(name);
        return it == cityToId.end() ? -1 : it->second;
    }

    string getCityName(int id) const {
        if (id < 0 || id >= (int)cityNames.size()) return "";
        return cityNames[id];
    }

    BFSResult bfsAll(int src) const {
        BFSResult result;
        int n = graph.size();
        result.dist.assign(n, -1);
        result.parent.assign(n, -1);

        queue<int> q;
        q.push(src);
        result.dist[src] = 0;

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : graph.neighbors(u)) {
                if (result.dist[v] == -1) {
                    result.dist[v] = result.dist[u] + 1;
                    result.parent[v] = u;
                    q.push(v);
                }
            }
        }
        return result;
    }

    vector<int> shortestPath(int src, int dst) const {
        BFSResult bfs = bfsAll(src);
        if (bfs.dist[dst] == -1) return {};
        vector<int> path;
        for (int cur = dst; cur != -1; cur = bfs.parent[cur]) path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }

    static vector<int> appendPath(const vector<int>& a, const vector<int>& b) {
        if (a.empty()) return b;
        if (b.empty()) return {};
        vector<int> out = a;
        for (size_t i = 1; i < b.size(); ++i) out.push_back(b[i]);
        return out;
    }

    static int connections(const vector<int>& path) {
        return path.empty() ? -1 : (int)path.size() - 1;
    }

    string pathToString(const vector<int>& path) const {
        if (path.empty()) return "";
        string out;
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) out += " to ";
            out += getCityName(path[i]);
        }
        return out;
    }

    void solveQ1(const string& A, const string& B, int maxConnections) const {
        if (!validateCities({A, B})) return;
        int a = getCityId(A), b = getCityId(B);
        vector<int> path = shortestPath(a, b);

        if (path.empty() || connections(path) > maxConnections) {
            cout << "There is no route from " << A << " to " << B
                 << " with less than or equal to " << maxConnections << " connections." << endl;
            return;
        }

        cout << pathToString(path) << endl;
        cout << "total connection: " << connections(path) << endl;
    }

    void solveQ2(const string& A, const string& B, const string& C, const string& D) const {
        if (!validateCities({A, B, C, D})) return;
        int a = getCityId(A), b = getCityId(B), c = getCityId(C), d = getCityId(D);

        vector<int> pAB = shortestPath(a, b);
        vector<int> pBC = shortestPath(b, c);
        vector<int> pCD = shortestPath(c, d);

        vector<int> route1;
        if (!pAB.empty() && !pBC.empty() && !pCD.empty()) {
            route1 = appendPath(appendPath(pAB, pBC), pCD);
        }

        vector<int> pAC = shortestPath(a, c);
        vector<int> pCB = shortestPath(c, b);
        vector<int> pBD = shortestPath(b, d);

        vector<int> route2;
        if (!pAC.empty() && !pCB.empty() && !pBD.empty()) {
            route2 = appendPath(appendPath(pAC, pCB), pBD);
        }

        if (route1.empty() && route2.empty()) {
            cout << "There is no route from " << A << " to " << D
                 << " that goes through " << B << " and " << C << "." << endl;
            return;
        }

        vector<int> best = route1.empty() ? route2 : route2.empty() ? route1
                         : connections(route1) <= connections(route2) ? route1 : route2;

        cout << pathToString(best) << endl;
        cout << "smallest number of connection: " << connections(best) << endl;
    }

    void solveQ3(const string& A) const {
        if (!validateCities({A})) return;
        int start = getCityId(A);
        BFSResult startBfs = bfsAll(start);

        vector<int> reachable;
        for (int i = 0; i < graph.size(); ++i) {
            if (startBfs.dist[i] != -1) reachable.push_back(i);
        }

        if (reachable.size() == 1) {
            cout << A << " to " << A << endl;
            cout << "smallest number of connection: 0" << endl;
            return;
        }

        int m = (int)reachable.size();
        int startPos = 0;
        for (int i = 0; i < m; ++i) {
            if (reachable[i] == start) {
                swap(reachable[0], reachable[i]);
                break;
            }
        }
        startPos = 0;

        vector<vector<int>> dist(m, vector<int>(m, -1));
        vector<vector<vector<int>>> pathMat(m, vector<vector<int>>(m));

        for (int i = 0; i < m; ++i) {
            BFSResult bfs = bfsAll(reachable[i]);
            for (int j = 0; j < m; ++j) {
                dist[i][j] = bfs.dist[reachable[j]];
                if (dist[i][j] != -1) {
                    vector<int> p;
                    for (int cur = reachable[j]; cur != -1; cur = bfs.parent[cur]) p.push_back(cur);
                    reverse(p.begin(), p.end());
                    pathMat[i][j] = p;
                }
            }
        }

        for (int i = 0; i < m; ++i) {
            if (dist[i][startPos] == -1) {
                cout << "There is no route that starts at " << A
                     << ", visits every reachable city, and returns to " << A << "." << endl;
                return;
            }
        }

        vector<int> routeOrder;
        if (m <= 18) {
            routeOrder = exactVisitAllOrder(dist, startPos);
        } else {
            routeOrder = greedyVisitAllOrder(dist, startPos);
        }

        if (routeOrder.empty()) {
            cout << "There is no route that starts at " << A
                 << ", visits every reachable city, and returns to " << A << "." << endl;
            return;
        }

        vector<int> expanded = {reachable[routeOrder[0]]};
        for (size_t i = 1; i < routeOrder.size(); ++i) {
            vector<int> segment = pathMat[routeOrder[i - 1]][routeOrder[i]];
            expanded = appendPath(expanded, segment);
        }

        cout << pathToString(expanded) << endl;
        cout << "smallest number of connection: " << connections(expanded);
        if (m > 18) cout << " (greedy approximation for large reachable set)";
        cout << endl;
    }

    void solveQ4(const string& A, const string& B, const string& C) const {
        if (!validateCities({A, B, C})) return;
        int a = getCityId(A), b = getCityId(B), c = getCityId(C);

        BFSResult bfsA = bfsAll(a);
        BFSResult bfsB = bfsAll(b);
        BFSResult bfsC = bfsAll(c);

        int bestCity = -1;
        int bestTotal = numeric_limits<int>::max();

        for (int v = 0; v < graph.size(); ++v) {
            if (v == a || v == b || v == c) continue;
            if (bfsA.dist[v] == -1 || bfsB.dist[v] == -1 || bfsC.dist[v] == -1) continue;
            int total = bfsA.dist[v] + bfsB.dist[v] + bfsC.dist[v];
            if (total < bestTotal) {
                bestTotal = total;
                bestCity = v;
            }
        }

        if (bestCity == -1) {
            cout << "There is no city different from " << A << ", " << B << ", and " << C
                 << " that all three of you can reach." << endl;
            return;
        }

        vector<int> pathA = reconstructFromParents(a, bestCity, bfsA.parent, bfsA.dist);
        vector<int> pathB = reconstructFromParents(b, bestCity, bfsB.parent, bfsB.dist);
        vector<int> pathC = reconstructFromParents(c, bestCity, bfsC.parent, bfsC.dist);

        cout << "You three should meet at " << getCityName(bestCity) << endl;
        cout << "Route for first person: " << pathToString(pathA)
             << " (" << connections(pathA) << " connections)" << endl;
        cout << "Route for second person: " << pathToString(pathB)
             << " (" << connections(pathB) << " connections)" << endl;
        cout << "Route for third person: " << pathToString(pathC)
             << " (" << connections(pathC) << " connections)" << endl;
        cout << "Total number of connection: " << bestTotal << endl;
    }

private:
    Graph graph;
    vector<string> cityNames;
    unordered_map<string, int> cityToId;

    bool validateCities(const vector<string>& cities) const {
        for (const string& city : cities) {
            if (!hasCity(city)) {
                cerr << "Error: city not found -> " << city << endl;
                return false;
            }
        }
        return true;
    }

    vector<int> reconstructFromParents(int src, int dst, const vector<int>& parent, const vector<int>& dist) const {
        if (dist[dst] == -1) return {};
        vector<int> path;
        for (int cur = dst; cur != -1; cur = parent[cur]) path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }

    vector<int> exactVisitAllOrder(const vector<vector<int>>& dist, int startPos) const {
        int m = (int)dist.size();
        const int INF = 1e9;
        int FULL = 1 << m;
        vector<vector<int>> dp(FULL, vector<int>(m, INF));
        vector<vector<int>> parent(FULL, vector<int>(m, -1));

        dp[1 << startPos][startPos] = 0;

        for (int mask = 0; mask < FULL; ++mask) {
            for (int u = 0; u < m; ++u) {
                if (dp[mask][u] == INF) continue;
                for (int v = 0; v < m; ++v) {
                    if (mask & (1 << v)) continue;
                    if (dist[u][v] == -1) continue;
                    int nextMask = mask | (1 << v);
                    int cand = dp[mask][u] + dist[u][v];
                    if (cand < dp[nextMask][v]) {
                        dp[nextMask][v] = cand;
                        parent[nextMask][v] = u;
                    }
                }
            }
        }

        int endNode = -1;
        int best = INF;
        int all = FULL - 1;
        for (int u = 0; u < m; ++u) {
            if (dp[all][u] == INF || dist[u][startPos] == -1) continue;
            int cand = dp[all][u] + dist[u][startPos];
            if (cand < best) {
                best = cand;
                endNode = u;
            }
        }
        if (endNode == -1) return {};

        vector<int> revOrder;
        int mask = all;
        int cur = endNode;
        while (cur != -1) {
            revOrder.push_back(cur);
            int p = parent[mask][cur];
            mask ^= (1 << cur);
            cur = p;
        }
        reverse(revOrder.begin(), revOrder.end());
        revOrder.push_back(startPos);
        return revOrder;
    }

    vector<int> greedyVisitAllOrder(const vector<vector<int>>& dist, int startPos) const {
        int m = (int)dist.size();
        vector<bool> visited(m, false);
        vector<int> order = {startPos};
        visited[startPos] = true;
        int cur = startPos;

        for (int step = 1; step < m; ++step) {
            int best = -1;
            int bestDist = numeric_limits<int>::max();
            for (int v = 0; v < m; ++v) {
                if (visited[v] || dist[cur][v] == -1) continue;
                if (dist[cur][v] < bestDist) {
                    bestDist = dist[cur][v];
                    best = v;
                }
            }
            if (best == -1) return {};
            visited[best] = true;
            order.push_back(best);
            cur = best;
        }

        if (dist[cur][startPos] == -1) return {};
        order.push_back(startPos);
        return order;
    }
};

static void printUsage(const string& prog) {
    cerr << "Usage:\n";
    cerr << "  " << prog << " 1 \"city_A\" \"city_B\" <num_connection>\n";
    cerr << "  " << prog << " 2 \"city_A\" through \"city_B\" and \"city_C\" to \"city_D\"\n";
    cerr << "  " << prog << " 3 \"city_A\"\n";
    cerr << "  " << prog << " 4 \"city_A\" \"city_B\" \"city_C\"\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    RouteSearch rs;
    if (!rs.loadData("city.name", "flight.txt")) return 1;

    int question = stoi(argv[1]);

    switch (question) {
        case 1:
            if (argc != 5) {
                printUsage(argv[0]);
                return 1;
            }
            rs.solveQ1(argv[2], argv[3], stoi(argv[4]));
            break;

        case 2:
            if (argc == 9 && string(argv[3]) == "through" && string(argv[5]) == "and" && string(argv[7]) == "to") {
                rs.solveQ2(argv[2], argv[4], argv[6], argv[8]);
            } else if (argc == 6) {
                rs.solveQ2(argv[2], argv[3], argv[4], argv[5]);
            } else {
                printUsage(argv[0]);
                return 1;
            }
            break;

        case 3:
            if (argc != 3) {
                printUsage(argv[0]);
                return 1;
            }
            rs.solveQ3(argv[2]);
            break;

        case 4:
            if (argc != 5) {
                printUsage(argv[0]);
                return 1;
            }
            rs.solveQ4(argv[2], argv[3], argv[4]);
            break;

        default:
            cerr << "Error: question number must be 1, 2, 3, or 4." << endl;
            return 1;
    }

    return 0;
}
