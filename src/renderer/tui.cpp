#include "renderer.hpp"
#include <cstdint>
#include <cstring>
#include <unistd.h>
constexpr const char* chess_pieces[32] = {
    "  ", "♙ ", "♖ ", "♘ ", "♗ ", "♕ ", "♖ ", "♔ ", "  ", "♟ ", "♜ ", "♞ ", "♝ ", "♛ ", "♜ ", "♚ ",
    "  ", "♟ ", "♜ ", "♞ ", "♝ ", "♛ ", "♜ ", "♚ ", "  ", "♙ ", "♖ ", "♘ ", "♗ ", "♕ ", "♖ ", "♔ ",
};
namespace Renderer {
TUI::TUI(int fd, Engine::Game& game) : m_game(game), m_fd(fd) {
	::write(m_fd, "\33[?47l", strlen("\33[?47l"));
}
TUI::~TUI() {
	::write(m_fd, "\33[?47h", strlen("\33[?47h"));
}
void TUI::write(const char* str) {
	size_t length = strlen(str);
	std::memcpy(m_cursor, str, length);
	m_cursor += length;
}
void TUI::commit() {
	::write(m_fd, m_buffer, m_cursor - m_buffer);
	m_cursor = m_buffer;
}
void TUI::render() {
	bool is_black{false};
	write("\33[H\33[J  A B C D E F G H\n");
	for (int8_t rank = 7; rank >= 0; rank--) {
		char rank_text[3] = {static_cast<char>('1' + rank), ' ', '\0'};
		write(rank_text);
		for (int8_t file = 0; file < 8; file++) {
			write(is_black ? "\33[40m\33[37m" : "\33[47m\33[30m");
			write(chess_pieces[(is_black ? 16 : 0) + static_cast<uint8_t>(m_game.get_board().get_piece(file, rank))]);
			is_black = !is_black;
		}
		is_black = !is_black;
		write("\33[0m ");
		write(rank_text);
		write("\n");
	}
	write("  A B C D E F G H\n");
	if (m_game.get_player() == Engine::Color::white)
		write("white: ");
	else
		write("black: ");
	commit();
}
} // namespace Renderer
