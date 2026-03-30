#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

//====定义角度变量====
float yaw = -90.0f;  //水平旋转角度，初始值指向负Z轴
float pitch = 0.0f;   //垂直旋转角度
float lastX = 400, lastY = 300;  //鼠标初始位置
bool firstMouse = true;

//====摄像机参数====
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);  //摄像机位置
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); //摄像机朝向
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);  //摄像机上向量

//====Shader 源码====
const char* vertexShaderSource = R"(
#version 330 core 
layout(location = 0) in vec3 aPos;  //输入顶点位置

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;  //输出颜色

void main()
{
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);  //橙色
}
)";

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;  //反转Y轴

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;  //调整灵敏度
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	//更新角度
	yaw += xoffset;
	pitch += yoffset;

	//限制俯仰角度
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	//计算方向向量
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(front);  //更新摄像机朝向
}

int main()
{
	glfwInit();  //初始化GLFW

	//设置OpenGL版本和模式
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  //OpenGL 主版本
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	//次版本
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//创建窗口
	GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "创建窗口失败" << std::endl;
		glfwTerminate();
		return -1;
	}

	//注册鼠标回调
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  //隐藏鼠标并捕获

	glfwMakeContextCurrent(window);  // 设置当前上下文

	//初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "初始化 GLAD 失败" << std::endl;
		return -1;
	}

	//设置视口
	glViewport(0, 0, 800, 600);

	//开启深度测试
	glEnable(GL_DEPTH_TEST);

	//====Shader 编译====

	//创建顶点着色器对象
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//创建片元着色器对象
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//创建着色器程序
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//删除Shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//====顶点数据====
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,  //左下
		 0.5f, -0.5f, 0.0f,  //右下
		 0.0f,  0.5f, 0.0f   //顶部
	};

	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO); //创建VAO
	glGenBuffers(1, &VBO); //创建VBO

	//绑定VAO
	glBindVertexArray(VAO);

	//绑定VBO并传数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//告诉GPU如何解析数据
	glVertexAttribPointer(
		0,  //location = 0
		3,  //每个顶点3个float
		GL_FLOAT,  //类型
		GL_FALSE,  //不归一化
		3 * sizeof(float),  //步长
		(void*)0  //偏移
	);

	glEnableVertexAttribArray(0);  //启用属性

	//====渲染循环====
	while (!glfwWindowShouldClose(window))
	{
		//输入控制
		float cameraSpeed = 0.005f; //调整移动速度
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;  //前移
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;  //后移
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;  //左移
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;  //右移

		//输入(按ESC关闭)
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		//清屏（背景色）
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//使用Shader
		glUseProgram(shaderProgram);

		//====创建变换矩阵====

		//Model 旋转
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

		//View 观察
		glm::mat4 view = glm::lookAt(
			cameraPos,  //摄像机位置
			cameraPos + cameraFront,  //目标位置
			cameraUp  //上向量
		);

		//Projection 投影
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),  //视野角度(FOV)
			800.0f / 600.0f,  //宽高比
			0.1f,  //近裁剪面
			100.0f  //远裁剪面
		);

		//将矩阵传递给Shader
		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//绑定VAO
		glBindVertexArray(VAO);

		//画三角形
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//交换缓冲区
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//====释放资源====
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}