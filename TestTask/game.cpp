#define _WIN32_WINNT 0x0502
#include <Windows.h>
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include "Framework.h"

#define N_TILES 40


class Player;
class Ball;
class Tile;
class Arcanoid;



class Player {
public:
	Player() = default;
	Player(int w, int h) : width(w), height(h), speed(6) {}
	

	int x, y;
	int width, height;
	int speed;
	Sprite* sprite;
};



class Turret : public Player {
public:
	Turret() = default;
	Turret(int w, int h) : Player(w, h) {}
	
	void moveTick() {
		if (x + borderValue >= scrw) {
			borderValue = scrw - 100;
			touched = true;
		}
		else {
			borderValue = 100;
			touched = false;
		}

		if (!touched) {
			if (x > randB && x % randB == 0) {
				prevX = x;
				checked = false;
			}

			if (!checked && (prevX > randB && prevX % randB == 0)) {// x % (100 - (rand() % 50 - 25)) == 0
				if (i < randBack) {
					x -= speed;
					i++;
					return;
				}
				else {
					randBack = rand() % 25 + 50;
					checked = true;
					i = 0;
				}
			}
			else {
				randB = (100 - (rand() % 50 - 25));
			}
			x += speed;
		}
		else {
			if (x < randB && x % randB == 0) {
				prevX = x;
				checked = false;
			}

			if (!checked && (prevX < randB && prevX % randB == 0)) {// x % (100 - (rand() % 50 - 25)) == 0
				if (i < randBack) {
					x += speed;
					i++;
					return;
				}
				else {
					randBack = rand() % 25 + 50;
					checked = true;
					i = 0;
				}
			}
			else {
				randB = (100 - (rand() % 50 - 25));
			}
			x -= speed;
		}
	}

	int borderValue = 100;
	int prevX;
	int randB = 100;
	int randBack = 80;
	int i = 0;
	int scrw;
	bool touched = false;
	bool checked = false;
};



class TurretBall {
public:
	TurretBall() = default;

	void calcMove(Player *player) {
		float vecLen = sqrt(pow((x - player->x + player->width / 2), 2) + pow((y - player->y - player->height / 2), 2));
		moveDeltaX = (x - player->x - player->width  / 2) / vecLen;
		moveDeltaY = (y - player->y - player->height / 2) / vecLen;
	}

	void moveTick() {
		y -= moveDeltaY * speed;
		x -= moveDeltaX * speed;
	}
	
	bool isDestr(int scrh) {
		if (y >= scrh) {
			return true;
		}
		else {
			return false;
		}
	}

	bool isColl(Player* play) {
		if (x > play->x && x < play->x + play->width &&
			y > play->y && y < play->y + play->height) {//top
			return true;
		}
		else if (y >= play->y && y         <= play->y + play->height && 
				(x < play->x && x + width > play->x)) {//left
			return true;
		}
		else if (y >= play->y && y		   <= play->y + play->height &&
				(x <  play->x + play->width && x + width > play->x + play->width)) {//right
			return true;
		}
		return false;
	}


	int speed{1};
	float x, y;
	float moveDeltaX, moveDeltaY;
	int width;
	Sprite* sprite;
};



class Tile {
public:
	Tile() = default;
	void calcPoints() {
		leftBotX = leftTopX;
		righTopX = leftTopX + width;
		righBotX = righTopX;
		righTopY = leftTopY;
		leftBotY = leftTopY + height;
		righBotY = leftBotY;
	}

	bool destr = false;
	int leftTopX, leftTopY;//given
	int leftBotX, leftBotY;
	int righTopX, righTopY;
	int righBotX, righBotY;
	int width, height;
	int hp;
	Sprite* tile;
};



class Ball {
public:
	Ball() = default;

	void calcMoveDelta() {
		//
		//
		float vecLen = sqrt(pow((prevColX - nextColX), 2) + pow((prevColY - nextColY), 2));
		moveTickX = (prevColX - nextColX) / vecLen;
		moveTickY = (prevColY - nextColY) / vecLen;
	}

	bool isCollision(Tile* tiles, const Player* play, Ball* ball, int nTiles, int scrw, int scrh, bool& horizontal, bool& endgame, bool& wascold) {
		range = (this->width / 2); //in range X pxs
		float range025 = ball->speed / 1.5;
		float range2 = range * range;
		float Ox = curX + (this->width / 2),
			Oy = curY + (this->width / 2);

		float xEngeMax = (curX)+this->width,
			yEngeMax = (curY)+this->width;
		float xEdge = curX,//start pos
			yEdge = curY;

		float precis = this->width * 0.00425; //0.05 = x * 16
		if (this->width > 27) {
			precis = 0.18;
		}
		else if (this->width < 10) {
			precis = 0.042;
		}
		if (ball->width > 20 && ball->width < 21) {
			precis = 0.16;// must be 0.16
		}

		//calculate all the ball's external points
		for (int i = 0; i < range + 1; i++) {//xEdge   
			yEdge = curY;
			for (int j = 0; j < range + 1; j++) {//yEdge
				if (range2 + precis >= (pow(xEdge - Ox, 2) + pow(yEdge - Oy, 2)) &&
					range2 - precis <= (pow(xEdge - Ox, 2) + pow(yEdge - Oy, 2)) ) {//a point of ball's arc
					float VertOyEdge = abs(Oy - yEdge), // dist from yEdge to Oy
						VertOxEdge = abs(Ox - xEdge);

					float MirrXedge, MirrYedge;
					if(xEdge - Ox > 0){
						MirrXedge = Ox + (xEdge - Ox);
					}
					else {
						MirrXedge = Ox - (xEdge - Ox);
					}
					if (yEdge - Oy > 0) {
						MirrYedge = Oy + (yEdge - Oy);
					}
					else {
						MirrYedge = Oy - (yEdge - Oy);
					}
					

					if (wascold) {
						if (MirrXedge <= ball->prevColX && ball->prevColX <= xEdge && //it must be in the function
							MirrYedge <= ball->prevColY && ball->prevColY <= yEdge) {
							wascold = true;
							return false;
						}
						else {
							wascold = false;
						}
					}

					//checks for collision with scr borders
					if (yEdge <= 0 || ball->curY <= 0/*&& (0 < xEdge && xEdge < scrw)*/ && 
						((ball->nextColY - ball->prevColY) < 0)) {//top screen border
						 
						horizontal = true;
						return true;
					}
					else if (xEdge <= 0 || ball->curX <= 0 /* && (0 < yEdge && yEdge < scrh)*/ &&
						((ball->nextColX - ball->prevColX) < 0)) {//left
						 
						horizontal = false;
						return true;
					}
					else if (yEdge >= scrh || ball->curY + ball->width >= scrh/*&&(0 < xEdge && xEdge < scrw)*/) {//bottom
						horizontal = true;
						endgame = true;
						return true;
					}
					else if (xEdge >= scrw || ball->curX + ball->width >= scrw/*&& (0 < yEdge < scrh)*/ &&
						((ball->nextColX - ball->prevColX) > 0)) {//right
						 
						horizontal = false;
						return true;
					}



					//checks for collisions with TILES
					for (int t = 0; t < nTiles; t++) {
						if ((tiles[t].hp != 0 &&
							tiles[t].leftBotX < xEdge && //bot
							xEdge < tiles[t].righBotX) &&
							((yEdge - range025) <= tiles[t].leftBotY) && // yEdge in ball's plane 
							((yEdge + range025) >= tiles[t].leftBotY) && //BOT
							((ball->nextColY - ball->prevColY) < 0)) { // because when yEdge is a point it may jump over it and dont consider
							 
							horizontal = true;
							if (tiles[t].hp == 0 && !tiles[t].destr) {
								ball->score += 10;
								tiles[t].destr = true;
								return false;
							}
							else if (tiles[t].hp > 0) {
								tiles[t].hp--;
								if (tiles[t].hp == 0) {
									if (tiles[t].hp == 0 && !tiles[t].destr) {
										ball->score += 10;
									}
									tiles[t].destr = true;
									return true;
								}
							}
							return true;
						}
						else if (tiles[t].hp != 0 &&
							tiles[t].leftTopX < xEdge &&
							xEdge < tiles[t].righTopX &&
							(yEdge - range025) <= tiles[t].leftTopY && // top
							(yEdge + range025) >= tiles[t].leftTopY && // top
							((ball->nextColY - ball->prevColY) > 0)) {//top
							 
							horizontal = true;
							if (tiles[t].hp == 0 && !tiles[t].destr) {
								ball->score += 10;
								tiles[t].destr = true;
								return false;
							}
							else if (tiles[t].hp > 0) {
								tiles[t].hp--;
								if (tiles[t].hp == 0) {
									if (tiles[t].hp == 0 && !tiles[t].destr) {
										ball->score += 10;
									}
									tiles[t].destr = true;
									return true;
								}
							}
							return true;
						}
						else if ((tiles[t].hp != 0 &&
							tiles[t].leftBotY > yEdge &&
							yEdge > tiles[t].leftTopY) &&
							((xEdge - range025) <= tiles[t].leftBotX) &&//left
							((xEdge + range025) >= tiles[t].leftBotX) &&//left
							((ball->nextColX - ball->prevColX) > 0)) {//left
							 
							horizontal = false;
							if (tiles[t].hp == 0 && !tiles[t].destr) {
								ball->score += 10;
								tiles[t].destr = true;
								return false;
							}
							else if (tiles[t].hp > 0) {
								tiles[t].hp--;
								if (tiles[t].hp == 0) {
									if (tiles[t].hp == 0 && !tiles[t].destr) {
										ball->score += 10;
									}
									tiles[t].destr = true;
									return true;
								}
							}
							return true;
						}
						else if ((tiles[t].hp != 0 &&
							tiles[t].leftBotY > yEdge &&
							yEdge > tiles[t].leftTopY) &&
							((xEdge - range025) <= tiles[t].righBotX) &&
							((xEdge + range025) >= tiles[t].righBotX) &&
							((ball->nextColX - ball->prevColX) < 0)) {//right
							 
							horizontal = false;
							if (tiles[t].hp == 0 && !tiles[t].destr) {
								ball->score += 10;
								tiles[t].destr = true;
								return false;
							}
							else if (tiles[t].hp > 0) {
								tiles[t].hp--;
								if (tiles[t].hp == 0) {
									if (tiles[t].hp == 0 && !tiles[t].destr) {
										ball->score += 10;
									}
									tiles[t].destr = true;
									return true;
								}
							}
							return true;
						}
					}

					//checks for collisions with the player
					if (play->x < xEdge &&
						xEdge < (play->x + play->width) &&
						(yEdge - range025) <= play->y &&
						(yEdge + range025) >= play->y) {//top
						horizontal = true;
						for (int i = 0; i < N_TILES; i++) {
							if (tiles[i].destr == true) {
								endgame = true;
							}
							else {
								endgame = false;
								break;
							}
						}
						return true;
					}
					else if (play->y < yEdge &&
						yEdge < play->y + play->height &&
						((xEdge - range025) <= play->x) &&//left
						((xEdge + range025) >= play->x)) {
						horizontal = false;
						for (int i = 0; i < N_TILES; i++) {
							if (tiles[i].destr == true) {
								endgame = true;
							}
							else {
								endgame = false;
								break;
							}
						}
						return true;
					}
					else if (play->y < yEdge &&
						yEdge < play->y + play->height &&
						((xEdge - range025) <= play->x + play->width) &&
						((xEdge + range025) >= play->x + play->width)) {//right
						horizontal = false;
						for (int i = 0; i < N_TILES; i++) {
							if (tiles[i].destr == true) {
								endgame = true;
							}
							else {
								endgame = false;
								break;
							}
						}
						return true;
					}
				}

				yEdge++;
			}

			xEdge++;
		}
		return false;
	}

	void moveTick() {
		curX -= moveTickX * speed;
		curY -= moveTickY * speed;
	}

	void addAbility(int& score) {
		if (score >= 20) {
			if ((rand() % 2) == 1) {//buffs
				for (int i = 0; i < 3; i++) {
					if (!Buffs[i]) {
						Buffs[i] = true;
						score -= 20;
						timeBuff[i] = 20;
						if (perc == 100) {// 50 90 130 170 160 120 80 140 180 60 100
							this->width += this->initw * 0.4;
							perc = 140;
						}
						else if (perc == 50) {
							this->width += this->initw * 0.4;
							perc = 90;
						}
						else if (perc == 140) {
							this->width += this->initw * 0.4;
							perc = 180;
						}
						else if (perc == 180) {
							this->width += this->initw * 0.2;
							perc = 200;
						}
						else if (perc == 60) {
							this->width += this->initw * 0.4;
							perc = 100;
						}
						else if (perc == 90) {
							this->width += this->initw * 0.4;
							perc = 130;
						}
						else if (perc == 130) {
							this->width += this->initw * 0.4;
							perc = 170;
						}
						else if (perc == 160) {
							this->width += this->initw * 0.4;
							perc = 200;
						}
						else if (perc == 120) {
							this->width += this->initw * 0.4;
							perc = 160;
						}
						else if (perc == 80) {
							this->width += this->initw * 0.3;
							perc = 120;
						}
						else if (perc == 170) {
							this->width += this->initw * 0.3;
							perc = 200;
						}
						break;
					}
				}
			}
			else {
				for (int i = 0; i < 2; i++) {
					if (!Debuffs[i]) {
						Debuffs[i] = true;
						score -= 20;
						timeDebuff[i] = 20; // CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE
						if (perc == 100) {// 50 90 130 170 160 120 80 140 180 60 100
							this->width -= this->initw * 0.4;
							perc = 60;
						}
						else if (perc == 200) {
							this->width -= this->initw * 0.4;
							perc = 160;
						}
						else if (perc == 140) {
							this->width -= this->initw * 0.4;
							perc = 100;
						}
						else if (perc == 180) {
							this->width -= this->initw * 0.4;
							perc = 140;
						}
						else if (perc == 60) {
							this->width -= this->initw * 0.1;
							perc = 50;
						}
						else if (perc == 90) {
							this->width -= this->initw * 0.4;
							perc = 50;
						}
						else if (perc == 130) {
							this->width -= this->initw * 0.4;
							perc = 90;
						}
						else if (perc == 160) {
							this->width -= this->initw * 0.4;
							perc = 120;
						}
						else if (perc == 120) {
							this->width -= this->initw * 0.4;
							perc = 80;
						}
						else if (perc == 80) {
							this->width -= this->initw * 0.3;
							perc = 50;
						}
						else if (perc == 170) {
							this->width -= this->initw * 0.4;
							perc = 130;
						}
						break;
					}
				}
			}
		}
	}

	bool checkAbility(time_t compt, time_t currt){
		if (!Buffs[0] && !Buffs[1] && !Buffs[2] && !Debuffs[0] && !Debuffs[1]) {
			this->width = this->initw;
		}

		if (compt - currt > 0) {//if one second have passed
			for (int i = 0; i < 3; i++) {
				if (Buffs[i]) {
					if (timeBuff[i] > 0) timeBuff[i]--;
					if (timeBuff[i] == 0) {
						Buffs[i] = false;

						if (perc == 100) {// 50 90 130 170 160 120 80 140 180 60 100
							this->width -= this->initw * 0.4;
							perc = 60;
						}
						else if (perc == 200) {
							this->width -= this->initw * 0.4;
							perc = 160;
						}
						else if (perc == 140) {
							this->width -= this->initw * 0.4;
							perc = 100;
						}
						else if (perc == 180) {
							this->width -= this->initw * 0.4;
							perc = 140;
						}
						else if (perc == 60) {
							this->width -= this->initw * 0.1;
							perc = 50;
						}
						else if (perc == 90) {
							this->width -= this->initw * 0.4;
							perc = 50;
						}
						else if (perc == 130) {
							this->width -= this->initw * 0.4;
							perc = 90;
						}
						else if (perc == 160) {
							this->width -= this->initw * 0.4;
							perc = 120;
						}
						else if (perc == 120) {
							this->width -= this->initw * 0.4;
							perc = 80;
						}
						else if (perc == 80) {
							this->width -= this->initw * 0.3;
							perc = 50;
						}
						else if (perc == 170) {
							this->width -= this->initw * 0.4;
							perc = 130;
						}
						break;
					}
				}
			}
			for (int i = 0; i < 2; i++) {
				if (Debuffs[i]) {
					if (timeDebuff[i] > 0) timeDebuff[i]--;
					if (timeDebuff[i] == 0) {
						Debuffs[i] = false;
						if (perc == 100) {// 50 90 130 170 160 120 80 140 180 60 100
							this->width += this->initw * 0.4;
							perc = 140;
						}
						else if (perc == 50) {
							if (Debuffs[0] && Debuffs[1] && !Buffs[0] && !Buffs[1] && !Buffs[2]) {
								this->width += this->initw * 0.1;
								perc = 60;
							}
							this->width += this->initw * 0.4;
							perc = 90;
						}
						else if (perc == 140) {
							this->width += this->initw * 0.4;
							perc = 180;
						}
						else if (perc == 180) {
							this->width += this->initw * 0.2;
							perc = 200;
						}
						else if (perc == 60) {
							this->width += this->initw * 0.4;
							perc = 100;
						}
						else if (perc == 90) {
							this->width += this->initw * 0.4;
							perc = 130;
						}
						else if (perc == 130) {
							this->width += this->initw * 0.4;
							perc = 170;
						}
						else if (perc == 160) {
							this->width += this->initw * 0.4;
							perc = 200;
						}
						else if (perc == 120) {
							this->width += this->initw * 0.4;
							perc = 160;
						}
						else if (perc == 80) {
							this->width += this->initw * 0.4;
							perc = 120;
						}
						else if (perc == 170) {
							this->width += this->initw * 0.3;
							perc = 200;
						}
						else if (perc == 200) {
							this->width += this->initw * 0.0;
							perc = 200;
						}
						break;
					}
				}
			}
			return true;
		}

		if (timeBuff[0] == 0 && timeDebuff[0] == 0) {//no buffs
			this->width = this->initw;
		}
		return false;
	}
	
	
	int timeBuff[3];
	bool Buffs[3]{false,false,false};
	bool FiBuff = false;
	bool SeBuff = false;
	bool ThBuff = false;
	int timeDebuff[2];
	bool Debuffs[2]{ false,false};
	bool FiDebuff = false;
	bool SeDebuff = false;
	int prevColX{0}, prevColY{0};
	float curX, curY;
	int relX, relY;
	int nextColX, nextColY;
	float speed;
	float range;
	bool isCold;
	bool isOnceCold;

	Sprite* sprite;
	const int initw = 16;
	float width;
	int perc = 100;//100%
	int score;
private:
	float moveTickX, moveTickY;
};



class Arcanoid : public Framework {

public:
	Arcanoid() {
		this->width = 1200;
		this->height = 700;
	}
	Arcanoid(std::string scr) {
		
		this->screen = scr;
		int p = screen.find("x");
		if (p >= 0) {
			int w = std::stoi(screen.substr(0, p));
			int h = std::stoi(screen.substr(p + 1, screen.size() - 1));
			this->width = w;
			this->height = h;
		}
		else {
			this->width = 1200;
			this->height = 700;
		}
	}

	virtual void PreInit(int& width, int& height, bool& fullscreen)	{
		width = this->width;
		height = this->height;
		fullscreen = false;
	}

	virtual bool Init() {
		timeBeg = time(NULL);
		tickBeg = getTickCount();
		scrCoef = 1;
		onTickCol = -1;
		isHeldR = false;
		isHeldL = false;
		isHeldU = false;
		isHeldD = false;
		isHeldLMB = false;
		isHeldLMBonce = false;
		isHeldLMBsec = false;
		end = false;
		wasCold = false;


		HWND hWnd = GetForegroundWindow();
		RECT rectClient, rectWindow;
		GetClientRect(hWnd, &rectClient);
		GetWindowRect(hWnd, &rectWindow);
		int posx, posy;
		posx = GetSystemMetrics(SM_CXSCREEN) / 2 - (rectWindow.right - rectWindow.left) / 2;
		posy = GetSystemMetrics(SM_CYSCREEN) / 2 - (rectWindow.bottom - rectWindow.top) / 2;
		MoveWindow(hWnd, posx, posy, rectClient.right - rectClient.left, rectClient.bottom - rectClient.top, TRUE);


		player.sprite = createSprite(".\\data\\49-Breakout-Tiles.png");
		player.x = (width - 76 / 2) / 2;
		player.y = (height / 1.2);
		player.speed = 5;
		setSpriteSize(player.sprite, player.width, player.height);

		ball.sprite = createSprite(".\\data\\63-Breakout-Tiles.png");
		ball.curX = player.x + player.width / 2 - ball.width / 2;
		ball.curY = player.y - ball.width;
		ball.width = 16;//const
		ball.isCold = false;
		ball.isOnceCold = false;
		ball.speed = 4;
		ball.score = 0;
		ball.perc = 100;
		ball.Buffs[0] = false;
		ball.Buffs[1] = false;
		ball.Buffs[2] = false;
		ball.Debuffs[0] = false;
		ball.Debuffs[1] = false;
		ball.FiBuff = ball.SeBuff = ball.ThBuff = ball.FiDebuff = ball.SeDebuff = false;
		setSpriteSize(ball.sprite, ball.width, ball.width);

		turret = Turret(0, 20);
		turret.sprite = createSprite(".\\data\\61-Breakout-Tiles.png");
		turret.x = 2;
		turret.y = 20;
		turret.scrw = this->width;
		turret.speed = 2;
		setSpriteSize(turret.sprite, 20, 40);

		for (int i = 0; i < 2; i++) {
			tball[i] = TurretBall();
			tball[i].sprite = createSprite(".\\data\\59-Breakout-Tiles.png");
			tball[i].width = 20;
			tball[i].speed = ball.speed;
			setSpriteSize(tball[i].sprite, 20, 20);
		}
		

		for (int i = 0; i < N_TILES; i++) {
			tiles[i].width = 80;
			tiles[i].height = 30;
		}
		for (int i = 0; i < N_TILES; i++) {
			int selct = rand() % 3;
			tiles[i].tile = createSprite(tileTypesPath[selct]);
			if (selct == 0) {
				tiles[i].destr = false;
				tiles[i].hp = 1;
			}
			else if (selct == 1) {
				tiles[i].hp = 2;
				tiles[i].destr = false;
			}
			else if (selct == 2) {
				tiles[i].hp = -1;
				tiles[i].destr = true;
			}
			setSpriteSize(tiles[i].tile, tiles[i].width, tiles[i].height);
		}
		return true;
	}

	virtual void Close() {

	}



	virtual bool Tick() {////////////////////////////////////////////////////////////////////////////////////
		drawTestBackground();
		int tilex = 50, tiley = 81;// 50 81
		int LRmarg = tilex;
		int c = 1;
		for(int i = 0; i < N_TILES; i++){
			if (c == 0) {
				tilex += tiles[0].width * 2;
			}
			else {
				tilex += tiles[0].width;
			}
			c++;
			if (c == 5) c = 0;

			if (tilex + tiles[0].width > width - LRmarg) { //L&R marging = tilex
				tiley += tiles[0].height;
				tilex = LRmarg;
			}
			tiles[i].leftTopX = tilex;
			tiles[i].leftTopY = tiley;
			tiles[i].calcPoints();


			if (tiles[i].hp == 0) {
				continue;
			}
			drawSprite(tiles[i].tile, tiles[i].leftTopX, tiles[i].leftTopY);
		}

		drawSprite(player.sprite, player.x, player.y);
		drawSprite(ball.sprite, ball.curX, ball.curY);

		drawSprite(turret.sprite, turret.x, turret.y);

		



		if (!isHeldLMB && !isHeldLMBonce) {//ball movement
			ball.curX = player.x + player.width / 2 - ball.width / 2;
			ball.curY = player.y - ball.width;
		}
		else if(!isHeldLMBsec) {//player selects the vector
			isHeldLMBonce = true;
			isHeldLMBsec = true;
			ball.nextColX = mouseX;
			ball.nextColY = mouseY;
			ball.prevColX = ball.curX;
			ball.prevColY = ball.curY;
			ball.calcMoveDelta();
		}
		else {//regular movement
			ball.moveTick();
			bool hor = false;
			ball.isCold = false;
			int dif = getTickCount() - onTickCol;


			if (ball.isOnceCold && onTickCol != -1) {
				ball.isCold = ball.isCollision(tiles, &player, &ball, N_TILES, width, height, hor, end, wasCold);
			}
			else if (!ball.isOnceCold) {
				ball.isCold = ball.isCollision(tiles, &player, &ball, N_TILES, width, height, hor, end, wasCold);
			}
			if (ball.isCold) {
				wasCold = true;
				repeatedColl = true;
				ball.isOnceCold = true;
				onTickCol = getTickCount();
				if (hor) {
					if ((ball.curX - ball.prevColX) < 0) { //movement direction
						ball.nextColX = ball.curX - abs(ball.prevColX - ball.curX);
					}
					else {
						ball.nextColX = ball.curX + abs(ball.prevColX - ball.curX);
					}
					ball.nextColY = ball.prevColY;
					
					ball.prevColX = ball.curX;
					ball.prevColY = ball.curY;
					
					ball.calcMoveDelta();
				}
				else {// !hor
					if ((ball.curY - ball.prevColY) < 0) { //movement direction
						ball.nextColY = ball.curY - abs(ball.prevColY - ball.curY);
					}
					else {
						ball.nextColY = ball.curY + abs(ball.prevColY - ball.curY);
					}
					ball.nextColX = ball.prevColX;
					
					ball.prevColX = ball.curX;
					ball.prevColY = ball.curY;
					
					ball.calcMoveDelta();
				}
			}
			else {
				repeatedColl = false;
			}


			turret.moveTick();
			//turret ball movement
			if (tball[0].y < height && tball[0].y == 0 || tball[0].y + tball[0].width >= height) {
				tball[0].x = turret.x;
				tball[0].y = turret.y + turret.height;
				tball[0].moveTick();
				tball[0].calcMove(&player);
				drawSprite(tball[0].sprite, tball[0].x, tball[0].y);
			}
			else if (tball[0].y < height) {
				tball[0].moveTick();
				drawSprite(tball[0].sprite, tball[0].x, tball[0].y);
				if (tball[0].isColl(&player)) {
					end = true;
				}
			}
		}

		//abilities
		if (isHeldRMB) {
			isHeldRMB = false;
			ball.addAbility(ball.score);
			setSpriteSize(ball.sprite, ball.width, ball.width);
		}
		//abilities lifetime
		if (isHeldRMBonce) {
			if (ball.checkAbility(time(NULL), timeBeg)) {
				timeBeg = time(NULL);
				setSpriteSize(ball.sprite, ball.width, ball.width);
			}
		}
		else {
			timeBeg = time(NULL);
		}



		if (isHeldL) {//player movement LEFT
			if (player.x > 0) {
				player.x -= player.speed;
				
			}
		}
		if (isHeldR) {//player movement RIGHT
			if (player.x < (width - player.width)) {
				player.x += player.speed;
			}
		}
		if (end) { Init(); }// GAME OVER

		//destroySprite(player.sprite);
		return false;
	}




	virtual void onMouseMove(int x, int y, int xrelative, int yrelative) {
		mouseX = x;
		mouseY = y;
	}

	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased) {
		if (button == FRMouseButton::LEFT && !isReleased) {
			isHeldLMB = true;
		}
		if (button == FRMouseButton::RIGHT && !isReleased) {
			if (ball.score >= 20) {
				isHeldRMB = true;
				isHeldRMBonce = true;
			}
		}
	}

	virtual void onKeyPressed(FRKey k) {
		if (k == FRKey::RIGHT) {
			isHeldR = true;
		}
		else if(k == FRKey::LEFT) {
			isHeldL = true;
		}
		else if (k == FRKey::UP) {
			isHeldU = true;
		}
		else if (k == FRKey::DOWN) {
			isHeldD = true;
		}
	}

	virtual void onKeyReleased(FRKey k) {
		if (k == FRKey::RIGHT) {
			isHeldR = false;
		}
		else if (k == FRKey::LEFT) {
			isHeldL = false;
		}
		else if (k == FRKey::UP) {
			isHeldU = false;
		}
		else if (k == FRKey::DOWN) {
			isHeldD = false;
		}
	}
	
	virtual const char* GetTitle() override	{
		return "Arcanoid by Ihor Protsyshyn";
	}

	std::string screen;
private:
	Turret turret = Turret();
	TurretBall tball[2];
	Player player = Player(76, 25);
	Ball ball = Ball();
	const char* tileTypesPath[3]{ ".\\data\\03-Breakout-Tiles.png", ".\\data\\13-Breakout-Tiles.png",".\\data\\07-Breakout-Tiles.png" };
	Tile tiles[N_TILES];
	

	int height, width;
	int mouseX, mouseY;


	bool end = false;
	time_t timeBeg;
	unsigned int tickBeg;
	short int fps;
	float scrCoef = 1;
	int onTickCol = -1;
	bool wasCold;
	bool isHeldR = false;
	bool isHeldL = false;
	bool isHeldU = false;
	bool isHeldD = false;
	bool isHeldLMB = false;
	bool isHeldRMB = false;
	bool isHeldRMBonce = false;
	bool isHeldLMBonce = false;
	bool isHeldLMBsec = false;
	bool repeatedColl = false;
};



int main(int argc, char *argv[]){
	if (argc > 2) {
		std::string str(argv[2]);
		Arcanoid* arcan(new Arcanoid(str));
		return run(arcan);
	}
	else {
		Arcanoid* arcan = new Arcanoid;
		return run(arcan);
	}
}