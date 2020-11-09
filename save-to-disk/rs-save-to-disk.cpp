// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams
#include <ctime>
#include <stdio.h>
#include <io.h>

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Helper function for writing metadata to disk as a csv file
void metadata_to_csv(const rs2::frame& frm, const std::string& filename);
void ProcessFrame(rs2::frame frame, int index, std::time_t result, rs2::colorizer color_map);

// This sample captures 30 frames and writes the last frame to disk.
// It can be useful for debugging an embedded system with no display.
int main(int argc, char * argv[]) try
{
    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map(2);

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;

	rs2::config config;

	config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 0, 1280, 720, rs2_format::RS2_FORMAT_ANY, 30);
	config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 0, 1280, 720, rs2_format::RS2_FORMAT_ANY, 30);

	int counter = 0;
	long timestamp;

	int counterVideo = 0, counterDepth=0;


	std::time_t result = std::time(nullptr);
	std::localtime(&result);

	rs2::frame bufferVideo [15];
	rs2::frame bufferDepth[15];

    // Start streaming with default recommended configuration
    pipe.start(config);

    // Capture 30 frames to give autoexposure, etc. a chance to settle
    for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();

	for (int i = 0; i < 15; i++)
	{
		for (auto&& frame : pipe.wait_for_frames())
		{
			if (auto vf = frame.as<rs2::video_frame>())
			{

				if (vf.is<rs2::depth_frame>())
				{
					bufferDepth[counterDepth] = frame;
					counterDepth++;
				}
				else
				{
					bufferVideo[counterVideo] = frame;
					counterVideo++;
				}
			}

			if (counterVideo==15 && counterDepth == 15)
				break;
		}
	}
	

	pipe.stop();

	counter = 0;

    // Wait for the next set of frames from the camera. Now that autoexposure, etc.
    // has settled, we will write these to disk
	for (int i = 0; i < 15; i++)
	{
		ProcessFrame(bufferVideo[i], i, result, color_map);
		ProcessFrame(bufferDepth[i], i, result, color_map);

	}



  //  for (auto&& frame : buffer)
  //  {
  //      // We can only save video frames as pngs, so we skip the rest
  //      if (auto vf = frame.as<rs2::video_frame>())
  //      {
  //          auto stream = frame.get_profile().stream_type();
  //          // Use the colorizer to get an rgb image for the depth stream
  //          if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

  //          // Write images to disk
  //          std::stringstream png_file;
  //          png_file << "rs-save-to-disk-output-" << result << "-" << counter << "-" << vf.get_profile().stream_name() << ".png";
  //          stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
  //                         vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
  //          std::cout << "Saved " << png_file.str() << std::endl;

  //          // Record per-frame metadata for UVC streams
  //          std::stringstream csv_file;
  //          csv_file << "rs-save-to-disk-output-" << result << "-" << counter << "-" << vf.get_profile().stream_name()
  //                   << "-metadata.csv";
  //          metadata_to_csv(vf, csv_file.str());

		//	std::cout << counter;

  //      }
		//counter++;
  //  }

    return EXIT_SUCCESS;
}

catch(const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch(const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

void ProcessFrame(rs2::frame frame, int index, std::time_t result, rs2::colorizer color_map)
{
	// We can only save video frames as pngs, so we skip the rest
	if (auto vf = frame.as<rs2::video_frame>())
	{
		auto stream = frame.get_profile().stream_type();
		// Use the colorizer to get an rgb image for the depth stream
		if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

		// Write images to disk
		std::stringstream png_file;
		png_file <<"C:/Users/mazharm/Desktop/rs/" << "rs-save-to-disk-output-" << result << "-" << index << "-" << vf.get_profile().stream_name() << ".png";
		stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
			vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
		std::cout << "Saved " << png_file.str() << std::endl;

		// Record per-frame metadata for UVC streams
		std::stringstream csv_file;
		csv_file << "C:/Users/mazharm/Desktop/rs/" << "rs-save-to-disk-output-" << result << "-" << index << "-" << vf.get_profile().stream_name()
			<< "-metadata.csv";
		metadata_to_csv(vf, csv_file.str());

		//std::cout << index;

	}
}

void metadata_to_csv(const rs2::frame& frm, const std::string& filename)
{
    std::ofstream csv;

    csv.open(filename);

    //    std::cout << "Writing metadata to " << filename << endl;
    csv << "Stream," << rs2_stream_to_string(frm.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

    // Record all the available metadata attributes
    for (size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++)
    {
        if (frm.supports_frame_metadata((rs2_frame_metadata_value)i))
        {
            csv << rs2_frame_metadata_to_string((rs2_frame_metadata_value)i) << ","
                << frm.get_frame_metadata((rs2_frame_metadata_value)i) << "\n";
        }
    }

    csv.close();
}
