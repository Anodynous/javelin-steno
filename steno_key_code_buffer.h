//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

#include "dictionary/dictionary.h"
#include "list.h"
#include "orthography.h"
#include "segment.h"
#include "state.h"
#include "steno_key_code.h"

//---------------------------------------------------------------------------

class StenoCompiledOrthography;

//---------------------------------------------------------------------------

// Large statically allocated buffers to avoid fragmentation preventing them
// from being allocated.
//
// StenoTokens are converted directly into these buffers, and functions are
// applied directly on them.
class StenoKeyCodeBuffer {
public:
  void Populate(StenoTokenizer *tokenizer);
  void Append(StenoTokenizer *tokenizer);

  static const size_t BUFFER_SIZE = 8192;

  const StenoCompiledOrthography *orthography;
  StenoDictionary *rootDictionary;

  size_t count = 0;
  size_t addTranslationCount = 0;
  size_t resetStateCount = 0;
  StenoState state;
  StenoKeyCode buffer[BUFFER_SIZE];

  void Reset();

  void ProcessText(const char *text);
  void ProcessCommand(const char *command);
  void ProcessOrthographicSuffix(const char *text, size_t length);

  void AppendText(const char *p, size_t n, StenoCaseMode outputCaseMode,
                  bool hasManualStateChange = false);

  char *ToString();
  char *ToUnresolvedString();

  static bool IsGlue(const char *p);

  bool ProcessKeyPresses(const char *p, const char *end);

  void RetroactiveCapitalize(int count);
  void RetroactiveTitleCase(int count);
  void RetroactiveUpperCase(int count);
  void RetroactiveLowerCase(int count);
  void RetroactiveQuotes(int count, const char *startQuote,
                         const char *endQuote);
  void RetroactiveDeleteSpace();

  bool ProcessFunction(const List<char *> &parameters);

  // parameters[0] == function name.
  bool AddTranslationFunction(const List<char *> &parameters);
  bool DisableDictionaryFunction(const List<char *> &parameters);
  bool EnableDictionaryFunction(const List<char *> &parameters);
  bool KeyboardLayoutFunction(const List<char *> &parameters);
  bool ResetStateFunction(const List<char *> &parameters);
  bool RetroCapitalizeFunction(const List<char *> &parameters);
  bool RetroDoubleQuotesFunction(const List<char *> &parameters);
  bool RetroLowerCaseFunction(const List<char *> &parameters);
  bool RetroSingleQuotesFunction(const List<char *> &parameters);
  bool RetroTitleCaseFunction(const List<char *> &parameters);
  bool RetroUpperCaseFunction(const List<char *> &parameters);
  bool SetCaseFunction(const List<char *> &parameters);
  bool SetSpaceFunction(const List<char *> &parameters);
  bool ToggleDictionaryFunction(const List<char *> &parameters);
  bool UnicodeFunction(const List<char *> &parameters);

  void operator=(const StenoKeyCodeBuffer &o);

private:
  static void Reverse(StenoKeyCode *start, StenoKeyCode *end);
};

//---------------------------------------------------------------------------
