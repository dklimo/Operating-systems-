#include <gtest/gtest.h>
#include "employee.h"
#include <fstream>
#include <filesystem>
#include <cstring>

using namespace std;

void createTestBinaryFile(const string& filename) {
    ofstream file(filename, ios::binary);
    if (file) {
        employee emp1 = { 1, "JohnDoe", 40.5 };
        file.write(reinterpret_cast<const char*>(&emp1), sizeof(employee));
        file.close();
    }
}

void createTestReportFile(const string& filename) {
    ofstream file(filename);
    if (file) {
        file << "ID: 1, Name: JohnDoe, Salary: 1000.50\n";
        file.close();
    }
}


TEST(EmployeeTest, WriteBinaryFile_ExistingFile) {
    const string test_filename = "test_binary.bin";
    createTestBinaryFile(test_filename);

    testing::internal::CaptureStdout();
    writeBinaryFile(test_filename);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("ID: 1") != string::npos);
    EXPECT_TRUE(output.find("Name: JohnDoe") != string::npos);
    EXPECT_TRUE(output.find("Hours: 40.5") != string::npos);

    filesystem::remove(test_filename);
}


TEST(EmployeeTest, WriteBinaryFile_NonExistingFile) {
    const string test_filename = "non_existing_file.bin";

    testing::internal::CaptureStderr();
    writeBinaryFile(test_filename);
    string output = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(output.find("Error opening file") != string::npos);
}


TEST(EmployeeTest, WriteBinaryFile_EmptyFile) {
    const string test_filename = "empty_file.bin";

    ofstream file(test_filename, ios::binary);
    file.close();

    testing::internal::CaptureStdout();
    writeBinaryFile(test_filename);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("ID:") == string::npos);

    filesystem::remove(test_filename);
}


TEST(EmployeeTest, WriteReport_ExistingFile) {
    const string test_filename = "test_report.txt";
    createTestReportFile(test_filename);

    testing::internal::CaptureStdout();
    writeReport(test_filename);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("ID: 1") != string::npos);
    EXPECT_TRUE(output.find("Name: JohnDoe") != string::npos);
    EXPECT_TRUE(output.find("Salary: 1000.50") != string::npos);

   filesystem::remove(test_filename);
}


TEST(EmployeeTest, WriteReport_NonExistingFile) {
    const string test_filename = "non_existing_report.txt";

    testing::internal::CaptureStderr();
    writeReport(test_filename);
    string output = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(output.find("Error opening file") != string::npos);
}


TEST(EmployeeTest, WriteReport_EmptyFile) {
    const string test_filename = "empty_report.txt";
    ofstream file(test_filename);
    file.close();

    testing::internal::CaptureStdout();
    writeReport(test_filename);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Contents of report file") != string::npos);

    filesystem::remove(test_filename);
}



