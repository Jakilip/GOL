#include "../LsUtilities/LsCore.h"
#include "../LsUtilities/LsMath.h"

#include "../LsRendering/LsProjector.h"

#include <iostream>
#include <cmath>

HeliosPoint LsProjector::makeHeliosPoint(LsVec2F pos, LsColor color) {
    HeliosPoint p;

    LsVec2F heliosCanvasSize = {4095.f, 4095.f};
    pos /= 16.f;

    p.x = static_cast<uint16_t>(std::clamp(std::round(pos.x), 0.f, heliosCanvasSize.x));
    p.y = static_cast<uint16_t>(std::clamp(std::round(pos.y), 0.f, heliosCanvasSize.y));

    p.r = color.r;
    p.g = color.g;
    p.b = color.b;
    p.i = color.a;

    return p;
}

std::vector<HeliosPoint> LsProjector::buildOptimalOutput() {
    std::vector<HeliosPoint> output;
    std::vector<LsLine> lines;
    std::vector<LsColor> colors;

    const LsVec2F canvasSizeF = {static_cast<float>(LsCanvasSize.x), static_cast<float>(LsCanvasSize.y)};

    // build a collection of lines, throwing away all lines invisible to the canvas, cropping the lines that are partially visible to the canvas
    for(auto& graphic : this->graphics) {
        auto glines = graphic.getLines();

        for(int i = 0; i < glines.size(); ++i) {
            auto& line = glines[i];

            // cropping lines. Unfortunately gotta dupe the same code 4 times for 2 axises and start + end

            if(line.start.x < 0 && line.end.x < 0)
                continue;
            if(line.start.x > canvasSizeF.x && line.end.x > canvasSizeF.x)
                continue;
            if(line.start.y < 0 && line.end.y < 0)
                continue;
            if(line.start.y > canvasSizeF.y && line.end.y > canvasSizeF.y)
                continue;

            if(line.start.x < 0 || line.start.x > canvasSizeF.x) {
                float xDiff;
                if(line.start.x < 0)
                    xDiff = line.start.x;
                else {
                    xDiff = line.start.x - canvasSizeF.x;
                }

                float vecMulti = xDiff / (line.end.x - line.start.x);
                LsVec2F lineVecToEdge = {vecMulti * (line.end.x - line.start.x), vecMulti * (line.end.y - line.start.y)};

                line.start -= lineVecToEdge;
            }
            if(line.end.x < 0 || line.end.x > canvasSizeF.x) {
                float xDiff;
                if(line.end.x < 0)
                    xDiff = line.end.x;
                else
                    xDiff = line.end.x - canvasSizeF.x;

                float vecMulti = xDiff / (line.start.x - line.end.x);
                LsVec2F lineVecToEdge = {vecMulti * (line.start.x - line.end.x), vecMulti * (line.start.y - line.end.y)};

                line.end -= lineVecToEdge;
            }

            if(line.start.y < 0 || line.start.y > canvasSizeF.y) {
                float yDiff;
                if(line.start.y < 0)
                    yDiff = line.start.y;
                else
                    yDiff = line.start.y - canvasSizeF.y;

                float vecMulti = yDiff / (line.end.y - line.start.y);
                LsVec2F lineVecToEdge = {vecMulti * (line.end.x - line.start.x), vecMulti * (line.end.y - line.start.y)};

                line.start -= lineVecToEdge;
            }
            if(line.end.y < 0 || line.end.y > canvasSizeF.y) {
                float yDiff;
                if(line.end.y < 0)
                    yDiff = line.end.y;
                else
                    yDiff = line.end.y - canvasSizeF.y;

                float vecMulti = yDiff / (line.start.y - line.end.y);
                LsVec2F lineVecToEdge = {vecMulti * (line.start.x - line.end.x), vecMulti * (line.start.y - line.end.y)};

                line.end -= lineVecToEdge;
            }

            // check for pesky corners
            if(line.start.x < -LsEpsilon || line.start.x > canvasSizeF.x + LsEpsilon || line.end.x < -LsEpsilon || line.end.x > canvasSizeF.x + LsEpsilon
                || line.start.y < -LsEpsilon || line.start.y > canvasSizeF.y + LsEpsilon || line.end.y < -LsEpsilon || line.end.y > canvasSizeF.y + LsEpsilon)
                continue;

            lines.emplace_back(glines[i]);
            colors.emplace_back(graphic.getColor(graphic.getColorChannel(i)));
        }
    }
    if(lines.empty())
        return output;

    //place blank line in between laser pos and first point
    lines.insert(lines.begin(), {laserPoint.currentPos, lines[0].start});
    colors.insert(colors.begin(), LsBlank);

    // generate blank lines in between line gaps
    for(int i = 0; i < lines.size()-1; ++i) {
        if(!pointsAreEqual(lines[i].end, lines[i+1].start)) {
            lines.insert(lines.begin()+i+1, {lines[i].end, lines[i+1].start});
            colors.insert(colors.begin()+i+1, LsBlank);
        }
    }

    //place blank point at the end
    lines.emplace_back(lines.back());
    colors.emplace_back(LsBlank);

    float totalDistance = 0.f;

    for(auto & line : lines) {
        totalDistance += distance(line.start, line.end);
    }
    float distancePerPoint = totalDistance / static_cast<float>(this->maxPPS);
    //distancePerPoint = totalDistance / lines.size();

    // generate output

    for(int i = 0; i < lines.size(); ++i) {
        auto& line = lines[i];

        LsVec2F lineVec = line.end - line.start;
        float lineDist = distance(line.end, line.start);

        float pointsInThisLine = std::max(2.f, std::floor(lineDist / distancePerPoint));
        //pointsInThisLine = 7.f;

        LsVec2F vecPerPoint = lineVec / (pointsInThisLine-1.f);

        for(int p = 0; p < static_cast<int>(pointsInThisLine); ++p) {
            output.emplace_back(makeHeliosPoint(line.start + (vecPerPoint * static_cast<float>(p)), colors.at(i)));
        }

    }

    laserPoint.currentPos = {static_cast<float>(output.back().x), static_cast<float>(output.back().y)};

    return output;
}

/*inline std::vector<std::vector<LsLine>> groupConnectedLines(std::vector<LsGraphic>& graphics) {
    std::vector<std::vector<LsLine>> groups;

    for(auto& graphic : graphics) {
        auto lines = graphic.getLines();

        while(!lines.empty()) {



        }
    }

    return groups;
}

inline std::vector<HeliosPoint> LsRenderer::buildOptimalOutput() {

    struct ConnectedComponent {
        std::vector<LsLine> lines;
        std::vector<LsColor> linesColors;
        std::vector<LsVec2F> edges;
    };

    std::vector<ConnectedComponent> connectedComponents;
    std::vector<ConnectedComponent> connectedGroups;
    std::vector<HeliosPoint> output;

    // Step 1: create Connected Components
    for(auto& graphic : this->graphics) {
        auto lines = graphic.getLines();

        while(!lines.empty()) {
            ConnectedComponent newCC;

            //start from line 0, travel until reaching one end of the connected component
            bool keepsGoing = true;

            while(keepsGoing) {
                keepsGoing = false;
                for(int i = 0; i < lines.size(); i++) {
                    if (pointsAreEqual(lines[0].end, lines[i].start) || pointsAreEqual(lines[0].start, lines[i].end)) {
                        keepsGoing = true;
                        break;
                    }
                }

                newCC.lines.emplace_back(lines[0]);
                newCC.linesColors.emplace_back();
                lines.erase(lines.begin());
            }

            connectedComponents.emplace_back(newCC);
        }

        for(auto& comp : connectedComponents) {
            std::cout << "\nEdges:\n";
            for(auto& edge : comp.edges) {
                std::cout << edge << ", \n";
            }
            std::cout << "\nLines:\n";
            for(auto& line : comp.lines) {
                std::cout << line.start << " - " << line.end << "\n";
            }
        }

    }

    return output;
}*/
