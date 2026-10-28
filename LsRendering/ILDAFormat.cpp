// ILDA (.ild) support for LsGraphic, implementing ILDA IDTF14 revision 011.
// Shit is made by claude. I am not doing this myself, there so are many different formats and all of them are poorly designed by the ILDA volk.
// The z axis of 3D formats is dropped on load and written as 0 on save.

#include "LsGraphic.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>

namespace {

    constexpr size_t HeaderSize = 32;
    constexpr size_t MaxRecords = 65535;
    constexpr size_t MaxPaletteSize = 256;

    constexpr uint8_t StatusLastPoint = 0x80;
    constexpr uint8_t StatusBlanking = 0x40;

    constexpr LsColor Black = {0, 0, 0, 255};

    struct IldaPoint {
        int16_t x = 0, y = 0;
        bool blanked = false;
        LsColor color = LsWhite;
        uint8_t colorIndex = 0; // only used when writing indexed formats
    };

    // Loading shifts the picture so its bottom left corner (smallest x and smallest y) sits at 0, 0 in graphic space.
    // Saving centers the picture on the middle of the ILDA coordinate space. ILDA coordinates are signed 16 bit,
    // so a picture larger than 65535 in either direction is clamped at the edges.
    // These two functions are the only place that needs to change if the coordinate mapping (or y direction) does.

    LsVec2F toGraphic(int16_t x, int16_t y, const LsVec2F& origin) {
        return {static_cast<float>(x) - origin.x, static_cast<float>(y) - origin.y};
    }

    int16_t toIlda(float v) {
        assert(std::isfinite(v));
        v = std::clamp(v, -32768.f, 32767.f);
        return static_cast<int16_t>(std::lround(v));
    }

    // ---- format properties ----

    bool isIndexedFormat(unsigned format) { return format == 0 || format == 1; }
    bool is3DFormat(unsigned format) { return format == 0 || format == 4; }

    size_t recordSize(unsigned format) {
        switch (format) {
            case 0: return 8;
            case 1: return 6;
            case 2: return 3;
            case 4: return 10;
            case 5: return 8;
            default: return 0; // format 3 was never approved, anything else is undefined
        }
    }

    // ---- reading helpers (big endian) ----

    uint16_t getU16(const uint8_t* p) {
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }

    int16_t getI16(const uint8_t* p) {
        return static_cast<int16_t>(getU16(p));
    }

    // ---- writing helpers (big endian) ----

    void putU16(std::vector<uint8_t>& out, uint16_t v) {
        out.push_back(static_cast<uint8_t>(v >> 8));
        out.push_back(static_cast<uint8_t>(v & 0xFF));
    }

    void putI16(std::vector<uint8_t>& out, int16_t v) {
        putU16(out, static_cast<uint16_t>(v));
    }

    /// Writes exactly 8 bytes, zero padded
    void putName(std::vector<uint8_t>& out, const char* name) {
        const size_t length = std::strlen(name);
        for (size_t i = 0; i < 8; ++i)
            out.push_back(i < length ? static_cast<uint8_t>(name[i]) : uint8_t{0});
    }

    void putHeader(std::vector<uint8_t>& out, unsigned format, const char* company, uint16_t recordCount, uint16_t totalFrames) {
        out.insert(out.end(), {'I', 'L', 'D', 'A'});
        out.insert(out.end(), {0, 0, 0}); // reserved
        out.push_back(static_cast<uint8_t>(format));
        putName(out, "");                 // frame / palette name
        putName(out, company);
        putU16(out, recordCount);
        putU16(out, 0);                   // frame / palette number
        putU16(out, totalFrames);
        out.push_back(0);                 // projector
        out.push_back(0);                 // reserved
    }

    // ---- point -> graphic conversion ----

    struct IldaSegment {
        int16_t x0, y0, x1, y1;
        LsColor color;
    };

    /// ILDA points are samples sent straight to the scanners, and the blanking bit decides whether the laser is on
    /// while the beam travels to that point. So a line is drawn from the previous point (lit or blanked) to every
    /// lit point, colored by the point it arrives at. Segments of zero length are skipped, and a lit point that
    /// leaves nothing else behind (for example a blanked point and a lit point at the same position) becomes a dot.
    /// If the frame has no color information (indexed without a palette) everything goes in channel 0.
    LsGraphic buildGraphic(const std::vector<IldaPoint>& points, bool colorsDefined) {
        std::vector<IldaSegment> segments;

        size_t i = 0;
        while (i < points.size()) {
            if (points[i].blanked) {
                ++i;
                continue;
            }

            // [i, end) is a run of lit points. The blanked point in front of it (if any) is where the beam starts from.
            size_t end = i;
            while (end < points.size() && !points[end].blanked) ++end;

            const size_t segmentsBefore = segments.size();
            for (size_t k = (i > 0 ? i : 1); k < end; ++k) {
                const IldaPoint& from = points[k - 1];
                const IldaPoint& to = points[k];
                if (from.x == to.x && from.y == to.y) continue;
                segments.push_back({from.x, from.y, to.x, to.y, to.color});
            }

            if (segments.size() == segmentsBefore) {
                const IldaPoint& dot = points[i];
                segments.push_back({dot.x, dot.y, dot.x, dot.y, dot.color});
            }

            i = end;
        }

        // The bottom left corner of the picture (smallest x and smallest y) becomes 0, 0
        LsVec2F origin = {0.f, 0.f};
        bool first = true;
        for (const IldaSegment& segment : segments) {
            for (const LsVec2F p : {LsVec2F{static_cast<float>(segment.x0), static_cast<float>(segment.y0)},
                                    LsVec2F{static_cast<float>(segment.x1), static_cast<float>(segment.y1)}}) {
                origin = first ? p : LsVec2F{std::min(origin.x, p.x), std::min(origin.y, p.y)};
                first = false;
            }
        }

        LsGraphic graphic;

        // One color channel per distinct RGB value, in order of first use
        std::unordered_map<uint32_t, unsigned> channelByColor;
        auto channelOf = [&](const LsColor& color) -> unsigned {
            if (!colorsDefined) return 0;

            const uint32_t key = (uint32_t{color.r} << 16) | (uint32_t{color.g} << 8) | uint32_t{color.b};
            const auto [it, inserted] = channelByColor.try_emplace(key, graphic.getColorCount());
            if (inserted) graphic.setColor(color, it->second);
            return it->second;
        };

        for (const IldaSegment& segment : segments)
            graphic.addLine(LsLine(toGraphic(segment.x0, segment.y0, origin), toGraphic(segment.x1, segment.y1, origin)), channelOf(segment.color));

        return graphic;
    }
}

bool LsGraphic::loadFromFile(const std::string& filePath, unsigned frameIndex) {
    auto fail = [&](const std::string& reason) {
        std::cerr << "LsGraphic: failed to load " << filePath << ": " << reason << "\n";
        return false;
    };

    std::ifstream in(filePath, std::ios::binary);
    if (!in) return fail("cannot open file");

    const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (in.bad()) return fail("error while reading");

    const uint8_t* data = bytes.data();
    const size_t size = bytes.size();

    // A palette applies to every following indexed frame of the same projector, until replaced
    std::array<std::vector<LsColor>, 256> palettes;
    std::array<bool, 256> hasPalette{};

    unsigned frameCounter = 0;
    size_t pos = 0;

    while (pos < size) {
        if (size - pos < HeaderSize) return fail("truncated header at byte " + std::to_string(pos));

        const uint8_t* header = data + pos;
        pos += HeaderSize;

        if (std::memcmp(header, "ILDA", 4) != 0)
            return fail("missing ILDA signature at byte " + std::to_string(pos - HeaderSize));

        const unsigned format = header[7];
        const size_t recSize = recordSize(format);
        if (recSize == 0) return fail("unsupported format code " + std::to_string(format));

        const uint16_t count = getU16(header + 24);
        const uint8_t projector = header[30];

        if (count == 0) {
            if (format == 2) continue; // empty palette, nothing to apply
            break;                     // end-of-file header
        }

        const size_t sectionBytes = static_cast<size_t>(count) * recSize;
        if (size - pos < sectionBytes) return fail("truncated data section at byte " + std::to_string(pos));

        const uint8_t* record = data + pos;
        pos += sectionBytes;

        if (format == 2) {
            std::vector<LsColor>& palette = palettes[projector];
            palette.resize(count);
            for (size_t i = 0; i < count; ++i, record += recSize)
                palette[i] = {record[0], record[1], record[2], 255}; // palette is R, G, B
            hasPalette[projector] = true;
            continue;
        }

        if (frameCounter++ != frameIndex) continue; // not the frame we want, palettes before it are still tracked

        const bool indexed = isIndexedFormat(format);
        const bool colorsDefined = !indexed || hasPalette[projector];
        const std::vector<LsColor>& palette = palettes[projector];

        std::vector<IldaPoint> points;
        points.reserve(count);

        for (size_t i = 0; i < count; ++i, record += recSize) {
            IldaPoint point;
            point.x = getI16(record);
            point.y = getI16(record + 2);

            // 3D formats carry a z value after y, it is skipped
            const size_t statusOffset = is3DFormat(format) ? 6 : 4;

            // The last point bit is ignored, the record count already tells where the frame ends
            point.blanked = (record[statusOffset] & StatusBlanking) != 0;

            const uint8_t* colorBytes = record + statusOffset + 1;
            if (point.blanked) {
                point.color = Black; // blanking takes precedence over any color information
            }
            else if (indexed) {
                const uint8_t index = colorBytes[0];
                if (!colorsDefined) point.color = LsWhite;
                else point.color = index < palette.size() ? palette[index] : Black;
            }
            else {
                // true color is stored as B, G, R
                point.color = {colorBytes[2], colorBytes[1], colorBytes[0], 255};
            }

            points.push_back(point);
        }

        *this = buildGraphic(points, colorsDefined);
        return true;
    }

    return fail("file contains " + std::to_string(frameCounter) + " frame(s), frame " + std::to_string(frameIndex) + " requested");
}

bool LsGraphic::saveToFile(const std::string& filePath, unsigned ildaFormat) const {
    assert(ildaFormat == 0 || ildaFormat == 1 || ildaFormat == 4 || ildaFormat == 5);

    auto fail = [&](const std::string& reason) {
        std::cerr << "LsGraphic: failed to save " << filePath << ": " << reason << "\n";
        return false;
    };

    const bool indexed = isIndexedFormat(ildaFormat);
    if (indexed && colors.size() > MaxPaletteSize)
        return fail("indexed ILDA formats support at most 256 colors, this graphic has " + std::to_string(colors.size()));

    // The picture is centered: the middle of the bounding box of all visible lines becomes ILDA 0, 0
    LsVec2F minCorner = {0.f, 0.f}, maxCorner = {0.f, 0.f};
    bool anyVisible = false;
    for (size_t i = 0; i < lines.size(); ++i) {
        assert(colorChannelMap[i] < colors.size());
        if (colors[colorChannelMap[i]].a == 0) continue;

        for (const LsVec2F& p : {lines[i].start, lines[i].end}) {
            assert(std::isfinite(p.x) && std::isfinite(p.y));
            minCorner = anyVisible ? LsVec2F{std::min(minCorner.x, p.x), std::min(minCorner.y, p.y)} : p;
            maxCorner = anyVisible ? LsVec2F{std::max(maxCorner.x, p.x), std::max(maxCorner.y, p.y)} : p;
            anyVisible = true;
        }
    }
    // Rounded to a whole unit so every point shifts by the same integer amount and the picture keeps its exact size
    const LsVec2F center = {std::round((minCorner.x + maxCorner.x) / 2.f), std::round((minCorner.y + maxCorner.y) / 2.f)};

    // Every stroke starts with a blanked point and a lit point at the same position, which is how
    // the reader tells a stroke start from a continuation. Lines that begin where the previous one ended
    // continue the stroke and only add their end point.
    std::vector<IldaPoint> points;
    bool strokeOpen = false;
    int16_t currentX = 0, currentY = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        const unsigned channel = colorChannelMap[i];
        const LsColor& color = colors[channel];
        if (color.a == 0) continue; // fully transparent (LsBlank) lines are invisible, nothing to store

        const int16_t startX = toIlda(lines[i].start.x - center.x), startY = toIlda(lines[i].start.y - center.y);
        const int16_t endX = toIlda(lines[i].end.x - center.x), endY = toIlda(lines[i].end.y - center.y);

        IldaPoint lit;
        lit.color = color;
        lit.colorIndex = static_cast<uint8_t>(channel); // only meaningful (and only used) when channel < 256

        const bool continuesStroke = strokeOpen && startX == currentX && startY == currentY;
        if (!continuesStroke) {
            IldaPoint blank;
            blank.x = startX;
            blank.y = startY;
            blank.blanked = true;
            points.push_back(blank);

            lit.x = startX;
            lit.y = startY;
            points.push_back(lit);
        }

        // A dot that starts a new stroke is already complete with the two points above
        const bool isLoneDot = !continuesStroke && endX == startX && endY == startY;
        if (!isLoneDot) {
            lit.x = endX;
            lit.y = endY;
            points.push_back(lit);
        }

        strokeOpen = true;
        currentX = endX;
        currentY = endY;
    }

    // A header with zero records marks the end of the file, so an empty frame cannot be stored
    if (points.empty()) return fail("graphic has no visible lines");
    if (points.size() > MaxRecords) return fail("graphic needs " + std::to_string(points.size()) + " points, the maximum is 65535");

    std::vector<uint8_t> out;

    // Indexed frames need a palette in front of them. The color channels map straight onto palette entries.
    if (indexed) {
        const size_t paletteSize = std::max<size_t>(colors.size(), 2); // palettes hold 2 to 256 colors
        putHeader(out, 2, "GOL", static_cast<uint16_t>(paletteSize), 0);
        for (size_t i = 0; i < paletteSize; ++i) {
            const LsColor color = i < colors.size() ? colors[i] : LsWhite;
            out.push_back(color.r);
            out.push_back(color.g);
            out.push_back(color.b);
        }
    }

    putHeader(out, ildaFormat, "GOL", static_cast<uint16_t>(points.size()), 1);

    for (size_t i = 0; i < points.size(); ++i) {
        const IldaPoint& point = points[i];

        putI16(out, point.x);
        putI16(out, point.y);
        if (is3DFormat(ildaFormat)) putI16(out, 0);

        uint8_t status = 0;
        if (point.blanked) status |= StatusBlanking;
        if (i + 1 == points.size()) status |= StatusLastPoint;
        out.push_back(status);

        if (indexed) {
            out.push_back(point.blanked ? uint8_t{0} : point.colorIndex);
        }
        else {
            const LsColor color = point.blanked ? Black : point.color;
            out.push_back(color.b);
            out.push_back(color.g);
            out.push_back(color.r);
        }
    }

    putHeader(out, ildaFormat, "", 0, 0); // end-of-file header

    std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
    if (!file) return fail("cannot open file for writing");

    file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
    file.flush();
    if (!file) return fail("error while writing");

    return true;
}

