#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
float fixedHeight = 0.2f;   // Altura base de la c�mara
float walkAmplitude = 0.2f; // Amplitud del movimiento de la c�mara
float walkSpeed = 10.0f;    // Velocidad del movimiento de la c�mara (afecta la frecuencia del movimiento)
float timeWalking = 0.0f;   // Tiempo acumulado de caminata
Camera camera(glm::vec3(-21.0f, fixedHeight, -15.0f));  // Inicia en la posici�n del c�rculo rojo en la imagen
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

bool checkRayCollision(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, const std::vector<glm::vec3>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 3) {
        float t;
        if (rayIntersectsTriangle(rayOrigin, rayDirection, vertices[i], vertices[i + 1], vertices[i + 2], t)) {
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lamp as Light Source", NULL, NULL);
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
    Shader modelShader("shaders/shader_exercise16_mloading.vs", "shaders/shader_exercise16_mloading.fs");
    Shader lampShader("shaders/lamp.vs", "shaders/lamp.fs");

    // load models
    Model casaModel("model/casa/casa.obj");
    Model lampModel("model/lamp/lamp.obj");
    Model ghostModel("model/ghost/ghost.obj");
    Model mueble("model/mueble/cuelgaRopa.obj");
    // Recoge todos los v�rtices del modelo para las pruebas de colisi�n
    std::vector<glm::vec3> modelVertices;
    for (auto& mesh : casaModel.meshes) {
        for (auto& vertex : mesh.vertices) {
            modelVertices.push_back(vertex.Position);
        }
    }
    glm::vec3 lastSafePosition = camera.Position;
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // camera settings
    camera.MovementSpeed = 1;

    // Posici�n de la luz (posici�n de la l�mpara)
    glm::vec3 lightPos1(7.52944, 1.12457, -60.3977);  // Posici�n de la primera l�mpara
    glm::vec3 lightPos2(2.46593, 1.7368, -64.9968);   // Posici�n de la segunda l�mpara
    glm::vec3 lightPos3(3.94016, 1.22564, -69.8362);
    glm::vec3 lightPos4(3.56388, 1.79457, -65.1359);
    glm::vec3 lightPos5(8.18261, 2.52822, -60.7235);
    glm::vec3 lightPos6(6.81825, 1.88915, -61.0953);
    glm::vec3 lightPos7(8.26567, 1.73277, -59.5191);
    glm::vec3 lightPos8(3.99078, 3.69825, -69.4986);
    glm::vec3 lightPos9(1.93347, 1.80134, -70.0527);
    glm::vec3 lightPos10(2.69675, 2.24108, -68.8472);

    // Reducir la atenuaci�n para hacer la luz m�s uniforme y de mayor alcance
    float constant = 1.0f;
    float linear = 0.1f;    // Disminuir para aumentar el alcance de la luz
    float quadratic = 0.012f; // Disminuir para que la luz caiga menos con la distancia
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);
        // Guarda la posici�n actual antes de procesar el input
        glm::vec3 initialPosition = camera.Position;

        // Print the camera position
        std::cout << "Camera Position - X: " << camera.Position.x << " Y: " << camera.Position.y << " Z: " << camera.Position.z << std::endl;

        // Determinar la direcci�n de los rayos para cada direcci�n de movimiento
        glm::vec3 forwardDirection = glm::normalize(camera.Front);
        glm::vec3 backwardDirection = -forwardDirection;
        glm::vec3 rightDirection = glm::normalize(camera.Right);
        glm::vec3 leftDirection = -rightDirection;

        float rayLength = 0.5f;  // Longitud del rayo para detectar colisiones cercanas

        bool canMoveForward = !checkRayCollision(camera.Position, forwardDirection, rayLength, modelVertices);
        bool canMoveBackward = !checkRayCollision(camera.Position, backwardDirection, rayLength, modelVertices);
        bool canMoveRight = !checkRayCollision(camera.Position, rightDirection, rayLength, modelVertices);
        bool canMoveLeft = !checkRayCollision(camera.Position, leftDirection, rayLength, modelVertices);

        // input
        bool isWalking = false; // Variable para detectar si la c�mara se est� moviendo

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && canMoveForward) {
            camera.ProcessKeyboard(FORWARD, deltaTime * 5.0f); // Aumenta la velocidad de la c�mara
            isWalking = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && canMoveBackward) {
            camera.ProcessKeyboard(BACKWARD, deltaTime * 5.0f); // Aumenta la velocidad de la c�mara
            isWalking = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && canMoveLeft) {
            camera.ProcessKeyboard(LEFT, deltaTime * 5.0f); // Aumenta la velocidad de la c�mara
            isWalking = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && canMoveRight) {
            camera.ProcessKeyboard(RIGHT, deltaTime * 5.0f); // Aumenta la velocidad de la c�mara
            isWalking = true;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Simula el movimiento de caminata aplicando un efecto senoidal a la altura de la c�mara
        if (isWalking) {
            timeWalking += deltaTime * walkSpeed;
            camera.Position.y = fixedHeight + walkAmplitude * std::sin(timeWalking);
        }
        else {
            // Si no est� caminando, regresa a la altura base
            camera.Position.y = fixedHeight;
        }




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
        modelShader.setVec3("lightPos1", lightPos1);
        modelShader.setVec3("lightPos2", lightPos2);
        modelShader.setVec3("lightPos3", lightPos3);
        modelShader.setVec3("lightPos4", lightPos4);

        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Luz c�lida

        // Configuraci�n de atenuaci�n de la luz puntual
        modelShader.setFloat("constant", constant);
        modelShader.setFloat("linear", linear);
        modelShader.setFloat("quadratic", quadratic);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 10.0));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        modelShader.setMat4("model", model);
        casaModel.Draw(modelShader);

        // Renderizar la primera l�mpara 1
        lampShader.use(); // Usar el shader espec�fico para la l�mpara

        lampShader.setVec3("lightPos", lightPos1);
        lampShader.setVec3("viewPos", camera.Position);
        lampShader.setVec3("lightColor", glm::vec3(1.0f, 0.8f, 0.6f)); // Color c�lido de la luz

        model = glm::mat4(1.0f);  // Reinicializar la matriz model para la l�mpara
        float time = glfwGetTime();
        float oscillation = sin(time) * 0.05f; // Oscilaci�n leve
        float angle = sin(time) * glm::radians(5.0f); // �ngulo peque�o para un balanceo suave

        // Cambia la posici�n base a una nueva ubicaci�n
        model = glm::translate(model, glm::vec3(7.52944f, 0.9f, -60.3977f)); // Nueva posici�n base
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Aplicar una rotaci�n 
        model = glm::translate(model, glm::vec3(oscillation, 0.0f, 0.0f)); // Oscilaci�n ligera

        // Escala de la l�mpara
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        // Configurar las matrices en el shader de la l�mpara
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        lampShader.setMat4("model", model);

        // Renderizar el modelo de la primera l�mpara
        lampModel.Draw(lampShader);

        // Renderizar la segunda l�mpara2
        lampShader.use(); // Usar el shader espec�fico para la l�mpara

        lampShader.setVec3("lightPos", lightPos2);
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

        lampShader.setVec3("lightPos", lightPos3);
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

        //----------------

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
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
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
