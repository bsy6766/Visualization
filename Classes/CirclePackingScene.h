#ifndef CIRCLEPACKINGSCENE_H
#define CIRCLEPACKINGSCENE_H

#include "cocos2d.h"

class Circle;

class CirclePackingScene : public cocos2d::CCScene
{
private:
	//default constructor
	CirclePackingScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);
	void onMouseUp(cocos2d::Event* event);
	void onMouseScroll(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	void initInputListeners();
	void releaseInputListeners();

	cocos2d::Label* backLabel;
	cocos2d::DrawNode* drawNode;

	std::vector<cocos2d::Image*> images;
	std::vector<cocos2d::Sprite*> imageSprites;
	std::vector<Circle*> circles;
	// Numbers of circles that have been spawned
	int spawnedCircleCount;
	std::queue<cocos2d::Vec2> circleSpawnPoints;

	// Initial number of circles that spawn on start
	int initialCircleCount;

	// Number of circles that spawn every tick
	int circleSpawnRate;

	// Pause simulation
	bool pause;

	// Simulate speed multiplier. 1.0 by default. 0 = stops simulation
	float simulateSpeedMultiplier;

	enum IMAGE_INDEX
	{
		DEAULT = 0,	//C++
		CAT,
		MAX_SIZE
	};

	IMAGE_INDEX currentImageIndex;

	enum SPRITE_Z_ORDER
	{
		BEHIND_CIRCLES = 0,
		CIRCLES,
		ABOVE_CIRCLES
	};

	int maxCircles;
	int searchSpawnPointWidthOffset;
	int searchSpawnPointHeightOffset;

	void initImages();
	void findCircleSpawnPoint(const IMAGE_INDEX index);
	void initCircles();
	void spawnCircles(const int rate);

public:
	//simple creator func
	static CirclePackingScene* createScene();

	//default destructor
	~CirclePackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(CirclePackingScene);
};

#endif