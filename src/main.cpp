#include "engine/engine.hpp"
#include "renderer/renderer.hpp"
int main() {
	Engine::Game game{new Engine::RandomPlayer(), new Engine::RandomPlayer()};
	Renderer::TUI tui{1, game};
	while (true) {
		tui.render();
		game.advance_turn();
	}
}
