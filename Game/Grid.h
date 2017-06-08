#pragma once
#include <cmath>
#include <algorithm>
#include <vector>


template <typename T>
class Cell
{
public:
	void Add(T* obj)
	{
		objects_.emplace_back(obj);
	}

	void Del(T* obj)
	{
		objects_.erase(std::remove(objects_.begin(), objects_.end(), obj), objects_.end());
	}

	const auto& Get() const
	{
		return objects_;
	}

private:
	std::vector<T*> objects_;
};

template <typename T>
class Grid
{
public:
	using CellType = Cell<T>;

	Grid(int map_width, int map_height, int cell_size)
		: cell_size_(cell_size)
	{
		num_x_ = (int)std::ceil(map_width / cell_size);
		num_y_ = (int)std::ceil(map_height / cell_size);

		cells_.resize(num_x_ * num_y_);
	}
	~Grid()
	{

	}

	CellType& GetCell(int idx_x, int idx_y)
	{
		int idx = idx_x + (idx_y * num_x_);
		return cells_.at(idx);
	}
	
	CellType& GetCellByPos(float pos_x, float pos_y)
	{
		return GetCell((int)(pos_x / cell_size_), (int)(pos_y / cell_size_));
	}


	std::vector<CellType>& GetCells() noexcept
	{
		return cells_;
	}

	bool IsIdxValid(int idx_x, int idx_y) const noexcept
	{
		return idx_x >= 0 && idx_y >= 0 &&
			idx_x < num_x_ && idx_y < num_y_;
	}

private:
	std::vector<CellType> cells_;
	int num_x_;
	int num_y_;
	int cell_size_;
};