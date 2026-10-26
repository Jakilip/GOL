#include "LsGraphic.h"

#include <iostream>
#include <algorithm>

std::vector<LsLine> LsGraphic::getLines() const { return this->lines; }

unsigned LsGraphic::getColorChannel(unsigned lineIndex) const { return this->colorChannelMap[lineIndex]; }

unsigned LsGraphic::getColorCount() const { return this->colors.size(); }

LsColor LsGraphic::getColor(unsigned channelIndex) const {
    if(channelIndex >= this->colors.size())
        throw std::runtime_error("LsGraphic::getColor access out of bounds. Requested index: "+std::to_string(channelIndex)+" but channel count: "+std::to_string(colors.size()));
    return this->colors[channelIndex];
}

LsVec2F LsGraphic::getSize() const {
    LsVec2F size = {0.f, 0.f};

    if(lines.empty()) {
        return size;
    }

    constexpr float inf = std::numeric_limits<float>::infinity();

    LsVec2F lo = {0, 0}, hi = {0, 0};

    for(const auto& l : lines) {
        lo.x = std::min({lo.x, l.start.x, l.end.x});
        lo.y = std::min({lo.y, l.start.y, l.end.y});
        hi.x = std::max({hi.x, l.start.x, l.end.x});
        hi.y = std::max({hi.y, l.start.y, l.end.y});
    }

    size = {hi.x - lo.x, hi.y - lo.y};

    return size;
}

void LsGraphic::addLine(LsVec2F start, LsVec2F end, unsigned colorChannel) {
    this->lines.emplace_back(start, end);
    this->colorChannelMap.push_back(colorChannel);
    if(colorChannel >= this->colors.size()) {
        this->colors.resize(colorChannel+1);
    }
}

void LsGraphic::addLine(LsLine line, unsigned colorChannel) {
    this->lines.emplace_back(line);
    this->colorChannelMap.push_back(colorChannel);
    if(colorChannel >= this->colors.size()) {
        this->colors.resize(colorChannel+1);
    }
}

void LsGraphic::setColor(LsColor color, unsigned colorChannel) {
    if(colorChannel >= this->getColorCount()) {
        this->colors.resize(colorChannel+1);
    }
    this->colors[colorChannel] = color;
}

void LsGraphic::logContents() const {
    std::cout << "| LsGraphic contents |\n\n";
    std::cout << this->lines.size() << " lines: \n\n";
    for(int i = 0; i < this->lines.size(); ++i) {
        std::cout << "Line " << i << " | CC " << this->colorChannelMap[i] << " | " << lines[i].start << " -> " << lines[i].end << "\n";
    }
    std::cout << "\n" << this->colors.size() << " color channels: \n\n";
    for(int i = 0; i < this->colors.size(); ++i) {
        std::cout << "Color Channel " << i << " : " << this->colors[i] << "\n";
    }
}
