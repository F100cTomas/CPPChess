#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
namespace Engine {
enum class Color : uint8_t { white, black, none };
enum class Piece : uint8_t {
	empty              = 0x00,
	white_pawn         = 0x01,
	white_rook_unmoved = 0x02,
	white_knight       = 0x03,
	white_bishop       = 0x04,
	white_queen        = 0x05,
	white_rook_moved   = 0x06,
	white_king         = 0x07,
	en_passant         = 0x08,
	black_pawn         = 0x09,
	black_rook_unmoved = 0x0A,
	black_knight       = 0x0B,
	black_bishop       = 0x0C,
	black_queen        = 0x0D,
	black_rook_moved   = 0x0E,
	black_king         = 0x0F,
};
constexpr Color piece_color(Piece piece) {
	if ((static_cast<uint8_t>(piece) & 0x07) == 0x00)
		return Color::none;
	return (static_cast<uint8_t>(piece) & 0x08) == 0 ? Color::white : Color::black;
}
class Move;
class AvailableMoves;
class Board {
	uint32_t m_data[8];

public:
	Board();
	Board(const Board&) = delete;

private:
	void set_piece(uint8_t index, Piece piece);

public:
	inline Piece get_piece(uint8_t index) const {
		return static_cast<Piece>((m_data[index / 8] >> ((index % 8) * 4)) & 0x0F);
	}
	inline Piece get_piece(uint8_t file, uint8_t rank) const {
		const uint8_t shift = (7 - file) * 4;
		return static_cast<Piece>((m_data[7 - rank] >> shift) & 0x0F);
	}
	void make_move(Move move);
};
class Move {
	uint8_t m_from{255}, m_to{255};

public:
	inline Move(uint8_t from, uint8_t to) : m_from(from), m_to(to) {}
	inline Move(const Move& move) : m_from(move.m_from), m_to(move.m_to) {}

public:
	inline uint8_t from() const {
		return m_from;
	}
	inline uint8_t to() const {
		return m_to;
	}
	bool is_capture(const Board& board);
};
class AvailableMoves {
	uint64_t m_moves[64];
	class MoveIterator {
		const AvailableMoves& m_moves;
		uint8_t               m_piece{0};
		uint8_t               m_dest{0};

	public:
		MoveIterator(const AvailableMoves& moves, bool is_end);
		void operator++();
		Move operator*() {
			return Move(m_piece, m_dest);
		}
		bool operator!=(const MoveIterator& iterator) {
			return m_piece != iterator.m_piece || m_dest != iterator.m_dest;
		}
	};

public:
	AvailableMoves(const Board& board, Color player);
	inline MoveIterator begin() const {
		return MoveIterator(*this, false);
	}
	MoveIterator end() const {
		return MoveIterator(*this, true);
	}

public:
	bool move_possible(Move move) const;
};
class Player {
public:
	virtual inline ~Player() {}
	virtual Move get_move(const AvailableMoves& available) = 0;
};
class Game {
	Board    m_board{};
	Color    m_turn{Color::white};
	uint8_t  m_50_move_timer{0};
	uint16_t m_turn_number{0};
	Player * m_white{nullptr}, *m_black{nullptr};

public:
	Game(Player* white, Player* black);
	Game(const Game&) = delete;

public:
	inline const Board& get_board() const {
		return m_board;
	}
	inline Board& get_board() {
		return m_board;
	}
	inline Color get_player() const {
		return m_turn;
	}
	void advance_turn();
};
class TerminalPlayer : public Player {
public:
	inline TerminalPlayer() {}
	inline ~TerminalPlayer() {}
	inline uint8_t get_tile() {
		uint8_t file = getchar();
		uint8_t rank = getchar();
		return 8 * ('8' - rank) + ('H' - file);
	}
	inline Move get_move(const AvailableMoves& available) override {
		while (true) {
			Move result = Move(get_tile(), get_tile());
			while (getchar() != '\n')
				;
			if (available.move_possible(result))
				return result;
		}
	}
};
inline void exit(int code) {
	::exit(code);
}
class RandomPlayer : public Player {
public:
	inline RandomPlayer() {
		srand(time(NULL));
	}
	inline ~RandomPlayer() {}
	inline Move get_move(const AvailableMoves& available) override {
		std::vector<Move> moves{};
		for (Move move: available) {
			moves.push_back(move);
		}
		if (moves.size() == 0)
			exit(1);
		while (getchar() != '\n')
			;
		return moves[rand() % moves.size()];
	}
};
} // namespace Engine
