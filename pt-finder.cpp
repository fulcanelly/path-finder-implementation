#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <ctime> 
#include <vector>

#define XXX 500
#define YYY 500
#define FPS 100
#define PI 3.14
#define nullcoord coord{-1,-1}
#define startIndent sideSize/2

using namespace std;

void timer_callback(int);
void display_callback();
void reshape_callback(int,int);
void mouseClicks(int,int,int,int);
void randomBlockFiller(int);

int rblocount = 0;
enum blockTypes{
	voided, //пустой -- только через него может проходить путь
    block, //блок -- через него не может идти путь
    final,
	start,
	way, //тип пути
};

int main(int argc, char **argv)
{
	if(argc >=2)
		rblocount = atoi(argv[1]);
	std::srand(time(0));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(500,500);
	glutCreateWindow("path-finder");
	glutDisplayFunc(display_callback);
	glutReshapeFunc(reshape_callback);
		randomBlockFiller(rblocount);

	glutMouseFunc(mouseClicks);
	glutTimerFunc(0,timer_callback,0);
	glClearColor(0.0,0.0,0.0,1.0);
	glutMainLoop();
	return 0;
}

struct coord
{
    int x;
	int y;

    coord operator+(coord another){
        auto resx = x+another.x;
        auto resy = y+another.y;
        return coord{resx, resy};
    }

    coord operator+=(coord another){
        return *this = another + *this;
    }

    void print(){
        cout<<"  { x: "<<x<<", y: "<<y<<" },"<<endl;
    }

    bool operator==(coord other){
        return x==other.x && y==other.y;
    }
    
    bool operator!=(coord other){
        return !(*this==other);
    }
};

struct point {   
    blockTypes type = voided;
    coord from = nullcoord;
	float indent = 1;
};

#define USING_OFFSET
vector<coord> offset = {
    {1,0},
    {0,1},
    {-1,0},
    {0,-1}
};

vector<vector<point>> field;
coord sct{0, 0}; //approximate mouse postion
int sideSize = 20;
int height;
int width;
uint fsize; 

coord indexOf(vector<vector<point>> array, int size, blockTypes obj){
	for(int x = 0; x < width; x++)
	for(int y = 0; y < height; y++)
		if(array[x][y].type == obj)
			return {x, y};
	return nullcoord;
}

coord freeRandCoord(){
	coord res;
	do {
		res = {rand()%width, rand()%height};
	} while(field[res.x][res.y].type != blockTypes::voided);
	return res;
}

void randomBlockFiller(int count)
{
	for(auto i = 0; i < count; i++)
	{
		auto postion = freeRandCoord();
		field[postion.x][postion.y].type = blockTypes::block;
	}
}

inline bool match(coord point){
    auto x = point.x;
    auto y = point.y;

    if( x >= 0 && y >= 0 && x < width && y < height) //checking for belong point to range
        if(
            field[x][y].type != block &&
            field[x][y].from == nullcoord
        )
            return true;
    return false;
} 

inline vector<coord> getPointNeighbours(coord center, bool match(coord))
{
    vector<coord> neighbours;
	#ifdef USING_OFFSET
	
   	for(auto shift:offset)
	{
	#else

   	for(int x = -1; x <= 1; ++x)
	for(int y = -1; y <= 1; ++y)
	{
		auto shift = coord{x, y};
	#endif


        auto newPoint = center+shift;
        if(match(newPoint))
            neighbours.push_back(newPoint);
    }
    return neighbours;
}

//does relations betwen points
vector<coord> select(vector<coord> directions)
{
    //in this loop each point connecting with previous point
    for(auto point:vector<coord>(directions))
    { 
        auto neighbours = getPointNeighbours(point, match);
        for(auto npoint:neighbours)
            field[npoint.x][npoint.y].from = point;
            //set previous point for back way
        directions.insert(directions.end(), neighbours.begin(), neighbours.end());
    }
    return directions;
}

//collect all of directions to one array of coords
vector<coord> getWay(coord start, coord end){
    vector<coord> way;
    coord current = start;
    int count = 0;
    while (field[current.x][current.y].from != end)
    {
        count++;
        if(count > fsize)
            break;
        current = field[current.x][current.y].from;
        way.push_back(current);
    }
    return way;
}

template<class vecType>
inline bool isVecEqual(vector<vecType> a, vector<vecType> b)
{
	bool res = true;
	if(a.size() != b.size())
		return false;
	for(int i = 0; i < b.size(); i++)
		res = res & a[i] == b[i];
	return res;
}

//find path netwen from and target points and return array of way points
vector<coord> findPath(coord from, coord target){
    vector<coord> directions = {from};
	vector<coord> pastDir;
    while(field[target.x][target.y].from == nullcoord)
	{
		directions = select(directions);
		if(isVecEqual(pastDir, directions))
			return vector<coord>();
		pastDir = directions;
	}
    return getWay(target, from);
}

///
inline blockTypes setUiniq(blockTypes type){
	coord last = indexOf(field, fsize, type);
	if(last != nullcoord)
		field[last.x][last.y].type = voided;
	return type;
}

coord indexTocoord(int index){
	int x = index%width;
	int y = (index-x)/width;
	return {x,y};
}

void clearPrviousWay()
{
	while(indexOf(field, fsize, way) != nullcoord)
	{
		coord deleting = indexOf(field, fsize, way);
		field[deleting.x][deleting.y].type = voided;
	}
}

int lastPathSize = 0;
void prepeareToFinder(){
	//clearing previous auxiliary data
	for(int x = 0; x < width; x++)
	for(int y = 0; y < height; y++)
		field[x][y].from = nullcoord;
	clearPrviousWay();
	//check for all needed variables
	auto start = indexOf(field, fsize, blockTypes::start);
	auto end = indexOf(field, fsize, blockTypes::final);
	if(start == nullcoord)
		return;
	if(end == nullcoord)
		return;

	//processing path
	vector<coord> path = findPath(start, end);
	vector<coord> pastPath;
	bool areEqual = isVecEqual(path, pastPath);
	//draw new way
	float fIndent;
	if(areEqual)
		fIndent = 1;
	else
		fIndent = startIndent;
		
	for(auto p:path)
	{
		field[p.x][p.y].type = way;
		field[p.x][p.y].indent = fIndent;
	}

	pastPath = path;

}

inline void setPoint(int code, coord postion)
{
	blockTypes type = voided;
	int indent = startIndent;
	
	switch (code)
	{
		case 0:
			if(field[postion.x][postion.y].type == block)
				type = voided;
			else
				type = block;
			break;
		case 3:
			type = setUiniq(start);
			break;
		case 4:
			type = setUiniq(final);
			break;
		case 1:
			break;
		case 2:
			cout<<"h:"<< height<<endl;
			cout<<"w:"<< width<<endl;
			cout<<"size:"<< fsize<<endl;
			postion.print();
			break;
	}

	field[postion.x][postion.y].type = type;
	field[postion.x][postion.y].indent = indent;
	prepeareToFinder();
}

inline void mouseClicks(int button, int state, int x, int y){
	if(!state){
		y = YYY-y;
		sct = {x/sideSize, y/sideSize};
		setPoint(button, sct);
	}
}

void reshape_callback(int w, int h){
	cout<<"w"<<w<<endl;
	height = h/sideSize+1;
	width = w/sideSize+1;
	fsize = height*width;
	field.resize(width, vector<point>(height));
	glViewport(0,0,GLsizei(w),GLsizei(h));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0.0,XXX,0.0,YYY,-1.0,1000.0);
}

void timer_callback(int)
{
	glutPostRedisplay();
	glutTimerFunc(1000/FPS,timer_callback,0);
}

void display_callback()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.4,0.4,0.0);
	//
	for(int x = 0; x < width; x++)
	for(int y = 0; y < height; y++)
	{
		point *itPoint = &field[x][y];
		{
			switch (itPoint->type)
			{
				case start:
					glColor3f(0.8,0.4,0.0);
					break;
				case final:
					glColor3f(0.4,0.8,0.8);
					break;
				case block:
					glColor3f(0.4,0.4,0.0);
					break;
				case way:
					glColor3f(0.0,0.8,0.25);
					break;
				default:
					glColor3f(0.0,0.0,0.0);
					break;
			}
			///
			float indent = itPoint->indent;
			float x0, y0, x1, y1;
			float indentOnX = x*sideSize;
			float indentOnY = y*sideSize;

			if(indent != 1)
			{	
				indent -=1;
				float shift = sideSize-indent;
				x0 = indent/2.0+indentOnX;
				y0 = indent/2.0+indentOnY;
				x1 = indentOnX + shift;
				y1 = indentOnY + shift;
				itPoint->indent = indent;
			} else {
				x0 = indentOnX;
				y0 = indentOnY;
				x1 = indentOnX + sideSize;
				y1 = indentOnY + sideSize;
			}
			glRectd(x0,y0,x1,y1);
		}	
	}

	glutSwapBuffers();
}

