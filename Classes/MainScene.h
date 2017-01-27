#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "cocos2d.h"

class MainScene : public cocos2d::Scene
{
private:
	//default constructor
	MainScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void onExit() override;

	std::vector<cocos2d::Label*> labels;
	int hoveringLableIndex;
	cocos2d::Size winSize;

	void initInputListeners();
	void releaseInputListeners();

	void checkMouseOver(const cocos2d::Vec2 mousePos);

	enum LABEL_INDEX
	{
		QUAD_TREE = 0,
		FLOCKING,
		CIRCLE_PACKING,
        RECT_PACKING,
        A_STAR_PATHFINDING,
        EAR_CLIPPING,
		EXIT
	};

public:
	//simple creator func
	static MainScene* createScene();

	//default destructor
	~MainScene() {};

	//Cocos2d Macro
	CREATE_FUNC(MainScene);
};

#endif
