#include "QuadTree.h"
#include "Component.h"

int QuadTree::CurrentMaxLevelSet = 6;
//cocos2d::Node* QuadTree::lineNode = nullptr;
cocos2d::DrawNode* QuadTree::lineDrawNode = nullptr;

QuadTree::QuadTree(const cocos2d::Rect& boundary, int level) 
: nw(nullptr)
, ne(nullptr)
, sw(nullptr)
, se(nullptr)
, boundary(boundary)
, leaf(true)
, level(level)
, clean(true)
{
	if (this->level == 0)
	{
		QuadTree::CurrentMaxLevelSet = DEFAULT_LEVEL;
	}
}

QuadTree::~QuadTree()
{
	clear();
	if (this->level == 0)
	{
		QuadTree::CurrentMaxLevelSet = DEFAULT_LEVEL;
	}
}

void QuadTree::clear()
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

	this->clean = true;
}

void QuadTree::showLines()
{
	if (this->leaf == false)
	{
		this->drawLines();
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

void QuadTree::drawLines()
{
	cocos2d::Vec2 center = cocos2d::Vec2(this->boundary.getMidX(), this->boundary.getMidY());
	cocos2d::Vec2 centerTop = cocos2d::Vec2(center.x, this->boundary.getMaxY());
	cocos2d::Vec2 centerBottom = cocos2d::Vec2(center.x, this->boundary.getMinY());
	cocos2d::Vec2 centerLeft = cocos2d::Vec2(this->boundary.getMinX(), center.y);
	cocos2d::Vec2 centerRight = cocos2d::Vec2(this->boundary.getMaxX(), center.y);

	QuadTree::lineDrawNode->setLineWidth(1.0f);
	QuadTree::lineDrawNode->drawLine(centerTop, centerBottom, cocos2d::Color4F::YELLOW);
	QuadTree::lineDrawNode->drawLine(centerLeft, centerRight, cocos2d::Color4F::YELLOW);
}

void QuadTree::increaseLevel()
{
	if (QuadTree::CurrentMaxLevelSet < MAX_LEVEL)
	{
		QuadTree::CurrentMaxLevelSet += 1;
	}
}

void QuadTree::decreaseLevel()
{
	if (QuadTree::CurrentMaxLevelSet > MIN_LEVEL)
	{
		QuadTree::CurrentMaxLevelSet -= 1;
	}
}

const int QuadTree::getCurrentLevelSetting()
{
	return QuadTree::CurrentMaxLevelSet;
}

void QuadTree::subDivide()
{
	//cocos2d::log("Subdividing on level %d", this->level);
	//sub divide only happen when there is more than 1 object.
	this->leaf = false;

	float width = this->boundary.size.width * 0.5f;
	float height = this->boundary.size.height * 0.5f;

	//top left 
	if(nw == nullptr)
		nw = new QuadTree(cocos2d::Rect(this->boundary.getMinX(), this->boundary.getMidY(), width, height), this->level + 1);

	//top right
	if (ne == nullptr)
		ne = new QuadTree(cocos2d::Rect(this->boundary.getMidX(), this->boundary.getMidY(), width, height), this->level + 1);

	//bot left
	if (sw == nullptr)
		sw = new QuadTree(cocos2d::Rect(this->boundary.getMinX(), this->boundary.getMinY(), width, height), this->level + 1);

	//bot right
	if (se == nullptr)
		se = new QuadTree(cocos2d::Rect(this->boundary.getMidX(), this->boundary.getMinY(), width, height), this->level + 1);

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

bool QuadTree::insert(Entity* entity)
{
	if (this->clean)
		clean = false;

	// Note: The line below is specifically made for this project. 
	auto bb = dynamic_cast<ECS::Sprite*>(entity->components[SPRITE])->sprite->getBoundingBox();
	if (this->boundary.intersectsRect(bb))
	{
		//check if it has subs
		if (leaf)
		{
			//don't have sub. check max level
			if (this->level < QuadTree::CurrentMaxLevelSet)
			{
				//not max level yet. subdivide. objects existing this node should be repositioned to new subs
				if (this->datas.size() < QuadTree::MAX_ENTITY_TO_SUBDIVIDE)
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
						return true;
					}
				}

			}
			else
			{
				this->datas.push_back(entity);
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

void QuadTree::setBoundary(const cocos2d::Rect & boundary)
{
	this->boundary = boundary;
}

void QuadTree::setLevel(const int level)
{
	if (level < 0)
	{
		this->level = 0;
	}
	this->level = level;
}

void QuadTree::queryAllEntities(const cocos2d::Rect& queryingArea, std::list<Entity*>& nearBoundingBoxes)
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