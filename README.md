# TrafficSignaling
Hash Code 2021. Given digraph of streets and fixed-pathed vehicles, optimize traffic lights cycling strategy.

Compile with `g++ -std=c++17`

A baseline by naively cycling with even amount of time. (Supprisingly outperforms most randomizations)
![image](https://user-images.githubusercontent.com/46303157/109381006-523f2280-789d-11eb-92a3-03c7636ff3c0.png)


### Version 2
Rewrote the whole simulation, much faster than the first draft, no way I could finish this in 4 hours. 

For optimization, I tried to start with some heuristics from static map data, such as giving more green light time to busy streets, and start the cycles from the most crowded. Only worked on dataset A, E, and F.

![image](https://raw.githubusercontent.com/suzx917/TrafficSignaling/main/screenshots/Screenshot_20210227_014046.png)

Working on more dynamic improvements.
