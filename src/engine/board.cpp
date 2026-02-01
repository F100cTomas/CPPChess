#include "engine.hpp"
#include <cstring>
constexpr uint32_t initial_position[8] = {
    0xABCDFCBA, // rank 8
    0x99999999, // rank 7
    0x00000000, // rank 6
    0x00000000, // rank 5
    0x00000000, // rank 4
    0x00000000, // rank 3
    0x11111111, // rank 2
    0x23457432, // rank 1
};
namespace Engine {
Board::Board() {
	std::memcpy(m_data, initial_position, 32);
}
void Board::set_piece(uint8_t index, Piece piece) {
	uint32_t&     row   = m_data[index / 8];
	const uint8_t shift = (index % 8) * 4;
	row &= ~(0x0F << shift);
	row |= (static_cast<uint32_t>(piece) << shift);
}
void Board::make_move(Move move) {
	Piece moved = get_piece(move.from());
	switch (moved) {
	case Piece::empty:
	case Piece::en_passant:
	case Piece::white_knight:
	case Piece::black_knight:
	case Piece::white_bishop:
	case Piece::black_bishop:
	case Piece::white_rook_moved:
	case Piece::black_rook_moved:
	case Piece::white_queen:
	case Piece::black_queen: break;
	case Piece::white_rook_unmoved: moved = Piece::white_rook_moved; break;
	case Piece::black_rook_unmoved: moved = Piece::black_rook_moved; break;
	case Piece::white_pawn:
		if (move.from() >= 48 && move.to() < 40) {
			set_piece(move.from() - 8, Piece::en_passant);
			break;
		}
		if (move.from() < 16) {
			for (uint8_t i = 16; i < 24; i++) {
				if (get_piece(i) == Piece::en_passant)
					set_piece(i, Piece::empty);
			}
			set_piece(move.from(), Piece::empty);
			switch (move.to() / 8) {
			default: set_piece(move.to(), Piece::white_queen); return;
			case 1: set_piece(move.to() - 8, Piece::white_knight); return;
			case 2: set_piece(move.to() - 16, Piece::white_rook_moved); return;
			case 3: set_piece(move.to() - 24, Piece::white_bishop); return;
			}
		}
		break;
	case Piece::black_pawn:
		if (move.from() < 16 && move.to() >= 24) {
			set_piece(move.from() + 8, Piece::en_passant);
			break;
		}
		if (move.from() >= 48) {
			for (uint8_t i = 40; i < 48; i++) {
				if (get_piece(i) == Piece::en_passant)
					set_piece(i, Piece::empty);
			}
			set_piece(move.from(), Piece::empty);
			switch (move.to() / 8) {
			default: set_piece(move.to(), Piece::white_queen); return;
			case 6: set_piece(move.to() + 8, Piece::white_knight); return;
			case 5: set_piece(move.to() + 16, Piece::white_rook_moved); return;
			case 4: set_piece(move.to() + 24, Piece::white_bishop); return;
			}
		}
		break;
	case Piece::white_king:
		if (move.from() == 59) {
			if (move.to() == 61) {
				if (get_piece(56) == Piece::white_rook_unmoved)
					set_piece(56, Piece::white_rook_moved);
				set_piece(63, Piece::empty);
				set_piece(60, Piece::white_rook_moved);
				break;
			}
			if (move.to() == 57) {
				if (get_piece(63) == Piece::white_rook_unmoved)
					set_piece(63, Piece::white_rook_moved);
				set_piece(56, Piece::empty);
				set_piece(58, Piece::white_rook_moved);
				break;
			}
		}
		if (get_piece(56) == Piece::white_rook_unmoved)
			set_piece(56, Piece::white_rook_moved);
		if (get_piece(63) == Piece::white_rook_unmoved)
			set_piece(63, Piece::white_rook_moved);
		break;
	case Piece::black_king:
		if (move.from() == 3) {
			if (move.to() == 5) {
				if (get_piece(0) == Piece::black_rook_unmoved)
					set_piece(0, Piece::black_rook_moved);
				set_piece(7, Piece::empty);
				set_piece(4, Piece::black_rook_moved);
				break;
			}
			if (move.to() == 1) {
				if (get_piece(7) == Piece::black_rook_unmoved)
					set_piece(7, Piece::black_rook_moved);
				set_piece(0, Piece::empty);
				set_piece(2, Piece::black_rook_moved);
				break;
			}
		}
		if (get_piece(0) == Piece::black_rook_unmoved)
			set_piece(0, Piece::black_rook_moved);
		if (get_piece(7) == Piece::black_rook_unmoved)
			set_piece(7, Piece::black_rook_moved);
		break;
	}
	for (uint8_t i = 16; i < 24; i++) {
		if (get_piece(i) == Piece::en_passant)
			set_piece(i, Piece::empty);
	}
	for (uint8_t i = 40; i < 48; i++) {
		if (get_piece(i) == Piece::en_passant)
			set_piece(i, Piece::empty);
	}
	set_piece(move.to(), moved);
	set_piece(move.from(), Piece::empty);
}
} // namespace Engine
