#include "module_jason3_amr2.h"
#include <fstream>
#include "common/ccsds/ccsds_1_0_jason/demuxer.h"
#include "common/ccsds/ccsds_1_0_jason/vcdu.h"
#include "logger.h"
#include <filesystem>
#include "imgui/imgui.h"
#include "amr2_reader.h"

// Return filesize
size_t getFilesize(std::string filepath);

namespace jason3
{
    namespace amr2
    {
        Jason3AMR2DecoderModule::Jason3AMR2DecoderModule(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters) : ProcessingModule(input_file, output_file_hint, parameters)
        {
        }

        void Jason3AMR2DecoderModule::process()
        {
            filesize = getFilesize(d_input_file);
            std::ifstream data_in(d_input_file, std::ios::binary);

            std::string directory = d_output_file_hint.substr(0, d_output_file_hint.rfind('/')) + "/AMR-2";

            if (!std::filesystem::exists(directory))
                std::filesystem::create_directory(directory);

            logger->info("Using input frames " + d_input_file);
            logger->info("Decoding to " + directory);

            time_t lastTime = 0;

            uint8_t buffer[1279];

            // CCSDS Demuxer
            ccsds::ccsds_1_0_jason::Demuxer ccsdsDemuxer(1101, false);

            logger->info("Demultiplexing and deframing...");

            AMR2Reader reader;

            while (!data_in.eof())
            {
                // Read buffer
                data_in.read((char *)buffer, 1279);

                int vcid = ccsds::ccsds_1_0_jason::parseVCDU(buffer).vcid;

                if (vcid == 1)
                {
                    std::vector<ccsds::CCSDSPacket> pkts = ccsdsDemuxer.work(buffer);

                    if (pkts.size() > 0)
                    {
                        for (ccsds::CCSDSPacket pkt : pkts)
                        {
                            if (pkt.header.apid == 2047)
                                continue;

                            if (pkt.header.apid == 1408)
                                reader.work(pkt);
                        }
                    }
                }

                progress = data_in.tellg();

                if (time(NULL) % 10 == 0 && lastTime != time(NULL))
                {
                    lastTime = time(NULL);
                    logger->info("Progress " + std::to_string(round(((float)progress / (float)filesize) * 1000.0f) / 10.0f) + "%");
                }
            }

            data_in.close();

            logger->info("Writing images.... (Can take a while)");

            for (int i = 0; i < 3; i++)
            {
                logger->info("Channel " + std::to_string(i + 1) + "...");
                WRITE_IMAGE(reader.getImage(i), directory + "/AMR2-" + std::to_string(i + 1) + "-MAP.png");

                cimg_library::CImg<unsigned short> image = reader.getImageNormal(i);
                image.equalize(1000);
                WRITE_IMAGE(image, directory + "/AMR2-" + std::to_string(i + 1) + ".png");
            }
        }

        void Jason3AMR2DecoderModule::drawUI(bool window)
        {
            ImGui::Begin("Jason-3 AMR-2 Decoder", NULL, window ? NULL : NOWINDOW_FLAGS);

            ImGui::ProgressBar((float)progress / (float)filesize, ImVec2(ImGui::GetWindowWidth() - 10, 20 * ui_scale));

            ImGui::End();
        }

        std::string Jason3AMR2DecoderModule::getID()
        {
            return "jason3_amr2";
        }

        std::vector<std::string> Jason3AMR2DecoderModule::getParameters()
        {
            return {};
        }

        std::shared_ptr<ProcessingModule> Jason3AMR2DecoderModule::getInstance(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters)
        {
            return std::make_shared<Jason3AMR2DecoderModule>(input_file, output_file_hint, parameters);
        }
    } // namespace swap
} // namespace proba