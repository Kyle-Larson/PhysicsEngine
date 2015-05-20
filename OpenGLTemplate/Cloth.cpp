#include "Cloth.h"


Cloth::Cloth(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : GameObject(position, rotation, scale)
{
	this->width = 10;
	this->height = 10;
	createCloth(width, height);
}


Cloth::~Cloth()
{
}

void Cloth::update(float dt)
{
	for (int i = 0; i < StructuralSprings.size(); i++)
	{
		StructuralSprings[i]->update(dt);
	}
	for (int i = 0; i < ShearSprings.size(); i++)
	{
		ShearSprings[i]->update(dt);
	}
	for (int i = 0; i < BendSprings.size(); i++)
	{
		BendSprings[i]->update(dt);
	}
	for (int i = 0; i < ClothNodes.size(); i++)
	{
		ClothNodes[i]->update(dt);
	}
}

void Cloth::draw()
{
	for (int i = 0; i < ClothNodes.size(); i++)
	{
		ClothNodes[i]->draw();
	}
	for (int i = 0; i < StructuralSprings.size(); i++)
	{
		StructuralSprings[i]->draw();
	}
	/*for (int i = 0; i < ShearSprings.size(); i++)
	{
		ShearSprings[i]->draw();
	}
	for (int i = 0; i < BendSprings.size(); i++)
	{
		BendSprings[i]->draw();
	}*/
}

void Cloth::createCloth(int width, int height)
{
	populateNodes(width, height);
	populateStructuralSprings(width, height);
	populateShearSprings(width, height);
	populateBendSprings(width, height);
}

//Adds a node to the Cloth collection, and populates the node with the
//Necessary components.
void Cloth::addNode(glm::vec3 position, bool immovable)
{
	GameObject* node = new GameObject(position, glm::vec3(0.0f), glm::vec3(0.01f));
	node->setMesh(Mesh::sphereMesh);
	node->addCollisionDetector(CollisionDetector::DetectorType::Sphere);
	if (immovable)
	{
		node->addPhysicsComponent(0.0f, 0.0f);
	}
	else
	{
		node->addPhysicsComponent(0.1f, 0.1f);
	}
	ClothNodes.push_back(node);
}

//Method populates cloth with ClothNodes and adds the nodes to the
//ClothNodes collection.
//Width and height are number of nodes, they do not affect the scale
void Cloth::populateNodes(int width, int height)
{
	//X distance between nodes
	glm::vec3 scale = transform->getScale();
	glm::vec3 position = transform->getPosition();
	float dX = 1.0f / width;
	float dY = 1.0f / height;
	float pX = -0.5f;
	float pY = -0.5f;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			glm::vec3 pos = glm::vec3(pX + dX * i, pY + dY * j, position.z);
			pos = glm::vec3(glm::vec4(pos, 1.0f) * transform->transformMatrix);
			if (j == 0 || j == height - 1 || i == 0 || i == width - 1)
				addNode(pos, false);
			else
				addNode(pos, false);
		}
	}
}

//Method creates spring initialized to the two node's distances, and then added to the provided spring list.
void Cloth::addSpring(GameObject& node1, GameObject& node2, std::vector<Spring*>& springList)
{
	Spring* spring = new Spring(*(node1.physicsComponent), *(node2.physicsComponent), glm::vec3(0.0f), glm::vec3(0.0f), 0.01f);
	spring->setMesh(Mesh::cubeMesh);
	springList.push_back(spring);
}

//Method populates the structural springs list.
//Width / Height: # of cloth nodes in width and height.
void Cloth::populateStructuralSprings(int width, int height)
{
	for (int j = 0; j < height - 1; j++)
	{
		for (int i = 0; i < width - 1; i++)
		{
			GameObject* node1 = ClothNodes[j * width + i];
			GameObject* node2 = ClothNodes[j * width + i + 1];
			addSpring(*node1, *node2, StructuralSprings);
			node2 = ClothNodes[(j + 1) * width + i];
			addSpring(*node1, *node2, StructuralSprings);
		}
		GameObject* node1 = ClothNodes[(j + 1) * width - 1];
		GameObject* node2 = ClothNodes[(j + 2) * width - 1];
		addSpring(*node1, *node2, StructuralSprings);
	}
	for (int i = 0; i < width - 1; i++)
	{
		GameObject* node1 = ClothNodes[(height - 1) * width + i];
		GameObject* node2 = ClothNodes[(height - 1) * width + i + 1];
		addSpring(*node1, *node2, StructuralSprings);
	}
}

//Method populates shear springs list
//Width / Height: # of cloth nodes in width and height
void Cloth::populateShearSprings(int width, int height)
{
	if (width > 1 && height > 1)
	{
		for (int i = 0; i < width - 1; i++)
		{
			//Top Row
			GameObject* node1 = ClothNodes[i];
			GameObject* node2 = ClothNodes[width + i + 1];
			addSpring(*node1, *node2, ShearSprings);

			//BottomRow
			node1 = ClothNodes[(height - 1) * width + i];
			node2 = ClothNodes[(height - 2) * width + i + 1];
			addSpring(*node1, *node2, ShearSprings);
		}
		for (int j = 1; j < height - 1; j++)
		{
			for (int i = 0; i < width - 1; i++)
			{
				GameObject* node1 = ClothNodes[j * width + i];
				GameObject* node2 = ClothNodes[(j - 1) * width + i + 1];
				addSpring(*node1, *node2, ShearSprings);
				node2 = ClothNodes[(j + 1) * width + i + 1];
				addSpring(*node1, *node2, ShearSprings);
			}
		}
	}
}

//Method populates bend springs list
//Width / Height: # of cloth nodes in width and height
void Cloth::populateBendSprings(int width, int height)
{
	//Structural bend portion
	for (int j = 0; j < height - 2; j++)
	{
		for (int i = 0; i < width - 2; i++)
		{
			GameObject* node1 = ClothNodes[j * width + i];
			GameObject* node2 = ClothNodes[j * width + i + 2];
			addSpring(*node1, *node2, BendSprings);
			node2 = ClothNodes[(j + 2) * width + i];
			addSpring(*node1, *node2, BendSprings);
		}
		GameObject* node1 = ClothNodes[(j + 1) * width - 1];
		GameObject* node2 = ClothNodes[(j + 3) * width - 1];
		addSpring(*node1, *node2, BendSprings);
	}
	for (int i = 0; i < width - 2; i++)
	{
		GameObject* node1 = ClothNodes[(height - 1) * width + i];
		GameObject* node2 = ClothNodes[(height - 1) * width + i + 2];
		addSpring(*node1, *node2, BendSprings);
	}



	//Shear bend portion
	if (width > 2 && height > 2)
	{
		for (int i = 0; i < width - 2; i++)
		{
			//Top Row
			GameObject* node1 = ClothNodes[i];
			GameObject* node2 = ClothNodes[2 * width + i + 2];
			addSpring(*node1, *node2, BendSprings);

			//BottomRow
			node1 = ClothNodes[(height - 1) * width + i];
			node2 = ClothNodes[(height - 3) * width + i + 2];
			addSpring(*node1, *node2, BendSprings);
		}
		for (int j = 2; j < height - 2; j++)
		{
			for (int i = 0; i < width - 2; i++)
			{
				GameObject* node1 = ClothNodes[j * width + i];
				GameObject* node2 = ClothNodes[(j - 2) * width + i + 2];
				addSpring(*node1, *node2, BendSprings);
				node2 = ClothNodes[(j + 2) * width + i + 2];
				addSpring(*node1, *node2, BendSprings);
			}
		}
	}
}

Command* Cloth::getPullCommand()
{
	if (ClothNodes.size() % 2 == 1)
	{
		return new MoveCommand(glm::vec3(1.0f, 0.0f, 0.0f), *(ClothNodes[width * height - 1]));
	}
	else
	{
		return new MoveCommand(glm::vec3(1.0f, 0.0f, 0.0f), *(ClothNodes[width * height - 1]));
	}
}