//---------------------------------------------------------------------------

#include "flash.h"
#include "base64.h"
#include "console.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

Flash Flash::instance;

//---------------------------------------------------------------------------

__attribute((weak)) void Flash::Erase(const void *target, size_t size) {
  assert((size & (BLOCK_SIZE - 1)) == 0);

  instance.erasedBytes += size;
  memset((void *)target, 0xff, size);
}

__attribute((weak)) void Flash::Write(const void *target, const void *data,
                                      size_t size) {
  assert(target != data);
  assert((size & (BLOCK_SIZE - 1)) == 0);

  instance.erasedBytes += size;
  instance.programmedBytes += size;
  memcpy((void *)target, data, size);
}

//---------------------------------------------------------------------------

bool Flash::RequiresErase(const void *target, size_t size) {
  size_t count = size / sizeof(size_t);
  const size_t *t = (const size_t *)target;

  for (size_t i = 0; i < count; ++i) {
    if (t[i] != (size_t)-1) {
      return true;
    }
  }

  return false;
}

bool Flash::RequiresErase(const void *target, const void *data, size_t size) {
  size_t count = size / sizeof(size_t);
  const size_t *t = (const size_t *)target;
  const size_t *d = (const size_t *)data;

  for (size_t i = 0; i < count; ++i) {
    // Programming lowers a bit from 1 to 0, erasing raises it to 1.
    // Erasing is required if the data needs a 1, but the current state is a 0.
    if (d[i] & ~t[i]) {
      return true;
    }
  }

  return false;
}

bool Flash::RequiresProgram(const void *target, const void *data, size_t size) {
  return memcmp(target, data, size) != 0;
}

//---------------------------------------------------------------------------

void Flash::BeginWrite(const uint8_t *address) {
  target = address;

  const void *baseAddress =
      (const void *)(intptr_t(address) & -WRITE_DATA_BUFFER_SIZE);
  size_t offsetIntoPage = size_t(address) & (WRITE_DATA_BUFFER_SIZE - 1);
  memcpy(buffer, baseAddress, offsetIntoPage);
}

void Flash::AddData(const uint8_t *data, size_t length) {
  size_t offset = intptr_t(target) & (WRITE_DATA_BUFFER_SIZE - 1);
  if (offset + length >= WRITE_DATA_BUFFER_SIZE) {
    size_t remainingBufferLength = WRITE_DATA_BUFFER_SIZE - offset;
    memcpy(buffer + offset, data, remainingBufferLength);

    const void *writeAddress =
        (const void *)(size_t(target) & -WRITE_DATA_BUFFER_SIZE);
    Flash::Write(writeAddress, buffer, WRITE_DATA_BUFFER_SIZE);

    target += remainingBufferLength;
    length -= remainingBufferLength;
    data += remainingBufferLength;
    offset = 0;
  }
  memcpy(buffer + offset, data, length);
  target += length;
}

void Flash::WriteRemaining() {
  size_t bytesToWrite = size_t(target) & (WRITE_DATA_BUFFER_SIZE - 1);
  if (bytesToWrite != 0) {
    // Copy the remainder
    size_t bytesToCopy = WRITE_DATA_BUFFER_SIZE - bytesToWrite;
    memcpy(buffer + bytesToWrite, target, bytesToCopy);

    const void *writeAddress =
        (const void *)(size_t(target) & -WRITE_DATA_BUFFER_SIZE);

    Flash::Write(writeAddress, buffer, WRITE_DATA_BUFFER_SIZE);
  }
  target = nullptr;
}

static int HexValue(int c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }
  if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  }
  if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  }

  return -1;
}

void Flash::BeginWriteBinding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing address\n\n");
    return;
  }

  if (instance.target != nullptr) {
    Console::Printf("ERR Write in progress\n\n");
  }

  size_t addressValue = 0;
  while (*p) {
    int hexValue = HexValue(*p++);
    if (hexValue == -1) {
      continue;
    }
    addressValue = 16 * addressValue + hexValue;
  }

  instance.BeginWrite((const uint8_t *)addressValue);

  Console::SendOk();
}

void Flash::WriteBinding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing data\n\n");
    return;
  }

  uint8_t decodeBuffer[256];
  size_t byteCount = Base64::Decode(decodeBuffer, (const uint8_t *)p);

  if (byteCount == 0) {
    Console::Printf("ERR No data\n\n");
    return;
  }

  instance.AddData(decodeBuffer, byteCount);

  Console::SendOk();
}

void Flash::EndWriteBinding(void *context, const char *commandLine) {
  instance.WriteRemaining();
  Console::SendOk();
}

//---------------------------------------------------------------------------

void Flash::PrintInfo() {
  Console::Printf("Flash session statistics\n");
  Console::Printf("  Erased bytes: %u\n", instance.erasedBytes);
  Console::Printf("  Programmed bytes: %u\n", instance.programmedBytes);
  Console::Printf("  Reprogrammed bytes: %u\n", instance.reprogrammedBytes);
}

//---------------------------------------------------------------------------
