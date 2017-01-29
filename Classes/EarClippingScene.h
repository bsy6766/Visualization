#ifndef EARCLIPPINGSCENE_H
#define EARCLIPPINGSCENE_H

#include "cocos2d.h"

class EarClippingScene : public cocos2d::Scene
{
private:
	//default constructor
	EarClippingScene() {};

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
	static EarClippingScene* createScene();

	//default destructor
	~EarClippingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(EarClippingScene);
};

#endif