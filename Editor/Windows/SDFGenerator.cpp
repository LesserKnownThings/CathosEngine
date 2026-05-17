#include "SDFGenerator.h"
#include "AtlasGenerator.h"
#include "BitmapAtlasStorage.h"
#include "Charset.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"
#include "ImmediateAtlasGenerator.h"
#include "Resources/AssetTraits.h"
#include "Resources/Font.h"
#include "TightAtlasPacker.h"
#include "WindowManager.h"
#include "glyph-generators.h"
#include "imgui.h"
#include "msdfgen/core/edge-coloring.h"
#include "msdfgen/ext/import-font.h"
#include "nlohmann/json.hpp"
#include "tinyfiledialogs.h"

#include "types.h"
#include <atomic>
#include <core/BitmapRef.hpp>
#include <ext/save-png.h>
#include <fstream>
#include <thread>
#include <vector>

REGISTER_WINDOW(SDFGenerator, "Windows/Tools/SDF Generator", true)

using json = nlohmann::json;
using namespace msdf_atlas;

constexpr int32_t MAX_STRING_BUFFER = 256;
constexpr std::array<const char*, 2> IMPORT_FONT_FILTER = { "*.ttf", "*.ttc" };
constexpr std::array<const char*, 1> EXPORT_FONT_FILTER = { "*.casset" };

struct Bounds
{
    float left = 0.0f;
    float bottom = 0.0f;
    float right = 0.0f;
    float top = 0.0f;
};

struct GeneratorData
{
    float maxCornerAngle = 3.0;
    float minScale = 24.0;

    float pixelRange = 4.0;
    float miterLimit = 1.0;

    int32_t threadCount = 4;
};

GeneratorData generatorData;
bool isGenerating = false;
std::atomic<bool> isGeneratorDone = false;
std::string fontPath;
msdfgen::BitmapConstSection<byte, 3> bitmap;
std::vector<GlyphGeometry> cachedGlyphs{};

inline void GenerateAtlas(const std::string& fontFile)
{
    bool success = false;

    std::thread generateGlyphs([&fontFile]()
                               {
        if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype())
        {
            if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFile.c_str()))
            {
                FontGeometry fontGeometry(&cachedGlyphs);
                fontGeometry.loadCharset(font, 1.0, Charset::ASCII);

                for (GlyphGeometry& glyph : cachedGlyphs)
                {
                    glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, generatorData.maxCornerAngle, 0);
                }
                TightAtlasPacker packer{};
                packer.setDimensionsConstraint(DimensionsConstraint::SQUARE);
                packer.setMinimumScale(generatorData.minScale);
                packer.setPixelRange(generatorData.pixelRange);
                packer.setMiterLimit(generatorData.miterLimit);
                packer.pack(cachedGlyphs.data(), cachedGlyphs.size());

                int32_t width = 0, height = 0;
                packer.getDimensions(width, height);

                ImmediateAtlasGenerator<float, 3, msdfGenerator, BitmapAtlasStorage<byte, 3>> generator(width, height);

                GeneratorAttributes attributes{};
                generator.setAttributes(attributes);
                generator.setThreadCount(generatorData.threadCount);

                generator.generate(cachedGlyphs.data(), cachedGlyphs.size());

                bitmap = generator.atlasStorage();

                isGeneratorDone = true;
                msdfgen::destroyFont(font);
            }
            msdfgen::deinitializeFreetype(ft);
    } });

    generateGlyphs.detach();
}

inline void ExportFontAsset()
{
    const char* path = tinyfd_saveFileDialog(
        "Export Font Asset",
        nullptr,
        EXPORT_FONT_FILTER.size(),
        EXPORT_FONT_FILTER.data(),
        "Export font asset");

    if (path)
    {
        std::vector<GlyphData> outGlyphs{};
        std::vector<KerningPair> kernings{};

        for (const GlyphGeometry& glyph : cachedGlyphs)
        {
            GlyphData data{};
            data.codePoint = glyph.getCodepoint();
            data.advance = glyph.getAdvance();
            glyph.getQuadPlaneBounds(data.planeLeft, data.planeBottom, data.planeRight, data.planeTop);
            glyph.getQuadAtlasBounds(data.uvLeft, data.uvBottom, data.uvRight, data.uvTop);
            data.uvLeft /= bitmap.width;
            data.uvBottom /= bitmap.height;
            data.uvRight /= bitmap.width;
            data.uvTop /= bitmap.height;
            outGlyphs.push_back(data);
        }

        FontGeometry fontGeometry(&cachedGlyphs);
        for (int32_t i = 0; i < cachedGlyphs.size(); ++i)
        {
            for (int32_t j = 0; j < cachedGlyphs.size(); ++j)
            {
                const GlyphGeometry& left = cachedGlyphs[i];
                const GlyphGeometry& right = cachedGlyphs[j];

                double advance;
                fontGeometry.getAdvance(advance, left.getCodepoint(), right.getCodepoint());

                float kerning = (float)(advance - left.getAdvance());

                if (kerning != 0.0f)
                {
                    kernings.push_back(KerningPair{ left.getCodepoint(), right.getCodepoint(), kerning });
                }
            }
        }

        std::ofstream stream(path, std::ios::binary);

        if (stream.is_open())
        {
            WriteAssetHeader(stream, FontAssetMetadata());
            stream.write(reinterpret_cast<const char*>(&bitmap.width), sizeof(int32_t));
            stream.write(reinterpret_cast<const char*>(&bitmap.height), sizeof(int32_t));
            stream.write(reinterpret_cast<const char*>(bitmap.pixels), bitmap.width * bitmap.height * 3);

            const int32_t glyphsSize = outGlyphs.size();
            stream.write(reinterpret_cast<const char*>(&glyphsSize), sizeof(int32_t));
            stream.write(reinterpret_cast<const char*>(outGlyphs.data()), sizeof(GlyphData) * glyphsSize);

            const int32_t kerningSize = kernings.size();
            stream.write(reinterpret_cast<const char*>(&kerningSize), sizeof(int32_t));
            stream.write(reinterpret_cast<const char*>(kernings.data()), sizeof(KerningPair) * kerningSize);
            stream.close();
        }
    }
}

void SDFGenerator::Draw()
{
    ImGui::Begin("MSDF Generator", &isShowing, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("MSDF Actions"))
        {
            if (ImGui::MenuItem("Import Font"))
            {
                const char* filePath = tinyfd_openFileDialog(
                    "Pick Font",
                    nullptr,
                    IMPORT_FONT_FILTER.size(),
                    IMPORT_FONT_FILTER.data(),
                    nullptr,
                    0);

                if (filePath)
                {
                    fontPath = filePath;
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (!fontPath.empty())
    {
        ImGui::Text("Font: %s", fontPath.c_str());

        ImGui::DragFloat("Max Corner Angle", &generatorData.maxCornerAngle);
        ImGui::DragFloat("Min Font Scale", &generatorData.minScale);
        ImGui::DragFloat("Pixel Range", &generatorData.pixelRange);
        ImGui::DragFloat("Miter Limit", &generatorData.miterLimit);
        ImGui::DragInt("Process Threads", &generatorData.threadCount, 1, std::thread::hardware_concurrency());

        if (ImGui::Button("Generate Atlas") && !isGeneratorDone)
        {
            isGenerating = true;
            isGeneratorDone = false;

            GenerateAtlas(fontPath);
        }

        if (!isGeneratorDone && isGenerating)
        {
            ImGui::Text("Generating ...");
        }
        else if (isGeneratorDone && isGenerating)
        {
            ImGui::Text("Done");

            if (ImGui::Button("Export Font Asset"))
            {
                ExportFontAsset();
            }
        }
    }

    ImGui::Separator();

    ImGui::End();
}