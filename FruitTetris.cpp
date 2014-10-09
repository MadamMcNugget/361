/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04.

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <stdlib.h>

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400;
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

int rotationType; // one of the 4 rotations
int shapeType; // which shape -> 0 = S, 1 = I, 2 = L

vec4 tileColours[4];

// Arrays storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] =
    {{vec2(-1, -1), vec2(-1,0), vec2(0, 0), vec2(1, 0)},
     {vec2(1, -1), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
     {vec2(1, 1), vec2(1,0), vec2(0, 0), vec2(-1, 0)},
     {vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

vec2 allRotationsIshape[4][4] =
    {{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1,0)},
     {vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
     {vec2(2, 0), vec2(1, 0), vec2(0, 0), vec2(-1, 0)},
     {vec2(0, 2), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

vec2 allRotationsSshape[4][4] =
    {{vec2(-1, -1), vec2(0, -1), vec2(0, 0), vec2(1,0)},
     {vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
     {vec2(1, 1), vec2(0, 1), vec2(0, 0), vec2(-1, 0)},
     {vec2(-1, 1), vec2(-1, 0), vec2(0, 0), vec2(0, -1)}};

// colours
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 purple = vec4(0.5137, 0.4353, 1.0, 1.0);
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black = vec4(0.0, 0.0, 0.0, 1.0);

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

// timer
int timer = 750; // in milliseconds

// level related things
int level = 1;
int deleted = 0;

// game over
bool gameOver = false;


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
    // Bind the VBO containing current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
    // For each of the 4 'cells' of the tile,
    for (int i = 0; i < 4; i++)
    {
        // Calculate the grid coordinates of the cell
        GLfloat x = tilepos.x + tile[i].x;
        GLfloat y = tilepos.y + tile[i].y;

        // Create the 4 corners of the square - these vertices are using location in pixels
        // These vertices are later converted by the vertex shader
        vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
        vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
        vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
        vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

        // Two points are used by two triangles each
        vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4};

        // Put new data in the VBO
        glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints);
    }

    glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------


// Called at the start of play and every time a tile is placed
void newtile()
{
    shapeType = rand() % 3; // Choose one of the 3 shapes we have
    rotationType = rand() % 4; // there are 4 positions for each shape

    // Update the geometry VBO of current tile
    for (int i = 0; i < 4; i++) {
        if (shapeType == 0)
            tile[i] = allRotationsSshape[rotationType][i];
        else if (shapeType == 1)
         tile[i] = allRotationsIshape[rotationType][i];
        else
            tile[i] = allRotationsLshape[rotationType][i]; }

    // random starting position
    int randx = rand() % 10;
    tilepos = vec2(randx , 19); // Put the tile at the top of the board
    for (int i ; i<4 ; i++ ) {
        while (tilepos.x + tile[i].x < 0)  // check left bound
            tilepos.x++;
        while (tilepos.x + tile[i].x > 9)  // check right bound
            tilepos.x--;
        while ((tilepos.y + tile[i].y) > 19) // check upper bound
            tilepos.y--;
    }

    updatetile();

    // Update the color VBO of current tile
    vec4 newcolours[24];
    for (int j = 0; j < 24; j++)
    {
        int randColour;

        if (remainder(j, 6) == 0) // Assign a new colour every 6 vertices
            randColour = rand() % 5;

        switch(randColour)
        {
            case 0:
                newcolours[j] = red;
                break;
            case 1:
                newcolours[j] = orange;
                break;
            case 2:
                newcolours[j] = yellow;
                break;
            case 3:
                newcolours[j] = green;
                break;
            case 4:
                newcolours[j] = purple;
                break;
        }

    }

    //storing the colours
    tileColours[0] = newcolours[1];
    tileColours[1] = newcolours[7];
    tileColours[2] = newcolours[13];
    tileColours[3] = newcolours[19];

    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // if new piece cannot fit on board
    for (int i=0 ; i<4 ; i++) {
        int xpos = tilepos.x + tile[i].x;
        int ypos = tilepos.y + tile[i].y;
        if (board[xpos][ypos]) { // if square already occupied
            gameOver = true;
            cout<<"Game Over\n";
            exit(EXIT_SUCCESS);}

    }
}

//-------------------------------------------------------------------------------------------------------------------


void initGrid()
{
    // ***Generate geometry data
    vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
    vec4 gridcolours[64]; // One colour per vertex

    // Vertical lines
    for (int i = 0; i < 11; i++){
        gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
        gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
    }

    // Horizontal lines
    for (int i = 0; i < 21; i++){
        gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
        gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
    }

    // Make all grid lines white
    for (int i = 0; i < 64; i++)
        gridcolours[i] = white;


    // *** set up buffer objects
    // Set up first VAO (representing grid lines)
    glBindVertexArray(vaoIDs[0]); // Bind the first VAO
    glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

    // Grid vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
    glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition); // Enable the attribute

    // Grid vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
    glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
    // *** Generate the geometric data
    vec4 boardpoints[1200];
    for (int i = 0; i < 1200; i++)
        boardcolours[i] = black; // Let the empty cells on the board be black

    // Each cell is a square (2 triangles with 6 vertices)
        for (int i = 0; i < 20; i++){
            for (int j = 0; j < 10; j++)
            {
                vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
                vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
                vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
                vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
                // Two points are reused
                boardpoints[6*(10*i + j) ] = p1;
                boardpoints[6*(10*i + j) + 1] = p2;
                boardpoints[6*(10*i + j) + 2] = p3;
                boardpoints[6*(10*i + j) + 3] = p2;
                boardpoints[6*(10*i + j) + 4] = p3;
                boardpoints[6*(10*i + j) + 5] = p4;
            }
        }

    // Initially no cell is occupied
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            board[i][j] = false;

    // *** set up buffer objects
    glBindVertexArray(vaoIDs[1]);
    glGenBuffers(2, &vboIDs[2]);

    // Grid cell vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
    glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Grid cell vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
    glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}


// No geometry for current tile initially
void initCurrentTile()
{
    glBindVertexArray(vaoIDs[2]);
    glGenBuffers(2, &vboIDs[4]);

    // Current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Current tile vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}

void init()
{
    // Load shaders and use the shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // Get the location of the attributes (for glVertexAttribPointer() calls)
    vPosition = glGetAttribLocation(program, "vPosition");
    vColor = glGetAttribLocation(program, "vColor");

    // Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
    glGenVertexArrays(3, &vaoIDs[0]);

    // Initialize the grid, the board, and the current tile
    initGrid();
    initBoard();
    initCurrentTile();

    // The location of the uniform variables in the shader program
    locxsize = glGetUniformLocation(program, "xsize");
    locysize = glGetUniformLocation(program, "ysize");

    // Game initialization
    newtile(); // create new next tile

    // set to default
    glBindVertexArray(0);
    glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------


// Rotates the current tile, if there is room
void rotate()
{
    bool outOfBounds = false;

    rotationType++;
    if (rotationType == 4)
        rotationType = 0;

    if (shapeType == 0) { // S shape
        for (int i = 0; i < 4; i++)
            tile[i] = allRotationsSshape[rotationType][i]; }
    else if (shapeType == 1) { // I shape
        for (int i = 0; i < 4; i++)
            tile[i] = allRotationsIshape[rotationType][i]; }
    else {
        for (int i = 0; i < 4; i++)  // L shape
            tile[i] = allRotationsLshape[rotationType][i]; }

    // check bounds
    for (int i=0 ; i<4 ; i++) {
        int xpos = tilepos.x + tile[i].x;
        int ypos = tilepos.y + tile[i].y;
        if (tilepos.x + tile[i].x < 0 || tilepos.x + tile[i].x > 9) // out of walls
            outOfBounds = true;
        if (board[xpos][ypos]) // if square already occupied
            outOfBounds = true;
    }

    // if out of bounds, undo everything
    if (outOfBounds) {

        rotationType--;
        if (rotationType == -1)
            rotationType = 3;

        if (shapeType == 0) { // S shape
            for (int i = 0; i < 4; i++)
                tile[i] = allRotationsSshape[rotationType][i]; }
        else if (shapeType == 1) { // I shape
            for (int i = 0; i < 4; i++)
                tile[i] = allRotationsIshape[rotationType][i]; }
        else {
            for (int i = 0; i < 4; i++)  // L shape
                tile[i] = allRotationsLshape[rotationType][i]; }
    }

    updatetile();
}

//-------------------------------------------------------------------------------------------------------------------

// Shuffle colour order of tile
void shuffleTile()
{
    vec2 extra;

    extra = tile[3];
    tile[3] = tile[2];
    tile[2] = tile[1];
    tile[1] = tile[0];
    tile[0] = extra;

    updatetile();
}


//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow()
{
    bool isFull = true;

	while (isFull) {
	
	    isFull = false;
	    int c = 0;

	    for (int i = 0 ; i < 10 ; i++)    // i = left/right x
	    {
	        if (board[i][0] == true) {		//***changed j to 0	
	            c++; }
	     }

	    if (c==10)
	        isFull = true;

	    if (isFull) // if row is full, delete row
	    {
	        for (int k = 0 ; k < 19 ; k++)    // k = up/down y    ***change j to 0
	        {
	            for (int l = 0 ; l < 10 ; l++) {    // l = left/right x
	                int a = k * 60 + l * 6;
	                int b = a+60;
	                for (int c = 0 ; c<6 ; c++)
	                    boardcolours[a+c] = boardcolours[b+c];   // change boardcolour vertices colours
	                board[l][k] = board[l][k+1]; }
	        }

	        for (int m=0 ; m<10 ; m++)
	            board[m][19] = false;
	        for (int c=1080 ; c<1200 ; c++)
	            boardcolours[c] = black;
	    }
	}
	
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
    glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);

}

//-------------------------------------------------------------------------------------------------------------------

// Game over when any of the top row is true
/*void checkGameOver()
{
    for (int i = 0 ; i< 10 ; i++) {
        if (board[i][19]) {
            gameOver = true;
            cout<<"Game Over\n";
        }
    }

}*/

//-------------------------------------------------------------------------------------------------------------------

// falling tiles
void fallTile()
{
	for (int i=0 ; i<10 ; i++)   // i = x
	{
		int spacenum = 0;		// number of spaces from bottom to tile above
		int spaceb = 20;  		// bottom of spaces
		bool space = false;     // check for space in column
		bool space2 = false;    // check for block above space in column
		int j=0;			    // counter
		
		while(j<20) {  // j = y
			if (board[i][j]) {
				if (space) {
					space2 = true;
					break; }
				j++; }
			else {  //block not occupied
				if (spaceb > j)
					spaceb = j;
				space = true;
				spacenum++;
				j++;
			}
		}
			
		while (spaceb+spacenum < 20) 
		{
			if (space2)
			{
				int pos = spaceb * 60 + i * 6;
				int pos2 = pos + (spacenum*60);
				board[i][spaceb] = board[i][spaceb+spacenum];    // update occupancy
				for (int c=0 ; c<6 ; c++)
					boardcolours[pos+c] = boardcolours[pos2+c];  // update colours
				for (int z=(1200-(spacenum*60)) ; z<1200 ; z++)
					boardcolours[z] = black;
			}
			spaceb++;
		}
		
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Check for rows/columns to see if the have three of the same fruits. no combos
// called when set
void three()
{
    for (int y=0 ; y<20 ; y++)  // y
    {
        for (int x=0 ; x<8 ; x++)  // check row
        {
            int tcol = y * 60 + x * 6;
            {

                if ((boardcolours[tcol].x == boardcolours[tcol+6].x) &&     // since (boardcolours[tcol] == boardcolours[tcol+6]
                    (boardcolours[tcol].y == boardcolours[tcol+6].y) &&     // does not seem to work
                    (boardcolours[tcol].z == boardcolours[tcol+6].z) &&
                     board[x][y] && board[x+1][y]  &&
                    (boardcolours[tcol].x == boardcolours[tcol+12].x) &&
                    (boardcolours[tcol].y == boardcolours[tcol+12].y) &&
                    (boardcolours[tcol].z == boardcolours[tcol+12].z) &&
                     board[x][y] && board[x+2][y])   // row has three of same colour
                { // if begins

                    for (int yy=y ; yy<19 ; yy++)
                    {
                        int tcoll = yy * 60 + x * 6;
                        board[x][yy] = board[x][yy+1];                           // update board occupancy
                        for (int c=0 ; c<6 ; c++)                                // update colours
                            boardcolours[tcoll+c] = boardcolours[tcoll+60+c];
                        board[x+1][yy] = board[x+1][yy+1];
                        for (int c=0 ; c<6 ; c++)
                            boardcolours[tcoll+c+6] = boardcolours[tcoll+66+c];
                        board[x+2][yy] = board[x+2][yy+1];
                        for (int c=0 ; c<6 ; c++)
                            boardcolours[tcoll+c+12] = boardcolours[tcoll+72+c];
\
                    }

                    deleted++;    // level up
                    if (deleted == 10)
                    {
                        level++;
                        timer = 750/level;
                        cout<<"Leveled Up! Now level "<<level<<"\n";
                        deleted = 0;
                    }

                }  // end if
            }
        } 
    }

    for (int y=0 ; y<18 ; y++)
    {

        for (int x=0 ; x<10 ; x++)   // check column
        {
            int tcol = y * 60 + x * 6;

            if ((boardcolours[tcol].x == boardcolours[tcol+60].x) &&     // since (boardcolours[tcol] == boardcolours[tcol+6]
                (boardcolours[tcol].y == boardcolours[tcol+60].y) &&     // does not seem to work
                (boardcolours[tcol].z == boardcolours[tcol+60].z) &&     // column has three of same colour
                 board[x][y] && board[x][y+1]  &&
                (boardcolours[tcol].x == boardcolours[tcol+120].x) &&
                (boardcolours[tcol].y == boardcolours[tcol+120].y) &&
                (boardcolours[tcol].z == boardcolours[tcol+120].z) &&
                 board[x][y] && board[x][y+2])
            {  // if begins

                for (int yy=y ; yy<16 ; yy++)                 //**FIX THIS BLOODY THING  -> RESET TOP
                {
                    if (yy<16)
                    {
                    int tcoll = yy * 60 + x * 6;
                    board[x][yy] = board[x][yy+3];                           // update board occupancy
                    for (int c=0 ; c<6 ; c++)                                // update colours
                        boardcolours[tcoll+c] = boardcolours[tcoll+180+c];
                    }
                    else
                    {
                        board[x][yy] = false;
                        boardcolours[yy*60+x+6] = black;
                    }
                }


                deleted++;   // level up
                if (deleted == 10)
                {
                    level++;
                    timer = 750/level;
                    cout<<"Leveled Up! Now level "<<level<<"\n";
                    deleted = 0;
                }

            }  // end if

        }
    }
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
    bool set = false;

    // check to see if tile needs to set
    for (int i=0 ; i<4 ; i++)
    {

        int xpos = tilepos.x + tile[i].x;
        int ypos = tilepos.y + tile[i].y;

        if (tilepos.y + tile[i].y == 0)  // bottom line
            set = true;

        if (board[xpos][ypos] == true) { // square is occupied
            tilepos.y++;
            set = true;  }
    }


    if (set) {
        for (int i = 0 ; i<4 ; i++)
        {
            int xpos = tilepos.x + tile[i].x;
            int ypos = tilepos.y + tile[i].y;
            int pos = ypos * 60 + xpos * 6;
						
            board[xpos][ypos] = true;
            for (int j = pos ; j<pos+6 ; j++)
                boardcolours[j] = tileColours[i];

        }

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
        glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vColor);

       /* int low = tilepos.y;
        for (int i = 0 ; i<4 ; i++) {
            if (tilepos.y + tile[i].y < low)
                low = tilepos.y + tile[i].y;
        } */
		fallTile();
        three();
        checkfullrow();
        //checkGameOver();
        if (!gameOver)
            newtile();

    }
}



//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
    tilepos = tilepos + direction;
    bool outOfBounds = false;

    // check bounds
    for (int i=0 ; i<4 ; i++) {
        int xpos = tilepos.x + tile[i].x;
        int ypos = tilepos.y + tile[i].y;
        if (tilepos.x + tile[i].x < 0 || tilepos.x + tile[i].x > 9)  // if walls exist
            outOfBounds = true;
        if (board[xpos][ypos]) // if square already occupied
            outOfBounds = true;
    }

    if (outOfBounds == true) {// undo if out of bounds
        tilepos = tilepos - direction;
        return false; }

    else {
        updatetile();
        settile();
        return true; }
}

//-------------------------------------------------------------------------------------------------------------------

// Makes the current tile fall every [timer] seconds
void fall(int i)
{
    if (!gameOver) {
        glutTimerFunc(timer, fall, 1);
        tilepos = tilepos + vec2(0, -1);
        updatetile();
        settile();  }

    if (tilepos.y < -1)
        tilepos.y = 0;

}


//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
    /*glutTimerFunc(timer, fall, 1);*/
    timer = 750;
    gameOver = false;
    level = 1;
    deleted = 0;
    init();
}

//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
    glUniform1i(locysize, ysize);

    glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
    glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

    glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
    glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

    glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
    glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)

    glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
    xsize = w;
    ysize = h;
    glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
    vec2 left = vec2(-1, 0);
    vec2 right = vec2(1, 0);
    vec2 down = vec2(0, -1);

    switch(key)
    {
        case 100: //left arrow
            movetile(left);
            break;
        case 101: //up arrow -> rotate counterclockwise
            rotate();
            break;
        case 102: //right arrow
            movetile(right);
            break;
        case 103: //down arrow
            movetile(down);
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 033: // Both escape key and 'q' cause the game to exit
            exit(EXIT_SUCCESS);
            break;
        case 'q':
            exit (EXIT_SUCCESS);
            break;
        case 'r': // 'r' key restarts the game
            restart();
            break;
        case ' ': // space bar -> shuffles tile order
            shuffleTile();
            break;
    }

    glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
    if (!gameOver)
        glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(xsize, ysize);
    glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
    glutCreateWindow("Fruit Tetris");
    glewInit();
    init();

    // Menu functions
  /*  glutCreateMenu(menu)
    glutAddMenuEntry("Clear Screen", 1);
    glutAddMenuEntry("Delete Row", 2);
    glutAddMenuEntry("Don't Delete Row", 3);
    glutAddMenuEntry("Level 1", 4);*/

    // Callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutTimerFunc(timer, fall, 1);

    glutMainLoop(); // Start main loop
    return 0;
}
