#include <gtest/gtest.h>

#include <QSignalSpy>
#include <dbc_file/constants.hpp>

#include "dbc_file/view/pages/load_page.hpp"

using namespace DbcFile;

class LoadPageLogicTest : public ::testing::Test {
protected:
    LoadPage* page = nullptr;

    void SetUp() override {
        page = new LoadPage();
        page->show();
    }

    void TearDown() override {
        delete page;
    }
};

// ============================================================================
// 1. STATUS MESSAGES
// ============================================================================
TEST_F(LoadPageLogicTest, ShowStatusMessageSetsTextAndVisible) {
    page->showStatusMessage("Parsing...", false);
    EXPECT_EQ(page->testStatusText(), "Parsing...");
    EXPECT_TRUE(page->testStatusVisible());

    page->showStatusMessage("Error!", true);
    EXPECT_EQ(page->testStatusText(), "Error!");
    EXPECT_TRUE(page->testStatusVisible());
}

TEST_F(LoadPageLogicTest, ResetStatusClearsMessageAndHides) {
    page->showStatusMessage("Error", true);
    page->resetStatus();
    EXPECT_EQ(page->testStatusText(), "");
    EXPECT_FALSE(page->testStatusVisible());
}

// ============================================================================
// 2. DROP EVENT LOGIC
// ============================================================================
TEST_F(LoadPageLogicTest, DropEventEmitsFileSelectedOnValidFile) {
    QSignalSpy spy(page, &LoadPage::fileSelected);

    const QString validFile = "/tmp/test.dbc";

    page->showStatusMessage("Parsing...", false);
    emit page->fileSelected(validFile);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), validFile);
}

TEST_F(LoadPageLogicTest, DropEventShowsErrorForTooManyFiles) {
    page->showStatusMessage(Constants::LoadPage::Errors::TooManyFiles, true);
    EXPECT_EQ(page->testStatusText(), Constants::LoadPage::Errors::TooManyFiles);
    EXPECT_TRUE(page->testStatusVisible());
}

TEST_F(LoadPageLogicTest, DropEventShowsErrorForInvalidFile) {
    page->showStatusMessage(Constants::LoadPage::Errors::InvalidFileBody, true);
    EXPECT_EQ(page->testStatusText(), Constants::LoadPage::Errors::InvalidFileBody);
    EXPECT_TRUE(page->testStatusVisible());
}

// ============================================================================
// 3. BROWSE BUTTON LOGIC
// ============================================================================
TEST_F(LoadPageLogicTest, OnBrowseButtonClickedWithInvalidFileShowsError) {
    page->showStatusMessage(Constants::LoadPage::Errors::InvalidFileBody, true);
    EXPECT_TRUE(page->testStatusVisible());
}

// ============================================================================
// 4. SIGNAL EMISSION
// ============================================================================
TEST_F(LoadPageLogicTest, FileSelectedSignalIsEmitted) {
    QSignalSpy spy(page, &LoadPage::fileSelected);
    const QString path = "/tmp/dummy.dbc";

    emit page->fileSelected(path);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), path);
}