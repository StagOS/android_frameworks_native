/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UI_INPUTREADER_LIGHT_INPUT_MAPPER_H
#define _UI_INPUTREADER_LIGHT_INPUT_MAPPER_H

#include "InputMapper.h"

namespace android {

class LightInputMapper : public InputMapper {
    // Refer to https://developer.android.com/reference/kotlin/android/graphics/Color
    /* Number of colors : {red, green, blue} */
    static constexpr size_t COLOR_NUM = 3;
    static constexpr int32_t MAX_BRIGHTNESS = 0xff;

public:
    explicit LightInputMapper(InputDeviceContext& deviceContext);
    ~LightInputMapper() override;

    uint32_t getSources() override;
    void populateDeviceInfo(InputDeviceInfo* deviceInfo) override;
    void dump(std::string& dump) override;
    void configure(nsecs_t when, const InputReaderConfiguration* config, uint32_t changes) override;
    void reset(nsecs_t when) override;
    void process(const RawEvent* rawEvent) override;
    bool setLightColor(int32_t lightId, int32_t color) override;
    bool setLightPlayerId(int32_t lightId, int32_t playerId) override;
    std::optional<int32_t> getLightColor(int32_t lightId) override;
    std::optional<int32_t> getLightPlayerId(int32_t lightId) override;

private:
    struct Light {
        explicit Light(InputDeviceContext& context, const std::string& name, int32_t id,
                       InputDeviceLightType type)
              : context(context), name(name), id(id), type(type) {}
        virtual ~Light() {}
        InputDeviceContext& context;
        std::string name;
        int32_t id;
        InputDeviceLightType type;

        virtual bool setLightColor(int32_t color) { return false; }
        virtual std::optional<int32_t> getLightColor() { return std::nullopt; }
        virtual bool setLightPlayerId(int32_t playerId) { return false; }
        virtual std::optional<int32_t> getLightPlayerId() { return std::nullopt; }

        virtual void dump(std::string& dump) {}

        std::optional<std::int32_t> getRawLightBrightness(int32_t rawLightId);
        void setRawLightBrightness(int32_t rawLightId, int32_t brightness);
    };

    struct SingleLight : public Light {
        explicit SingleLight(InputDeviceContext& context, const std::string& name, int32_t id,
                             int32_t rawId)
              : Light(context, name, id, InputDeviceLightType::SINGLE), rawId(rawId) {}
        int32_t rawId;

        bool setLightColor(int32_t color) override;
        std::optional<int32_t> getLightColor() override;
        void dump(std::string& dump) override;
    };

    struct RgbLight : public Light {
        explicit RgbLight(InputDeviceContext& context, int32_t id,
                          const std::unordered_map<LightColor, int32_t>& rawRgbIds,
                          std::optional<int32_t> rawGlobalId)
              : Light(context, "RGB", id, InputDeviceLightType::RGB),
                rawRgbIds(rawRgbIds),
                rawGlobalId(rawGlobalId) {
            brightness = rawGlobalId.has_value()
                    ? getRawLightBrightness(rawGlobalId.value()).value_or(MAX_BRIGHTNESS)
                    : MAX_BRIGHTNESS;
        }
        // Map from color to raw light id.
        std::unordered_map<LightColor, int32_t /* rawLightId */> rawRgbIds;
        // Optional global control raw light id.
        std::optional<int32_t> rawGlobalId;
        int32_t brightness;

        bool setLightColor(int32_t color) override;
        std::optional<int32_t> getLightColor() override;
        void dump(std::string& dump) override;
    };

    struct MultiColorLight : public Light {
        explicit MultiColorLight(InputDeviceContext& context, const std::string& name, int32_t id,
                                 int32_t rawId)
              : Light(context, name, id, InputDeviceLightType::MULTI_COLOR), rawId(rawId) {}
        int32_t rawId;

        bool setLightColor(int32_t color) override;
        std::optional<int32_t> getLightColor() override;
        void dump(std::string& dump) override;
    };

    struct PlayerIdLight : public Light {
        explicit PlayerIdLight(InputDeviceContext& context, const std::string& name, int32_t id,
                               const std::unordered_map<int32_t, int32_t>& rawLightIds)
              : Light(context, name, id, InputDeviceLightType::PLAYER_ID),
                rawLightIds(rawLightIds) {}
        // Map from player Id to raw light Id
        std::unordered_map<int32_t, int32_t> rawLightIds;

        bool setLightPlayerId(int32_t palyerId) override;
        std::optional<int32_t> getLightPlayerId() override;
        void dump(std::string& dump) override;
    };

    int32_t mNextId = 0;

    // Light color map from light color to the color index.
    static const std::unordered_map<std::string, size_t> LIGHT_COLORS;

    // Light map from light ID to Light
    std::unordered_map<int32_t, std::unique_ptr<Light>> mLights;
};

} // namespace android

#endif // _UI_INPUTREADER_LIGHT_INPUT_MAPPER_H