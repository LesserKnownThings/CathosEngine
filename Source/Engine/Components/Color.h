#pragma once

#include <glm/ext/vector_float4.hpp>

namespace Color
{
// === CORE & NEUTRALS ===
constexpr glm::vec4 White{ 1.0f, 1.0f, 1.0f, 1.0f };
constexpr glm::vec4 Black{ 0.0f, 0.0f, 0.0f, 1.0f };
constexpr glm::vec4 Clear{ 0.0f, 0.0f, 0.0f, 0.0f };
constexpr glm::vec4 Shadow{ 0.0f, 0.0f, 0.0f, 0.4f };

constexpr glm::vec4 LightGray{ 0.75f, 0.75f, 0.75f, 1.0f };
constexpr glm::vec4 Gray{ 0.5f, 0.5f, 0.5f, 1.0f };
constexpr glm::vec4 DarkGray{ 0.25f, 0.25f, 0.25f, 1.0f };
constexpr glm::vec4 Charcoal{ 0.12f, 0.12f, 0.12f, 1.0f };
constexpr glm::vec4 Ivory{ 1.0f, 1.0f, 0.94f, 1.0f };

// === PRIMARY & SECONDARY ===
constexpr glm::vec4 Red{ 1.0f, 0.0f, 0.0f, 1.0f };
constexpr glm::vec4 Green{ 0.0f, 1.0f, 0.0f, 1.0f };
constexpr glm::vec4 Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
constexpr glm::vec4 Yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
constexpr glm::vec4 Cyan{ 0.0f, 1.0f, 1.0f, 1.0f };
constexpr glm::vec4 Magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
constexpr glm::vec4 Orange{ 1.0f, 0.5f, 0.0f, 1.0f };
constexpr glm::vec4 Purple{ 0.5f, 0.0f, 0.5f, 1.0f };

// === ENGINE UI & EDITOR STATES ===
constexpr glm::vec4 UiBg{ 0.15f, 0.15f, 0.15f, 1.0f };        // Dark window background
constexpr glm::vec4 UiPanel{ 0.22f, 0.22f, 0.22f, 1.0f };     // Sub-panel or child layout box
constexpr glm::vec4 UiButton{ 0.28f, 0.28f, 0.28f, 1.0f };    // Default interactive element
constexpr glm::vec4 UiHover{ 0.35f, 0.42f, 0.53f, 1.0f };     // Highlighted element
constexpr glm::vec4 UiPressed{ 0.18f, 0.31f, 0.52f, 1.0f };   // Activated element
constexpr glm::vec4 UiDisabled{ 0.3f, 0.3f, 0.3f, 0.5f };     // Faded interactive block
constexpr glm::vec4 UiAccent{ 0.0f, 0.47f, 0.84f, 1.0f };     // Focus borders/Active tabs (Engine Blue)
constexpr glm::vec4 UiText{ 0.92f, 0.92f, 0.92f, 1.0f };      // Highly legible off-white text
constexpr glm::vec4 UiTextDark{ 0.1f, 0.1f, 0.1f, 1.0f };     // Text for light panels
constexpr glm::vec4 UiTextDisabled{ 0.5f, 0.5f, 0.5f, 1.0f }; // Grayed out labels
constexpr glm::vec4 Glass{ 1.0f, 1.0f, 1.0f, 0.15f };         // Semi-transparent overlay

// === EXTENDED RICH PALETTE ===
constexpr glm::vec4 Maroon{ 0.5f, 0.0f, 0.0f, 1.0f };
constexpr glm::vec4 Navy{ 0.0f, 0.0f, 0.5f, 1.0f };
constexpr glm::vec4 Teal{ 0.0f, 0.5f, 0.5f, 1.0f };
constexpr glm::vec4 Olive{ 0.5f, 0.5f, 0.0f, 1.0f };
constexpr glm::vec4 Lime{ 0.75f, 1.0f, 0.0f, 1.0f };
constexpr glm::vec4 ForestGreen{ 0.13f, 0.55f, 0.13f, 1.0f };
constexpr glm::vec4 SeaGreen{ 0.18f, 0.55f, 0.34f, 1.0f };
constexpr glm::vec4 Emerald{ 0.04f, 0.73f, 0.42f, 1.0f };
constexpr glm::vec4 Mint{ 0.6f, 1.0f, 0.6f, 1.0f };

constexpr glm::vec4 SkyBlue{ 0.53f, 0.81f, 0.92f, 1.0f };
constexpr glm::vec4 DeepSkyBlue{ 0.0f, 0.75f, 1.0f, 1.0f };
constexpr glm::vec4 Indigo{ 0.29f, 0.0f, 0.51f, 1.0f };
constexpr glm::vec4 Violet{ 0.93f, 0.51f, 0.93f, 1.0f };
constexpr glm::vec4 Plum{ 0.55f, 0.27f, 0.52f, 1.0f };
constexpr glm::vec4 Pink{ 1.0f, 0.75f, 0.8f, 1.0f };
constexpr glm::vec4 HotPink{ 1.0f, 0.41f, 0.71f, 1.0f };

constexpr glm::vec4 Gold{ 1.0f, 0.84f, 0.0f, 1.0f };
constexpr glm::vec4 Silver{ 0.75f, 0.75f, 0.75f, 1.0f };
constexpr glm::vec4 Bronze{ 0.8f, 0.5f, 0.2f, 1.0f };
constexpr glm::vec4 Copper{ 0.72f, 0.45f, 0.2f, 1.0f };
constexpr glm::vec4 Brown{ 0.6f, 0.4f, 0.2f, 1.0f };
constexpr glm::vec4 Beige{ 0.96f, 0.96f, 0.86f, 1.0f };
constexpr glm::vec4 Khaki{ 0.94f, 0.9, 0.55f, 1.0f };
constexpr glm::vec4 Coral{ 1.0f, 0.5f, 0.31f, 1.0f };
constexpr glm::vec4 Salmon{ 0.98f, 0.5f, 0.45f, 1.0f };
constexpr glm::vec4 Crimson{ 0.86f, 0.08f, 0.24f, 1.0f };

// === PASTELS (SOFT / FLAT UI) ===
constexpr glm::vec4 PastelRed{ 1.0f, 0.41f, 0.38f, 1.0f };
constexpr glm::vec4 PastelGreen{ 0.47f, 0.87f, 0.47f, 1.0f };
constexpr glm::vec4 PastelBlue{ 0.68f, 0.85f, 0.90f, 1.0f };
constexpr glm::vec4 PastelYellow{ 1.0f, 1.0f, 0.6f, 1.0f };
constexpr glm::vec4 PastelOrange{ 1.0f, 0.7f, 0.4f, 1.0f };
constexpr glm::vec4 PastelPurple{ 0.71f, 0.49f, 0.86f, 1.0f };
constexpr glm::vec4 Lavender{ 0.9f, 0.9f, 0.98f, 1.0f };
}; // namespace Color