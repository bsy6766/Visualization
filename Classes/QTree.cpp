#include "QTree.h"
#include "Component.h"

int QTree::CurrentMaxLevelSet = 6;
cocos2d::Node* QTree::lineNode = nullptr;

QTree::QTree(const cocos2d::Rect& boundary, int level) : nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr), boundary(boundary), leaf(true), level(level), xAxis(nullptr), yAxis(nullptr), clean(true)
{
	if (this->level == 0)
	{
		QTree::CurrentMaxLevelSet = DEFAULT_LEVEL;
	}
}

QTree::~QTree()
{
	if(xAxis != nullptr)
		xAxis->removeFromParentAndCleanup(true);
	if(yAxis != nullptr)
		yAxis->removeFromParentAndCleanup(true);
	clear();
	if (this->level == 0)
	{
		QTree::CurrentMaxLevelSet = DEFAULT_LEVEL;
	}
}

void QTree::clear()
{
	if (this->clean)
		return;

	if (nw != nullptr)
	{
		if (CLEAN_UP_SUB_DIV)
		{
			delete nw;
			this->nw = nullptr;
		}
		else
		{
			nw->clear();
		}
	}

	if (ne != nullptr)
	{
		if (CLEAN_UP_SUB_DIV)
		{
			delete ne;
			this->ne = nullptr;
		}
		else
		{
			ne->clear();
		}
	}

	if (sw != nullptr)
	{
		if (CLEAN_UP_SUB_DIV)
		{
			delete sw;
			this->sw = nullptr;
		}
		else
		{
			sw->clear();
		}
	}

	if (se != nullptr)
	{
		if (CLEAN_UP_SUB_DIV)
		{
			delete se;
			this->se = nullptr;
		}
		else
		{
			se->clear();
		}
	}

	// Empty data
	datas.clear();

	// Must be leaf on fresh
	this->leaf = true;

	if (xAxis != nullptr)
		xAxis->setOpacity(0);
	if (yAxis != nullptr)
		yAxis->setOpacity(0);

	this->clean = true;
}

void QTree::showLines()
{
	if (this->leaf == false)
	{
		this->initLines();

		xAxis->setOpacity(255);
		yAxis->setOpacity(255);
	}

	if (nw != nullptr)
	{
		nw->showLines();
	}

	if (ne != nullptr)
	{
		ne->showLines();
	}

	if (sw != nullptr)
	{
		sw->showLines();
	}

	if (se != nullptr)
	{
		se->showLines();
	}
}

void QTree::initLines()
{
	float midX = this->boundary.getMidX();
	float midY = this->boundary.getMidY();
	auto pos = cocos2d::Vec2(midX, midY);

	if (xAxis == nullptr)
	{
		xAxis = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
		xAxis->setPosition(pos);
		xAxis->setScaleX(this->boundary.size.width);
		xAxis->setOpacity(0);
		xAxis->setColor(cocos2d::Color3B::YELLOW);
		xAxis->retain();
		QTree::lineNode->addChild(xAxis);
	}

	if (yAxis == nullptr)
	{
		yAxis = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
		yAxis->setPosition(pos);
		yAxis->setScaleY(this->boundary.size.height);
		yAxis->setOpacity(0);
		yAxis->setColor(cocos2d::Color3B::YELLOW);
		yAxis->retain();
		QTree::lineNode->addChild(yAxis);
	}
}

void QTree::increaseLevel()
{
	if (QTree::CurrentMaxLevelSet < MAX_LEVEL)
	{
		QTree::CurrentMaxLevelSet += 1;
	}
}

void QTree::decreaseLevel()
{
	if (QTree::CurrentMaxLevelSet > MIN_LEVEL)
	{
		QTree::CurrentMaxLevelSet -= 1;
	}
}

const int QTree::getCurrentLevelSetting()
{
	return QTree::CurrentMaxLevelSet;
}

void QTree::subDivide()
{
	//cocos2d::log("Subdividing on level %d", this->level);
	//sub divide only happen when there is more than 1 object.
	this->leaf = false;

	float width = this->boundary.size.width * 0.5f;
	float height = this->boundary.size.height * 0.5f;

	//top left 
	if(nw == nullptr)
		nw = new QTree(cocos2d::Rect(this->boundary.getMinX(), this->boundary.getMidY(), width, height), this->level + 1);

	//top right
	if (ne == nullptr)
		ne = new QTree(cocos2d::Rect(this->boundary.getMidX(), this->boundary.getMidY(), width, height), this->level + 1);

	//bot left
	if (sw == nullptr)
		sw = new QTree(cocos2d::Rect(this->boundary.getMinX(), this->boundary.getMinY(), width, height), this->level + 1);

	//bot right
	if (se == nullptr)
		se = new QTree(cocos2d::Rect(this->boundary.getMidX(), this->boundary.getMinY(), width, height), this->level + 1);

	//so...replace the object
	std::list<Entity*> remainingData;

	for (auto data : this->datas)
	{
		remainingData.push_back(data);
	}

	this->datas.clear();

	for (auto data : remainingData)
	{
		if (this->nw->insert(data))
		{
			continue;
		}
		else if (this->ne->insert(data))
		{
			continue;
		}
		else if (this->sw->insert(data))
		{
			continue;
		}
		else if (this->se->insert(data))
		{
			continue;
		}
		else
		{
			this->datas.push_back(data);
		}
	}

	remainingData.clear();
}

bool QTree::insert(Entity* entity)
{
	if (this->clean)
		clean = false;

	auto bb = dynamic_cast<ECS::Sprite*>(entity->components[SPRITE])->sprite->getBoundingBox();
	if (this->boundary.intersectsRect(bb))
	{
		//check if it has subs
		if (leaf)
		{
			//don't have sub. check max level
			if (this->level < QTree::CurrentMaxLevelSet)
			{
				//not max level yet. subdivide. objects existing this node should be repositioned to new subs
				if (this->datas.size() < CAPACITY)
				{
					this->datas.push_back(entity);
					return true;
				}
				else
				{
					subDivide();

					//insert new object to subs
					if (this->nw->insert(entity))
					{
						return true;
					}
					else if (this->ne->insert(entity))
					{
						return true;
					}
					else if (this->sw->insert(entity))
					{
						return true;
					}
					else if (this->se->insert(entity))
					{
						return true;
					}
					else
					{
						//doesn't fit. remain here
						this->datas.push_back(entity);
						//cocos2d::log("Inserting at level %d with id %d, rect (%f, %f, %f, %f)", level, entity,  box.origin.x, box.origin.y, box.size.width, box.size.height);
						return true;
					}
				}

			}
			else
			{
				this->datas.push_back(entity);
				//cocos2d::log("Inserting at level %d with id %d, rect (%f, %f, %f, %f)", level, entity,  box.origin.x, box.origin.y, box.size.width, box.size.height);
				return true;
			}
		}
		else
		{					
			//insert new object to subs
			if (this->nw->insert(entity))
			{
				return true;
			}
			else if (this->ne->insert(entity))
			{
				return true;
			}
			else if (this->sw->insert(entity))
			{
				return true;
			}
			else if (this->se->insert(entity))
			{
				return true;
			}
			else
			{
				//doesn't fit. remain here
				this->datas.push_back( entity ); 
				return true;
			}
		}
	}
	else
	{
		return false;
	}

	return false;
}

void QTree::setBoundary(const cocos2d::Rect & boundary)
{
	this->boundary = boundary;
}

void QTree::setLevel(const int level)
{
	if (level < 0)
	{
		this->level = 0;
	}
	this->level = level;
}

void QTree::queryAllEntities(const cocos2d::Rect& queryingArea, std::list<Entity*>& nearBoundingBoxes)
{
	if (this->boundary.intersectsRect(queryingArea))
	{
		if (leaf)
		{
			for (auto data : this->datas)
			{
				nearBoundingBoxes.push_back(data);
			}
		}
		else
		{
			//not a leaf. Add objects in this node and proceed to children
			for (auto data : this->datas)
			{
				nearBoundingBoxes.push_back(data);
			}

			if (this->nw != nullptr)
			{
				this->nw->queryAllEntities(queryingArea, nearBoundingBoxes);
			}

			if (this->ne != nullptr)
			{
				this->ne->queryAllEntities(queryingArea, nearBoundingBoxes);
			}

			if (this->sw != nullptr)
			{
				this->sw->queryAllEntities(queryingArea, nearBoundingBoxes);
			}

			if (this->se != nullptr)
			{
				this->se->queryAllEntities(queryingArea, nearBoundingBoxes);
			}
		}
	}
}