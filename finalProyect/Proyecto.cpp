#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <Windows.h>
#include <mmsystem.h>

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, bool canMoveForward, bool canMoveBackward, bool canMoveLeft, bool canMoveRight);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.85f, -65.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Funcion para calcular la intersecci�n de rayos con un tri�ngulo
bool rayIntersectsTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, float& t) {
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return false;    // El rayo es paralelo al tri�ngulo
    }
    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    t = f * glm::dot(edge2, q);
    if (t > EPSILON) {
        return true; // Hay una colisi�n con el tri�ngulo
    }
    else {
        return false; // No hay colisi�n
    }
}

bool checkRayCollision(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, const std::vector<glm::vec3>& vertices, const glm::mat4& modelMatrix) {
    for (size_t i = 0; i < vertices.size(); i += 3) {
        float t;
        // Transformar los v�rtices del modelo a las coordenadas del mundo
        glm::vec3 v0 = glm::vec3(modelMatrix * glm::vec4(vertices[i], 1.0f));
        glm::vec3 v1 = glm::vec3(modelMatrix * glm::vec4(vertices[i + 1], 1.0f));
        glm::vec3 v2 = glm::vec3(modelMatrix * glm::vec4(vertices[i + 2], 1.0f));

        if (rayIntersectsTriangle(rayOrigin, rayDirection, v0, v1, v2, t)) {
            if (t < rayLength) {
                return true;  // Colisi�n detectada dentro del rango
            }
        }
    }
    return false;
}

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Raycasting with Camera", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader modelShader("shaders/vs_Proyecto.vs", "shaders/fs_Proyecto.fs");
    Shader lampShader("shaders/lamp.vs", "shaders/lamp.fs");

    // load models
    Model casaModel("model/casa/casa.obj");
    Model lampModel("model/lamp/lamp.obj");
    Model ghostModel("model/ghost/ghost.obj");
    Model mueble("model/mueble/cuelgaRopa.obj");
    Model cerdo("model/cerdo/cerdo.obj");
    Model torso("model/torso/torso.obj");
    Model egg("model/egg/easter_egg.obj");
    Model ventana("model/ventana/Ventanas.obj");
    Model calaveras("model/calaveras/calaveras.obj");
    Model reloj("model/reloj/reloj.obj");



    // camera settings
    camera.MovementSpeed = 1;

    // Posici�n de las luces (posici�n de las l�mparas)
    glm::vec3 lightPositions[4] = {
        glm::vec3(7.52944, 1.12457, -60.3977),  // Posici�n de la primera l�mpara
        glm::vec3(2.46593, 1.7368, -64.9968),   // Posici�n de la segunda l�mpara
        glm::vec3(3.94016, 1.22564, -69.8362),  // Posici�n de la tercera l�mpara
        glm::vec3(3.56388, 1.79457, -65.1359)   // Posici�n de la cuarta l�mpara
    };

    // Reducir la atenuaci�n para hacer la luz m�s uniforme y de mayor alcance
    float constant = 1.0f;
    float linear = 0.1f;    // Disminuir para aumentar el alcance de la luz
    float quadratic = 0.012f; // Disminuir para que la luz caiga menos con la distancia

    // Recoge todos los v�rtices del modelo para las pruebas de colisi�n
    std::vector<glm::vec3> modelVertices;
    for (auto& mesh : casaModel.meshes) {
        for (auto& vertex : mesh.vertices) {
            modelVertices.push_back(vertex.Position);
        }
    }

    glm::vec3 lastSafePosition = camera.Position;

    // Declara la variable `model` una vez al inicio
    glm::mat4 model = glm::mat4(1.0f);

    // Reproducir m�sica de fondo (archivo WAV)
    PlaySound(TEXT("model/terror.wav"), NULL, SND_ASYNC | SND_LOOP);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Determinar la direcci�n de los rayos para cada direcci�n de movimiento
        glm::vec3 forwardDirection = glm::normalize(camera.Front);
        glm::vec3 backwardDirection = -forwardDirection;
        glm::vec3 rightDirection = glm::normalize(camera.Right);
        glm::vec3 leftDirection = -rightDirection;

        float rayLength = 0.5f;  // Longitud del rayo para detectar colisiones cercanas

        // Crear la matriz de transformaci�n para la casa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.402749f, -0.332003f, -49.6566f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        // Verificar colisiones usando la matriz de transformaci�n del modelo
        bool canMoveForward = !checkRayCollision(camera.Position, forwardDirection, rayLength, modelVertices, model);
        bool canMoveBackward = !checkRayCollision(camera.Position, backwardDirection, rayLength, modelVertices, model);
        bool canMoveRight = !checkRayCollision(camera.Position, rightDirection, rayLength, modelVertices, model);
        bool canMoveLeft = !checkRayCollision(camera.Position, leftDirection, rayLength, modelVertices, model);

        // Input processing with collision checks
        processInput(window, canMoveForward, canMoveBackward, canMoveLeft, canMoveRight);

        // Mantener la altura de la c�mara constante
        camera.Position.y = 0.85f;

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fondo completamente negro para asegurar oscuridad en la escena
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Configuraciones de la proyecci�n y vista
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Renderizar la casa
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setVec3Array("lightPositions", 4, lightPositions);  // Pasar el array de posiciones de luces
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Luz c�lida

        // Configuraci�n de atenuaci�n de la luz puntual
        modelShader.setFloat("constant", constant);
        modelShader.setFloat("linear", linear);
        modelShader.setFloat("quadratic", quadratic);

        // Renderizar la casa con la matriz de modelo actualizada
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.402749f, -0.332003f, -49.6566f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        modelShader.setMat4("model", model);
        casaModel.Draw(modelShader);

        // Renderizar las calaveras
        calaveras.Draw(modelShader);

        // Renderizar el reloj
        reloj.Draw(modelShader);

        // CERDO

        // Renderizar el cerdo
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para el cerdo
        model = glm::translate(model, glm::vec3(3.62067f, 0.12f, -70.2754f)); // Posici�n del cerdo

        // Rotar el cerdo para que mire hacia la c�mara
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotaci�n de 90 grados alrededor del eje Y para que mire hacia el frente

        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));  // Escalar el cerdo
        modelShader.setMat4("model", model);
        cerdo.Draw(modelShader);

        // Renderizar la primera l�mpara
        lampShader.use(); // Usar el shader espec�fico para la l�mpara

        lampShader.setVec3("lightPos", lightPositions[0]);
        lampShader.setVec3("viewPos", camera.Position);
        lampShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Color c�lido de la luz

        // Reasigna `model` para la l�mpara 1
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para la l�mpara
        float time = glfwGetTime();
        float oscillation = sin(time) * 0.05f; // Oscilaci�n leve
        float angle = sin(time) * glm::radians(5.0f); // �ngulo peque�o para un balanceo suave

        model = glm::translate(model, glm::vec3(7.52944f, 0.9f, -60.3977f)); // Nueva posici�n base
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Aplicar una rotaci�n 
        model = glm::translate(model, glm::vec3(oscillation, 0.0f, 0.0f)); // Oscilaci�n ligera

        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        // Configurar las matrices en el shader de la l�mpara
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        lampShader.setMat4("model", model);

        // Renderizar el modelo de la primera l�mpara
        lampModel.Draw(lampShader);

        // Renderizar la segunda l�mpara
        lampShader.use(); // Usar el shader espec�fico para la l�mpara

        lampShader.setVec3("lightPos", lightPositions[1]);
        lampShader.setVec3("viewPos", camera.Position);
        lampShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Color c�lido de la luz

        model = glm::mat4(1.0f);  // Reinicializar la matriz model para la segunda l�mpara
        oscillation = sin(time + 1.0f) * 0.05f; // Oscilaci�n leve diferente para la segunda l�mpara
        angle = sin(time + 1.0f) * glm::radians(5.0f); // �ngulo peque�o para un balanceo suave

        // Oscilaci�n y rotaci�n de la segunda l�mpara
        model = glm::translate(model, glm::vec3(2.7847, 0.85, -65.2366)); // Posici�n base
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Aplicar una rotaci�n 
        model = glm::translate(model, glm::vec3(oscillation, 0.0f, 0.0f)); // Oscilaci�n ligera

        // Escala de la l�mpara
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        // Configurar las matrices en el shader de la l�mpara
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        lampShader.setMat4("model", model);

        // Renderizar el modelo de la segunda l�mpara
        lampModel.Draw(lampShader);

        //-----------------
        // Renderizar la tercera l�mpara 3
        lampShader.use(); // Usar el shader espec�fico para la l�mpara

        lampShader.setVec3("lightPos", lightPositions[2]);
        lampShader.setVec3("viewPos", camera.Position);
        lampShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Color c�lido de la luz

        model = glm::mat4(1.0f);  // Reinicializar la matriz model para la tercera l�mpara
        oscillation = sin(time + 2.0f) * 0.05f; // Oscilaci�n leve diferente para la tercera l�mpara
        angle = sin(time + 2.0f) * glm::radians(5.0f); // �ngulo peque�o para un balanceo suave

        // Cambia la posici�n base a una nueva ubicaci�n
        model = glm::translate(model, glm::vec3(3.68961f, 0.88f, -69.9629f)); // Nueva posici�n base
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Aplicar una rotaci�n 
        model = glm::translate(model, glm::vec3(oscillation, 0.0f, 0.0f)); // Oscilaci�n ligera

        // Escala de la l�mpara
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        // Configurar las matrices en el shader de la l�mpara
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        lampShader.setMat4("model", model);

        // Renderizar el modelo de la tercera l�mpara
        lampModel.Draw(lampShader);

        //--------------------------------------------------
        

        //---TORSO
        modelShader.use();  // Aseg�rate de usar el shader adecuado para el modelo
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para el torso
        model = glm::translate(model, glm::vec3(12.5226f, 0.15f, -70.0827f)); // Posici�n del torso
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));  // Ajusta la escala seg�n sea necesario
        modelShader.setMat4("model", model);
        torso.Draw(modelShader);

        //---TORSO 2 
        modelShader.use();  // Aseg�rate de usar el shader adecuado para el modelo
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para el torso
        model = glm::translate(model, glm::vec3(11.5226f, 0.15f, -70.0827f)); // Posici�n del torso
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotar hacia la izquierda (90 grados negativos)
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));  // Ajusta la escala seg�n sea necesario
        modelShader.setMat4("model", model);
        torso.Draw(modelShader);

        //---HUEVO (EGG)
        modelShader.use();  // Aseg�rate de usar el shader adecuado para el modelo
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para el huevo
        model = glm::translate(model, glm::vec3(3.63193f, 0.15f, -69.0763f)); // Posici�n del huevo
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Girar 180 grados en el eje Y
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));  // Ajusta la escala seg�n sea necesario
        modelShader.setMat4("model", model);
        egg.Draw(modelShader);

        //----------------FANTASMA

        // Renderizar el fantasma
        modelShader.use();  // Volver a usar el shader del modelo para el fantasma

        // Definir la posici�n base del fantasma
        glm::vec3 ghostBasePosition = glm::vec3(7.45636f, 0.3, -58.3212f);

        // Calcular la posici�n oscilante del fantasma en el eje Z
        float ghostPositionZ = ghostBasePosition.z + sin(time) * 10.0f; // Oscilaci�n en el eje Z

        // Configurar la matriz de modelo para el fantasma
        model = glm::mat4(1.0f);  // Reinicializar la matriz model para el fantasma
        model = glm::translate(model, glm::vec3(ghostBasePosition.x, ghostBasePosition.y, ghostPositionZ));
        model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));
        modelShader.setMat4("model", model);

        // Renderizar el modelo del fantasma
        ghostModel.Draw(modelShader);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window, bool canMoveForward, bool canMoveBackward, bool canMoveLeft, bool canMoveRight)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && canMoveForward)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && canMoveBackward)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && canMoveLeft)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && canMoveRight)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}