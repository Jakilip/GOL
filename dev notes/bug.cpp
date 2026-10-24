struct LsLine {
    LsVec2F start, end;
    [[nodiscard]] bool isPoint() const { return pointsAreEqual(start, end); }
    LsLine(const LsVec2F& start, const LsVec2F& end) : start(start), end(end) {};
    LsLine(const LsVec2F& point) : start(point), end(point) {};
    LsLine() = default;
};

std::vector<LsLine> lines;

if(lines.empty())
    return;

lines.insert(lines.begin(), {laserPoint.currentPos, lines[0].start});