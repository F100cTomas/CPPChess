#include "engine.hpp"
#include <cstdint>
#include <cstdio>
namespace Engine {
namespace {
constexpr uint64_t bit(uint8_t index) {
	if (index >= 64)
		return 0;
	return 1ULL << index;
}
constexpr bool en_passant(Piece piece, Color color) {
	return piece_color(piece) == color || piece == Piece::en_passant;
}
uint64_t white_pawn_moves(uint8_t index, const Board& board) {
	uint64_t result = (piece_color(board.get_piece(index - 8)) == Color::none ? bit(index - 8) : 0);
	if (index >= 48 && index < 56 && result != 0)
		result |= bit(index - 16);
	if (index % 8 != 7 && en_passant(board.get_piece(index - 7), Color::black))
		result |= bit(index - 7);
	if (index % 8 != 0 && en_passant(board.get_piece(index - 9), Color::black))
		result |= bit(index - 9);
	if (index < 16) {
		for (uint8_t i = 0; i < 8; i++) {
			if ((result >> i) & 1) {
				result |= bit(i + 8) | bit(i + 16) | bit(i + 24);
			}
		}
	}
	return result;
}
uint64_t black_pawn_moves(uint8_t index, const Board& board) {
	uint64_t result = (piece_color(board.get_piece(index + 8)) == Color::none ? bit(index + 8) : 0);
	if (index >= 8 && index < 16 && result != 0)
		result |= bit(index + 16);
	if (index % 8 != 0 && piece_color(board.get_piece(index - 7)) == Color::white)
		result |= bit(index + 7);
	if (index % 8 != 7 && piece_color(board.get_piece(index - 9)) == Color::white)
		result |= bit(index + 9);
	if (index >= 48) {
		for (uint8_t i = 56; i < 64; i++) {
			if ((result >> i) & 1) {
				result |= bit(i - 8) | bit(i - 16) | bit(i - 24);
			}
		}
	}
	return result;
}
uint64_t knight_moves(uint8_t index, const Board& board) {
	uint64_t result =
	    (index % 8 > 0 ? bit(index - 17) | bit(index + 15) : 0) | (index % 8 < 7 ? bit(index - 15) | bit(index + 17) : 0)
	    | (index % 8 > 1 ? bit(index - 10) | bit(index + 6) : 0) | (index % 8 < 6 ? bit(index - 6) | bit(index + 10) : 0);
	const Color color = piece_color(board.get_piece(index));
	for (uint8_t i = 0; i < 64; i++) {
		if (((result >> i) & 1ULL) && color == piece_color(board.get_piece(i))) {
			result &= ~bit(i);
		}
	}
	return result;
}
uint64_t beam_moves(uint8_t index, const Board& board, int8_t dx, int8_t dy) {
	const Color color  = piece_color(board.get_piece(index));
	uint64_t    result = 0;
	uint8_t     file   = dx + index % 8;
	while (file < 8 && index < 64) {
		index += 8 * dy + dx;
		if (piece_color(board.get_piece(index)) == color)
			return result;
		result |= bit(index);
		if (piece_color(board.get_piece(index)) != Color::none)
			return result;
		file += dx;
	}
	return result;
}
uint64_t bishop_moves(uint8_t index, const Board& board) {
	return beam_moves(index, board, 1, 1) | beam_moves(index, board, -1, 1) | beam_moves(index, board, 1, -1)
	       | beam_moves(index, board, -1, -1);
}
uint64_t rook_moves(uint8_t index, const Board& board) {
	return beam_moves(index, board, 1, 0) | beam_moves(index, board, 0, 1) | beam_moves(index, board, -1, 0)
	       | beam_moves(index, board, 0, -1);
}
uint64_t queen_moves(uint8_t index, const Board& board) {
	return beam_moves(index, board, 1, 0) | beam_moves(index, board, 0, 1) | beam_moves(index, board, -1, 0)
	       | beam_moves(index, board, 0, -1) | beam_moves(index, board, 1, 1) | beam_moves(index, board, -1, 1)
	       | beam_moves(index, board, 1, -1) | beam_moves(index, board, -1, -1);
}
uint64_t white_king_castles(const Board& board) {
	uint64_t result = 0;
	if (piece_color(board.get_piece(60)) == Color::none && piece_color(board.get_piece(61)) == Color::none
	    && piece_color(board.get_piece(62)) == Color::none && board.get_piece(63) == Piece::white_rook_unmoved)
		result |= bit(61);
	if (piece_color(board.get_piece(58)) == Color::none && piece_color(board.get_piece(57)) == Color::none
	    && board.get_piece(56) == Piece::white_rook_unmoved)
		result |= bit(57);
	return result;
}
uint64_t black_king_castles(const Board& board) {
	uint64_t result = 0;
	if (piece_color(board.get_piece(4)) == Color::none && piece_color(board.get_piece(5)) == Color::none
	    && piece_color(board.get_piece(6)) == Color::none && board.get_piece(7) == Piece::black_rook_unmoved)
		result |= bit(5);
	if (piece_color(board.get_piece(2)) == Color::none && piece_color(board.get_piece(1)) == Color::none
	    && board.get_piece(0) == Piece::black_rook_unmoved)
		result |= bit(1);
	return result;
}
uint64_t king_moves(uint8_t index, const Board& board) {
	uint64_t result = (bit(index - 8) | bit(index + 8))
	                  | (index % 8 > 0 ? bit(index + 9) | bit(index + 1) | bit(index - 7) : 0)
	                  | (index % 8 < 7 ? bit(index + 7) | bit(index - 1) | bit(index - 9) : 0);
	const Color color = piece_color(board.get_piece(index));
	for (uint8_t i = 0; i < 64; i++) {
		if (((result >> i) & 1ULL) && color == piece_color(board.get_piece(i))) {
			result &= ~bit(i);
		}
	}
	if (board.get_piece(index) == Piece::white_king && index == 59)
		result |= white_king_castles(board);
	else if (board.get_piece(index) == Piece::black_king && index == 3)
		result |= black_king_castles(board);
	return result;
}
/*
Piece test_tile(const Board& board, uint8_t tile, uint8_t min_file, uint8_t max_file) {
	if (tile >= 64 || tile % 8 < min_file || tile % 8 > max_file)
		return Piece::empty;
	return board.get_piece(tile);
}
uint64_t is_tile_in_danger_from_diagonal(const Board& board, uint8_t tile, Color color, int8_t dx, int8_t dy) {
  if (tile >= 64)
    return 0xFFFFFFFFFFFFFFFF;
  uint64_t result = 0;
  uint8_t  file   = dx + tile % 8;
  while (file < 8 && tile < 64) {
    tile += 8 * dy + dx;
    result |= bit(tile);
    Piece piece = board.get_piece(tile);
    if (piece_color(piece) != Color::none) {
      if ((color == Color::white && (piece == Piece::black_bishop || piece == Piece::black_queen))
          || (color == Color::black && (piece == Piece::white_bishop || piece == Piece::white_queen))) {
        printf("%lX\n", result);
        exit(1);
        return result;
      }
      return 0xFFFFFFFFFFFFFFFF;
    }
    file += dx;
  }
  return 0xFFFFFFFFFFFFFFFF;
}
uint64_t is_tile_in_danger_from_orthogonal(const Board& board, uint8_t tile, Color color, int8_t dx, int8_t dy) {
  if (tile >= 64)
    return 0xFFFFFFFFFFFFFFFF;
  uint64_t result = 0;
  uint8_t  file   = dx + tile % 8;
  while (file < 8 && tile < 64) {
    tile += 8 * dy + dx;
    result |= bit(tile);
    Piece piece = board.get_piece(tile);
    if (piece_color(piece) != Color::none) {
      if ((color == Color::white
           && (piece == Piece::black_rook_moved || piece == Piece::black_rook_unmoved || piece == Piece::black_queen))
          || (color == Color::black
              && (piece == Piece::white_rook_moved || piece == Piece::white_rook_unmoved
                  || piece == Piece::white_queen))) {
        printf("%lX\n", result);
        exit(1);
        return result;
      }
      return 0xFFFFFFFFFFFFFFFF;
    }
    file += dx;
  }
  return 0xFFFFFFFFFFFFFFFF;
}
*/
uint64_t is_tile_in_danger(const Board& board, uint8_t tile, Color color) {
	/*
	if (tile >= 64)
	  return 0xFFFFFFFFFFFFFFFF;
	uint64_t    result = 0xFFFFFFFFFFFFFFFF;
	const Piece tile1  = test_tile(board, tile - 17, 1, 7);
	if ((color == Color::white && tile1 == Piece::black_knight)
	    || (color == Color::black && tile1 == Piece::white_knight))
	  result &= bit(tile - 17);
	if (result == 0)
	  exit(1);
	const Piece tile2 = test_tile(board, tile - 15, 0, 6);
	if ((color == Color::white && tile2 == Piece::black_knight)
	    || (color == Color::black && tile2 == Piece::white_knight))
	  result &= bit(tile - 15);
	if (result == 0)
	  exit(1);
	const Piece tile3 = test_tile(board, tile - 10, 2, 7);
	if ((color == Color::white && tile3 == Piece::black_knight)
	    || (color == Color::black && tile3 == Piece::white_knight))
	  result &= bit(tile - 10);
	if (result == 0)
	  exit(1);
	const Piece tile4 = test_tile(board, tile - 9, 1, 7);
	if ((color == Color::white && (tile4 == Piece::black_pawn || tile4 == Piece::black_king))
	    || (color == Color::black && tile4 == Piece::white_king))
	  result &= bit(tile - 9);
	if (result == 0)
	  exit(1);
	const Piece tile5 = test_tile(board, tile - 8, 0, 7);
	if ((color == Color::white && tile5 == Piece::black_king) || (color == Color::black && tile5 == Piece::white_king))
	  result &= bit(tile - 8);
	if (result == 0)
	  exit(1);
	const Piece tile6 = test_tile(board, tile - 7, 0, 6);
	if ((color == Color::white && (tile6 == Piece::black_pawn || tile6 == Piece::black_king))
	    || (color == Color::black && tile6 == Piece::white_king))
	  result &= bit(tile - 7);
	if (result == 0)
	  exit(1);
	const Piece tile7 = test_tile(board, tile - 6, 0, 5);
	if ((color == Color::white && tile7 == Piece::black_knight)
	    || (color == Color::black && tile7 == Piece::white_knight))
	  result &= bit(tile - 6);
	if (result == 0)
	  exit(1);
	const Piece tile8 = test_tile(board, tile - 1, 1, 7);
	if ((color == Color::white && tile8 == Piece::black_king) || (color == Color::black && tile8 == Piece::white_king))
	  result &= bit(tile - 1);
	if (result == 0)
	  exit(1);
	const Piece tile9 = test_tile(board, tile + 1, 0, 6);
	if ((color == Color::white && tile9 == Piece::black_king) || (color == Color::black && tile9 == Piece::white_king))
	  result &= bit(tile + 1);
	if (result == 0)
	  exit(1);
	const Piece tile10 = test_tile(board, tile + 6, 2, 7);
	if ((color == Color::white && tile10 == Piece::black_knight)
	    || (color == Color::black && tile10 == Piece::white_knight))
	  result &= bit(tile + 6);
	if (result == 0)
	  exit(1);
	const Piece tile11 = test_tile(board, tile + 7, 1, 7);
	if ((color == Color::white && tile11 == Piece::black_king)
	    || (color == Color::black && (tile11 == Piece::white_pawn || tile11 == Piece::white_king)))
	  result &= bit(tile + 7);
	if (result == 0)
	  exit(1);
	const Piece tile12 = test_tile(board, tile + 8, 0, 7);
	if ((color == Color::white && tile12 == Piece::black_king) || (color == Color::black && tile12 == Piece::white_king))
	  result &= bit(tile + 8);
	if (result == 0)
	  exit(1);
	const Piece tile13 = test_tile(board, tile + 9, 0, 6);
	if ((color == Color::white && (tile13 == Piece::black_king))
	    || (color == Color::black && (tile13 == Piece::white_pawn || tile13 == Piece::white_king)))
	  result &= bit(tile + 9);
	if (result == 0)
	  exit(1);
	const Piece tile14 = test_tile(board, tile + 10, 0, 5);
	if ((color == Color::white && tile14 == Piece::black_knight)
	    || (color == Color::black && tile14 == Piece::white_knight))
	  result &= bit(tile + 10);
	if (result == 0)
	  exit(1);
	const Piece tile15 = test_tile(board, tile + 15, 1, 7);
	if ((color == Color::white && tile15 == Piece::black_knight)
	    || (color == Color::black && tile15 == Piece::white_knight))
	  result &= bit(tile + 15);
	if (result == 0)
	  exit(1);
	const Piece tile16 = test_tile(board, tile + 17, 0, 6);
	if ((color == Color::white && tile16 == Piece::black_knight)
	    || (color == Color::black && tile16 == Piece::white_knight))
	  result &= bit(tile + 17);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_diagonal(board, tile, color, -1, -1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_orthogonal(board, tile, color, 0, -1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_diagonal(board, tile, color, 1, -1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_orthogonal(board, tile, color, 1, 0);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_diagonal(board, tile, color, 1, 1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_orthogonal(board, tile, color, 0, 1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_diagonal(board, tile, color, -1, 1);
	if (result == 0)
	  exit(1);
	result &= is_tile_in_danger_from_orthogonal(board, tile, color, -1, 0);
	if (result == 0)
	  exit(1);
	return result;
	*/
	return 0xFFFFFFFFFFFFFFFF;
}
void generate_pin_squares(uint64_t (&moves)[64], const Board& board, uint8_t king_index, Color king_color) {
	uint64_t king_attacks = is_tile_in_danger(board, king_index, king_color);
	if (king_attacks != 0xFFFFFFFFFFFFFFFF) {
		for (uint8_t i = 0; i < 64; i++)
			moves[i] &= king_attacks;
	}
	uint64_t& king_moves = moves[king_index];
	for (uint8_t i = 0; i < 64; i++) {
		if (((king_moves >> i) & 1) && is_tile_in_danger(board, i, king_color) != 0xFFFFFFFFFFFFFFFF)
			king_moves &= ~bit(i);
	}
}
} // namespace
AvailableMoves::MoveIterator::MoveIterator(const AvailableMoves& moves, bool is_end) :
    m_moves(moves), m_piece(is_end ? 64 : 0) {
	if (!is_end)
		++(*this);
}
void AvailableMoves::MoveIterator::operator++() {
	do {
		m_dest++;
		if (m_dest >= 64) {
			m_piece++;
			m_dest = 0;
		}
	} while (m_piece < 64 && !m_moves.move_possible(Move(m_piece, m_dest)));
}
AvailableMoves::AvailableMoves(const Board& board, Color player) {
	uint8_t king_index{255};
	for (uint8_t i = 0; i < 64; i++) {
		const Piece piece = board.get_piece(i);
		if (piece_color(piece) != player) {
			m_moves[i] = 0;
			continue;
		}
		switch (piece) {
		case Piece::empty:
		case Piece::en_passant: m_moves[i] = 0; break;
		case Piece::white_pawn: m_moves[i] = white_pawn_moves(i, board); break;
		case Piece::black_pawn: m_moves[i] = black_pawn_moves(i, board); break;
		case Piece::white_knight:
		case Piece::black_knight: m_moves[i] = knight_moves(i, board); break;
		case Piece::white_bishop:
		case Piece::black_bishop: m_moves[i] = bishop_moves(i, board); break;
		case Piece::white_rook_unmoved:
		case Piece::white_rook_moved:
		case Piece::black_rook_unmoved:
		case Piece::black_rook_moved: m_moves[i] = rook_moves(i, board); break;
		case Piece::white_queen:
		case Piece::black_queen: m_moves[i] = queen_moves(i, board); break;
		case Piece::white_king:
		case Piece::black_king:
			king_index = i;
			m_moves[i] = king_moves(i, board);
			break;
		}
	}
	if (king_index == 255) {
		for (uint8_t i = 0; i < 64; i++)
			m_moves[i] = 0;
		return;
	}
	generate_pin_squares(m_moves, board, king_index, player);
}
bool AvailableMoves::move_possible(Move move) const {
	return m_moves[move.from()] & (1ULL << move.to());
}
} // namespace Engine
