#ifndef TITLESCENE_H
#define TITLESCENE_H

#include <cocos2d.h>

class TitleScene : public cocos2d::Scene
{
private:
	//default constructor
	TitleScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseDown(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	void initInputListeners();
	void releaseInputListeners();

	cocos2d::Sprite* logo;

	const float time = 5.0f;
	float elapsedTime;
public:
	//simple creator func
	static TitleScene* createScene();

	//default destructor
	~TitleScene() {};

	//Cocos2d Macro
	CREATE_FUNC(TitleScene);
};

#endif