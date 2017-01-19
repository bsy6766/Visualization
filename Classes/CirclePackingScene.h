#ifndef CIRCLEPACKINGSCENE_H
#define CIRCLEPACKINGSCENE_H

#include "cocos2d.h"

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

	std::vector<cocos2d::Image*> images;

	enum IMAGE_INDEX
	{
		DEAULT = 0,
		CAT,
		MAX_SIZE
	};

	void initImages();

public:
	//simple creator func
	static CirclePackingScene* createScene();

	//default destructor
	~CirclePackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(CirclePackingScene);
};

#endif