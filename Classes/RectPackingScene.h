#ifndef RECTPACKINGSCENE_H
#define RECTPACKINGSCENE_H

#include "cocos2d.h"
#include "CustomNode.h"

class RectPackingScene : public cocos2d::CCScene
{
private:
	//default constructor
	RectPackingScene() {};

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

	LabelsNode* labelsNode;

public:
	//simple creator func
	static RectPackingScene* createScene();

	//default destructor
	~RectPackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(RectPackingScene);
};

#endif