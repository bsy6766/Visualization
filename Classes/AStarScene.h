#ifndef ASTARSCENE_H
#define ASTARSCENE_H

#include "cocos2d.h"

class AStarScene : public cocos2d::Scene
{
private:
	//default constructor
	AStarScene() {};

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

public:
	//simple creator func
	static AStarScene* createScene();

	//default destructor
	~AStarScene() {};

	//Cocos2d Macro
	CREATE_FUNC(AStarScene);
};

#endif