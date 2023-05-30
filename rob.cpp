#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <list>

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
	int	type;
	int	initialResources;
	int neigh[6];

	//cube coordinates
	Hex	cube;

	int resources;
	int myAnts;
	int opAnts;

	Cell();
	~Cell();
};

Cell::Cell()
{
}

Cell::~Cell()
{
}


class Map
{
private:

public:
	vector<Cell>	Cells;
	int				numberOfCells;
	int				numberOfBases;
	int				myBaseIndex;
	int				opBaseIndex;
	Map(int nCells);
	~Map();
};

Map::Map(int nCells) : numberOfCells(nCells), Cells(nCells)
{
}

Map::~Map()
{
}


/******		FUNCTIONS		******/

list<int>	initCellList(int a, int b) {

	list<int> cellList;
	while (a <= b) {
		cellList.push_back(a);
		a++;
	}
    return cellList;
}

bool	findCell(int cellIndex, list<int> list) {
	
	for (int cell : list) {
		if (cell == cellIndex)
			return true;		
	}
	return false;
}

int	nextResourceful(Map lvMap, int type)
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

void	map_neighbors(const Cell& cur_Cell, Map& lvlMap) {
	
	
	for (int i = 0; i < 6; i++) { //protect not to reassign
		if (cur_Cell.neigh[i] != -1) {
			lvlMap.Cells[cur_Cell.neigh[i]].cube = cur_Cell.cube + dirVector[i];
		}
	}
};

void	map_coordinates(Map& lvMap) {
	
	list<int>	unmapped;
	list<int>	mapped;
	// list<int>	neighMapped;
	int			curCell;

	unmapped = initCellList(0, lvMap.numberOfCells - 1);
	
	lvMap.Cells[lvMap.myBaseIndex].cube.set(0, 0, 0); // defining base coordinate as origin

	unmapped.remove(lvMap.myBaseIndex);
	mapped.push_back(lvMap.myBaseIndex);

	while(!unmapped.empty())
	{
		curCell = mapped.back();
		mapped.remove(curCell);
		map_neighbors(lvMap.Cells[curCell], lvMap);
		for (int i = 0; i < 6; i++){
			if (lvMap.Cells[curCell].neigh[i] != -1 && findCell(lvMap.Cells[curCell].neigh[i], unmapped)) {
				unmapped.remove(lvMap.Cells[curCell].neigh[i]);
				mapped.push_back(lvMap.Cells[curCell].neigh[i]);
			}
		}
	}
};

/******		Main		******/

int main()
{
	int numberOfCells;
	cin >> numberOfCells; cin.ignore();
	
	Map 		levelMap(numberOfCells);
	vector<int>	ResCells;
	vector<int>	EggCells;

	for (int i = 0; i < numberOfCells; i++) {
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

	// DISTANCE CALC //



	// usleep(950000);
	// game loop

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
