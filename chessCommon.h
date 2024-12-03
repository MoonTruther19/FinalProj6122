/*

Objective:
Game of Chess common file
*/

#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
// Include GLM
#include <glm/glm.hpp>

// Mesh properties has table
typedef struct 
{
    bool hasBones;
    bool hasFaces;
    bool hasNormals;
    bool hasPositions;
    bool hasTangentsAndBitangents;
    bool hasTextureCoords;
    bool hasVertexColors;
    unsigned int numOfUVChannels;
} meshPropsT;

// Structure to hold target
// model matrix generation
typedef struct
{
    unsigned int rCnt;
    unsigned int rDis;
    float rAngle;
    glm::vec3 rAxis;
    glm::vec3 cScale;
    glm::vec3 tPos;
    bool alive = true;
    bool player;
} tPosition;

// Chess board scaling
const float CBSCALE = 0.6f;
// Chess board square box size (per side)
//const float CHESS_BOX_SIZE = 3.f;
const float CHESS_BOX_SIZE = (float)(CBSCALE * 5.4);
// Chess pieces scaling
const float CPSCALE = 0.015f;
// Platform height
const float PHEIGHT = -3.0f;
// Hash to hold the target Model matrix spec for each Chess component
typedef std::unordered_map <std::string, tPosition> tModelMap;

void setupChessBoard(tModelMap& cTModelMap);
bool movePiece(const std::string& sourceNotation, const std::string& targetNotation, tModelMap& cTModelMap);
bool commandChecker(const std::string& command, tModelMap& cTModelMap);
bool isThisACapture(const std::string& pieceName, const std::string& targetName, tModelMap& cTModelMap);
std::string getPieceAtPosition(const glm::vec3& position, tModelMap& cTModelMap);

#endif
