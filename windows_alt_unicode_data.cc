//---------------------------------------------------------------------------

#include "windows_alt_unicode_data.h"
#include <stddef.h>

//---------------------------------------------------------------------------

struct WindowsAltUnicodeEntry {
  uint16_t unicode;
  uint16_t alt;
};

// clang-format off
static const WindowsAltUnicodeEntry DATA[] = {
    {0x00A0, 255}, // ' '
    {0x00A1, 173}, // '¡'
    {0x00A2, 155}, // '¢'
    {0x00A3, 156}, // '£'
    {0x00A5, 157}, // '¥'
    {0x00A7, 21},  // '§'
    {0x00AA, 166}, // 'ª'
    {0x00AB, 174}, // '«'
    {0x00AC, 170}, // '¬'
    {0x00B0, 248}, // '°'
    {0x00B1, 241}, // '±'
    {0x00B2, 253}, // '2'
    {0x00B5, 230}, // 'μ'
    {0x00B6, 20},  // '¶'
    {0x00B7, 250}, // '·'
    {0x00BA, 167}, // 'º'
    {0x00BB, 175}, // '»'
    {0x00BC, 172}, // '¼'
    {0x00BD, 171}, // '½'
    {0x00BF, 168}, // '¿'
    {0x00C1, 181}, // 'Á'
    {0x00C4, 142}, // 'Ä'
    {0x00C5, 143}, // 'Å'
    {0x00C6, 146}, // 'Æ'
    {0x00C7, 128}, // 'Ç'
    {0x00C9, 144}, // 'É'
    {0x00D1, 165}, // 'Ñ'
    {0x00D6, 153}, // 'Ö'
    {0x00DC, 154}, // 'Ü'
    {0x00DF, 225}, // 'ß'
    {0x00E0, 133}, // 'à'
    {0x00E1, 160}, // 'á'
    {0x00E2, 131}, // 'â'
    {0x00E4, 132}, // 'ä'
    {0x00E5, 134}, // 'å'
    {0x00E6, 145}, // 'æ'
    {0x00E7, 135}, // 'ç'
    {0x00E8, 138}, // 'è'
    {0x00E9, 130}, // 'é'
    {0x00EA, 136}, // 'ê'
    {0x00EB, 137}, // 'ë'
    {0x00EC, 141}, // 'ì'
    {0x00ED, 161}, // 'í'
    {0x00EE, 140}, // 'î'
    {0x00EF, 139}, // 'ï'
    {0x00F1, 164}, // 'ñ'
    {0x00F2, 149}, // 'ò'
    {0x00F3, 162}, // 'ó'
    {0x00F4, 147}, // 'ô'
    {0x00F6, 148}, // 'ö'
    {0x00F7, 246}, // '÷'
    {0x00F9, 151}, // 'ù'
    {0x00FA, 163}, // 'ú'
    {0x00FB, 150}, // 'û'
    {0x00FC, 129}, // 'ü'
    {0x00FF, 152}, // 'ÿ'
    {0x0192, 159}, // 'ƒ'
    {0x0393, 226}, // 'Γ'
    {0x0398, 233}, // 'Θ'
    {0x03A3, 228}, // 'Σ'
    {0x03A6, 232}, // 'Φ'
    {0x03A9, 234}, // 'Ω'
    {0x03B1, 224}, // 'α'
    {0x03B4, 235}, // 'δ'
    {0x03B5, 238}, // 'ε'
    {0x03C0, 227}, // 'π'
    {0x03C3, 229}, // 'σ'
    {0x03C4, 231}, // 'τ'
    {0x03C6, 237}, // 'φ'
    {0x2022, 7},   // '•'
    {0x203C, 19},  // '‼'
    {0x207F, 252}, // 'n'
    {0x20A7, 158}, // '₧'
    {0x2190, 27},  // '←'
    {0x2191, 24},  // '↑'
    {0x2192, 26},  // '→'
    {0x2193, 25},  // '↓'
    {0x2194, 29},  // '↔'
    {0x2195, 18},  // '↕'
    {0x21A8, 23},  // '↨'
    {0x2219, 249}, // '∙'
    {0x221A, 251}, // '√'
    {0x221E, 236}, // '∞'
    {0x221F, 28},  // '∟'
    {0x2229, 239}, // '∩'
    {0x2248, 247}, // '≈'
    {0x2261, 240}, // '≡'
    {0x2264, 243}, // '≤'
    {0x2265, 242}, // '≥'
    {0x2302, 127}, // '⌂'
    {0x2310, 169}, // '⌐'
    {0x2320, 244}, // '⌠'
    {0x2321, 245}, // '⌡'
    {0x2500, 196}, // '─'
    {0x2502, 179}, // '│'
    {0x250C, 218}, // '┌'
    {0x2510, 191}, // '┐'
    {0x2514, 192}, // '└'
    {0x2518, 217}, // '┘'
    {0x251C, 195}, // '├'
    {0x2524, 180}, // '┤'
    {0x252C, 194}, // '┬'
    {0x2534, 193}, // '┴'
    {0x253C, 197}, // '┼'
    {0x2550, 205}, // '═'
    {0x2551, 186}, // '║'
    {0x2552, 213}, // '╒'
    {0x2553, 214}, // '╓'
    {0x2554, 201}, // '╔'
    {0x2555, 184}, // '╕'
    {0x2556, 183}, // '╖'
    {0x2557, 187}, // '╗'
    {0x2558, 212}, // '╘'
    {0x2559, 211}, // '╙'
    {0x255A, 200}, // '╚'
    {0x255B, 190}, // '╛'
    {0x255C, 189}, // '╜'
    {0x255D, 188}, // '╝'
    {0x255E, 198}, // '╞'
    {0x255F, 199}, // '╟'
    {0x2560, 204}, // '╠'
    {0x2562, 182}, // '╢'
    {0x2563, 185}, // '╣'
    {0x2564, 209}, // '╤'
    {0x2565, 210}, // '╥'
    {0x2566, 203}, // '╦'
    {0x2567, 207}, // '╧'
    {0x2568, 208}, // '╨'
    {0x2569, 202}, // '╩'
    {0x256A, 216}, // '╪'
    {0x256B, 215}, // '╫'
    {0x256C, 206}, // '╬'
    {0x2580, 223}, // '▀'
    {0x2584, 220}, // '▄'
    {0x2588, 219}, // '█'
    {0x258C, 221}, // '▌'
    {0x2590, 222}, // '▐'
    {0x2591, 176}, // '░'
    {0x2592, 177}, // '▒'
    {0x2593, 178}, // '▓'
    {0x25A0, 254}, // '■'
    {0x25AC, 22},  // '▬'
    {0x25B2, 30},  // '▲'
    {0x25BA, 16},  // '►'
    {0x25BC, 31},  // '▼'
    {0x25C4, 17},  // '◄'
    {0x25CB, 9},   // '○'
    {0x25D8, 8},   // '◘'
    {0x25D9, 10},  // '◙'
    {0x263A, 1},   // '☺'
    {0x263B, 2},   // '☻'
    {0x263C, 15},  // '☼'
    {0x2640, 12},  // '♀'
    {0x2642, 11},  // '♂'
    {0x2660, 6},   // '♠'
    {0x2663, 5},   // '♣'
    {0x2665, 3},   // '♥'
    {0x2666, 4},   // '♦'
    {0x266A, 13},  // '♪'
    {0x266B, 14},  // '♫'
};
// clang-format on

//---------------------------------------------------------------------------

uint32_t WindowsAltUnicodeData::GetAltCodeForUnicode(uint32_t unicode) {
  const WindowsAltUnicodeEntry *left = DATA;
  const WindowsAltUnicodeEntry *right = DATA + sizeof(DATA) / sizeof(*DATA);

  while (left < right) {
    const WindowsAltUnicodeEntry *mid = left + size_t(right - left) / 2;

    int compare = (int)unicode - (int)mid->unicode;
    if (compare < 0) {
      right = mid;
    } else if (compare == 0) {
      return mid->alt;
    } else {
      left = mid + 1;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"
#include <assert.h>

TEST_BEGIN("WindowsAltUnicodeData tests") {
  assert(WindowsAltUnicodeData::GetAltCodeForUnicode(0x9f) == 0);
  assert(WindowsAltUnicodeData::GetAltCodeForUnicode(0xa0) == 255);
  assert(WindowsAltUnicodeData::GetAltCodeForUnicode(0xa1) == 173);
  assert(WindowsAltUnicodeData::GetAltCodeForUnicode(0x266b) == 14);
  assert(WindowsAltUnicodeData::GetAltCodeForUnicode(0x266c) == 0);
}
TEST_END

//---------------------------------------------------------------------------
