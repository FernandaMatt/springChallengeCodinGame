#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <list>
// #include <map>
// #include <iterator>
#include <iomanip>


using namespace std;


/******			STRUCTS		******/

struct Hex {

	int q, r, s; //cube coordinates
    
	Hex(void): q(-1), r(-1), s(-1){}; //cube default constructor NOT ASSIGNING -1??
	Hex(int q_, int r_, int s_): q(q_), r(r_), s(s_) { //cube constructor
		assert (q + r + s == 0);
	}

	void	set(int q_, int r_, int s_);

	Hex	operator+(const Hex& cpHex) const {
		return Hex(q + cpHex.q, r + cpHex.r, s + cpHex.s);
	}
	
	Hex&	operator=(const Hex& cpHex) {
		q = cpHex.q;
		r = cpHex.r;
		s = cpHex.s;
		return *this;
	};

	Hex&	operator+=(const Hex& cpHex) {
		q += cpHex.q;
		r += cpHex.r;
		s += cpHex.s;
		return *this;
	}

	bool	operator==(const Hex& cpHex) const {
        return (q == cpHex.q) && (r == cpHex.r) && (s == cpHex.s);
    }
};



void	Hex::set(int q_, int r_, int s_){

	this->q = q_;
	this->r = r_;
	this->s = s_;
};

Hex dirVector[] = {
    Hex(+1, 0, -1), Hex(+1, -1, 0), Hex(0, -1, +1), 
    Hex(-1, 0, +1), Hex(-1, +1, 0), Hex(0, +1, -1), 
};

/******		CLASSES		******/

class Cell
{
private:

public:
	int	index;;
	int	type;
	int	initialResources;
	int neigh[6];

	//cube coordinates
	Hex	cube;

	//a * variables
	int g = 0;
	int h = 0;
	int f = 0;

	int resources;
	int myAnts;
	int opAnts;

	Cell();
	~Cell();

};

bool operator==(const Cell& cell1, const Cell& cell2)
{
	return cell1.cube == cell2.cube;
}

Cell::Cell()
{
}

Cell::~Cell()
{
}


class gridLvl
{
private:

public:
	vector<Cell>	Cells;
	int				numberOfCells;
	int				numberOfBases;
	int				myBaseIndex;
	int				opBaseIndex;
	gridLvl(int nCells);
	~gridLvl();
};

gridLvl::gridLvl(int nCells) : numberOfCells(nCells), Cells(nCells)
{
}

gridLvl::~gridLvl()
{
}


/******		HEX FUNCTIONS		******/

Hex hex_subtract(Hex a, Hex b) {
    return Hex(a.q - b.q, a.r - b.r, a.s - b.s);
}

int hex_length(Hex hex) {
    return int((abs(hex.q) + abs(hex.r) + abs(hex.s)) / 2);
}

int hex_distance(Hex a, Hex b) {
    return hex_length(hex_subtract(a, b));
}

list<int>	initCellList(int a, int b) {

	list<int> cellList;
	while (a <= b) {
		cellList.push_back(a);
		a++;
	}
    return cellList;
}

bool	findCellindex(int cellIndex, list<int> list) {
	
	for (int cell : list) {
		if (cell == cellIndex)
			return true;		
	}
	return false;
}

int	nextResourceful(gridLvl lvMap, int type)
{
	int	nextRes = -1;

	for (size_t i = 0; i < lvMap.numberOfCells; i++)
	{
		if (lvMap.Cells[i].type == type && lvMap.Cells[i].resources > 0){
			nextRes = i;
			return (nextRes);
		}
	}
	return (nextRes);
}

/******		MAP FUNCTIONS		******/

void	map_coordinates(gridLvl& lvMap) {
	
	list<int>	unmapped;
	list<int>	mapped;
	int			curCell;

	unmapped = initCellList(0, lvMap.numberOfCells - 1);
	
	lvMap.Cells[lvMap.myBaseIndex].cube.set(0, 0, 0); // defining base coordinate as origin

	unmapped.remove(lvMap.myBaseIndex);
	mapped.push_back(lvMap.myBaseIndex);

	while(!unmapped.empty())
	{
		curCell = mapped.back();
		mapped.remove(curCell);
		for (int i = 0; i < 6; i++){
			if (lvMap.Cells[curCell].neigh[i] != -1 && findCellindex(lvMap.Cells[curCell].neigh[i], unmapped)) {
				lvMap.Cells[lvMap.Cells[curCell].neigh[i]].cube = lvMap.Cells[curCell].cube + dirVector[i];
				unmapped.remove(lvMap.Cells[curCell].neigh[i]);
				mapped.push_back(lvMap.Cells[curCell].neigh[i]);
			}
		}
	}
};

/******		PATHFINDING ALGORITHM		******/

//auxiliary functions
bool	findCell(Cell *end, list<Cell*> list) {
	
	for (Cell *cell : list) {
		if (cell->index == end->index)
			return true;
	}
	return false;
}

bool	compareFGHValues (Cell *a, Cell *b) {
	if (a->f < b->f)
		return (true);
	else if (b->f < a->f)
		return (false);
	else {
		if (a->h < b->h)
			return (true);
		else if (a->h > b->h)
			return (false);
	}
	return false;
}

//A*
list<int>	backTrack(Cell endCell, list<Cell*> closedCells, vector<Cell> &grid) {

	list<int> finalPath;
	Cell *currentCell;
	currentCell = &endCell;
	finalPath.push_front(currentCell->index);
	while (currentCell->g != 0) {
		for (int neighIndex : currentCell->neigh) {
			if ( neighIndex == -1) {
				continue ;
			}
			Cell *neigh = &grid[neighIndex];
			if (findCell(neigh, closedCells)) {
				if (neigh->g < currentCell->g) {
					finalPath.push_front(neigh->index);
					currentCell = neigh;
					break ;
				}
			}
		}
	}
	return (finalPath);
}

list<int>	aStar(int startCell, int end, vector<Cell> &grid) {

	list<Cell*>	openCells;
	list<Cell*>	closedCells;
	int			g;

	Cell	*currentCell = &grid[startCell];
	Cell	&endCell = grid[end];
	currentCell->g = 0;
	currentCell->h = hex_distance(currentCell->cube, endCell.cube);
	currentCell->f = currentCell->g + currentCell->h;
	openCells.push_back(currentCell);

	while (!openCells.empty())
	{
		openCells.sort(&compareFGHValues);
		currentCell = openCells.front();
		openCells.remove(currentCell);
		closedCells.push_front(currentCell);
		g = currentCell->g + 1;
		if (findCell(&endCell, closedCells)) {
			break;
		}
		for (int neighIndex : currentCell->neigh) {
			if (neighIndex == -1) {
				continue;
			}
			Cell *neigh = &grid[neighIndex];
			if (findCell(neigh, closedCells)) {
				continue;
			}
			if (!findCell(neigh, openCells)) {
				neigh->g = g;
				neigh->h = hex_distance(neigh->cube, endCell.cube);
				neigh->f = neigh->g + neigh->h;
				openCells.push_front(neigh);
			}
			else if (neigh->f > g + neigh->h)
				neigh->g = g;
		}
	}
	list<int> finalPath;
	if (findCell(&endCell, closedCells)) {
		finalPath = backTrack(endCell, closedCells, grid);
	}
	return (finalPath);
}

/******		DISTANCE CALC AND MAP		******/

int	distance(int start, int end, vector<Cell> &grid) {
	list<int> path;
	path = aStar(start, end, grid);
	return (path.size() - 1);
}

bool cmp(pair<int, int>& a, pair<int, int>& b)
{
    return a.second < b.second;
}

vector<vector<int>> calcDistances(int nOfCells, vector<Cell> &grid) {

	vector<vector<int>> mDist(nOfCells, vector<int>(nOfCells));

	for (int i = 0; i < nOfCells; i++) {
		for (int j = 0; j < nOfCells; j++) {
			mDist[i][j] = distance(i, j, grid);
		}
	}
	return (mDist);
}

/******		Main		******/

int main()
{
	int numberOfCells;
	cin >> numberOfCells; cin.ignore();
	
	gridLvl 		levelMap(numberOfCells);
	vector<int>	ResCells;
	vector<int>	EggCells;
	list<int>	path;
	int			dist;

	for (int i = 0; i < numberOfCells; i++) {
		levelMap.Cells[i].index = i;
		cin	>> levelMap.Cells[i].type >> levelMap.Cells[i].initialResources
			>> levelMap.Cells[i].neigh[0] >> levelMap.Cells[i].neigh[1]
			>> levelMap.Cells[i].neigh[2] >> levelMap.Cells[i].neigh[3]
			>> levelMap.Cells[i].neigh[4] >> levelMap.Cells[i].neigh[5];
			cin.ignore();
		if (levelMap.Cells[i].type == 2)
			ResCells.push_back(i);
		if (levelMap.Cells[i].type == 1)
			EggCells.push_back(i);
	}

	cin >> levelMap.numberOfBases; cin.ignore();
	for (int i = 0; i < levelMap.numberOfBases; i++) {
		cin >> levelMap.myBaseIndex; cin.ignore();
	}
	for (int i = 0; i < levelMap.numberOfBases; i++) {
		cin >> levelMap.opBaseIndex; cin.ignore();
	}

	// MAPPING COORDINATES //

	map_coordinates(levelMap);

	// CALCULATING DISTANCES //

	vector<vector<int>> distMatrix;

	distMatrix = calcDistances(levelMap.numberOfCells, levelMap.Cells);


	while (1) {
		for (int i = 0; i < numberOfCells; i++) {
			cin >> levelMap.Cells[i].resources >> levelMap.Cells[i].myAnts
			>> levelMap.Cells[i].opAnts;
			cin.ignore();
		}

		for (int i = 0; i < numberOfCells; i++) {
			if (levelMap.Cells[i].resources != 0) {
				cout << "LINE " << levelMap.myBaseIndex << " " << i ;
				if (levelMap.Cells[i].type == 1)
					cout << " 3 ;";
				else
					cout << " 2 ;";
			}
		}
		cout << endl;
	}
}

//PRINT DISTANCE MATRIX

	// cerr << "   ";
	// for (int i = 0; i < levelMap.numberOfCells; i++) {
	// 	cerr << setw(2) << i << " ";
	// }
	// cerr << endl;
	// int i = 0;
	// for (vector <int> line : distMatrix) {
	// 	cerr << setw(2) << i << " ";
	// 	for (int dist : line) {
	// 		cerr << setw(2) << dist << " ";
	// 	}
	// 	cerr << endl;
	// 	i++;
	// }
