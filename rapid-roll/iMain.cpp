// Rapid Roll version 1.9

# include "iGraphics.h"
# include <stdlib.h>
# include <math.h>

// misc. game defines
#define screenWidth 500
#define screenHeight 500
#define platformNumber 5
#define radiusOfBall 10
#define platformWidth 80
#define platformHeight 5
#define randMod ( screenWidth - platformWidth ) // max x val for platform initialization
#define leeway 3 // amount of approximation in calculating if ball is on a platform ( one pixel is small enough , no need for float values )
#define highScoreNo 3

// ball properties
double x = 250 , y = 250 ;
double ballSpeedX = 0 ;
double ballSpeedY ;

//platform properties
double platformSpeed = 0.03 ; // inital speed of platforms, it'll increase in-game
double platX[ platformNumber ] ;
double platY[ platformNumber ] ;
int isPlatRed[ platformNumber ] ; // 1 if red , 0 if blue

//other variables
int lives ;
double score ;
int screenNo ;
int menuScreen = 0 ; // for menu
int gameScreen = 1 ; // in-game where the ball and platforms are rising
int gameOverScreen = 2 ; // type name of player when game ends
int highScoreScreen = 3 ; // show current high scores
int controlsScreen = 4 ; // show controls

// highscore res
char names[ highScoreNo ][ 80 ] ;
int sortedScores[ highScoreNo ] ;
FILE * fp ;
char nameInputBuffer[ 80 ] ;

/*
	loads names and scores for all high scores saved in file
	to memory
 */
void loadHighScores()
{
	fp = fopen( "highscores.txt" , "r" ) ;

	for ( int i = 0 ; i < highScoreNo ; i++ ) {
		fscanf( fp , "%s" , names[ i ] ) ; // input name in one line
		fscanf( fp , "%d\n" , &sortedScores[ i ] ) ; // input score in one line
	}

	fclose( fp ) ;
}

/*
	saves names and scores for all high scores saved in file
	from memory
 */
void saveHighScores()
{
	fp = fopen( "highscores.txt" , "w" ) ;

	for ( int i = 0 ; i < highScoreNo ; i++ ) {
		fprintf( fp , "%s\n" , names[ i ] ) ; // output name in one line
		fprintf( fp , "%d\n" , sortedScores[ i ] ) ; // output score in one line
	}

	fclose( fp ) ;
}

/*
	checks if made a high score
	updates high score if so
*/
void checkScore()
{
	int currentScore = (int)score ; // int from double

	// if made a high score
	if ( currentScore > sortedScores[ highScoreNo - 1 ] ) {

		// insert new score
		sortedScores[ highScoreNo - 1 ] = currentScore ;
		strcpy( names[ highScoreNo - 1 ] , nameInputBuffer ) ;

		int idx = highScoreNo - 1 ;
		while ( idx != 0 && sortedScores[ idx ] > sortedScores[ idx - 1 ] ) {
			idx-- ;

			// swap name and score values
			char temp[ 80 ] ;
			int tempInt ;

			strcpy( temp , names[ idx ] ) ;
			tempInt = sortedScores[ idx ] ;

			sortedScores[ idx ] = sortedScores[ idx + 1 ] ;
			strcpy( names[ idx ] , names[ idx + 1 ] ) ;

			sortedScores[ idx + 1 ] = tempInt ;
			strcpy( names[ idx + 1 ] , temp ) ;
		}
	}
}

/*
	resets all values as required to start the game
*/
void newGameInit()
{
	// initialize game properties
	score = 0 ;
	lives = 5 ;

	// initializing ball co-ords
	x = screenWidth / 2 , y = screenHeight / 2 ;

	// initializing platform co-ords
	platX[ 0 ] = screenWidth / 2 - platformWidth / 2 ;//210 ;
	platY[ 0 ] = screenHeight / 2 - radiusOfBall - platformHeight ;//235 ;
	isPlatRed[ 0 ] = 0 ; // first platform is blue
	for ( int i = 1 ; i < platformNumber ; i++ ) {
		platX[ i ] = rand() % randMod ;
		platY[ i ] = platY[ i - 1 ] - screenHeight / platformNumber ;
		isPlatRed[ i ] = 0 ; // all platforms are blue at first
	}
}

/*
	function makePlatform() draws a platform on the screen
	( n ) denotes the index of the current platform being created
*/
void makePlatform ( int n )
{
	// draw the platform
	if ( isPlatRed[ n ] == 0 ) iSetColor( 0 , 0 , 250 ) ; // blue
	else iSetColor( 255 , 0 , 0 ) ; // red
	iFilledRectangle( platX[ n ] , platY[ n ] , platformWidth , platformHeight ) ;

	// go up
	platY[ n ] = platY[ n ] + platformSpeed ;

	// make new platform
	if ( platY[ n ] > screenHeight ) {
		if ( rand() % 10 == 0 ) isPlatRed[ n ] = 1 ; // one out of 3 platforms is likely to be red
		else isPlatRed[ n ] = 0 ;
		platX[ n ] = rand() % randMod ;
		platY[ n ] = -platformHeight ; // start below screen ( jeno dekha na jaay )
	}

	// reset color
	iSetColor( 255 , 255 , 255 ) ;
}

/*
	when the ball is not on top of any platform,
	makes the ball fall from one platform to another ( or to the floor )
*/
void fall( void )
{
	y = y - ballSpeedY ;
	ballSpeedY = ballSpeedY + 0.0003 ;

	// score depends on how far ball falls
	score = score + ballSpeedY / 10 ; // div by 10 as score shouldn't increase too much
}

/*
	check to see if the ball is on a platform at the moment
	returns index of platform if it is, -1 otherwise
*/
int isOnPlatform ( void )
{
	for ( int i = 0 ; i < platformNumber ; i++ ) {

		// projected value of y with respect to platform i
		double yShouldBe = platY[ i ] + platformHeight + radiusOfBall ;

		if ( x >= platX[ i ] - radiusOfBall && x <= platX[ i ] + platformWidth + radiusOfBall 
			&& y >= yShouldBe - leeway && y <= yShouldBe + leeway ) {
			y = yShouldBe ;
			return i ;
		}
	}

	return -1 ;
}

/*
	change the speed of
	platforms with change
	of time.
	call from either iDraw
	or iSetTimer
*/
void changeSpeed( void )
{
	platformSpeed = platformSpeed + 0.000001 ; // fast || // 0.0000005 ;// medium || //0.00000005 ; // slowly increasing speed
}

/*
	if ball goes out of bounds
*/
int ballDead()
{
	return y <= 0 || y >= screenHeight ;
}

/*
	make new ball
*/
void reSpawn()
{
	int mid = screenHeight / 2 ; // middle y co-ords of screen
	int idx = 0 , difference = 9000 + 1 ; // idx is the index of platform nearest to mid

	// find nearest
	for ( int i = 0 ; i < platformNumber ; i++ ) {
		int thisDifference = abs( platY[ i ] + platformHeight - mid ) ;
		if ( thisDifference < difference && isPlatRed[ i ] != 1 ) {
			idx = i ;
			difference = thisDifference ;
		}
	}

	// assign ball co-ords on top of nearest platform ( nearest to mid )
	y = radiusOfBall + platformHeight + platY[ idx ] ;
	x = platformWidth / 2 + platX[ idx ] ;
}

void draw_ball( void )
{
	iFilledCircle( x , y , radiusOfBall ) ; // draws the ball

	// checks boundary of the screen ( doesn't let ball go out of screen left or right )
	if ( x + ballSpeedX - radiusOfBall >= 0 && x + ballSpeedX + radiusOfBall <= screenWidth ) {
		x = x + ballSpeedX ;
	}

	// grips ball speed ( ball decelerates with time )
	if ( ballSpeedX > 0 ) ballSpeedX = ballSpeedX - 0.002 ;
	else ballSpeedX = ballSpeedX + 0.002 ;

	// moves the ball as required
	int onPlat = isOnPlatform() ;
	if ( onPlat != -1 ) {
		y = y + platformSpeed ; // upore uthbe
		ballSpeedY = 0;        // speed reset
	}
	else {
		fall() ;
	}

	// kills the ball
	if ( ballDead() || onPlat != -1 && isPlatRed[ onPlat ] ) {
		lives-- ;
		if ( lives != 0 ) {
			reSpawn() ;
		}
		else {
			screenNo = gameOverScreen ;
			//exit( 0 ) ;
		}
	}
}

void showScore()
{
	iSetColor( 25 , 25 , 25 ) ;
	iFilledRectangle( 500 , 0 , 400 , 500 ) ;
	iSetColor( 255 , 255 , 255 ) ; // reset color

	char livesString[ 80 ] ;
	char scoreString[ 80 ] ;

	// int to string
	sprintf( livesString , "%d" , lives ) ;
	sprintf( scoreString , "%d" , ( int )score ) ;

	// draw string on screen
	iText ( 500 + 100 , 450 , "LIVES" ) ;
	iText ( 500 + 100 , 400 , livesString ) ;
	iText ( 500 + 300 , 450 , "SCORE" ) ;
	iText ( 500 + 300 , 400 , scoreString ) ;
}

void drawGame( )
{
	//iShowBMP( 0 , 0 , "sky1.bmp" ) ;

	draw_ball() ;

	// draws platforms
	for ( int i = 0 ; i < platformNumber ; i++ ) {
		makePlatform( i ) ;
	}

	// increase speed of game ( i.e. speed of platforms )
	changeSpeed() ;
}

void drawHighScore()
{
	//iShowBMP( 0 , 350 , "highscore.bmp" ) ;
	iText( 200 , 350 , "High Scores" , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
	iText( 52 , screenHeight - 270 , " Rank                 Name                  Score" ) ;
	for ( int i = 0 ; i < highScoreNo ; i++ ) {
		char rankString[ 80 ] , scoreString[ 80 ] ;
		sprintf( rankString , "%d" , ( i + 1 ) ) ;
		iText( 65 , screenHeight - 300 - i * 30 , rankString ) ;
		iText( 220 , screenHeight - 300 - i * 30 , names[ i ] ) ;
		sprintf( scoreString , "%d" , sortedScores[ i ] ) ;
		iText( 420 , screenHeight - 300 - i * 30 , scoreString ) ;
	}
}

void drawGameOver()
{
	//iShowBMP() ; // gameOver
	iText( 50 , 300 , "Enter your Name:" ) ;
	iRectangle( 200 , 280 , 200 , 50 ) ;
	//strcpy( nameInputBuffer , "text" ) ;
	iText( 220 , 300 , nameInputBuffer ) ;
}

void drawMenu()
{
	iSetColor( 25 , 25 , 25 ) ;
	iFilledRectangle( 500 , 0 , 400 , 500 ) ;
	iSetColor( 255 , 255 , 255 ) ; // reset color

	iText( 640 , 400 , "Start Game" , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
	iText( 650 , 300 , "Controls" , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
	iText( 635 , 200 , "High Scores" , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
	iText( 670 , 100 , "Exit" , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
}

/* 
	function iDraw() is called again and again by the system.
*/
void iDraw()
{
	iClear();

	// right half of screen
	if ( screenNo == gameScreen || screenNo == gameOverScreen ) {
		showScore() ;
	}
	else {
		drawMenu() ;
	}

	// left half of screen
	if ( screenNo == menuScreen ) {
		iShowBMP( 0 , 0 , "mainman.bmp" ) ;
	}
	else if ( screenNo == gameScreen ) {
		drawGame() ;
	}
	else if ( screenNo == controlsScreen ) {
		iShowBMP( 0 , 0 , "controls.bmp" ) ;
	}
	else if ( screenNo == highScoreScreen ) {
		drawHighScore() ;
	}
	else if ( screenNo == gameOverScreen ) {
		drawGameOver() ;
	}

	// screen seperator
	iLine( screenWidth , 0 , screenWidth , screenHeight ) ;
}

/* 
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
*/
void iMouseMove(int mx, int my)
{
	//place your codes here
}

/* 
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// changing screens: effecting what iDraw shows
		// when we can see the menu
		if ( screenNo != gameScreen && screenNo != gameOverScreen ) {
			if ( mx >= 640 && mx <= 755 && my >= screenHeight - 105 && my <= screenHeight - 80 ) {
				newGameInit() ;
				screenNo = gameScreen ;
			}
			else if ( mx >= 645 && mx <= 740 && my >= screenHeight - 200 && my <= screenHeight - 180 ) {
				screenNo = controlsScreen ;
			}
			else if ( mx >= 635 && mx <= 760 && my >= screenHeight - 300 && my <= screenHeight - 280 ) {
				screenNo = highScoreScreen ;
			}
			else if ( mx >= 670 && mx <= 715 && my >= screenHeight - 405 && my <= screenHeight - 380 ) {
				exit( 0 ) ;
			}
			else {
				screenNo = menuScreen ;
			}
		}
	}
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		//place your codes here	
	}
}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed. 
*/
void iKeyboard(unsigned char key)
{
	if ( screenNo == gameOverScreen ) {

		int len = strlen( nameInputBuffer ) ;

		if ( key >= 'a' && key <= 'z' || key >= 'A' && key <= 'Z' || key >= '0' && key <= '9' ) {
			nameInputBuffer[ len ] = key ;
		}
		else if ( key == '\b' && len != 0 ) {
			nameInputBuffer[ len - 1 ] = '\0' ;
		}
		else if ( key == '\n' || key == '\r' ) {
			loadHighScores() ;
			checkScore() ; // changes highscores if necessary
			saveHighScores() ;
			for ( int i = 0 ; i < 80 ; i++ ) nameInputBuffer[ i ] = '\0' ; // reset name input
			screenNo = highScoreScreen ;
		}
	}
	if(key == 'q')
	{
		//do something with 'q'
	}

	//place your codes for other keys here
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use 
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6, 
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12, 
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP, 
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT 
*/
void iSpecialKeyboard(unsigned char key)
{

	if(key == GLUT_KEY_END)
	{
		exit(0);	
	}
	if ( key == GLUT_KEY_LEFT ) {
		//x = x - ballSpeedX ; // move ball left
		ballSpeedX = -0.4 ;
	}
	if ( key == GLUT_KEY_RIGHT ) {
		//x = x + ballSpeedX ; // move ball right
		ballSpeedX = 0.4 ;
	}
	//place your codes for other keys here
}

/*
	initializing seed for generating
	random numbers.
	As in Teach Yourself C
	by Herbert Schild
	pg. 455
*/
void initializeRandomNumbers( void )
{
	int utime , ltime ;
	ltime = time( NULL ) ;
	utime = ( unsigned int ) ltime / 2 ;
	srand( utime ) ;
}

int main()
{
	initializeRandomNumbers() ;
	screenNo = menuScreen ;
	loadHighScores() ;

	// 400 is for showing score and lives
	iInitialize( screenWidth + 400 , screenHeight , "Rapid Roll") ;
	return 0;
}
