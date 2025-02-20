float dist(sf::Vector2f p1, sf::Vector2f p2);
float distsq(sf::Vector2f p1, sf::Vector2f p2);
size_t split(const std::string& txt, std::vector<std::string>& strs, char ch);
int loadLevel();

class Object
{
public:
	Object(float radius, sf::Vector2f position);
	void update(float dt, sf::RenderWindow& window);
	void draw(sf::RenderWindow& window);
	sf::Vector2f velocity;
	sf::Sprite shape;
	float radius;
};

class Player : public Object
{
public:
	using Object::Object;
	void update(float dt, sf::RenderWindow& window, sf::Texture& character);
	enum class AState {
		WALKINGUP,
		WALKINGDOWN,
		WALKINGLEFT,
		WALKINGRIGHT,
		IDLE
	};
	AState aState = AState::IDLE;
	float walkframe = 0;
};

class Creature : public Object
{
public:
	using Object::Object;
	void update(float dt, sf::RenderWindow& window);
	float cooldown = 0;
};

class Wall
{
public:
	sf::Vector2f p1, p2;
	sf::Color color;
	sf::VertexArray shape;
	int id;
	bool exists = true;
	Wall(sf::Vector2f p1, sf::Vector2f p2);
	void draw(sf::RenderWindow& window);
	sf::Vector2f closestPoint(Object& ball, float dt) const;
};

class Point
{
public:
	sf::Vector2f position;
	Point(sf::Vector2f position);
	void collide(Object& ball, float dt);
};

class Polygon
{
public:
	sf::VertexArray shape;
	Polygon(std::vector<sf::Vector2f> verticies);
	void draw(sf::RenderWindow& window);
};

class TileMap // mostly stolen from https://www.sfml-dev.org/tutorials/2.6/graphics-vertex-array.php
{
public:
	sf::VertexArray m_vertices;
	std::vector<int> pointsI;
	std::vector<int> wallsI;
	int map[300*300];
	TileMap(int* map);
	TileMap();
	void draw(sf::RenderWindow& window, sf::Texture& dirt);
	void recalculate();
};

class Button
{
public:
	sf::Sprite shape;
	sf::Vector2f size;
	sf::Color color;
	Button(sf::Vector2f position, sf::Vector2f size, std::function<void()> callback);
	void draw(sf::RenderWindow& window);
	void update(sf::RenderWindow& window);
	std::function<void()> callback;
};
