
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "appleseed.foundation.stubs.h"

// appleseed.foundation headers.
#include "foundation/platform/compiler.h"
#include "foundation/platform/snprintf.h"
#include "foundation/utility/log/logtargetbase.h"
#include "foundation/utility/foreach.h"

// Standard headers.
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <list>
#include <vector>

using namespace std;

namespace foundation
{

//
// Logger class implementation.
//

namespace
{
    const size_t InitialBufferSize = 1024;          // in bytes
    const size_t MaxBufferSize = 1024 * 1024;       // in bytes
}

struct Logger::Impl
{
    typedef list<LogTargetBase*> LogTargetContainer;

    bool                m_enabled;
    LogTargetContainer  m_targets;
    vector<char>        m_message_buffer;
};

Logger::Logger()
  : impl(new Impl())
{
    impl->m_enabled = true;
    impl->m_message_buffer.resize(InitialBufferSize);
}

Logger::~Logger()
{
    delete impl;
}

void Logger::set_enabled(const bool enabled)
{
    impl->m_enabled = enabled;
}

void Logger::add_target(LogTargetBase* target)
{
    assert(target);

    impl->m_targets.push_back(target);
}

void Logger::remove_target(LogTargetBase* target)
{
    assert(target);

    impl->m_targets.remove(target);
}

namespace
{
    bool write_to_buffer(
        vector<char>&   buffer,
        const size_t    max_buffer_size,
        const char*     format,
        va_list         argptr)
    {
        while (true)
        {
            va_list argptr_copy;
            va_copy(argptr_copy, argptr);

            const size_t buffer_size = buffer.size();

            const int result =
                portable_vsnprintf(&buffer[0], buffer_size, format, argptr_copy);

            if (result < 0)
                return false;

            const size_t needed_buffer_size = static_cast<size_t>(result) + 1;

            if (needed_buffer_size <= buffer_size)
                return true;

            if (buffer_size >= max_buffer_size)
                return false;

            buffer.resize(min(needed_buffer_size, max_buffer_size));
        }
    }
}

void Logger::write(
    const LogMessage::Category  category,
    const char*                 file,
    const size_t                line,
    const char*                 format, ...)
{
    if (impl->m_enabled)
    {
        // Print the formatted message into the temporary buffer.
        va_list argptr;
        va_start(argptr, format);
        write_to_buffer(impl->m_message_buffer, MaxBufferSize, format, argptr);

        // Send the message to every log targets.
        for (const_each<Impl::LogTargetContainer> i = impl->m_targets; i; ++i)
        {
            LogTargetBase* target = *i;
            target->write(
                category,
                file,
                line,
                &impl->m_message_buffer[0]);
        }
    }

    // Terminate the application if the message category is 'Fatal'.
    if (category == LogMessage::Fatal)
        exit(EXIT_FAILURE);
}

}   // namespace foundation
