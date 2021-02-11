- Camdyn Zook czook@iastate.edu
- Syed Ahmad Alhussain syedal@iastate.edu
- Nithin Sebastian nithins@iastate.edu

2/10/2021 - We created dungeonGen.c in 3 main parts: The hardness, room generation, and hallway generation.
Creating the hardness of each tile wasn't that hard (pun intended). we just set a random value to each "tile"
Room Generation wasn't too bad we randomly placed and sized each room and if we detected an overlap we would restart the generation
hallway Generation was the hardest part, but with the help of piazza we determined that the best course of action was to find the slope
to each of the rooms starting x, y
ran with: gcc dungeonGen.c -o dungeonGen -lm