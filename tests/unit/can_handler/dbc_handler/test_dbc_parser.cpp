/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iterator>

#include "can_handler/dbc_handler/dbc_parser.hpp"
#include "gtest/gtest.h"

using namespace CanHandler;

namespace {
const std::string kMinimalValidDbc =
    "VERSION \"1.0\" "
    "NS_ : "
    "BS_: "
    "BU_: ECU1 ECU2 "
    "BO_ 100 TestMessage: 8 ECU1 "
    "SG_ TestSignal : 0|8@1+ (1,0) [0|255] \"V\" ECU2 ";

const std::string kRangeRecalcDbc =
    "VERSION \"1.0\" "
    "NS_ : "
    "BS_: "
    "BU_: ECU1 ECU2 "
    "BO_ 200 RangeMessage: 8 ECU1 "
    "SG_ Balance : 0|8@1+ (2,1) [0|0] \"\" ECU2 ";

const std::string kInvalidByteOrderDbc =
    "VERSION \"1.0\" "
    "NS_ : "
    "BS_: "
    "BU_: ECU1 ECU2 "
    "BO_ 300 InvalidMessage: 8 ECU1 "
    "SG_ Broken : 0|8@2+ (1,0) [0|255] \"\" ECU2 ";
}  // namespace

TEST(DbcParserTest, ParseMinimalValidDbcSuccessfully)
{
    DbcParser parser;
    parser.provideNewFile(kMinimalValidDbc);

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->metaData.version, "1.0");
    EXPECT_EQ(config->nodeDefinitions.size(), 2);
    ASSERT_EQ(config->messageDefinitions.size(), 1);

    const auto& message = config->messageDefinitions.front();
    EXPECT_EQ(message.messageId, 100U);
    EXPECT_EQ(message.messageName, "TestMessage");
    EXPECT_EQ(message.transmitterName, "ECU1");
    ASSERT_EQ(message.signalDescriptions.size(), 1);

    const auto& signal = message.signalDescriptions.front();
    EXPECT_EQ(signal.signalName, "TestSignal");
    EXPECT_EQ(signal.startBit, 0U);
    EXPECT_EQ(signal.signalSize, 8U);
    EXPECT_FALSE(signal.byteOrder);
    EXPECT_FALSE(signal.valueType);
}

TEST(DbcParserTest, RecalculatesPhysicalRangeWhenMinAndMaxAreZero)
{
    DbcParser parser;
    parser.provideNewFile(kRangeRecalcDbc);

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    ASSERT_EQ(config->messageDefinitions.size(), 1);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();

    // unsigned 8-bit raw range [0..255], factor 2, offset 1
    EXPECT_DOUBLE_EQ(signal.minimum, 1.0);
    EXPECT_DOUBLE_EQ(signal.maximum, 511.0);
}

TEST(DbcParserTest, ReturnsNullptrForInvalidSignalDefinition)
{
    DbcParser parser;
    parser.provideNewFile(kInvalidByteOrderDbc);

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenRequiredNodesSectionIsMissing)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" NS_ : BS_: BO_ 10 Msg: 8 ECU "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, CanParseLargeProjectDbcFile)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"\" NS_ : NS_DESC_ CM_ BA_DEF_ BA_ VAL_ CAT_DEF_ CAT_ FILTER BA_DEF_DEF_ "
        "EV_DATA_ ENVVAR_DATA_ SGTYPE_ SGTYPE_VAL_ BA_DEF_SGTYPE_ BA_SGTYPE_ SIG_TYPE_REF_ "
        "VAL_TABLE_ SIG_GROUP_ SIG_VALTYPE_ SIGTYPE_VALTYPE_ BO_TX_BU_ BA_DEF_REL_ BA_REL_ "
        "BA_DEF_DEF_REL_ BU_SG_REL_ BU_EV_REL_ BU_BO_REL_ BS_: BU_: ECU "
        "BO_ 1 Msg: 8 ECU SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->messageDefinitions.size(), 1);
}

TEST(DbcParserTest, ParsesMultipleMessages)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg1: 8 ECU1 "
        "SG_ Sig1 : 0|8@1+ (1,0) [0|255] \"\" ECU2 "
        "BO_ 200 Msg2: 8 ECU1 "
        "SG_ Sig2 : 0|16@1+ (1,0) [0|65535] \"\" ECU2 "
        "BO_ 300 Msg3: 4 ECU2 "
        "SG_ Sig3 : 0|32@1+ (1,0) [0|4294967295] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->messageDefinitions.size(), 3);
}

TEST(DbcParserTest, ParsesMultipleSignalsInMessage)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 MultiSigMsg: 8 ECU1 "
        "SG_ Sig1 : 0|8@1+ (1,0) [0|255] \"V\" ECU2 "
        "SG_ Sig2 : 8|8@1+ (0.5,0) [0|127.5] \"A\" ECU2 "
        "SG_ Sig3 : 16|16@1- (1,-32768) [-32768|32767] \"rpm\" ECU2 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    ASSERT_EQ(config->messageDefinitions.size(), 1);
    EXPECT_EQ(config->messageDefinitions.front().signalDescriptions.size(), 3);
}

TEST(DbcParserTest, ParsesSignedSignalCorrectly)
{
    std::setlocale(LC_NUMERIC, "C");

    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ SignedSig : 0|16@1- (0.1,-100) [-100|6453.5] \"unit\" ECU2 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_TRUE(signal.valueType);  // signed
    EXPECT_DOUBLE_EQ(signal.factor, 0.1);
    EXPECT_DOUBLE_EQ(signal.offset, -100);
}

TEST(DbcParserTest, ParsesBigEndianSignal)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ BigEndianSig : 0|16@0+ (1,0) [0|65535] \"\" ECU2 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_TRUE(signal.byteOrder);  // DBC @0 = big endian → byteOrder = true
}

TEST(DbcParserTest, ParsesMultiplexorSignal)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ MuxSwitch M : 0|8@1+ (1,0) [0|255] \"\" ECU2 "
        "SG_ MuxSig1 m0 : 8|8@1+ (1,0) [0|255] \"\" ECU2 "
        "SG_ MuxSig2 m1 : 8|8@1+ (1,0) [0|255] \"\" ECU2 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signals = config->messageDefinitions.front().signalDescriptions;
    ASSERT_EQ(signals.size(), 3);

    auto it = signals.begin();
    EXPECT_TRUE(it->multiplexer);
    EXPECT_EQ(it->multiplexedBy, -1);

    ++it;
    EXPECT_FALSE(it->multiplexer);
    EXPECT_EQ(it->multiplexedBy, 0);

    ++it;
    EXPECT_FALSE(it->multiplexer);
    EXPECT_EQ(it->multiplexedBy, 1);
}

TEST(DbcParserTest, ParsesMultipleReceivers)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 ECU3 ECU4 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU2,ECU3,ECU4 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_EQ(signal.receivers.size(), 3);
}

TEST(DbcParserTest, ParsesValueDescriptions)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ State : 0|8@1+ (1,0) [0|3] \"\" ECU2 "
        "VAL_ 100 State 0 \"Off\" 1 \"On\" 2 \"Error\" 3 \"Unknown\" ; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    ASSERT_EQ(config->signalValueDescriptions.size(), 1);

    const auto& valDesc = config->signalValueDescriptions.front();
    EXPECT_EQ(valDesc.messageId, 100U);
    EXPECT_EQ(valDesc.signalName, "State");
    EXPECT_EQ(valDesc.signalDescriptions.size(), 4);
}

TEST(DbcParserTest, ParsesComments)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU2 "
        "CM_ \"General comment\"; "
        "CM_ BU_ ECU1 \"Node comment\"; "
        "CM_ BO_ 100 \"Message comment\"; "
        "CM_ SG_ 100 Sig \"Signal comment\"; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_GE(config->comments.size(), 4);
}

TEST(DbcParserTest, ParsesAttributeDefinitions)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BA_DEF_ \"GenMsgCycleTime\" INT 0 10000; "
        "BA_DEF_DEF_ \"GenMsgCycleTime\" 100; "
        "BA_ \"GenMsgCycleTime\" BO_ 100 50; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesValueTables)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "VAL_TABLE_ StateValues 0 \"Init\" 1 \"Running\" 2 \"Stopped\"; "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesEnvironmentVariables)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "EV_ EnvVar: 0 [0|100] \"\" 0 2 DUMMY_NODE_VECTOR0 Vector__XXX; "
        "ENVVAR_DATA_ EnvVar: 8; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesSignalTypes)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SGTYPE_ SigType1: 1@1+ (0.1,0) [0|25.5] \"\" 0 NoValDef; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMessageTransmitters)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU2 "
        "BO_TX_BU_ 100 : ECU1,ECU2; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesSignalGroups)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig1 : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SG_ Sig2 : 8|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SIG_GROUP_ 100 Group1 1 : Sig1 Sig2; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesSignalExtendedValueTypes)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|32@1+ (1,0) [0|4294967295] \"\" ECU1 "
        "SIG_VALTYPE_ 100 Sig : 1; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesBitTimingWithParameters)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: 500000:0,0 "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenMissingBitTiming)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenTrailingContentAfterParsing)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "UNEXPECTED_TRAILING_CONTENT");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingColon)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg 8 ECU1 "  // Missing colon before 8
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingPipe)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0 8@1+ (1,0) [0|255] \"\" ECU1 ");  // Missing | between startBit and size

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingAt)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8 1+ (1,0) [0|255] \"\" ECU1 ");  // Missing @ before byteOrder

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForInvalidValueType)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1* (1,0) [0|255] \"\" ECU1 ");  // Invalid * instead of + or -

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingParenthesis)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ 1,0) [0|255] \"\" ECU1 ");  // Missing opening (

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingBracket)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) 0|255] \"\" ECU1 ");  // Missing opening [

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrForMissingCommentSemicolon)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "CM_ \"Comment without semicolon\" ");  // Missing semicolon

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ParsesSignedRangeRecalculation)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ SignedSig : 0|8@1- (1,0) [0|0] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();

    // Signed 8-bit: -128 to 127
    EXPECT_DOUBLE_EQ(signal.minimum, -128.0);
    EXPECT_DOUBLE_EQ(signal.maximum, 127.0);
}

TEST(DbcParserTest, HandlesCIdentifierAtEndOfFile)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2");  // Missing space after ECU2

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, HandlesStringWithoutClosingQuote)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0 "  // Missing closing quote
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, HandlesInvalidDoubleFormat)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (abc,0) [0|255] \"\" ECU1 ");  // Invalid double

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, HandlesAttributeWithoutSemicolon)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BA_DEF_ \"Attr\" INT 0 100 ");  // Missing semicolon

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, EmptyNodeList)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: "  // Empty node list
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_TRUE(config->nodeDefinitions.empty());
}

TEST(DbcParserTest, MessageWithoutSignals)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 ");  // No signals

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    ASSERT_EQ(config->messageDefinitions.size(), 1);
    EXPECT_TRUE(config->messageDefinitions.front().signalDescriptions.empty());
}

// Additional tests for higher coverage

TEST(DbcParserTest, ParsesEnvironmentVariableComment)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "CM_ EV_ EnvVar \"Environment variable comment\"; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->comments.size(), 1);
    EXPECT_EQ(config->comments.front(),
              "Environment variable: EnvVar Environment variable comment");
}

TEST(DbcParserTest, ParsesMultipleValueTables)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "VAL_TABLE_ Table1 0 \"Val1\" 1 \"Val2\"; "
        "VAL_TABLE_ Table2 0 \"State1\" 1 \"State2\"; "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleEnvironmentVariables)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "EV_ EnvVar1: 0 [0|100] \"\" 0 2 DUMMY_NODE_VECTOR0 Vector__XXX; "
        "EV_ EnvVar2: 1 [0|200] \"\" 0 2 DUMMY_NODE_VECTOR0 Vector__XXX; "
        "ENVVAR_DATA_ EnvVar: 8; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleSignalTypes)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SGTYPE_ SigType1: 1@1+ (0.1,0) [0|25.5] \"\" 0 NoValDef; "
        "SGTYPE_ SigType2: 2@1+ (1,0) [0|100] \"\" 0 NoValDef; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleMessageTransmitters)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg1: 8 ECU1 "
        "SG_ Sig1 : 0|8@1+ (1,0) [0|255] \"\" ECU2 "
        "BO_ 200 Msg2: 8 ECU2 "
        "SG_ Sig2 : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BO_TX_BU_ 100 : ECU1,ECU2; "
        "BO_TX_BU_ 200 : ECU2; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleAttributeDefinitions)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BA_DEF_ \"Attr1\" INT 0 100; "
        "BA_DEF_ \"Attr2\" STRING; "
        "BA_DEF_ \"Attr3\" FLOAT 0.0 100.0; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleAttributeDefaults)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BA_DEF_ \"Attr1\" INT 0 100; "
        "BA_DEF_ \"Attr2\" INT 0 100; "
        "BA_DEF_DEF_ \"Attr1\" 50; "
        "BA_DEF_DEF_ \"Attr2\" 75; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleAttributeValues)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "BA_DEF_ \"Attr1\" INT 0 100; "
        "BA_DEF_DEF_ \"Attr1\" 50; "
        "BA_ \"Attr1\" BO_ 100 60; "
        "BA_ \"Attr1\" SG_ 100 Sig 70; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesMultipleSignalTypeReferences)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig1 : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SG_ Sig2 : 8|8@1+ (1,0) [0|255] \"\" ECU1 "
        "SGTYPE_ SigType1: 1@1+ (0.1,0) [0|25.5] \"\" 0 NoValDef; "
        "SGTYPE_ 100 Sig1 : SigType1; "
        "SGTYPE_ 100 Sig2 : SigType1; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesCommentsInBothParsingPhases)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 "
        "CM_ \"Early comment\"; "
        "BA_DEF_ \"Attr1\" INT 0 100; "
        "BA_DEF_DEF_ \"Attr1\" 50; "
        "BA_ \"Attr1\" BO_ 100 60; "
        "CM_ \"Late comment\"; ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->comments.size(), 2);
}

TEST(DbcParserTest, ParsesNegativeDoubleValues)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1.5,-50.25) [-50.25|205.75] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_DOUBLE_EQ(signal.factor, 1.5);
    EXPECT_DOUBLE_EQ(signal.offset, -50.25);
    EXPECT_DOUBLE_EQ(signal.minimum, -50.25);
    EXPECT_DOUBLE_EQ(signal.maximum, 205.75);
}

TEST(DbcParserTest, ParsesDecimalFactorAndOffset)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|16@1+ (0.01,0) [0|655.35] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_DOUBLE_EQ(signal.factor, 0.01);
    EXPECT_DOUBLE_EQ(signal.offset, 0);
}

TEST(DbcParserTest, ReturnsNullptrForInvalidCIdentifierFormat)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig-Invalid : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");  // Hyphen in identifier

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ParsesSignalWithSingleReceiver)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 ECU2 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU2 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_EQ(signal.receivers.size(), 1);
    EXPECT_EQ(signal.receivers.front(), "ECU2");
}

TEST(DbcParserTest, ParsesLarger16BitSignalRange)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|16@1- (1,0) [0|0] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();

    // Signed 16-bit: -32768 to 32767
    EXPECT_DOUBLE_EQ(signal.minimum, -32768.0);
    EXPECT_DOUBLE_EQ(signal.maximum, 32767.0);
}

TEST(DbcParserTest, ParsesLarger32BitUnsignedSignalRange)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|32@1+ (1,0) [0|0] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();

    // Unsigned 32-bit: 0 to 4294967295
    EXPECT_DOUBLE_EQ(signal.minimum, 0.0);
    EXPECT_DOUBLE_EQ(signal.maximum, 4294967295.0);
}

TEST(DbcParserTest, ParsesRangeRecalcWithFactorAndOffset)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (0.5,10) [0|0] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();

    // Unsigned 8-bit: 0 to 255, factor 0.5, offset 10
    EXPECT_DOUBLE_EQ(signal.minimum, 10.0);
    EXPECT_DOUBLE_EQ(signal.maximum, 137.5);
}

TEST(DbcParserTest, ReturnsNullptrForValueDescriptionWithoutSemicolon)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ State : 0|8@1+ (1,0) [0|3] \"\" ECU1 "
        "VAL_ 100 State 0 \"Off\" 1 \"On\" ");  // Missing semicolon

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenMissingClosingParenthesisInSignal)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0 [0|255] \"\" ECU1 ");  // Missing closing )

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenMissingClosingBracketInSignal)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255 \"\" ECU1 ");  // Missing closing ]

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenMissingCommaInFactorOffset)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1 0) [0|255] \"\" ECU1 ");  // Missing comma between factor and offset

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenMissingPipeInRange)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0 255] \"\" ECU1 ");  // Missing | in range

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenNewSymbolsHasMissingColon)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ "  // Missing colon
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenBitTimingHasInvalidFormat)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: 500000 0,0 "  // Missing colon after first number
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ReturnsNullptrWhenBitTimingHasMissingComma)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: 500000:0 0 "  // Missing comma
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_EQ(config, nullptr);
}

TEST(DbcParserTest, ParsesEmptyStringVersion)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    EXPECT_TRUE(config->metaData.version.empty());
}

TEST(DbcParserTest, ParsesVersionMissing)
{
    DbcParser parser;
    parser.provideNewFile(
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    EXPECT_NE(config, nullptr);
}

TEST(DbcParserTest, ParsesSignalWithEmptyUnit)
{
    DbcParser parser;
    parser.provideNewFile(
        "VERSION \"1.0\" "
        "NS_ : "
        "BS_: "
        "BU_: ECU1 "
        "BO_ 100 Msg: 8 ECU1 "
        "SG_ Sig : 0|8@1+ (1,0) [0|255] \"\" ECU1 ");

    const auto config = parser.parseDbc();

    ASSERT_NE(config, nullptr);
    const auto& signal = config->messageDefinitions.front().signalDescriptions.front();
    EXPECT_TRUE(signal.unit.empty());
}

TEST(DbcParserTest, HandlesConcurrentFileProvision)
{
    DbcParser parser;

    // First file
    parser.provideNewFile(kMinimalValidDbc);
    auto config1 = parser.parseDbc();

    // Second file - should override first
    parser.provideNewFile(kRangeRecalcDbc);
    auto config2 = parser.parseDbc();

    ASSERT_NE(config2, nullptr);
    EXPECT_EQ(config2->messageDefinitions.front().messageId, 200U);
}
