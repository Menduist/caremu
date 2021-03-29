# Car traffic emulator

![Sample screenshot](https://github.com/Menduist/caremu/blob/master/screenshot.png?raw=true)

Caremu is a naive traffic emulator written in C. It imports [OpenStreetMap](https://www.openstreetmap.org/) to create real-life roads, and then throws a few thousands fake cars in the map.
Each car will pick a random point in the map, and steer it's way on the shortest path to this point.

It was originally planned to go further than this (ie. make cars avoid each other, add red lights, speed limitations, etc), but this would be impractical since it would cap the program to a few hundred cars for performance reasons.

## Usage

You must [export OSM data](https://www.openstreetmap.org/export) to use this program. The export feature is quite limited in size, but you can easily export a city neighborhood.
This repository contains two sample file in the **maps** folder, "fresno" and "paris".

The [SFML](https://www.sfml-dev.org/) is used for display & input.

You can then `make && ./caremu [-m osm map file path] [-c cars count]`, use the arrow keys to move and `-`/`=` to zoom in/out

## Demo

[Here](https://www.youtube.com/watch?v=V0FpzEIb5ro) is a video of the program running with 10,000 cars around the city of Strasbourg, France.
