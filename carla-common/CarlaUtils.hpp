/*
 * Carla common utils
 * Copyright (C) 2011-2015 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#ifndef CARLA_UTILS_HPP_INCLUDED
#define CARLA_UTILS_HPP_INCLUDED

#include "CarlaDefines.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef CARLA_PROPER_CPP11_SUPPORT
# include <cstdint>
#else
# include <stdint.h>
#endif

#ifdef CARLA_OS_WIN
# include <winsock2.h>
# include <windows.h>
#else
# include <unistd.h>
#endif

// --------------------------------------------------------------------------------------------------------------------
// misc functions

/*
 * Return "true" or "false" according to yesNo.
 */
static inline
const char* bool2str(const bool yesNo) noexcept
{
    return yesNo ? "true" : "false";
}

/*
 * Set a string as empty/null.
 */
static inline
void nullStrBuf(char* const strBuf) noexcept
{
    strBuf[0] = '\0';
}

/*
 * Dummy function.
 */
static inline
void pass() noexcept {}

// --------------------------------------------------------------------------------------------------------------------
// string print functions

/*
 * Print a string to stdout with newline (gray color).
 * Does nothing if DEBUG is not defined.
 */
#ifndef DEBUG
# define carla_debug(...)
#else
static inline
void carla_debug(const char* const fmt, ...) noexcept
{
    try {
        ::va_list args;
        ::va_start(args, fmt);
#ifndef QTCREATOR_TEST
        std::fprintf(stdout, "\x1b[30;1m");
#endif
        std::vfprintf(stdout, fmt, args);
#ifndef QTCREATOR_TEST
        std::fprintf(stdout, "\x1b[0m\n");
#else
        std::fprintf(stdout, "\n");
#endif
        ::va_end(args);
    } catch (...) {}
}
#endif

/*
 * Print a string to stdout with newline.
 */
static inline
void carla_stdout(const char* const fmt, ...) noexcept
{
    try {
        ::va_list args;
        ::va_start(args, fmt);
        std::vfprintf(stdout, fmt, args);
        std::fprintf(stdout, "\n");
        ::va_end(args);
    } catch (...) {}
}

/*
 * Print a string to stderr with newline.
 */
static inline
void carla_stderr(const char* const fmt, ...) noexcept
{
    try {
        ::va_list args;
        ::va_start(args, fmt);
        std::vfprintf(stderr, fmt, args);
        std::fprintf(stderr, "\n");
        ::va_end(args);
    } catch (...) {}
}

/*
 * Print a string to stderr with newline (red color).
 */
static inline
void carla_stderr2(const char* const fmt, ...) noexcept
{
    try {
        ::va_list args;
        ::va_start(args, fmt);
#ifndef QTCREATOR_TEST
        std::fprintf(stderr, "\x1b[31m");
#endif
        std::vfprintf(stderr, fmt, args);
#ifndef QTCREATOR_TEST
        std::fprintf(stderr, "\x1b[0m\n");
#else
        std::fprintf(stderr, "\n");
#endif
        ::va_end(args);
    } catch (...) {}
}

// --------------------------------------------------------------------------------------------------------------------
// carla_safe_assert*

/*
 * Print a safe assertion error message.
 */
static inline
void carla_safe_assert(const char* const assertion, const char* const file, const int line) noexcept
{
    carla_stderr2("Carla assertion failure: \"%s\" in file %s, line %i", assertion, file, line);
}

/*
 * Print a safe assertion error message, with 1 extra integer value.
 */
static inline
void carla_safe_assert_int(const char* const assertion, const char* const file, const int line,
                                                                                const int value) noexcept
{
    carla_stderr2("Carla assertion failure: \"%s\" in file %s, line %i, value %i", assertion, file, line, value);
}
static inline
void carla_safe_assert_uint(const char* const assertion, const char* const file, const int line,
                                                                                 const uint value) noexcept
{
    carla_stderr2("Carla assertion failure: \"%s\" in file %s, line %i, value %u", assertion, file, line, value);
}

/*
 * Print a safe assertion error message, with 2 extra integer values.
 */
static inline
void carla_safe_assert_int2(const char* const assertion, const char* const file, const int line,
                                                                                 const int v1, const int v2) noexcept
{
    carla_stderr2("Carla assertion failure: \"%s\" in file %s, line %i, v1 %i, v2 %i", assertion, file, line, v1, v2);
}
static inline
void carla_safe_assert_uint2(const char* const assertion, const char* const file, const int line,
                                                                                  const uint v1, const uint v2) noexcept
{
    carla_stderr2("Carla assertion failure: \"%s\" in file %s, line %i, v1 %u, v2 %u", assertion, file, line, v1, v2);
}

// --------------------------------------------------------------------------------------------------------------------
// carla_safe_exception*

/*
 * Print a safe exception error message.
 */
static inline
void carla_safe_exception(const char* const exception, const char* const file, const int line) noexcept
{
    carla_stderr2("Carla exception caught: \"%s\" in file %s, line %i", exception, file, line);
}

// --------------------------------------------------------------------------------------------------------------------
// carla_*sleep

/*
 * Sleep for 'secs' seconds.
 */
static inline
void carla_sleep(const uint secs) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(secs > 0,);

    try {
#ifdef CARLA_OS_WIN
        ::Sleep(secs * 1000);
#else
        ::sleep(secs);
#endif
    } CARLA_SAFE_EXCEPTION("carla_sleep");
}

/*
 * Sleep for 'msecs' milliseconds.
 */
static inline
void carla_msleep(const uint msecs) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(msecs > 0,);

    try {
#ifdef CARLA_OS_WIN
        ::Sleep(msecs);
#else
        ::usleep(msecs * 1000);
#endif
    } CARLA_SAFE_EXCEPTION("carla_msleep");
}

// --------------------------------------------------------------------------------------------------------------------
// carla_setenv

/*
 * Set environment variable 'key' to 'value'.
 */
static inline
void carla_setenv(const char* const key, const char* const value) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(key != nullptr && key[0] != '\0',);
    CARLA_SAFE_ASSERT_RETURN(value != nullptr,);

#ifdef CARLA_OS_WIN
    try {
        ::SetEnvironmentVariableA(key, value);
    } CARLA_SAFE_EXCEPTION("carla_setenv");
#else
    ::setenv(key, value, 1);
#endif
}

/*
 * Unset environment variable 'key'.
 */
static inline
void carla_unsetenv(const char* const key) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(key != nullptr && key[0] != '\0',);

#ifdef CARLA_OS_WIN
    try {
        ::SetEnvironmentVariableA(key, nullptr);
    } CARLA_SAFE_EXCEPTION("carla_unsetenv");
#else
    ::unsetenv(key);
#endif
}

// --------------------------------------------------------------------------------------------------------------------
// carla_strdup

/*
 * Custom 'strdup' function.
 * Returned value is always valid, and must be freed with "delete[] var".
 * May throw.
 */
static inline
const char* carla_strdup(const char* const strBuf)
{
    CARLA_SAFE_ASSERT(strBuf != nullptr);

    const std::size_t bufferLen = (strBuf != nullptr) ? std::strlen(strBuf) : 0;
    char* const       buffer    = new char[bufferLen+1];

    if (strBuf != nullptr && bufferLen > 0)
        std::memcpy(buffer, strBuf, bufferLen);

    buffer[bufferLen] = '\0';

    return buffer;
}

/*
 * Custom 'strdup' function.
 * Calls "std::free(strBuf)".
 * Returned value is always valid, and must be freed with "delete[] var".
 * May throw.
 */
static inline
const char* carla_strdup_free(char* const strBuf)
{
    const char* const buffer(carla_strdup(strBuf));
    std::free(strBuf);
    return buffer;
}

/*
 * Custom 'strdup' function, safe version.
 * Returned value may be null.
 */
static inline
const char* carla_strdup_safe(const char* const strBuf) noexcept
{
    CARLA_SAFE_ASSERT(strBuf != nullptr);

    const std::size_t bufferLen = (strBuf != nullptr) ? std::strlen(strBuf) : 0;
    char* buffer;

    try {
        buffer = new char[bufferLen+1];
    } CARLA_SAFE_EXCEPTION_RETURN("carla_strdup_safe", nullptr);

    if (strBuf != nullptr && bufferLen > 0)
        std::memcpy(buffer, strBuf, bufferLen);

    buffer[bufferLen] = '\0';

    return buffer;
}

// --------------------------------------------------------------------------------------------------------------------
// memory functions

/*
 * Add array values to another array.
 */
template<typename T>
static inline
void carla_add(T dest[], const T src[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(dest != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(src != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    for (std::size_t i=0; i<count; ++i)
        *dest++ += *src++;
}

/*
 * Copy array values to another array.
 */
template<typename T>
static inline
void carla_copy(T dest[], const T src[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(dest != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(src != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memcpy(dest, src, count*sizeof(T));
}

/*
 * Fill an array with a fixed value.
 */
template<typename T>
static inline
void carla_fill(T data[], const T& value, const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(data != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    if (value == 0)
    {
        std::memset(data, 0, count*sizeof(T));
    }
    else
    {
        for (std::size_t i=0; i<count; ++i)
            *data++ = value;
    }
}

/*
 * Clear a byte array.
 */
static inline
void carla_zeroBytes(uint8_t bytes[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(bytes != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memset(bytes, 0, count*sizeof(uint8_t));
}

/*
 * Clear a char array.
 */
static inline
void carla_zeroChars(char chars[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(chars != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memset(chars, 0, count*sizeof(char));
}

/*
 * Clear a pointer array.
 */
template<typename T>
static inline
void carla_zeroPointers(T* ptrs[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(ptrs != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memset(ptrs, 0, count*sizeof(T*));
}

/*
 * Clear a single struct.
 */
template <typename T>
static inline
void carla_zeroStruct(T& s) noexcept
{
    std::memset(&s, 0, sizeof(T));
}

/*
 * Clear a struct array.
 */
template <typename T>
static inline
void carla_zeroStructs(T structs[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(structs != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memset(structs, 0, count*sizeof(T));
}

/*
 * Copy a single struct.
 */
template <typename T>
static inline
void carla_copyStruct(T& dest, const T& src) noexcept
{
    std::memcpy(&dest, &src, sizeof(T));
}

/*
 * Copy a struct array.
 */
template <typename T>
static inline
void carla_copyStructs(T dest[], const T src[], const std::size_t count) noexcept
{
    CARLA_SAFE_ASSERT_RETURN(dest != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(src != nullptr,);
    CARLA_SAFE_ASSERT_RETURN(count > 0,);

    std::memcpy(dest, src, count*sizeof(T));
}

// --------------------------------------------------------------------------------------------------------------------

#endif // CARLA_UTILS_HPP_INCLUDED