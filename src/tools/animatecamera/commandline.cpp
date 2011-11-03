
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2011 Francois Beaune
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
#include "commandline.h"

// Project headers.
#include "defaults.h"

// appleseed.foundation headers.
#include "foundation/utility/autoreleaseptr.h"
#include "foundation/utility/log.h"
#include "foundation/utility/string.h"

// Standard headers.
#include <cstdio>
#include <cstdlib>

using namespace foundation;
using namespace std;


//
// CommandLine class implementation.
//

CommandLine::CommandLine()
{
    // Help.
    m_help.add_name("--help");
    m_help.add_name("-h");
    m_help.set_description("print program usage and exit");
    m_parser.add_option_handler(&m_help);

    // Message coloring.
    m_message_coloring.add_name("--message-coloring");
    m_message_coloring.set_description("enable message coloring");
    m_parser.add_option_handler(&m_message_coloring);

    // Progress messages.
    m_progress_messages.add_name("--progress");
    m_progress_messages.add_name("-p");
    m_progress_messages.set_description("print progress messages");
    m_parser.add_option_handler(&m_progress_messages);

    // Filenames.
    m_filenames.set_min_value_count(2);
    m_filenames.set_max_value_count(2);
    m_parser.set_default_option_handler(&m_filenames);

    // Frames.
    m_frame_count.add_name("--frames");
    m_frame_count.add_name("-f");
    m_frame_count.set_description(
        "set the number of frames in the animation (default " +
        to_string(DefaultFrameCount) + ")");
    m_frame_count.set_syntax("count");
    m_frame_count.set_min_value_count(1);
    m_frame_count.set_max_value_count(1);
    m_parser.add_option_handler(&m_frame_count);

    // Camera distance.
    m_camera_distance.add_name("--distance");
    m_camera_distance.add_name("-d");
    m_camera_distance.set_description(
        "set the normalized distance from the camera to the scene (default " +
        to_string(DefaultNormalizedCameraDistance) + ")");
    m_camera_distance.set_syntax("scalar");
    m_camera_distance.set_min_value_count(1);
    m_camera_distance.set_max_value_count(1);
    m_parser.add_option_handler(&m_camera_distance);

    // Camera elevation.
    m_camera_elevation.add_name("--elevation");
    m_camera_elevation.add_name("-e");
    m_camera_elevation.set_description(
        "set the normalized elevation of the camera (default " +
        to_string(DefaultNormalizedCameraElevation) + ")");
    m_camera_elevation.set_syntax("scalar");
    m_camera_elevation.set_min_value_count(1);
    m_camera_elevation.set_max_value_count(1);
    m_parser.add_option_handler(&m_camera_elevation);
}

void CommandLine::parse(const int argc, const char* argv[])
{
    // Parse the command line.
    m_parser.parse(argc, argv);

    // Create a logger.
    Logger logger;

    // Create and configure a log target that outputs to stderr, and attach it to the logger.
    auto_release_ptr<LogTargetBase> log_target(
        m_message_coloring.found()
            ? create_console_log_target(stderr)
            : create_open_file_log_target(stderr));
    log_target->set_formatting_flags(
        LogMessage::Info,
        LogMessage::DisplayMessage);
    logger.add_target(log_target.get());

    // Print program usage.
    if (m_help.found() || !m_filenames.found())
        print_program_usage(argv[0], logger);

    // Exit if necessary.
    if (m_help.found() || !m_filenames.found())
        exit(0);

    // Print messages generated during command line parsing.
    m_parser.print_messages(logger);
}

void CommandLine::print_program_usage(
    const char*     program_name,
    Logger&         logger) const
{
    LOG_INFO(logger, "usage: %s [options] master.appleseed output.appleseed", program_name);
    LOG_INFO(logger, "options:");
    m_parser.print_usage(logger);
}