#include "Config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <random>
#include <string>
#include <string_view>

using namespace warden;
using ::testing::HasSubstr;

namespace {

/// Writes TOML to a temporary file, removed afterwards. A random name keeps test runs going on at
/// the same time apart.
class TempConfigFile {
public:
  explicit TempConfigFile(std::string_view content)
        : m_path{std::filesystem::temp_directory_path() /
                 std::format("warden-{}-{}.toml",
                             ::testing::UnitTest::GetInstance()->current_test_info()->name(),
                             std::random_device{}())} {
    std::ofstream{m_path} << content;
  }
  ~TempConfigFile() { std::filesystem::remove(m_path); }

  TempConfigFile(const TempConfigFile &)            = delete;
  TempConfigFile &operator=(const TempConfigFile &) = delete;
  TempConfigFile(TempConfigFile &&)                 = delete;
  TempConfigFile &operator=(TempConfigFile &&)      = delete;

  [[nodiscard]] std::string string() const { return m_path.string(); }

private:
  std::filesystem::path m_path;
};

} // namespace

TEST(LoadConfigTest, UnsetSettingsUseDefaults) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group-1\""}; // the one setting without a default
  const auto result = loadConfig(file.string());
  ASSERT_TRUE(result.has_value()) << result.error();

  const Config defaults{};
  EXPECT_EQ(result->readInterval, defaults.readInterval);
  EXPECT_EQ(result->averagingWindow, defaults.averagingWindow);
  EXPECT_EQ(result->faultAfterFailedReads, defaults.faultAfterFailedReads);
  EXPECT_FLOAT_EQ(result->thresholds.temperature, defaults.thresholds.temperature);
  EXPECT_FLOAT_EQ(result->thresholds.humidity, defaults.thresholds.humidity);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.min, defaults.plausibleValues.temperature.min);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.max, defaults.plausibleValues.temperature.max);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.min, defaults.plausibleValues.humidity.min);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.max, defaults.plausibleValues.humidity.max);
  EXPECT_EQ(result->mqtt.deviceId, ""); // main() then picks one
}

TEST(LoadConfigTest, ReadsAllValues) {
  const TempConfigFile file{R"(
read_interval_s          = 2
averaging_window         = 4
fault_after_failed_reads = 3

[thresholds]
temperature = 25.5
humidity    = 60.0

[plausible_values]
min_temperature = -10.0
max_temperature =  50.0
min_humidity    =   5.0
max_humidity    =  95.0

[mqtt]
group_id  = "group-7"
device_id = "group-7-lab"
)"};
  const auto result = loadConfig(file.string());
  ASSERT_TRUE(result.has_value()) << result.error();

  EXPECT_EQ(result->readInterval, std::chrono::seconds{2});
  EXPECT_EQ(result->averagingWindow, 4U);
  EXPECT_EQ(result->faultAfterFailedReads, 3U);
  EXPECT_FLOAT_EQ(result->thresholds.temperature, 25.5F);
  EXPECT_FLOAT_EQ(result->thresholds.humidity, 60.0F);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.min, -10.0F);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.max, 50.0F);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.min, 5.0F);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.max, 95.0F);
  EXPECT_EQ(result->mqtt.groupId, "group-7");
  EXPECT_EQ(result->mqtt.deviceId, "group-7-lab");
}

TEST(LoadConfigTest, MissingGroupIdIsAnError) {
  const TempConfigFile file{""};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("mqtt.group_id must be set"));
}

TEST(LoadConfigTest, GroupIdWithTopicSeparatorIsAnError) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group/1\""};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("mqtt.group_id must not contain /, + or #"));
}

TEST(LoadConfigTest, MissingFileIsAnError) {
  const auto result = loadConfig("/nonexistent/warden.toml");
  EXPECT_FALSE(result.has_value());
}

TEST(LoadConfigTest, MalformedTomlIsAnError) {
  const TempConfigFile file{"read_interval_s = = 5"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, ZeroReadIntervalIsRejected) {
  const TempConfigFile file{"read_interval_s = 0"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, NegativeReadIntervalIsRejected) {
  const TempConfigFile file{"read_interval_s = -5"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, ZeroAveragingWindowIsRejected) {
  const TempConfigFile file{"averaging_window = 0"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, TooShortReadIntervalIsRejected) {
  const TempConfigFile file{"read_interval_s = 1"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("read_interval_s"));
}

TEST(LoadConfigTest, TooLongReadIntervalIsRejected) {
  const TempConfigFile file{"read_interval_s = 3601"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("read_interval_s"));
}

TEST(LoadConfigTest, TooLargeAveragingWindowIsRejected) {
  const TempConfigFile file{"averaging_window = 1001"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("averaging_window"));
}

TEST(LoadConfigTest, ZeroFaultAfterFailedReadsIsRejected) {
  const TempConfigFile file{"fault_after_failed_reads = 0"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("fault_after_failed_reads"));
}

TEST(LoadConfigTest, TooLargeFaultAfterFailedReadsIsRejected) {
  const TempConfigFile file{"fault_after_failed_reads = 101"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("fault_after_failed_reads"));
}

TEST(LoadConfigTest, HumidityThresholdAbove100IsRejected) {
  const TempConfigFile file{"[thresholds]\nhumidity = 101"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("thresholds.humidity"));
}

TEST(LoadConfigTest, InvertedTemperatureBoundsAreRejected) {
  const TempConfigFile file{"[plausible_values]\nmin_temperature = 50.0\nmax_temperature = 10.0"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, InvertedHumidityBoundsAreRejected) {
  const TempConfigFile file{"[plausible_values]\nmin_humidity = 90.0\nmax_humidity = 10.0"};
  EXPECT_FALSE(loadConfig(file.string()).has_value());
}

TEST(LoadConfigTest, PlausibleHumidityAbove100IsRejected) {
  const TempConfigFile file{"[plausible_values]\nmax_humidity = 120.0"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("max_humidity"));
}

TEST(LoadConfigTest, ThresholdOutsidePlausibleValuesIsRejected) {
  const TempConfigFile file{"[plausible_values]\nmax_temperature = 25.0"}; // below the 28 C threshold
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("thresholds.temperature"));
}

TEST(LoadConfigTest, ShippedConfigFileShowsTheDefaults) {
  // The Pi has no source tree, so tools/pi/test.sh passes the file's path there.
  const char *copiedConfig = std::getenv("WARDEN_CONFIG_FILE");
  const auto result        = loadConfig(copiedConfig != nullptr ? copiedConfig : WARDEN_CONFIG_FILE);
  ASSERT_TRUE(result.has_value()) << result.error();

  const Config defaults;
  EXPECT_EQ(result->readInterval, defaults.readInterval);
  EXPECT_EQ(result->averagingWindow, defaults.averagingWindow);
  EXPECT_EQ(result->faultAfterFailedReads, defaults.faultAfterFailedReads);
  EXPECT_FLOAT_EQ(result->thresholds.temperature, defaults.thresholds.temperature);
  EXPECT_FLOAT_EQ(result->thresholds.humidity, defaults.thresholds.humidity);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.min, defaults.plausibleValues.temperature.min);
  EXPECT_FLOAT_EQ(result->plausibleValues.temperature.max, defaults.plausibleValues.temperature.max);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.min, defaults.plausibleValues.humidity.min);
  EXPECT_FLOAT_EQ(result->plausibleValues.humidity.max, defaults.plausibleValues.humidity.max);
  EXPECT_EQ(result->mqtt.host, defaults.mqtt.host);
  EXPECT_EQ(result->mqtt.port, defaults.mqtt.port);
}

TEST(LoadConfigTest, WholeNumberIsAcceptedForDecimalSetting) {
  const TempConfigFile file{"[thresholds]\ntemperature = 30\n[mqtt]\ngroup_id = \"group-1\""};
  const auto result = loadConfig(file.string());
  ASSERT_TRUE(result.has_value()) << result.error();
  EXPECT_FLOAT_EQ(result->thresholds.temperature, 30.0F);
}

TEST(LoadConfigTest, NanIsAnError) {
  const TempConfigFile file{"[thresholds]\ntemperature = nan"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("thresholds.temperature"));
}

TEST(LoadConfigTest, InfinityIsAnError) {
  const TempConfigFile file{"[thresholds]\nhumidity = inf"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("thresholds.humidity"));
}

TEST(LoadConfigTest, ReadsTheBroker) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group-1\"\nhost = \"192.168.1.20\"\nport = 1884"};
  const auto result = loadConfig(file.string());
  ASSERT_TRUE(result.has_value()) << result.error();
  EXPECT_EQ(result->mqtt.host, "192.168.1.20");
  EXPECT_EQ(result->mqtt.port, 1884);
}

TEST(LoadConfigTest, TheBrokerDefaultsToThisComputer) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group-1\"\n"};
  const auto result = loadConfig(file.string());
  ASSERT_TRUE(result.has_value()) << result.error();
  EXPECT_EQ(result->mqtt.host, "host.docker.internal");
  EXPECT_EQ(result->mqtt.port, 1883);
}

TEST(LoadConfigTest, EmptyHostIsAnError) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group-1\"\nhost = \"\""};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("mqtt.host must be set"));
}

TEST(LoadConfigTest, PortOutOfRangeIsAnError) {
  const TempConfigFile file{"[mqtt]\ngroup_id = \"group-1\"\nport = 70000"};
  const auto result = loadConfig(file.string());
  ASSERT_FALSE(result.has_value());
  EXPECT_THAT(result.error(), HasSubstr("mqtt.port must be between 1 and 65535"));
}
