CSCE 2110 Project - World Airline Route Search

Files:
- routeSearch.cpp
- WA.cpp
- graphGen.cpp
- city.name
- flight.txt
- makefile

Compile:
  g++ -std=c++17 -O2 routeSearch.cpp -o routeSearch
  g++ -std=c++17 graphGen.cpp -o graphGen

Run:
  Question 1:
    ./routeSearch 1 "Moscow, Russia" "London, United Kingdom" 5

  Question 2:
    ./routeSearch 2 "Moscow, Russia" "Seoul, South Korea" "Beijing, People's Republic of China" "Sydney, Australia"

  Question 3:
    ./routeSearch 3 "Moscow, Russia"

  Question 4:
    ./routeSearch 4 "Moscow, Russia" "Seoul, South Korea" "Tokyo, Japan"

Notes:
- Q1, Q2, and Q4 use BFS shortest paths.
- Q3 computes a minimum visit-all-and-return route exactly for smaller reachable sets and uses a greedy approximation for larger reachable sets.
- Input validation is included for missing city names.
