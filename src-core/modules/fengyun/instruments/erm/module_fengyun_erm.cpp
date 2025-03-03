#include "module_fengyun_erm.h"
#include <fstream>
#include "logger.h"
#include <filesystem>
#include "imgui/imgui.h"
#include "erm_reader.h"
#include "common/ccsds/ccsds_1_0_1024/demuxer.h"

// Return filesize
size_t getFilesize(std::string filepath);

namespace fengyun
{
    namespace erm
    {
        FengyunERMDecoderModule::FengyunERMDecoderModule(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters) : ProcessingModule(input_file, output_file_hint, parameters)
        {
        }

        void FengyunERMDecoderModule::process()
        {
            filesize = getFilesize(d_input_file);
            std::ifstream data_in(d_input_file, std::ios::binary);

            std::string directory = d_output_file_hint.substr(0, d_output_file_hint.rfind('/')) + "/ERM";

            logger->info("Using input frames " + d_input_file);
            logger->info("Decoding to " + directory);

            time_t lastTime = 0;

            int vcidFrames = 0, ccsds_frames = 0;

            // Read buffer
            uint8_t buffer[1024];

            logger->info("Demultiplexing and deframing...");

            // Demuxer
            ccsds::ccsds_1_0_1024::Demuxer ccsdsDemuxer = ccsds::ccsds_1_0_1024::Demuxer(882, true);

            // Reader
            ERMReader erm_reader;

            while (!data_in.eof())
            {
                // Read buffer
                data_in.read((char *)buffer, 1024);

                // Extract VCID
                int vcid = buffer[5] % ((int)pow(2, 6));
                //logger->debug(vcid);
                if (vcid == 12)
                {
                    vcidFrames++;

                    std::vector<ccsds::CCSDSPacket> ccsdsFrames = ccsdsDemuxer.work(buffer);
                    ccsds_frames += ccsdsFrames.size();

                    for (ccsds::CCSDSPacket &pkt : ccsdsFrames)
                    {
                        if (pkt.header.apid == 5)
                        {
                            erm_reader.work(pkt);
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

            logger->info("VCID 12 Frames         : " + std::to_string(vcidFrames));
            logger->info("CCSDS Frames           : " + std::to_string(ccsds_frames));
            logger->info("ERM Lines              : " + std::to_string(erm_reader.lines));

            logger->info("Writing images.... (Can take a while)");

            if (!std::filesystem::exists(directory))
                std::filesystem::create_directory(directory);

            logger->info("Channel 1...");
            WRITE_IMAGE(erm_reader.getChannel(), directory + "/ERM-1.png");
        }

        void FengyunERMDecoderModule::drawUI(bool window)
        {
            ImGui::Begin("FengYun ERM Decoder", NULL, window ? NULL : NOWINDOW_FLAGS);

            ImGui::ProgressBar((float)progress / (float)filesize, ImVec2(ImGui::GetWindowWidth() - 10, 20 * ui_scale));

            ImGui::End();
        }

        std::string FengyunERMDecoderModule::getID()
        {
            return "fengyun_erm";
        }

        std::vector<std::string> FengyunERMDecoderModule::getParameters()
        {
            return {};
        }

        std::shared_ptr<ProcessingModule> FengyunERMDecoderModule::getInstance(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters)
        {
            return std::make_shared<FengyunERMDecoderModule>(input_file, output_file_hint, parameters);
        }
    } // namespace virr
} // namespace fengyun