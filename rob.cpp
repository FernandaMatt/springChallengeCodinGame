#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <list>
// #include <map>
#include <iterator>
#include <iomanip>
#include <set>


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

//auxiliary functions
list<int>	initCellList(int a, int b) {

	list<int> cellList;
	while (a <= b) {
		cellList.push_back(a);
		a++;
	}
    return cellList;
}

bool	findCellIndex(int cellIndex, list<int> list) {
	
	for (int cell : list) {
		if (cell == cellIndex)
			return true;		
	}
	return false;
}

//map cube coordinates
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
			if (lvMap.Cells[curCell].neigh[i] != -1 && findCellIndex(lvMap.Cells[curCell].neigh[i], unmapped)) {
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

/******		DISTANCE CALC		******/

int	distance(int start, int end, vector<Cell> &grid) {
	list<int> path;
	path = aStar(start, end, grid);
	return (path.size() - 1);
}

bool cmp(pair<int, int>& a, pair<int, int>& b)
{
    return a.second < b.second;
}

vector<vector<int>> calcDistancesMatrix(int nOfCells, vector<Cell> &grid) {

	vector<vector<int>> mDist(nOfCells, vector<int>(nOfCells));

	for (int i = 0; i < nOfCells; i++) {
		for (int j = 0; j < nOfCells; j++) {
			mDist[i][j] = distance(i, j, grid);
		}
	}
	return (mDist);
}

/******		CALCULATE PATH		******/

//auxiliary functions

list<int> vectorToList(const vector<int>& vec) {
    list<int> lst(vec.begin(), vec.end());
    return (lst);
}

int	findClosestCell(int cellIndex, list<int>cellList, vector<vector<int>> dMatrix) {

	int	closestCell;

	closestCell = cellList.front();
	for (int cell : cellList) {
		if (dMatrix[cellIndex][cell] < dMatrix[cellIndex][closestCell])
			closestCell = cell;
	}
	return (closestCell);
}

void removeCommonValues(std::list<int>& a, const std::list<int>& b) {

    a.remove_if( [&b](int value) {return (find(b.begin(), b.end(), value) != b.end());} );
}

void addUniqueValues(std::list<int>& cmpList, std::list<int>& assingList) {

    set<int> uniqueValues(assingList.begin(), assingList.end());  // Copy elements of list assingList to cmpList set

    for (const auto& value : cmpList) {
        uniqueValues.insert(value);  // Insert each value from list cmpList into the set
    }
    assingList.assign(uniqueValues.begin(), uniqueValues.end());  // Copy unique values from the set back to list assingList
}

vector<pair<int, list<int>>> recalcPaths(int baseIndex, vector<pair<int, list<int>>> paths, vector<Cell> grid, vector<vector<int>> distMatrix) {

    list<int> targets;

    for (auto i : paths)
        targets.push_back(i.first);

    list<int>	        shortestPath;
    list<int>	        mappedCells;
    vector<pair<int, list<int>>> newPaths;
	int		        	targ;
	int		        	closestMappedCell;
    static int          is = 0;

    // cerr << "Rodada: " << is << endl << endl;
    mappedCells.push_back(baseIndex);
    while (!targets.empty()) {
        removeCommonValues(targets, mappedCells);
        targ = findClosestCell(baseIndex, targets, distMatrix);
        targets.remove(targ);
        cerr << targ << endl;
        cerr << "Closest MappedCell:" << endl;
        closestMappedCell = findClosestCell(targ, mappedCells, distMatrix);
        cerr << closestMappedCell << endl;
        shortestPath = aStar(closestMappedCell, targ, grid);
        pair<int, list<int>> path;
        path.first = targ;
        path.second = shortestPath;
        newPaths.push_back(path);
        cerr << "Shortest path from closest ResCell to closest MappedCell:" << endl;
        for (int cell : shortestPath)
            cerr << cell << ", ";
        cerr << endl;
        addUniqueValues(shortestPath, mappedCells);
        cerr << "Remaining targets:" << endl;
        for (int Res : targets)
            cerr << Res << ", ";
        cerr << endl;
        cerr << "MappedCells:" << endl;
        for (int Res : mappedCells)
            cerr << Res << ", ";
        cerr << endl;
        shortestPath.remove(baseIndex);
    }
    cerr << "Returned MappedCells:" << endl;
    for (int Res : mappedCells)
        cerr << Res << ", ";
    cerr << endl;
	return(newPaths);
}

//calculate path
list<int> calculatePath(int baseIndex, list<int> ResCells, vector<Cell> grid, vector<vector<int>> distMatrix) {

	static list<int>	mappedCells = {baseIndex};
	static list<int>	shortestPath;
	int		        	currentRes;
	int		        	closestMappedCell;
    static vector<pair<int, list<int>>>    paths;
    static int          npaths = 0;
    static int  is = 0;

	// mappedCells.push_back(baseIndex);
    cerr << "Rodada: " << is << endl << endl;
    // cerr << "ResCells:" << endl;
    // for (int Res : ResCells)
    //     cerr << Res << ", ";
    // cerr << endl;

    if (is > 0) {

        for (auto it = paths.begin(); it != paths.end(); ) {

            if (!findCellIndex(it->first, ResCells)) {

                for (int cell : it->second) {

                    mappedCells.remove(cell);
                    shortestPath.remove(cell);
                }
                it = paths.erase(it); // Remove o par do vetor
            }
            else
                ++it; // Incrementa o iterador apenas se o par não foi removido
        }
        paths = recalcPaths(baseIndex, paths, grid, distMatrix);
    }
    // if (mappedCells.size() > 1) {
    //     for (pair path : paths) {
    //         if (!findCellIndex(path.first, ResCells)) {
    //             for (int cell : path.second) {
    //                 mappedCells.remove(cell);
    //                 shortestPath.remove(cell);
    //             }
    //     }
    // }
    // }
    if (shortestPath.empty()) {
        removeCommonValues(ResCells, mappedCells);
        currentRes = findClosestCell(baseIndex, ResCells, distMatrix); //find the resource cell closest to the base
        // cerr << "Closest ResCell:" << endl;
        // cerr << currentRes << endl;
        // cerr << "Closest MappedCell:" << endl;
        closestMappedCell = findClosestCell(currentRes, mappedCells, distMatrix); //find the Cell already with beacons/ants closest to the resource cell
        // cerr << closestMappedCell << endl;
        shortestPath = aStar(closestMappedCell, currentRes, grid); // calculating shortest path to target resource from the closest cell with beacons/ants
        pair<int, list<int>> path;
        path.first = currentRes;
        path.second = shortestPath;
        paths.push_back(path);
        // cerr << "Shortest path from closest ResCell to closest MappedCell:" << endl;
        // for (int cell : shortestPath)
        //     cerr << cell << ", ";
        // cerr << endl;
        addUniqueValues(shortestPath, mappedCells);
        // cerr << "Remaining ResCells:" << endl;
        // for (int Res : ResCells)
        //     cerr << Res << ", ";
        // cerr << endl;
        // cerr << "MappedCells:" << endl;
        // for (int Res : mappedCells)
        //     cerr << Res << ", ";
        // cerr << endl;
        shortestPath.remove(baseIndex);
        npaths++;
    }
    mappedCells.push_back(shortestPath.front());
    shortestPath.pop_front();
    is++;
    if (!findCellIndex(baseIndex, mappedCells)) {
        closestMappedCell = findClosestCell(baseIndex, mappedCells, distMatrix);
        shortestPath = aStar(closestMappedCell, baseIndex, grid);
        addUniqueValues(shortestPath, mappedCells);
    }
    cerr << "Returned MappedCells:" << endl;
    for (int Res : mappedCells)
        cerr << Res << ", ";
    cerr << endl;
	return(mappedCells);
}

/******		MAIN		******/

std::vector<int> mergeVectors(const std::vector<int>& vectorA, const std::vector<int>& vectorB) {

    std::vector<int> mergedVector;

    mergedVector.reserve(vectorA.size() + vectorB.size());
    std::merge(vectorA.begin(), vectorA.end(), vectorB.begin(), vectorB.end(), std::back_inserter(mergedVector));
    return (mergedVector);
}

std::list<int> mergeLists(const std::list<int>& listA, const std::list<int>& listB) {
    std::list<int> mergedList(listA.begin(), listA.end());  // Copia elementos de listA para mergedList
    mergedList.insert(mergedList.end(), listB.begin(), listB.end());  // Insere elementos de listB em mergedList
    return mergedList;
}

int main()
{
	// READ INITIAL INPUT //

	int numberOfCells;
	cin >> numberOfCells; cin.ignore();

	gridLvl 	levelMap(numberOfCells);
	list<int>	crisCells;
	list<int>	eggCells;

	for (int i = 0; i < numberOfCells; i++) {
		levelMap.Cells[i].index = i;
		cin	>> levelMap.Cells[i].type >> levelMap.Cells[i].initialResources
			>> levelMap.Cells[i].neigh[0] >> levelMap.Cells[i].neigh[1]
			>> levelMap.Cells[i].neigh[2] >> levelMap.Cells[i].neigh[3]
			>> levelMap.Cells[i].neigh[4] >> levelMap.Cells[i].neigh[5];
			cin.ignore();
		if (levelMap.Cells[i].type == 2)
			crisCells.push_back(i);
		if (levelMap.Cells[i].type == 1)
			eggCells.push_back(i);
	}
	cin >> levelMap.numberOfBases; cin.ignore();
	for (int i = 0; i < levelMap.numberOfBases; i++) {
		cin >> levelMap.myBaseIndex; cin.ignore();
	}
	for (int i = 0; i < levelMap.numberOfBases; i++) {
		cin >> levelMap.opBaseIndex; cin.ignore();
	}

    // resCells = mergeLists(crisCells, eggCells);


	// MAPPING COORDINATES //

	map_coordinates(levelMap);

	// CALCULATING DISTANCES //

	vector<vector<int>> distMatrix;

	distMatrix = calcDistancesMatrix(levelMap.numberOfCells, levelMap.Cells);

    list<int>   path;

	// GAME LOOP //
	while (1) {
	    list<int>	resCells;
		for (int i = 0; i < numberOfCells; i++) {
			cin >> levelMap.Cells[i].resources >> levelMap.Cells[i].myAnts
			>> levelMap.Cells[i].opAnts;
			cin.ignore();
            if (levelMap.Cells[i].resources > 0)
                resCells.push_back(i);
        }
        cerr << endl;
        path = calculatePath(levelMap.myBaseIndex, resCells, levelMap.Cells, distMatrix);
		for (int cellIndex : path) {
            cout << "BEACON " << cellIndex << " " << 1 << ";" ;
		}
        // cout << "LINE 31 21 1; ";
        // cout << "LINE 21 24 1";
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
