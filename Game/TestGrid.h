#pragma once
#include <iostream>
#include <cassert>
#include "Grid.h"

void TestGrid()
{
    BoundingBox bb(Vector3(0, 0, 0), Vector3(100, 1, 100));

    Grid<> grid(bb, Vector3(10, 10, 10));
    std::cout << grid.TileX() << "," << grid.TileY() << "," << grid.TileZ() << std::endl;

    auto pos = Vector3(45, 0, 45);
    auto* cell = grid.GetCell(pos);
    assert(cell);
    std::cout << cell->X() << "," << cell->Y() << "," << cell->Z() << std::endl;

    std::cout << "=====GetCells=====" << std::endl;
    auto area = BoundingBox(pos + Vector3(-15, 0, -15), pos + Vector3(15, 1, 15));
    auto cells = grid.GetCells(area);
    for (auto* c : cells)
    {
        std::cout << c->X() << "," << c->Y() << "," << c->Z() << std::endl;
    }
    std::cout << "=====GetCells=====" << std::endl;
}