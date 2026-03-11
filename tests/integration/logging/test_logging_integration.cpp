#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include "core/event/can_driver_event.hpp"
#include "logging/logging_component.hpp"
#include "logging/view/logging_view.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace Logging;
using namespace testing;

class LoggingIntegrationTest : public ::testing::Test
{
   protected:
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<LoggingComponent> component;
    std::unique_ptr<Logging::LoggingModel> model;
    LoggingView* view = nullptr;

    void SetUp() override
    {
        ON_CALL(mockBroker, _subscribeEvent(_)).WillByDefault(Return());

        component = std::make_unique<LoggingComponent>(mockBroker);
        model = std::make_unique<Logging::LoggingModel>();
        component->onStart();

        view = qobject_cast<LoggingView*>(component->getView());
        ASSERT_NE(view, nullptr);

        view->show();
        QCoreApplication::processEvents();
    }

    void TearDown() override
    {
        if (view) view->hide();

        component.reset();
        view = nullptr;

        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }
};

TEST_F(LoggingIntegrationTest, ShouldHideOverlayWhenDeviceIsReady)
{
    EXPECT_CALL(mockBroker,
                _publishEvent(Eq(std::type_index(typeid(Core::CheckCanDeviceReadyEvent))), _))
        .WillOnce([](std::type_index, const void* data) {
            auto* event = static_cast<Core::CheckCanDeviceReadyEvent*>(const_cast<void*>(data));
            event->isReady = true;
        });

    component->onStart();

    auto* overlay = view->findChild<QWidget*>("deviceNotConfiguredOverlay");
    ASSERT_NE(overlay, nullptr) << "Overlay widget not found.";
    EXPECT_FALSE(overlay->isVisible());
}

TEST_F(LoggingIntegrationTest, ShouldShowOverlayWhenDeviceIsNotReady)
{
    EXPECT_CALL(mockBroker,
                _publishEvent(Eq(std::type_index(typeid(Core::CheckCanDeviceReadyEvent))), _))
        .WillOnce([](std::type_index, const void* data) {
            auto* event = static_cast<Core::CheckCanDeviceReadyEvent*>(const_cast<void*>(data));
            event->isReady = false;
        });

    component->onStart();

    auto* overlay = view->findChild<QWidget*>("deviceNotConfiguredOverlay");
    ASSERT_NE(overlay, nullptr) << "Overlay widget not found.";
    EXPECT_TRUE(overlay->isVisible());
}

TEST_F(LoggingIntegrationTest, ShouldPropagateDbcConfigurationToModel)
{
    ASSERT_NE(component, nullptr);

    QSignalSpy spy(component.get(), &LoggingComponent::dbcConfigurationChanged);

    Core::DbcSignalDescription sig;
    sig.signalName = "speed";
    sig.multiplexer = false;
    sig.multiplexedBy = -1;
    sig.startBit = 0;
    sig.signalSize = 16;
    sig.byteOrder = false;
    sig.valueType = false;
    sig.factor = 1.0;
    sig.offset = 0.0;
    sig.minimum = 0.0;
    sig.maximum = 250.0;
    sig.unit = "km/h";

    Core::DbcMessageDescription msg;
    msg.messageId = 0x123;
    msg.messageName = "VehicleSpeed";
    msg.messageSize = 8;
    msg.transmitterName = "ECU";
    msg.signalDescriptions.push_back(sig);

    Core::DbcConfig config;
    config.messageDefinitions.push_back(msg);

    emit component->dbcConfigurationChanged(config);

    QCoreApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}

//Der Test covert irgendwie nix 
TEST_F(LoggingIntegrationTest, ShouldShowAndHideDetailView)
{
    ASSERT_NE(view, nullptr);

    model->startNewDbcLogSession();
    model->stopActiveSession();
    view->setModel(model.get());

    auto* table = view->getHistoryTable();
    ASSERT_NE(table, nullptr);

    view->show();
    QCoreApplication::processEvents();

    auto* table_model = table->model();
    ASSERT_NE(table_model, nullptr);

    ASSERT_GT(table_model->rowCount(), 0);

    QModelIndex index = table_model->index(0, 0);
    ASSERT_TRUE(index.isValid());

    table->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    QCoreApplication::processEvents();

    EXPECT_TRUE(table->selectionModel()->hasSelection());

    table->selectionModel()->clearSelection();

    QCoreApplication::processEvents();

    EXPECT_FALSE(table->selectionModel()->hasSelection());
}


TEST_F(LoggingIntegrationTest, ShouldCreateDetailViewForSession)
{
    ASSERT_NE(component, nullptr);

    model->startNewRawLogsSession();
    model->stopActiveSession();

    QModelIndex index = model->index(0, 0);
    ASSERT_TRUE(index.isValid());

    emit view->detailRequested(index);
    QCoreApplication::processEvents();

    SUCCEED();
}

// TEST_F(LoggingIntegrationTest, ShouldAttemptExportLogSession)
// {
//     ASSERT_NE(component, nullptr);

//     model->startNewRawLogsSession();
//     model->stopActiveSession();

//     QString sessionId = model->getCurrentSessionId();

//     component->exportLogSession(sessionId, "/tmp/test_export.csv");
//     SUCCEED();
// }

TEST_F(LoggingIntegrationTest, ShouldCleanupResourcesOnStop)
{
    ASSERT_NE(component, nullptr);

    component->onStop();

    SUCCEED();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}