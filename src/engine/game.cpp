#include "engine.hpp"
namespace Engine {
Game::Game(Player* white, Player* black) : m_white(white), m_black(black) {}
void Game::advance_turn() {
	AvailableMoves available{m_board, m_turn};
	Move move = (m_turn == Color::white ? m_white : m_black)->get_move(available);
	if (move.is_capture(m_board))
		m_50_move_timer = 0;
	else
		m_50_move_timer++;
	m_turn_number++;
	m_board.make_move(move);
	m_turn = (m_turn == Color::white ? Color::black : Color::white);
}
} // namespace Engine
