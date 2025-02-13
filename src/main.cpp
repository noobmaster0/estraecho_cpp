#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream> 
#include <string>
#include <stdarg.h>
#include <math.h>
#include <functional>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imguiThemes.h"

#include "header.hpp"

#define resX 1000
#define resY 1000
#define PPM 50.f // pixels per meter
#define cellSize 50
#define TSS 16 // tile sprite size

sf::Texture character;
sf::Texture dirt;

const float playerSpeed = 0.1f*PPM;
int idCounter = 0;
bool confineCam = true;

sf::Vector2u mapSize = {500,500};

int lvlCtr = 1;
bool outOfLevels = false;

sf::Mouse mouse;
sf::Keyboard keyboard;

std::vector<Polygon> polygons;
std::vector<Wall> walls;
std::vector<Point> points;
std::vector<Button> buttons;
std::vector<Creature> creatures;
TileMap map;
Player player(25.f,sf::Vector2f(75,50));

float sound = 0;

int main()
{
#ifdef APPLE
	std::cout << "Bad hardware & software detected :(\n";
	return 1;
#endif // APPLE


	sf::RenderWindow window(sf::VideoMode(resX, resY), "Estra echo c++ edition (the faster one)");

	sf::Font font;
#ifdef __linux__
	if (!font.loadFromFile("/usr/share/fonts/TTF/arial.ttf"))
#else 
	if (!font.loadFromFile("C:/Windows/fonts/Arial.ttf"))
#endif
	{
		std::cout << "Failed to Load Arial.ttf";
		return 1;
	}

	if (!dirt.loadFromFile("resources/TileMap.png"))
		return 1;
	dirt.setRepeated(true);

	if (!character.loadFromFile("resources/character.png"))
		return 1;

	sf::Texture door;
	if (!door.loadFromFile("resources/door.png"))
		return 1;

	sf::Texture trapdoor;
	if (!trapdoor.loadFromFile("resources/trapdoor.png"))
		return 1;

	sf::Text test("", font, 30);

	sf::View cam({ 200,200 }, { 1000,1000 });

	loadLevel();

	creatures.emplace_back(25, sf::Vector2f({300,300}));


	sf::CircleShape soundCircle(50);
	soundCircle.setOutlineColor(sf::Color::Red);
	soundCircle.setOutlineThickness(1);
	soundCircle.setFillColor(sf::Color::Transparent);
	soundCircle.setOrigin(25, 25);

	float dt = 0;
	sf::Clock clock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::Resized)
			{
				window.setSize(sf::Vector2u(resX, resY));
			}
		}
		window.clear(sf::Color(0, 0, 0));

		sf::Vector2f mp = window.mapPixelToCoords(mouse.getPosition(window));
		float r = 25;

		if (mouse.isButtonPressed(sf::Mouse::Button::Left))
			player.shape.setPosition(mp - sf::Vector2f(1, 1) * r);


		player.velocity = sf::Vector2f(0, 0);

		float pSpeed = playerSpeed;

		if (keyboard.isKeyPressed(sf::Keyboard::LShift))
		{
			pSpeed *= 2;
		}

		sf::Vector2f mV = { 0,0 };

		if (keyboard.isKeyPressed(sf::Keyboard::Key::W))
		{
			mV += sf::Vector2f(0, -1);
		}

		if (keyboard.isKeyPressed(sf::Keyboard::Key::S))
		{
			mV += sf::Vector2f(0, 1);
		}

		if (keyboard.isKeyPressed(sf::Keyboard::Key::D))
		{
			mV += sf::Vector2f(1, 0);
		}

		if (keyboard.isKeyPressed(sf::Keyboard::Key::A))
		{
			mV += sf::Vector2f(-1, 0);
		}

		float l = dist({ 0,0 }, mV);
		if (l != 0)
		{
			mV /= l;
		}
		
		mV *= pSpeed;
		player.velocity += mV;

		map.draw(window);

		for (auto& polygon : polygons)
		{
			polygon.draw(window);
		}

		for (auto& wall : walls) {
			wall.draw(window);
		}

		for (auto& wall : walls) {
			wall.closestPoint(player, dt);
		}

		for (auto& point : points)
		{
			point.collide(player, dt);
		}

		player.update(dt, window);
		player.draw(window);

		for (auto& creature : creatures)
		{
			creature.update(dt, window);
			creature.draw(window);
		}

		for (auto& button : buttons)
		{
			button.update(window);
			button.draw(window);
		}

		/*
		if (lemmingsRemaining <= 1 && lemmingsNum <= 1 && !outOfLevels)
		{
			if (end.num >= .75 * lemmingsNumOg)
			{
				std::cout << "You win\n";
				lvlCtr++;
			}
			else
			{
				std::cout << "You Loose\n";
			}
			loadLevel();
		}
		*/

		sound = dist({ 0,0 }, player.velocity) * 30;

		soundCircle.setRadius(sound);
		soundCircle.setOrigin(sound,sound);
		soundCircle.setPosition(player.shape.getPosition() + sf::Vector2f(1, 1) * r);
		window.draw(soundCircle);

		sf::Vector2f playPos = player.shape.getPosition() + sf::Vector2f(1, 1) * r;
		sf::Vector2f camPos = { 0, 0 };


		if (confineCam)
		{
			if (playPos.x - cam.getSize().x / 2 < 0)
			{
				camPos.x = cam.getSize().x / 2;
			}
			else if (playPos.x + cam.getSize().x / 2 > mapSize.x * cellSize)
			{
				camPos.x = mapSize.x * cellSize - cam.getSize().x / 2;
			}
			else
			{
				camPos.x = playPos.x;
			}

			if (playPos.y - cam.getSize().y / 2 < 0)
			{
				camPos.y = cam.getSize().y / 2;
			}
			else if (playPos.y + cam.getSize().y / 2 > mapSize.y * cellSize)
			{
				camPos.y = mapSize.y * cellSize - cam.getSize().y / 2;
			}
			else
			{
				camPos.y = playPos.y;
			}
		}
		else
		{
			camPos = playPos;
		}

		cam.setCenter(camPos);

		window.setView(cam);
		window.display();
		dt = clock.restart().asSeconds();
	}
	return 0;
}

float dist(sf::Vector2f p1, sf::Vector2f p2)
{
	return (sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x)));
}

float distsq(sf::Vector2f p1, sf::Vector2f p2)
{
	return (((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x)));
}

size_t split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

int loadLevel()
{
	sf::Image mapMask;
	if (!mapMask.loadFromFile("resources/" + std::to_string(lvlCtr) + "/mapmask.png"))
	{
		outOfLevels = true;
		return 1;
	}

	sf::Vector2u size = sf::Vector2u(mapMask.getSize().x, mapMask.getSize().y);

	mapSize = size;

	int* mapT = new int[300*300];


	for (unsigned int i = 0; i < 300; ++i) {
		for (unsigned int j = 0; j < 300; ++j) {
			if (i < size.x && j < size.y)
			{
				if (mapMask.getPixel(i, j).r <= 255 / 2 && mapMask.getPixel(i, j).g <= 255 / 2 && mapMask.getPixel(i, j).b <= 255 / 2)
				{
					mapT[i + j * 300] = 0;
				}
				else
				{
					mapT[i + j * 300] = 1;
				}
			}
			else
			{
				mapT[i + j * 300] = 0;
			}
		}
	}

	map = TileMap(mapT);
	
	map.recalculate();

	delete mapT;

	std::ifstream file;
	file.open("resources/" + std::to_string(lvlCtr) + "/lvl.txt");
	
	std::string text;

	if (file.is_open())
	{

		while (std::getline(file, text)) {

			std::vector<std::string> tokens;

			// Output the text from the file
			split(text, tokens, ' ');
			std::string command = tokens[0];

			if (command == "w")
			{
				walls.emplace_back(sf::Vector2f(std::stoi(tokens[1]), std::stoi(tokens[2])), sf::Vector2f(std::stoi(tokens[3]), std::stoi(tokens[4])));
			}
			else if (command == "p")
			{
				std::vector<sf::Vector2f> verticies;
				int n = std::count(text.begin(), text.end(), ' ') / 2;
				for (int i = 1; i <= n; i++)
				{
					verticies.emplace_back(std::stoi(tokens[2 * i - 1]), std::stoi(tokens[2 * i]));
				}
				polygons.emplace_back(verticies);
				verticies.clear();
			}
			else if (command == "c")
			{
				if (tokens[1] == "t")
				{
					confineCam = true;
				}
				else
				{
					confineCam = false;
				}
			}
			else if (command == " " || command == "")
			{ }
			else
			{
				std::cout << "Unrecognized symbol: \"" << command << "\"\n";
			}
		}
	}
	else
	{
		std::cout << "failed to open level\n";
		return -1;
	}
	
	file.close();
	return 0;
}

Object::Object(float radius, sf::Vector2f position)
{
	shape = sf::CircleShape(radius);
	this->radius = radius;
	shape.setPosition(position);
}

void Object::draw(sf::RenderWindow& window)
{
	window.draw(this->shape);
}

void Object::update(float dt, sf::RenderWindow& window)
{
	shape.setPosition(shape.getPosition() + velocity * dt * (float)PPM);
}

void Player::update(float dt, sf::RenderWindow& window)
{
	shape.setPosition(shape.getPosition() + velocity * dt * (float)PPM);
}

void Creature::update(float dt, sf::RenderWindow& window)
{

	if (distsq(shape.getPosition(), player.shape.getPosition()) <= sound * sound || cooldown >= 0)
	{
		if (distsq(shape.getPosition(), player.shape.getPosition()) <= sound * sound)
		{
			cooldown = 5.f;
		}
		velocity = (shape.getPosition() - player.shape.getPosition()) / dist(player.shape.getPosition(), shape.getPosition()) * 5.f;
	}

	shape.setPosition(shape.getPosition() + velocity * dt * (float)PPM);
	cooldown -= dt;
}

Wall::Wall(sf::Vector2f p1, sf::Vector2f p2)
{
	this->p1 = p1;
	this->p2 = p2;
	shape = sf::VertexArray(sf::LinesStrip, 2);
	color = sf::Color::Red;
	id = idCounter;
	idCounter++;
}

void Wall::draw(sf::RenderWindow& window)
{
	if (!exists) return;
	shape[0].position = p1;
	shape[1].position = p2;
	shape[0].color = color;
	shape[1].color = color;
	window.draw(shape);
}

sf::Vector2f Wall::closestPoint(Object& ball, float dt)
{
	if (!exists) return sf::Vector2f(-100,-100);
	sf::Vector2f other = ball.shape.getPosition() + sf::Vector2f(ball.radius, ball.radius);
	float m = (p2.y - p1.y) / (p2.x - p1.x);

	sf::Vector2f closeP;
	if (p2.x == p1.x) {
		// Vertical line case
		closeP.x = p1.x;
		closeP.y = other.y;
	}
	else {
		float x = (other.x + other.y * m + m * m * p1.x - m * p1.y) / (m * m + 1);
		float y = m * (x - p1.x) + p1.y;
		closeP = sf::Vector2f(x, y);
	}

	if (dist(closeP, ball.shape.getPosition() + sf::Vector2f(ball.radius, ball.radius)) <= ball.radius &&
		dist(p1, closeP) + dist(p2, closeP) == dist(p1, p2))
	{
		sf::Vector2f normal = (other - closeP);
		normal = normal / dist(sf::Vector2f(0, 0), normal);
		ball.shape.setPosition(closeP + normal * ball.radius - sf::Vector2f(ball.radius, ball.radius));
		
		float normalComponent = (ball.velocity.x * normal.x + ball.velocity.y * normal.y);
		if (normalComponent < 0) {
			ball.velocity = ball.velocity - normalComponent * normal;
		}
	}

	return closeP;
}

Point::Point(sf::Vector2f positon)
{
	this->position = positon;
}

void Point::collide(Object& ball, float dt)
{
	sf::Vector2f other = ball.shape.getPosition() + sf::Vector2f(ball.radius, ball.radius);

	// Check collision with endpoints
	if (distsq(position, other) <= ball.radius * ball.radius)
	{
		sf::Vector2f normal = (other - position);
		normal = normal / dist(sf::Vector2f(0, 0), normal);
		ball.shape.setPosition(ball.shape.getPosition() + normal * (ball.radius - dist(position, other)));
		// Cancel out only the normal component of velocity
		float normalComponent = (ball.velocity.x * normal.x + ball.velocity.y * normal.y);
		if (normalComponent < 0.1f) {  // Allow for small positive values
			ball.velocity = ball.velocity - normalComponent * normal * 60.f * dt;
		}
	}
}

Polygon::Polygon(std::vector<sf::Vector2f> verticies)
{
	shape = sf::VertexArray(sf::TrianglesFan, verticies.size());
	int i = 0;
	for (auto& vertex : verticies)
	{
		shape[i] = sf::Vertex(vertex);
		shape[i].color = sf::Color(50,50,50);
		i += 1;
	}

	for (size_t i = 0; i < verticies.size(); i++)
	{
		// Get current vertex and next vertex (loop back to first vertex if at the end)
		sf::Vector2f currentVertex = verticies[i];
		sf::Vector2f nextVertex = verticies[(i + 1) % verticies.size()];

		points.emplace_back(currentVertex);

		// Create a wall between these vertices
		walls.emplace_back(currentVertex, nextVertex);
	}
}

void Polygon::draw(sf::RenderWindow& window)
{
	window.draw(shape);
}

TileMap::TileMap(int* map)
{
	int height = 300, width = 300;

	m_vertices.setPrimitiveType(sf::Triangles);
	m_vertices.resize(1000 / 5 * 1000 / 5 * 6);

	for (unsigned int i = 0; i < width; ++i)
	{
		for (unsigned int j = 0; j < height; ++j)
		{
			this->map[(i + j * width)] = map[i + j * width];
		}
	}

	recalculate();

	return;
}

TileMap::TileMap()
{

	int height = 300, width = 300;
	sf::Vector2u tileSize = { 5,5 };

	m_vertices.setPrimitiveType(sf::Triangles);
	m_vertices.resize(300 * 300 * 6);

	for (unsigned int i = 0; i < width; ++i)
	{
		for (unsigned int j = 0; j < height; ++j)
		{
			this->map[(i + j * width)] = false;
		}
	}

	recalculate();

	return;
}

void TileMap::draw(sf::RenderWindow& window)
{
	sf::RenderStates states;
	states.texture = &dirt;

	window.draw(m_vertices, states);
}

void TileMap::recalculate() {
	int height = 300, width = 300;
	sf::Vector2u tileSize = { cellSize, cellSize };
	// Resize vertex array to accommodate triangles
	m_vertices.clear();
	m_vertices.setPrimitiveType(sf::Triangles);
	m_vertices.resize(width * height * 6);

	// Remove walls marked for deletion in reverse order
	std::sort(wallsI.begin(), wallsI.end(), std::greater<int>());
	for (int index : wallsI) {
		walls.erase(walls.begin() + index);
	}
	wallsI.clear();

	// Remove points marked for deletion in reverse order
	std::sort(pointsI.begin(), pointsI.end(), std::greater<int>());
	for (int index : pointsI) {
		points.erase(points.begin() + index);
	}
	pointsI.clear();

	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			if (!map[i + j * width])
				continue; // Skip empty tiles

			// Create tile vertices
			sf::Vertex* triangles = &m_vertices[(i + j * width) * 6];
			triangles[0].position = sf::Vector2f(i * tileSize.x, j * tileSize.y);
			triangles[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
			triangles[2].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);
			triangles[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);
			triangles[4].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
			triangles[5].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);

			triangles[0].texCoords = sf::Vector2f( map[i + j * width] * TSS,map[i + j * width] * TSS );
			triangles[1].texCoords = sf::Vector2f( (map[i + j * width] + 1) * TSS,map[i + j * width] * TSS );
			triangles[2].texCoords = sf::Vector2f((map[i + j * width]) * TSS,(map[i + j * width] + 1) * TSS );
			triangles[3].texCoords = sf::Vector2f((map[i + j * width]) * TSS,(map[i + j * width] + 1) * TSS );
			triangles[4].texCoords = sf::Vector2f((map[i + j * width] + 1) * TSS,(map[i + j * width]) * TSS );
			triangles[5].texCoords = sf::Vector2f((map[i + j * width] + 1) * TSS,(map[i + j * width] + 1) * TSS );

			// Store adjacent cell states
			bool above = (j > 0) ? map[i + (j - 1) * width] : true;
			bool right = (i < width - 1) ? map[(i + 1) + j * width] : true;
			bool below = (j < height - 1) ? map[i + (j + 1) * width] : true;
			bool left = (i > 0) ? map[(i - 1) + j * width] : true;

			// Wall creation with corner detection
			if (!above) { // Check above
				walls.emplace_back(triangles[0].position, triangles[1].position);
				wallsI.push_back(walls.size() - 1);

				// Check corners for top wall
				if (!left) { // Top-left corner
					points.emplace_back(triangles[0].position);
					pointsI.push_back(points.size() - 1);
				}
				if (!right) { // Top-right corner
					points.emplace_back(triangles[1].position);
					pointsI.push_back(points.size() - 1);
				}
			}

			if (!right) { // Check right
				walls.emplace_back(triangles[1].position, triangles[5].position);
				wallsI.push_back(walls.size() - 1);

				// Check corners for right wall
				if (!above) { // Top-right corner
					points.emplace_back(triangles[1].position);
					pointsI.push_back(points.size() - 1);
				}
				if (!below) { // Bottom-right corner
					points.emplace_back(triangles[5].position);
					pointsI.push_back(points.size() - 1);
				}
			}

			if (!below) { // Check below
				walls.emplace_back(triangles[5].position, triangles[3].position);
				wallsI.push_back(walls.size() - 1);

				// Check corners for bottom wall
				if (!right) { // Bottom-right corner
					points.emplace_back(triangles[5].position);
					pointsI.push_back(points.size() - 1);
				}
				if (!left) { // Bottom-left corner
					points.emplace_back(triangles[3].position);
					pointsI.push_back(points.size() - 1);
				}
			}

			if (!left) { // Check left
				walls.emplace_back(triangles[3].position, triangles[0].position);
				wallsI.push_back(walls.size() - 1);

				// Check corners for left wall
				if (!below) { // Bottom-left corner
					points.emplace_back(triangles[3].position);
					pointsI.push_back(points.size() - 1);
				}
				if (!above) { // Top-left corner
					points.emplace_back(triangles[0].position);
					pointsI.push_back(points.size() - 1);
				}
			}
		}
	}
}

Button::Button(sf::Vector2f positon, sf::Vector2f size, std::function<void()> callback)
{
	this->callback = callback;
	this->size = size;
	this->color = sf::Color::Green;
	shape.setColor(color);
	shape.setPosition(positon);
	shape.setScale(size);
}

void Button::draw(sf::RenderWindow& window)
{
	shape.setScale(size.x / shape.getTextureRect().width, size.y / shape.getTextureRect().height);
	window.draw(shape);
}

void Button::update(sf::RenderWindow& window)
{
	if (mouse.isButtonPressed(sf::Mouse::Left))
	{
		sf::Vector2f mousePos = sf::Vector2f(mouse.getPosition(window));
		if ((mousePos.x >= shape.getPosition().x && mousePos.y >= shape.getPosition().y) && (mousePos.x <= shape.getPosition().x + size.x && mousePos.y <= shape.getPosition().y + size.y))
		{
			callback();
			shape.setColor(sf::Color::Black);
		}
		else
		{
			shape.setColor(color);
		}
	}
	else
	{
		shape.setColor(color);
	}

}
