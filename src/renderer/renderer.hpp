#pragma once
#include "../engine/engine.hpp"
namespace Renderer {
class TUI {
	Engine::Game&       m_game;
	int                m_fd{1};
	char*              m_cursor{m_buffer};
	char               m_buffer[1024];

private:
	void write(const char* str);
	void commit();

public:
	TUI(int fd, Engine::Game& game);
	~TUI();
	void render();
};
} // namespace Renderer
