//---------------------------------------------------------------------------

#include "state.h"

//---------------------------------------------------------------------------

const StenoCaseMode StenoState::NEXT_WORD_CASE_MODE[] = {
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER,       //
    StenoCaseMode::UPPER,       //
    StenoCaseMode::TITLE,       //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::UNSPECIFIED, //
};

const StenoCaseMode StenoState::NEXT_LETTER_CASE_MODE[] = {
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER,       //
    StenoCaseMode::UPPER,       //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER_ONCE,  //
    StenoCaseMode::UPPER_ONCE,  //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::UNSPECIFIED, //
};

//---------------------------------------------------------------------------
