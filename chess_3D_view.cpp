// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
// User supporting files
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
// Lab3 specific chess class
#include "chessComponent.h"
#include "chessCommon.h"
#include "ECE_ChessEngine.hpp"
#include <fstream>

// Global light variable
glm::vec3 lightPos = glm::vec3(0, 0, 15);
glm::vec3 deathSpawn = glm::vec3( -5.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT);
GLfloat lightPower = 400.0;

// Global variables
std::vector<chessComponent> gchessComponents;
tModelMap cTModelMap;
GLuint MatrixID, ViewMatrixID, ModelMatrixID;
GLuint LightID, LightSwitchID, TextureID;
GLuint programID;


// Sets up the chess board
//void setupChessBoard(tModelMap& cTModelMap);
//bool movePiece(const std::string& sourceNotation, const std::string& targetNotation, tModelMap& cTModelMap);
//bool commandChecker(const std::string& command, tModelMap& cTModelMap);
//bool isThisACapture(const std::string& pieceName, const std::string& targetName, tModelMap& cTModelMap);
//std::string getPieceAtPosition(const glm::vec3& position, tModelMap& cTModelMap);


double targetFrameTime = 1.0 / 10.0; // 10 FPS
double lastFrameTime = glfwGetTime(); // Initialize with the current time

void waitForNextFrame() {
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - lastFrameTime;

    while (elapsedTime < targetFrameTime) {
        // Re-check elapsed time to stay in the loop
        currentTime = glfwGetTime();
        elapsedTime = currentTime - lastFrameTime;
    }

    // Update the last frame time for the next frame
    lastFrameTime = currentTime;
}

void renderScene() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pass light intensity to Fragment Shader
    glUniform1f(LightSwitchID, lightPower);

    // Compute projection and view matrices
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();

    // Render all chess game components
    for (auto component = gchessComponents.begin(); component != gchessComponents.end(); component++) {
        tPosition cTPosition = cTModelMap[component->getComponentID()];

        // Render multiple instances if required
        for (unsigned int pit = 0; pit < cTPosition.rCnt; pit++) {
            tPosition cTPositionMorph = cTPosition;
            if (pit != 0) {
                cTPositionMorph = cTModelMap[component->getComponentID() + std::to_string(pit)];
            }

            // Generate the Model matrix
            glm::mat4 ModelMatrix = component->genModelMatrix(cTPositionMorph);
            // Generate the MVP matrix
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            // Pass matrices to the shader
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            // Pass the light position
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

            // Bind and set up the texture
            component->setupTexture(TextureID);

            // Render the mesh
            component->renderMesh();
        }
    }

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();

}

int main(void) {
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Game Of Chess 3D", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
     programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
     MatrixID = glGetUniformLocation(programID, "MVP");
     ViewMatrixID = glGetUniformLocation(programID, "V");
     ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "myTextureSampler" uniform
     TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "lightToggleSwitch" uniform
     LightSwitchID = glGetUniformLocation(programID, "lightIntensity");


    // Create a vector of chess components class
    // Each component is fully self sufficient

    // Load the OBJ files
    bool cBoard = loadAssImpLab3("Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents);
    bool cComps = loadAssImpLab3("Lab3/Chess/chess-mod.obj", gchessComponents);

    // Proceed if OBJ loading is successful
    if (!cBoard || !cComps)
    {
        // Quit the program (Failed OBJ loading)
        std::cout << "Program failed due to OBJ loading failure, please CHECK!" << std::endl;
        return -1;
    }

    // Setup the Chess board locations
    setupChessBoard(cTModelMap);

    // Load it into a VBO (One time activity)
    // Run through all the components for rendering
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
    {
        // Setup VBO buffers
        cit->setupGLBuffers();
        // Setup Texture
        cit->setupTextureBuffers();
    }

    // Use our shader (Not changing the shader per chess component)
    glUseProgram(programID);

    // Get a handle for our "LightPosition" uniform
     LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Input for chess player
    std::string input;
    std::string botResponse;
    // Initialize camera angle
    computeMatricesFromInputFinal(45, 270, 45);
    bool readyForBot = false;

    // Setup the bot
    InitializeEngine();

    // Main rendering loop
    do {
        // Call the render helper function
        renderScene();

        // Input handling
        std::cout << "Please enter a command: ";
        std::getline(std::cin, input);
        readyForBot = commandChecker(input, cTModelMap);
        if (readyForBot)
        {
            sendMove("position startpos moves e2e4");
            sendMove("go depth 10");
            std::cout << getResponseMove(botResponse);
        }

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    // Cleanup code remains unchanged ...
    return 0;
}


bool commandChecker(const std::string& command, tModelMap& cTModelMap) 
{
    // Regular expressions for valid commands
    std::regex moveRegex("^move ([a-h][1-8])([a-h][1-8])$");
    std::regex cameraRegex("^camera (1[0-9]|[2-7][0-9]|80) (\\d{1,2}|[1-2]\\d{2}|3[0-5]\\d|360) (\\d+(\\.\\d+)?)$");
    std::regex lightPosRegex("^light (1[0-9]|[2-7][0-9]|80) (\\d{1,2}|[1-2]\\d{2}|3[0-5]\\d|360) (\\d+(\\.\\d+)?)$");
    std::regex lightPowerRegex("^power (\\d+(\\.\\d+)?)$");

    if (command == "quit") 
    {
        std::cout << "Thanks for playing!!" << std::endl;
        exit(0);
    }
    else if (std::regex_match(command, moveRegex)) 
    {
        // Extract source and target locations
        std::smatch match;
        if (std::regex_search(command, match, moveRegex)) 
        {
            std::string source = match[1].str();
            std::string target = match[2].str();
            return movePiece(source, target, cTModelMap);
        }
        return false;
    }
    else if (std::regex_match(command, cameraRegex)) 
    {
        std::smatch match;
        if (std::regex_search(command, match, cameraRegex))
        {
            float cTheta = std::stof(match[1].str());
            float cPhi = std::stof(match[2].str());
            float cRadius = std::stof(match[3].str());
            computeMatricesFromInputFinal(cTheta, cPhi, cRadius);
        }
        return false;
    }
    else if (std::regex_match(command, lightPosRegex))
    {
        std::smatch match;
        if (std::regex_search(command, match, lightPosRegex))
        {
            float cTheta = std::stof(match[1].str());
            float cPhi = std::stof(match[2].str());
            float cRadius = std::stof(match[3].str());
            lightPos = computeMatricesFromInputLightFinal(cTheta, cPhi, cRadius);
        }
        return false;
    }
    else if (std::regex_match(command, lightPowerRegex))
    {
        std::smatch match;
        if (std::regex_search(command, match, lightPowerRegex))
        {
            lightPower = std::stof(match[1].str());
        }
        return false;
    }
    else 
    {
        std::cout << "Invalid command or move!!" << std::endl;
        return false;
    }
}


bool isPathClear(const glm::vec3& current, const glm::vec3& target, const std::string& pieceName, tModelMap& cTModelMap) {
    glm::vec3 position;

    int xAxisStep = 0;
    int yAxisStep = 0;

    if ((target.x - current.x) > 1)
    {
        xAxisStep = 1;
    }
    else if ((target.x - current.x) < -1)
    {
        xAxisStep = -1;
    }

    if ((target.y - current.y) > 1)
    {
        yAxisStep = 1;
    }
    else if ((target.y - current.y) < -1)
    {
        yAxisStep = -1;
    }

    if (!xAxisStep && !yAxisStep)
    {
        return true;
    }

    position.x = current.x + CHESS_BOX_SIZE * xAxisStep * 0.2;
    position.y = current.y + CHESS_BOX_SIZE * yAxisStep * 0.2;
    position.z = -3;

    //cTModelMap[pieceName].tPos = position;
    waitForNextFrame();
    renderScene();

    if (getPieceAtPosition(position, cTModelMap) == "")
    {
        cTModelMap[pieceName].tPos = position;
        return isPathClear(position, target, pieceName, cTModelMap);
    }
    else
    {
        return false;
    }
}

// Checks if a move is valid for the piece
bool isValidMove(const std::string& pieceName, const std::string& targetName, const glm::vec3& current, const glm::vec3& target, tModelMap& cTModelMap) {
    // Calculate differences in x and y positions
    if (std::abs(target.x) > 11.4 || std::abs(target.y) > 11.4)
    {
        return false;
    }

    float dx = target.x - current.x;
    float dy = target.y - current.y;
    std::cout << "dx value: " << dx << std::endl;
    std::cout << "dy value: " << dy << std::endl;
    glm::vec3 savedPos;
    savedPos.x = current.x;
    savedPos.y = current.y;
    savedPos.z = current.z;
    constexpr float EPSILON = 1.0; // Tolerance for floating-point comparison

    if (pieceName.find("TORRE") != std::string::npos) 
    {
        // Rook moves in straight lines along x or y
        if (!isPathClear(current, target, pieceName, cTModelMap))
        {
            cTModelMap[pieceName].tPos = savedPos;
            return false;
        }
        else if ((std::abs(dx) < 1e-6 || std::abs(dy) < 1e-6) && isThisACapture(pieceName, targetName, cTModelMap)) 
        {
            return true;
        }
    }
    else if (pieceName.find("ALFIERE") != std::string::npos) 
    {
        // Bishop moves diagonally (absolute change in x == absolute change in y)
        if (std::abs(std::abs(dx) - std::abs(dy)) < EPSILON) {
            if (!isPathClear(current, target, pieceName, cTModelMap)) {
                cTModelMap[pieceName].tPos = savedPos;
                return false;
            }

            // Check if this move is a capture
            if (isThisACapture(pieceName, targetName, cTModelMap)) {
                return true; // Valid diagonal capture
            }
        }
    }
    else if (pieceName.find("REGINA") != std::string::npos) 
    {
        // Queen moves like both rook and bishop
        if (!isPathClear(current, target, pieceName, cTModelMap))
        {
            cTModelMap[pieceName].tPos = savedPos;
            return false;
        }
        if ((std::abs(dx) < 1e-6 || std::abs(dy) < 1e-6 || std::abs(std::abs(dx) - std::abs(dy)) < EPSILON) &&
            isThisACapture(pieceName, targetName, cTModelMap))
        {
            return true;
        }
    }
    else if (pieceName.find("RE") != std::string::npos) 
    {
        // King moves one square in any direction
        return std::abs(dx) <= CHESS_BOX_SIZE && std::abs(dy) <= CHESS_BOX_SIZE;
    }
    else if (pieceName.find("PEDONE") != std::string::npos) 
    {
        // Pawn moves one square forward (dy > 0 for one player, < 0 for the other)
        if (dy < 0 && cTModelMap[pieceName].player)
        {
            return false;
        }
        if (std::abs(dy) > 3.25 && std::abs(std::abs(current.y) - 2.5 * CHESS_BOX_SIZE) > EPSILON) 
        {
            return false;
        }
        if (!isPathClear(current, target, pieceName, cTModelMap)) {
            cTModelMap[pieceName].tPos = savedPos;
            return false;
        }
        if (std::abs(dx) > 1e-6 && isThisACapture(pieceName, targetName, cTModelMap))
        {
            return true;
        }
        return (std::abs(dx) < 1e-6 && dy);
    }
    else if (pieceName.find("Object") != std::string::npos) {
        // Knight moves in an L-shape (2 in one direction and 1 in the other)
        /*if (!isPathClear(current, target, pieceName, cTModelMap)) {
            cTModelMap[pieceName].tPos = savedPos;
            return false;
        }*/
        
        return (std::abs(std::abs(dx) - 2 * CHESS_BOX_SIZE) < EPSILON && std::abs(std::abs(dy) - CHESS_BOX_SIZE) < EPSILON) ||
            (std::abs(std::abs(dx) - CHESS_BOX_SIZE) < EPSILON && std::abs(std::abs(dy) - 2 * CHESS_BOX_SIZE) < EPSILON);
    }

    // Invalid piece type or unknown name
    return false;
}


// Checks if a target position is occupied and returns the piece name if occupied
std::string getPieceAtPosition(const glm::vec3& position, tModelMap& cTModelMap) {
    constexpr float EPSILON = 1e-6; // Tolerance for floating-point comparison

    for (const auto& pair : cTModelMap) {
        const std::string& name = pair.first;
        const tPosition& data = pair.second;

        if (std::abs(data.tPos.x - position.x) < EPSILON &&
            std::abs(data.tPos.y - position.y) < EPSILON) {
            return name;
        }
    }
    return "";
}

// Determines if a move is a capture must be used last in any if statements otherwise things will break
bool isThisACapture(const std::string& pieceName, const std::string& targetName, tModelMap& cTModelMap) 
{
    //std::cout << targetName << std::endl;
    if (targetName.empty() && pieceName.find("PEDONE") == std::string::npos)
    {
        return true;
    }
    else if (cTModelMap[targetName].player != cTModelMap[pieceName].player)
    {
        cTModelMap[targetName].alive = false;
        cTModelMap[targetName].tPos = deathSpawn;
        std::cout << "after death should happen" << std::endl;
        deathSpawn.y =+ CHESS_BOX_SIZE;
        if (deathSpawn.y > 11.4)
        {
            deathSpawn.y = -5.5 * CHESS_BOX_SIZE;
            deathSpawn.x = -CHESS_BOX_SIZE;
        }
        return true;
    }
    return false;
}

// Move piece if valid
bool movePiece(const std::string& sourceNotation, const std::string& targetNotation, tModelMap& cTModelMap) {
    // Get the source and target positions
    glm::vec3 sourcePosition = chessNotationToPosition(sourceNotation);
    glm::vec3 targetPosition = chessNotationToPosition(targetNotation);
    std::cout << sourcePosition.x << ", " << sourcePosition.y << std::endl;

    // Get the piece at the source position
    std::string pieceName = getPieceAtPosition(sourcePosition, cTModelMap);
    std::string targetName = getPieceAtPosition(targetPosition, cTModelMap);
    std::cout << pieceName << std::endl;

    if (pieceName.empty()) {
        std::cerr << "Error: No piece at " << sourceNotation << std::endl;
        return false;
    }

    // Validate the move
    if (isValidMove(pieceName, targetName, sourcePosition, targetPosition, cTModelMap)) 
    {
        // Update so it doesnt loop again at somepoint
        targetName = getPieceAtPosition(targetPosition, cTModelMap);
        std::cout << "Target player bool: " << cTModelMap[targetName].player << std::endl;
        std::cout << "Moving player bool: " << cTModelMap[pieceName].player << std::endl;
        if (targetName.empty()) 
        {
            // Move the piece
            cTModelMap[pieceName].tPos = targetPosition;
            std::cout << pieceName << " moved from " << sourceNotation << " to " << targetNotation << std::endl;
            return true;
        }
        else 
        {
            std::cerr << "Target position " << targetNotation << " is occupied!" << std::endl;
        }
    }
    else {
        std::cerr << "Invalid move for " << pieceName << std::endl;
    }
    return false;

}


void setupChessBoard(tModelMap& cTModelMap)
{
    // Target spec Hash
    cTModelMap = {
        // Chess board
        {"12951_Stone_Chess_Board", {1, 0, 0.f, {1, 0, 0}, glm::vec3(CBSCALE), {0.f, 0.f, PHEIGHT}}},

        // First player pieces
        {"TORRE3",      {2, 7, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"TORRE31",     {1, 7, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 3.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"Object3",     {2, 5, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-2.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"Object31",    {1, 5, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 2.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"ALFIERE3",    {2, 3, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-1.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"ALFIERE31",   {2, 3, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 1.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"REGINA2",     {1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 0.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"RE2",         {1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-0.5 * CHESS_BOX_SIZE, -3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE13",    {8, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE131",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-2.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE132",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-1.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE133",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-0.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE134",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 0.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE135",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 1.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE136",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 2.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},
        {"PEDONE137",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 3.5 * CHESS_BOX_SIZE, -2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, true}},

        // Bot pieces
        {"TORRE02",     {2, 7, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"TORRE021",    {1, 7, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 3.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"Object02",    {2, 5, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-2.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"Object021",   {1, 5, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 2.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"ALFIERE02",   {2, 3, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-1.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"ALFIERE021",  {2, 3, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 1.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"REGINA01",    {1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-0.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"RE01",        {1, 0, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 0.5 * CHESS_BOX_SIZE, 3.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE12",    {8, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-3.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE121",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-2.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE122",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-1.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE123",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), {-0.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE124",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 0.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE125",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 1.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE126",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 2.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}},
        {"PEDONE127",   {1, 1, 90.f, {1, 0, 0}, glm::vec3(CPSCALE), { 3.5 * CHESS_BOX_SIZE, 2.5 * CHESS_BOX_SIZE, PHEIGHT}, true, false}}
    };

}
