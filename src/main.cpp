#include <iostream>
#include <SFML/Graphics.hpp>
#include "universe_worker.h"
#include "universe_render.h"

int main()
{
	//Настраиваем мир моделирования
	world.params.width		= 900;// sf::VideoMode::getDesktopMode().width;
	world.params.height		= 900;// sf::VideoMode::getDesktopMode().height;

	//Запускаем рабочий поток, выполняющий просчет взаимодействия элементалей
	sf::Thread universeWorkerThread(&universe_worker_func);
	universeWorkerThread.launch();
	sf::Thread universeControlThread(&universe_control_func);
	universeControlThread.launch();

	sf::ContextSettings settings;
	settings.antialiasingLevel		= 0;
	//sf::RenderWindow		window(sf::VideoMode(world.width, world.heihgt), "Universe", sf::Style::Fullscreen, settings);
	sf::RenderWindow		window(sf::VideoMode(world.params.width, world.params.height), "Universe", sf::Style::Default, settings);
	//Поскольку рендер выполняется в отдельном потоке, то в этом потоке окно делаем неактивным
	window.setActive(false);

	//Запускаем рабочий поток рендера сцены
	sf::Thread renderThread(&universe_render_func, &window);
	renderThread.launch();

	while (window.isOpen())
	{
		//Обработка событий от системы (выход по нажатию кнопки)
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)		window.close();
			}
		}
		sf::sleep(sf::milliseconds(100));
	}

	//Уничтожаем рабочий поток просчета взаимодействия элементалей
	universeWorkerThread.terminate();
	universeControlThread.terminate();
	//Уничтожаем поток рендера сцены
	renderThread.terminate();
	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
