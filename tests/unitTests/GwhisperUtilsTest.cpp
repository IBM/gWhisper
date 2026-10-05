// Copyright 2026 IBM Corporation
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <utils/gwhisperUtils.hpp>
#include <fstream>
#include <filesystem>

TEST(GwhisperUtilsTest, ReadFromFileExists) {
    std::string testPath = "test_file_temp.txt";
    std::ofstream out(testPath);
    out << "hello world";
    out.close();

    std::string content = gwhisper::util::readFromFile(testPath);
    EXPECT_EQ("hello world", content);

    std::filesystem::remove(testPath);
}

TEST(GwhisperUtilsTest, ReadFromFileNotExists) {
    std::string content = gwhisper::util::readFromFile("non_existent_file_12345.txt");
    EXPECT_EQ("FAIL", content);
}

TEST(GwhisperUtilsTest, CreateFolder) {
    std::string folderPath = "test_folder_temp";
    std::string res = gwhisper::util::createFolder(folderPath);
    EXPECT_EQ("OK", res);
    EXPECT_TRUE(std::filesystem::is_directory(folderPath));

    // Try creating again (should skip/succeed)
    res = gwhisper::util::createFolder(folderPath);
    EXPECT_EQ("OK", res);

    std::filesystem::remove_all(folderPath);
}

TEST(GwhisperUtilsTest, CreateFileNew) {
    std::string filePath = "test_create_file_temp.txt";
    std::filesystem::remove(filePath); // Ensure it doesn't exist

    std::string res = gwhisper::util::createFile(filePath);
    EXPECT_EQ("OK", res);
    EXPECT_TRUE(std::filesystem::exists(filePath));

    std::filesystem::remove(filePath);
}

TEST(GwhisperUtilsTest, CreateFileExists) {
    std::string filePath = "test_create_file_temp.txt";
    std::string copyPath = "test_create_file_temp_copy.txt";
    std::string copyPath1 = "test_create_file_temp_copy_1.txt";
    std::string copyPath2 = "test_create_file_temp_copy_2.txt";

    // Clean up before test
    std::filesystem::remove(filePath);
    std::filesystem::remove(copyPath);
    std::filesystem::remove(copyPath1);
    std::filesystem::remove(copyPath2);

    // Create the first file
    std::string res1 = gwhisper::util::createFile(filePath);
    EXPECT_EQ("OK", res1);
    EXPECT_TRUE(std::filesystem::exists(filePath));

    // Try to create it again - should append "_copy" before the extension
    std::string res2 = gwhisper::util::createFile(filePath);
    EXPECT_EQ("OK", res2);
    EXPECT_TRUE(std::filesystem::exists(copyPath));

    // Third time - should append "_copy_1"
    std::string res3 = gwhisper::util::createFile(filePath);
    EXPECT_EQ("OK", res3);
    EXPECT_TRUE(std::filesystem::exists(copyPath1));

    // Fourth time - should append "_copy_2"
    std::string res4 = gwhisper::util::createFile(filePath);
    EXPECT_EQ("OK", res4);
    EXPECT_TRUE(std::filesystem::exists(copyPath2));

    // Cleanup
    std::filesystem::remove(filePath);
    std::filesystem::remove(copyPath);
    std::filesystem::remove(copyPath1);
    std::filesystem::remove(copyPath2);
}
