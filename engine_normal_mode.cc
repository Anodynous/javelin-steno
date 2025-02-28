//---------------------------------------------------------------------------

#include "clock.h"
#include "console.h"
#include "engine.h"
#include "key.h"
#include "segment.h"
#include "state.h"
#include "str.h"
#include "thread.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

#define ENABLE_PROFILE 0

//---------------------------------------------------------------------------

#if JAVELIN_THREADS

struct StenoEngine::UpdateNormalModeTextBufferThreadData {
  UpdateNormalModeTextBufferThreadData(StenoEngine *engine,
                                       ConversionBuffer *conversionBuffer,
                                       StenoSegmentList *segmentList,
                                       size_t startingOffset)
      : engine(engine), conversionBuffer(conversionBuffer),
        segmentList(segmentList), startingOffset(startingOffset) {}

  StenoEngine *engine;
  ConversionBuffer *conversionBuffer;
  StenoSegmentList *segmentList;
  size_t startingOffset;

  void ConvertText() {
    engine->ConvertText(*conversionBuffer, *segmentList, startingOffset);
  }

  static void ConvertTextEntryPoint(void *data) {
    ((UpdateNormalModeTextBufferThreadData *)data)->ConvertText();
  }
};

#endif

void StenoEngine::ProcessNormalModeStroke(StenoStroke stroke) {
  history.PruneIfFull();

  size_t previousSourceStrokeCount = history.GetCount();
  history.Add(stroke, state);

#if ENABLE_PROFILE
  uint32_t t0 = Clock::GetMicroseconds();
#endif

  size_t maximumConversionStrokes = dictionary.GetMaximumOutlineLength() +
                                    SEGMENT_CONVERSION_PREFIX_SUFFIX_LIMIT;
  size_t startingStroke = history.GetStartingStroke(maximumConversionStrokes);
  size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList nextSegmentList;
  CreateSegments(history.GetCount(), nextConversionBuffer, conversionCount,
                 nextSegmentList);
  history.UpdateDefinitionBoundaries(history.GetCount() - conversionCount,
                                     nextSegmentList);

#if ENABLE_PROFILE
  uint32_t t1 = Clock::GetMicroseconds();
#endif

  StenoSegmentList previousSegmentList;
  if (nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(previousSourceStrokeCount, previousConversionBuffer,
                   conversionCount - 1, previousSegmentList);
  } else {
    CreateSegmentsUsingLongerResult(previousSourceStrokeCount,
                                    previousConversionBuffer,
                                    conversionCount - 1, previousSegmentList,
                                    nextConversionBuffer, nextSegmentList);
  }

#if ENABLE_PROFILE
  uint32_t t2 = Clock::GetMicroseconds();
#endif

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegmentList, nextSegmentList);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if ENABLE_PROFILE
  uint32_t t3 = Clock::GetMicroseconds();
#endif

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer, &previousSegmentList, startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer, &nextSegmentList, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer, previousSegmentList, startingOffset);
  ConvertText(nextConversionBuffer, nextSegmentList, startingOffset);
#endif

#if ENABLE_PROFILE
  uint32_t t4 = Clock::GetMicroseconds();
#endif

  state = nextConversionBuffer.keyCodeBuffer.state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;

  if (nextConversionBuffer.keyCodeBuffer.addTranslationCount >
      previousConversionBuffer.keyCodeBuffer.addTranslationCount) {
    PrintPaperTape(stroke, previousSegmentList, nextSegmentList);

    history.RemoveBack();
    InitiateAddTranslationMode();
    return;
  }

#if ENABLE_PROFILE
  uint32_t t5 = Clock::GetMicroseconds();
#endif

  bool printSuggestions = true;
  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    history.SetBackCombineUndo();

    if (previousConversionBuffer.keyCodeBuffer.count ==
        nextConversionBuffer.keyCodeBuffer.count) {
      history.SetBackHasManualStateChange();
      if (state.caseMode != StenoCaseMode::NORMAL ||
          state.overrideCaseMode != StenoCaseMode::NORMAL) {
        printSuggestions = false;
      } else if (history.GetCount() >= 2) {
        StenoState previousState = history.Back(2).state;
        if (previousState.isManualStateChange) {
          printSuggestions = false;
        }
      }
    }
  }

#if ENABLE_PROFILE
  uint32_t t6 = Clock::GetMicroseconds();
#endif

  PrintTextLog(previousConversionBuffer.keyCodeBuffer,
               nextConversionBuffer.keyCodeBuffer);
  PrintPaperTape(stroke, previousSegmentList, nextSegmentList);
  if (printSuggestions) {
    PrintSuggestions(previousSegmentList, nextSegmentList);
  }

  if (nextConversionBuffer.keyCodeBuffer.resetStateCount >
      previousConversionBuffer.keyCodeBuffer.resetStateCount) {
    ResetState();
    return;
  }

#if ENABLE_PROFILE
  uint32_t t7 = Clock::GetMicroseconds();

  Console::Printf("Timings: %u %u %u %u %u %u %u\n\n", t1 - t0, t2 - t1,
                  t3 - t2, t4 - t3, t5 - t5, t6 - t5, t7 - t6);
#endif
}

void StenoEngine::ProcessNormalModeUndo() {
  size_t maximumConversionStrokes = dictionary.GetMaximumOutlineLength() +
                                    SEGMENT_CONVERSION_PREFIX_SUFFIX_LIMIT;

  size_t undoCount = history.GetUndoCount(maximumConversionStrokes);
  if (undoCount == 0) {
    Key::Press(KeyCode::BACKSPACE);
    Key::Release(KeyCode::BACKSPACE);
    PrintPaperTapeUndo(0);
    return;
  }

  size_t startingStroke = history.GetStartingStroke(maximumConversionStrokes);
  size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList previousSegmentList;
  CreateSegments(history.GetCount(), previousConversionBuffer, conversionCount,
                 previousSegmentList);

  state = history.Back(undoCount).state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;
  history.RemoveBack(undoCount);

  size_t nextConversionCount =
      undoCount >= conversionCount ? 0 : conversionCount - undoCount;

  StenoSegmentList nextSegmentList;
  if (previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(history.GetCount(), nextConversionBuffer,
                   nextConversionCount, nextSegmentList);

  } else {
    CreateSegmentsUsingLongerResult(
        history.GetCount(), nextConversionBuffer, nextConversionCount,
        nextSegmentList, previousConversionBuffer, previousSegmentList);
  }

  history.UpdateDefinitionBoundaries(history.GetCount() - nextConversionCount,
                                     nextSegmentList);

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegmentList, nextSegmentList);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer, &previousSegmentList, startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer, &nextSegmentList, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer, previousSegmentList, startingOffset);
  ConvertText(nextConversionBuffer, nextSegmentList, startingOffset);
#endif

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  PrintTextLog(previousConversionBuffer.keyCodeBuffer,
               nextConversionBuffer.keyCodeBuffer);
  PrintPaperTapeUndo(undoCount);
}

void StenoEngine::CreateSegments(size_t sourceStrokeCount,
                                 ConversionBuffer &buffer,
                                 size_t conversionLimit,
                                 StenoSegmentList &segmentList) {
  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);
  BuildSegmentContext context(segmentList, dictionary, orthography);
  buffer.segmentBuilder.CreateSegments(context);
}

void StenoEngine::CreateSegmentsUsingLongerResult(
    size_t sourceStrokeCount, ConversionBuffer &buffer, size_t conversionLimit,
    StenoSegmentList &segmentList, const ConversionBuffer &longerBuffer,
    const StenoSegmentList &longerSegmentList) {

  // state pointers are used to track segment length, so a proper stroke
  // history needs to be set up and translated.

  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);

  size_t startingOffset = 0;
  if (conversionLimit > sourceStrokeCount) {
    conversionLimit = sourceStrokeCount;
  }
  for (const StenoSegment &segment : longerSegmentList) {
    if (startingOffset + segment.strokeLength > conversionLimit) {
      break;
    }
    startingOffset += segment.strokeLength;
    segmentList.Add(StenoSegment(
        segment.strokeLength,
        buffer.segmentBuilder.GetStatePointer(
            longerBuffer.segmentBuilder.GetStateIndex(segment.state)),
        segment.lookup.Clone()));
  }

  if (startingOffset == conversionLimit) {
    return;
  }

  buffer.segmentBuilder.TransferStartFrom(longerBuffer.segmentBuilder,
                                          startingOffset);
  BuildSegmentContext context(segmentList, dictionary, orthography);
  buffer.segmentBuilder.CreateSegments(context, startingOffset);
}

void StenoEngine::ConvertText(ConversionBuffer &buffer,
                              StenoSegmentList &segmentList,
                              size_t startingOffset) {
  StenoTokenizer *tokenizer = segmentList.CreateTokenizer(startingOffset);
  buffer.keyCodeBuffer.Populate(tokenizer);
  if (placeSpaceAfter && !buffer.keyCodeBuffer.state.joinNext &&
      segmentList.IsNotEmpty()) {
    buffer.keyCodeBuffer.AppendSpace();
  }
  delete tokenizer;
}

//---------------------------------------------------------------------------

void StenoEngine::PrintPaperTapeUndo(size_t undoCount) const {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[StenoStroke::MAX_STRING_LENGTH];
  UNDO_STROKE.ToWideString(buffer);
  Console::Printf(
      "EV {\"event\":\"paper_tape\",\"data\":\"%s\",\"undo\":%zu}\n\n", buffer,
      undoCount);
}

void StenoEngine::PrintPaperTape(
    StenoStroke stroke, const StenoSegmentList &previousSegmentList,
    const StenoSegmentList &nextSegmentList) const {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[256];
  stroke.ToWideString(buffer);
  Console::Printf("EV {\"event\":\"paper_tape\",\"data\":\"%s\"", buffer);

  size_t commonIndex = 0;
  while (commonIndex < previousSegmentList.GetCount() &&
         commonIndex < nextSegmentList.GetCount() &&
         Str::Eq(previousSegmentList[commonIndex].lookup.GetText(),
                 nextSegmentList[commonIndex].lookup.GetText())) {
    ++commonIndex;
  }

  size_t undoCount = previousSegmentList.GetCount() - commonIndex;
  if (undoCount > 0) {
    Console::Printf(",\"undo\":%zu", undoCount);
  }

  StenoSegmentList newSegments;
  for (size_t i = commonIndex; i < nextSegmentList.GetCount(); ++i) {
    newSegments.Add(nextSegmentList[i]);
  }

  Console::Printf(",\"text\":\"");
  StenoTokenizer *tokenizer = newSegments.CreateTokenizer();
  bool isFirstToken = true;
  while (tokenizer->HasMore()) {
    if (isFirstToken) {
      isFirstToken = false;
    } else {
      Console::Printf(" ");
    }
    Console::WriteAsJson(tokenizer->GetNext().text, buffer);
  }
  delete tokenizer;

  newSegments.Reset();
  Console::Printf("\"}\n\n");
}

void StenoEngine::PrintSuggestions(const StenoSegmentList &previousSegmentList,
                                   const StenoSegmentList &nextSegmentList) {
  if (!IsSuggestionsEnabled()) {
    return;
  }

  char buffer[256];

  // Finger spelling suggestions.
  if (Str::IsFingerSpellingCommand(nextSegmentList.Back().lookup.GetText())) {
    if (state.isManualStateChange || state.joinNext) {
      return;
    }

    // Get the last word out of the buffer and look that up.
    char *p = buffer + sizeof(buffer) - 1;
    *p = '\0';
    const StenoKeyCode *skc =
        &nextConversionBuffer.keyCodeBuffer
             .buffer[nextConversionBuffer.keyCodeBuffer.count - 1];

    if (placeSpaceAfter && skc > nextConversionBuffer.keyCodeBuffer.buffer) {
      skc--;
    }

    size_t keyCodeCount = 0;
    while (skc >= nextConversionBuffer.keyCodeBuffer.buffer &&
           !skc->IsWhitespace() && !skc->IsRawKeyCode()) {
      uint32_t unicode = skc->GetUnicode();
      size_t length = Utf8Pointer::BytesForCharacterCode(unicode);
      p -= length;
      Utf8Pointer(p).Set(unicode);
      ++keyCodeCount;
      --skc;
    }

    if (keyCodeCount > 1) {
      PrintSuggestion(p, 1, keyCodeCount);
    }
    return;
  }

  // General suggestions. Search back up to 8 word segments.
  char *lastLookup = nullptr;
  for (size_t wordCount = 1; wordCount < 8; ++wordCount) {
    Pump();
    char *newLookup =
        PrintSegmentSuggestion(wordCount, nextSegmentList, lastLookup);
    free(lastLookup);
    lastLookup = newLookup;
    if (!lastLookup) {
      return;
    }
  }
  free(lastLookup);
}

void StenoEngine::PrintSuggestion(const char *p, size_t arrowPrefixCount,
                                  size_t strokeThreshold) const {
  StenoReverseDictionaryLookup result(strokeThreshold, p);
  ReverseLookup(result);
  if (result.resultCount == 0) {
    return;
  }

  Console::Printf("EV {"
                  "\"event\":\"suggestion\","
                  "\"combine_count\":%zu,"
                  "\"text\":\"%J\","
                  "\"outlines\":[",
                  arrowPrefixCount, p);
  for (size_t i = 0; i < result.resultCount; ++i) {
    const StenoReverseDictionaryResult &lookup = result.results[i];
    Console::Printf(i == 0 ? "\"%T\"" : ",\"%T\"", lookup.strokes,
                    lookup.length);
  }
  Console::Printf("]}\n\n");
}

static bool ShouldShowSuggestions(const StenoSegmentList &segmentList) {
  // Count the number of suffix "{*!}" entries.
  // There must be more that number of entries before that.
  size_t joinPreviousCount = 0;
  for (size_t i = segmentList.GetCount(); i != 0;) {
    --i;
    if (!Str::Eq(segmentList[i].lookup.GetText(), "{*!}")) {
      break;
    }
    ++joinPreviousCount;
  }
  return segmentList.GetCount() > 2 * joinPreviousCount;
}

char *StenoEngine::PrintSegmentSuggestion(size_t wordCount,
                                          const StenoSegmentList &segmentList,
                                          char *lastLookup) {

  size_t startSegmentIndex = segmentList.GetCount();
  StenoState lastState = state;
  for (size_t i = 0; i < wordCount; ++i) {
    if (startSegmentIndex == 0) {
      return nullptr;
    }
    while (startSegmentIndex != 0) {
      --startSegmentIndex;
      if (segmentList[startSegmentIndex].ContainsKeyCode()) {
        return nullptr;
      }

      // Consider it a word start if it isn't a suffix stroke and it isn't
      // a fingerspelling joined to a previous fingerspelling.
      // This will still give suggestions after prefixes, e.g.
      //   overwatching: AUFR/WAFP/-G will suggest to combine WAFPG
      const char *startSegmentText =
          segmentList[startSegmentIndex].lookup.GetText();
      if (!Str::IsJoinPrevious(startSegmentText) &&
          (startSegmentIndex == 0 ||
           !Str::IsFingerSpellingCommand(startSegmentText) ||
           !Str::IsFingerSpellingCommand(
               segmentList[startSegmentIndex - 1].lookup.GetText()))) {
        break;
      }
    }
    lastState = *segmentList[startSegmentIndex].state;
  }

  if (segmentList.GetCount() - startSegmentIndex >=
      PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
    return nullptr;
  }

  StenoSegmentList testSegments;

  size_t strokeThresholdCount = 0;
  for (size_t i = startSegmentIndex; i < segmentList.GetCount(); ++i) {
    testSegments.Add(segmentList[i]);
    strokeThresholdCount += segmentList[i].strokeLength;
  }

  StenoTokenizer *tokenizer = testSegments.CreateTokenizer();
  previousConversionBuffer.keyCodeBuffer.Populate(tokenizer);
  delete tokenizer;

  if (!ShouldShowSuggestions(testSegments)) {
    testSegments.Reset();
    return Str::Dup("");
  }

  char *lookup =
      testSegments.HasManualStateChange() ||
              Str::HasPrefix(testSegments.Back().lookup.GetText(), "{:")
          ? previousConversionBuffer.keyCodeBuffer.ToString()
          : previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();

  char *spaceRemoved = *lookup == ' ' ? lookup + 1 : lookup;

  // Special case {*!} and function calls at the start to avoid suggestions.
  if (Str::Eq(testSegments[0].lookup.GetText(), "{*!}") ||
      Str::HasPrefix(testSegments[0].lookup.GetText(), "{:")) {
    testSegments.Reset();
    return lookup;
  }

  bool printSuggestion = *spaceRemoved != '\0' &&
                         (startSegmentIndex != segmentList.GetCount() - 1 ||
                          strokeThresholdCount != 1);

  if (state.joinNext) {
    bool usePrefixSyntax = true;
    if (segmentList.GetCount() >= 2) {
      // If the previous state is the same, and the new state ends in a space,
      // truncate the space and treat it as a non-space lookup.
      if (*segmentList.Back().state == state) {
        size_t length = Str::Length(spaceRemoved);
        if (length != 0 && spaceRemoved[length - 1] == ' ') {
          spaceRemoved = Str::DupN(spaceRemoved, length - 1);
          free(lookup);
          lookup = spaceRemoved;
          usePrefixSyntax = false;
        }
      }
    }

    if (usePrefixSyntax) {
      char *prefixLookup = Str::Asprintf("{%s^}", spaceRemoved);
      free(lookup);
      lookup = prefixLookup;
      spaceRemoved = prefixLookup;
    }
  }

  testSegments.Reset();

  if (!printSuggestion && lastLookup) {
    char *lastLookupSpaceRemoved =
        *lastLookup == ' ' ? lastLookup + 1 : lastLookup;

    printSuggestion = !Str::Eq(spaceRemoved, lastLookupSpaceRemoved);
  }

  if (printSuggestion) {
    PrintSuggestion(spaceRemoved, strokeThresholdCount, strokeThresholdCount);
  }

  return lookup;
}

void StenoEngine::PrintTextLog(
    const StenoKeyCodeBuffer &previousKeyCodeBuffer,
    const StenoKeyCodeBuffer &nextKeyCodeBuffer) const {
  if (!IsTextLogEnabled()) {
    return;
  }

  const StenoKeyCode *previousData = previousKeyCodeBuffer.buffer;
  size_t previousLength = previousKeyCodeBuffer.count;
  const StenoKeyCode *nextData = nextKeyCodeBuffer.buffer;
  size_t nextLength = nextKeyCodeBuffer.count;
  size_t i = 0;
  while (i < previousLength && i < nextLength &&
         previousData[i].HasSameOutput(nextData[i])) {
    ++i;
  }

  size_t backspaceCount = 0;
  for (size_t j = i; j < previousLength; ++j) {
    if (!previousData[j].IsRawKeyCode()) {
      ++backspaceCount;
    }
  }

  Console::Printf("EV {\"event\":\"text_log\",\"text\":\"");
  static const char BACKSPACES[] =
      "\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b";

  while (backspaceCount > 0) {
    size_t writeCount = backspaceCount > 16 ? 16 : backspaceCount;
    Console::Write(BACKSPACES, 2 * writeCount);
    backspaceCount -= writeCount;
  }

  if (i < nextLength) {
    char *text = nextKeyCodeBuffer.ToString(i);
    Console::WriteAsJson(text);
    free(text);
  }
  Console::Printf("\"}\n\n");
}

//---------------------------------------------------------------------------
