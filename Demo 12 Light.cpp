#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//====定义角度变量====
float yaw = -90.0f;  //水平旋转角度，初始值指向负Z轴
float pitch = 0.0f;   //垂直旋转角度
float lastX = 400, lastY = 300;  //鼠标初始位置
bool firstMouse = true;

//====摄像机参数====
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);  //摄像机位置
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); //摄像机朝向
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);  //摄像机上向量

//====Shader 源码====
const char* vertexShaderSource = R"(
#version 330 core 
layout(location = 0) in vec3 aPos;  //输入顶点位置
layout(location = 1) in vec3 aNormal; //输入法线
layout(location = 2) in vec2 aTexCoord; //输入纹理坐标

out vec3 FragPos; //输出片元位置
out vec3 Normal;  //输出法线
out vec2 TexCoord; //输出纹理坐标

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0)); //计算片元位置
	Normal = mat3(transpose(inverse(model))) * aNormal; //计算变换后的法线
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord; //传递纹理坐标到片元着色器
}
)";

const char* fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;  //输出颜色

in vec3 Normal; //接收法线
in vec3 FragPos; //接收片元位置
in vec2 TexCoord; //接收纹理坐标

uniform vec3 lightPos; //光源位置
uniform vec3 objectColor; //物体颜色
uniform vec3 lightColor; //光源颜色

uniform sampler2D texture1; //纹理采样器
uniform sampler2D texture2; //第二个纹理采样器
uniform float mixValue; //混合值

void main()
{
	//环境光
	float ambientStrength = 0.2;
	vec3 ambient = ambientStrength * lightColor;

	//漫反射
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	//纹理颜色
	vec4 texColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue); //混合两个纹理颜色

	//最终颜色
	vec3 result = (ambient + diffuse) * texColor.rgb * objectColor; //将光照与纹理颜色结合

	FragColor = vec4(result, texColor.a); //输出最终颜色
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
	GLFWwindow* window = glfwCreateWindow(800, 600, "3D Cube", NULL, NULL);
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

	//面剔除
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);  //剔除背面
	glFrontFace(GL_CCW);  //逆时针为正面

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
		// positions          // normals           // texcoords

		// ===== 后面 (Z-) =====
		-0.5f,-0.5f,-0.5f,   0,0,-1,   0.0f,0.0f,
		 0.5f,-0.5f,-0.5f,   0,0,-1,   1.0f,0.0f,
		 0.5f, 0.5f,-0.5f,   0,0,-1,   1.0f,1.0f,
		 0.5f, 0.5f,-0.5f,   0,0,-1,   1.0f,1.0f,
		-0.5f, 0.5f,-0.5f,   0,0,-1,   0.0f,1.0f,
		-0.5f,-0.5f,-0.5f,   0,0,-1,   0.0f,0.0f,

		// ===== 前面 (Z+) =====
		-0.5f,-0.5f, 0.5f,   0,0,1,   0.0f,0.0f,
		 0.5f,-0.5f, 0.5f,   0,0,1,   1.0f,0.0f,
		 0.5f, 0.5f, 0.5f,   0,0,1,   1.0f,1.0f,
		 0.5f, 0.5f, 0.5f,   0,0,1,   1.0f,1.0f,
		-0.5f, 0.5f, 0.5f,   0,0,1,   0.0f,1.0f,
		-0.5f,-0.5f, 0.5f,   0,0,1,   0.0f,0.0f,

		// ===== 左面 (X-) =====
		-0.5f, 0.5f, 0.5f,  -1,0,0,   1.0f,0.0f,
		-0.5f, 0.5f,-0.5f,  -1,0,0,   1.0f,1.0f,
		-0.5f,-0.5f,-0.5f,  -1,0,0,   0.0f,1.0f,
		-0.5f,-0.5f,-0.5f,  -1,0,0,   0.0f,1.0f,
		-0.5f,-0.5f, 0.5f,  -1,0,0,   0.0f,0.0f,
		-0.5f, 0.5f, 0.5f,  -1,0,0,   1.0f,0.0f,

		// ===== 右面 (X+) =====
		 0.5f, 0.5f, 0.5f,   1,0,0,   1.0f,0.0f,
		 0.5f, 0.5f,-0.5f,   1,0,0,   1.0f,1.0f,
		 0.5f,-0.5f,-0.5f,   1,0,0,   0.0f,1.0f,
		 0.5f,-0.5f,-0.5f,   1,0,0,   0.0f,1.0f,
		 0.5f,-0.5f, 0.5f,   1,0,0,   0.0f,0.0f,
		 0.5f, 0.5f, 0.5f,   1,0,0,   1.0f,0.0f,

		 // ===== 底面 (Y-) =====
		 -0.5f,-0.5f,-0.5f,   0,-1,0,   0.0f,1.0f,
		  0.5f,-0.5f,-0.5f,   0,-1,0,   1.0f,1.0f,
		  0.5f,-0.5f, 0.5f,   0,-1,0,   1.0f,0.0f,
		  0.5f,-0.5f, 0.5f,   0,-1,0,   1.0f,0.0f,
		 -0.5f,-0.5f, 0.5f,   0,-1,0,   0.0f,0.0f,
		 -0.5f,-0.5f,-0.5f,   0,-1,0,   0.0f,1.0f,

		 // ===== 顶面 (Y+) =====
		 -0.5f, 0.5f,-0.5f,   0,1,0,   0.0f,1.0f,
		  0.5f, 0.5f,-0.5f,   0,1,0,   1.0f,1.0f,
		  0.5f, 0.5f, 0.5f,   0,1,0,   1.0f,0.0f,
		  0.5f, 0.5f, 0.5f,   0,1,0,   1.0f,0.0f,
		 -0.5f, 0.5f, 0.5f,   0,1,0,   0.0f,0.0f,
		 -0.5f, 0.5f,-0.5f,   0,1,0,   0.0f,1.0f
	};

	glm::vec3 cubePositions[] = {
		{ 0.0f,  0.0f,  0.0f},
		{2.0f,  5.0f, -15.0f},
		{-1.5f, -2.2f, -2.5f},
		{-3.8f, -2.0f, -12.3f},
		{2.4f, -0.4f, -3.5f},
	};

	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO); //创建VAO
	glGenBuffers(1, &VBO); //创建VBO

	//绑定VAO
	glBindVertexArray(VAO);

	//绑定VBO并传数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//====纹理加载====
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load("container.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		GLenum format;

		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "加载纹理失败: " << stbi_failure_reason() << std::endl;
	}

	stbi_image_free(data);

	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	int width2, height2, nrChannels2;
	unsigned char* data2 = stbi_load("container2.png", &width2, &height2, &nrChannels2, 0);

	if (data2)
	{
		GLenum format2;

		if (nrChannels2 == 1)
			format2 = GL_RED;
		else if (nrChannels2 == 3)
			format2 = GL_RGB;
		else if (nrChannels2 == 4)
			format2 = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format2, width2, height2, 0, format2, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "加载纹理失败: " << stbi_failure_reason() << std::endl;
	}
	stbi_image_free(data2);

	//告诉GPU如何解析数据
	// 位置
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 法线
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// UV
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//将矩阵传递给Shader
	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

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

		//绑定纹理
		glActiveTexture(GL_TEXTURE0);  //激活纹理单元0
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);  //激活纹理单元1
		glBindTexture(GL_TEXTURE_2D, texture2);

		//使用Shader
		glUseProgram(shaderProgram);

		glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
		glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

		static float mixValue = 0.5f; //混合值

		//调整混合值
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			mixValue += 0.001f;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			mixValue -= 0.001f;
		}

		if (mixValue > 1.0f)
			mixValue = 1.0f;
		if (mixValue < 0.0f)
			mixValue = 0.0f;

		//传入光源参数
		glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"),
			1.2f, 1.0f, 2.0f);

		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"),
			1.0f, 1.0f, 1.0f);

		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
			1.0f, 1.0f, 1.0f);

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

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//绑定VAO
		glBindVertexArray(VAO);

		//画立方体
		for (int i = 0; i < 5; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, (float)glfwGetTime() + glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

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